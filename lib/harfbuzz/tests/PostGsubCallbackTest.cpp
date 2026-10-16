#include <hb.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

struct State {
  int calls = 0;
  int destroyed = 0;
  int messages = 0;
  bool badInput = false;
  bool skipGsub = false;
};

static void check(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

static void release(void* data) { ++static_cast<State*>(data)->destroyed; }

static void postGsub(hb_buffer_t* buffer, hb_font_t*, void* data) {
  auto& state = *static_cast<State*>(data);
  ++state.calls;
  unsigned count = 0;
  auto* glyphs = hb_buffer_get_glyph_infos(buffer, &count);
  if (count != 1 || glyphs[0].codepoint != 42) { state.badInput = true; return; }
  glyphs[0].codepoint = 7;
}

static hb_bool_t message(hb_buffer_t*, hb_font_t*, const char* text, void* data) {
  auto& state = *static_cast<State*>(data);
  ++state.messages;
  return !(state.skipGsub && std::strcmp(text, "start table GSUB") == 0);
}

static void shape(hb_font_t* font, hb_buffer_t* buffer, bool changed = true) {
  hb_buffer_clear_contents(buffer);
  hb_buffer_add_utf8(buffer, "a", 1, 0, 1);
  hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
  hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
  hb_buffer_set_language(buffer, hb_language_from_string("en", -1));
  const char* shapers[] = {"ot", nullptr};
  check(hb_shape_full(font, buffer, nullptr, 0, shapers), "OT shaping failed");
  unsigned count = 0;
  const auto* glyphs = hb_buffer_get_glyph_infos(buffer, &count);
  const auto* positions = hb_buffer_get_glyph_positions(buffer, nullptr);
  check(count == 1 && glyphs[0].codepoint == (changed ? 7u : 42u), "Post-GSUB glyph replacement was not retained");
  check(positions[0].x_advance == (changed ? 700 : 420), "Callback did not run before advance initialization");
}

int main() {
  try {
    auto* face = hb_face_builder_create();
    hb_face_set_upem(face, 1000);
    hb_face_set_glyph_count(face, 64);
    auto* font = hb_font_create(face);
    hb_face_destroy(face);
    auto* funcs = hb_font_funcs_create();
    hb_font_funcs_set_nominal_glyph_func(funcs, [](hb_font_t*, void*, hb_codepoint_t, hb_codepoint_t* glyph, void*) -> hb_bool_t { *glyph = 42; return true; }, nullptr, nullptr);
    hb_font_funcs_set_glyph_h_advance_func(funcs, [](hb_font_t*, void*, hb_codepoint_t glyph, void*) -> hb_position_t { return glyph == 7 ? 700 : 420; }, nullptr, nullptr);
    hb_font_set_funcs(font, funcs, nullptr, nullptr);
    hb_font_funcs_destroy(funcs);
    auto* buffer = hb_buffer_create();
    State first, second, rejected;
    hb_buffer_set_post_gsub_func(buffer, postGsub, &first, release);
    shape(font, buffer);
    check(first.calls == 1 && !first.badInput && first.messages == 0, "Direct callback requires tracing or ran at the wrong stage");
    hb_buffer_reset(buffer);
    shape(font, buffer);
    check(first.calls == 2 && first.destroyed == 0, "Reset discarded callback ownership");
    auto* similar = hb_buffer_create_similar(buffer);
    shape(font, similar, false);
    hb_buffer_destroy(similar);
    check(first.calls == 2 && first.destroyed == 0, "Similar buffer copied callback ownership");
    hb_buffer_set_post_gsub_func(buffer, postGsub, &second, release);
    check(first.destroyed == 1, "Replacing callback did not release previous data");
    hb_buffer_set_message_func(buffer, message, &second, nullptr);
    shape(font, buffer);
    check(second.calls == 1 && second.messages > 0 && !second.badInput, "Tracing interfered with direct callback");
    second.skipGsub = true;
    shape(font, buffer, false);
    check(second.calls == 1, "Skipped GSUB invoked the callback");
    hb_buffer_set_message_func(buffer, nullptr, nullptr, nullptr);
    hb_buffer_set_post_gsub_func(buffer, nullptr, nullptr, nullptr);
    check(second.destroyed == 1, "Disabling callback did not release data");
    shape(font, buffer, false);
    hb_buffer_set_post_gsub_func(buffer, nullptr, &rejected, release);
    hb_buffer_set_post_gsub_func(hb_buffer_get_empty(), postGsub, &rejected, release);
    check(rejected.destroyed == 2, "Rejected registrations leaked data");
    hb_buffer_set_post_gsub_func(buffer, postGsub, &rejected, release);
    hb_buffer_destroy(buffer);
    check(first.destroyed == 1 && second.destroyed == 1 && rejected.destroyed == 3, "Destruction leaked or double-freed callback data");
    hb_font_destroy(font);
    std::cout << "PASS post-GSUB order, tracing independence, reuse and callback ownership\n";
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
