#include <digitalkhatt/engine.h>

#include <hb.h>
#include <hb-ot.h>

#include <algorithm>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

#define CHECK(condition, message)                                             \
  do {                                                                        \
    if (!(condition)) {                                                       \
      std::cerr << "conformance failure: " << message << '\n';                \
      return 1;                                                               \
    }                                                                         \
  } while (false)

template <typename T, void (*Destroy)(T*)>
struct HbDeleter {
  void operator()(T* value) const {
    if (value) Destroy(value);
  }
};

using Blob = std::unique_ptr<hb_blob_t, HbDeleter<hb_blob_t, hb_blob_destroy>>;
using Face = std::unique_ptr<hb_face_t, HbDeleter<hb_face_t, hb_face_destroy>>;
using Font = std::unique_ptr<hb_font_t, HbDeleter<hb_font_t, hb_font_destroy>>;
using Buffer =
    std::unique_ptr<hb_buffer_t, HbDeleter<hb_buffer_t, hb_buffer_destroy>>;
using Line = std::unique_ptr<dk_line_t, decltype(&dk_line_destroy)>;
using Engine = std::unique_ptr<dk_engine_t, decltype(&dk_engine_destroy)>;

struct GlyphData {
  uint32_t glyph_id = 0;
  uint32_t cluster = 0;
  int32_t x_advance = 0;
  int32_t y_advance = 0;
  int32_t x_offset = 0;
  int32_t y_offset = 0;
  double left_tatweel = 0;
  double right_tatweel = 0;
};

struct LineData {
  int64_t width = 0;
  std::vector<GlyphData> glyphs;
};

struct PathDigest {
  uint64_t value = UINT64_C(1469598103934665603);
  uint32_t commands = 0;

  friend bool operator==(const PathDigest& left, const PathDigest& right) {
    return left.value == right.value && left.commands == right.commands;
  }
};

void hash(PathDigest& state, const void* bytes, size_t size) {
  const auto* data = static_cast<const unsigned char*>(bytes);
  for (size_t index = 0; index < size; ++index) {
    state.value ^= data[index];
    state.value *= UINT64_C(1099511628211);
  }
}

void command(PathDigest& state, char type,
             std::initializer_list<float> values) {
  hash(state, &type, sizeof(type));
  for (float value : values) {
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    hash(state, &bits, sizeof(bits));
  }
  ++state.commands;
}

dk_status_t api_move_to(void* data, float x, float y) {
  command(*static_cast<PathDigest*>(data), 'M', {x, y});
  return DK_STATUS_OK;
}
dk_status_t api_line_to(void* data, float x, float y) {
  command(*static_cast<PathDigest*>(data), 'L', {x, y});
  return DK_STATUS_OK;
}
dk_status_t api_quadratic_to(void* data, float cx, float cy, float x,
                             float y) {
  command(*static_cast<PathDigest*>(data), 'Q', {cx, cy, x, y});
  return DK_STATUS_OK;
}
dk_status_t api_cubic_to(void* data, float c1x, float c1y, float c2x,
                         float c2y, float x, float y) {
  command(*static_cast<PathDigest*>(data), 'C', {c1x, c1y, c2x, c2y, x, y});
  return DK_STATUS_OK;
}
dk_status_t api_close_path(void* data) {
  command(*static_cast<PathDigest*>(data), 'Z', {});
  return DK_STATUS_OK;
}

dk_path_sink_v1_t path_sink(PathDigest* digest, uint32_t size) {
  return {size,          digest,       api_move_to, api_line_to,
          api_quadratic_to, api_cubic_to, api_close_path};
}

struct CancelState {
  size_t calls = 0;
};
dk_status_t cancel(CancelState& state) {
  ++state.calls;
  return DK_STATUS_CANCELLED;
}
dk_status_t cancel_move_to(void* data, float, float) {
  return cancel(*static_cast<CancelState*>(data));
}
dk_status_t cancel_line_to(void* data, float, float) {
  return cancel(*static_cast<CancelState*>(data));
}
dk_status_t cancel_quadratic_to(void* data, float, float, float, float) {
  return cancel(*static_cast<CancelState*>(data));
}
dk_status_t cancel_cubic_to(void* data, float, float, float, float, float,
                            float) {
  return cancel(*static_cast<CancelState*>(data));
}
dk_status_t cancel_close_path(void* data) {
  return cancel(*static_cast<CancelState*>(data));
}

void hb_move_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float x,
                float y, void*) {
  (void)api_move_to(data, x, y);
}
void hb_line_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float x,
                float y, void*) {
  (void)api_line_to(data, x, y);
}
void hb_quadratic_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float cx,
                     float cy, float x, float y, void*) {
  (void)api_quadratic_to(data, cx, cy, x, y);
}
void hb_cubic_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float c1x,
                 float c1y, float c2x, float c2y, float x, float y, void*) {
  (void)api_cubic_to(data, c1x, c1y, c2x, c2y, x, y);
}
void hb_close_path(hb_draw_funcs_t*, void* data, hb_draw_state_t*, void*) {
  (void)api_close_path(data);
}

hb_draw_funcs_t* direct_draw_functions() {
  static hb_draw_funcs_t* functions = [] {
    auto* value = hb_draw_funcs_create();
    hb_draw_funcs_set_move_to_func(value, hb_move_to, nullptr, nullptr);
    hb_draw_funcs_set_line_to_func(value, hb_line_to, nullptr, nullptr);
    hb_draw_funcs_set_quadratic_to_func(value, hb_quadratic_to, nullptr,
                                        nullptr);
    hb_draw_funcs_set_cubic_to_func(value, hb_cubic_to, nullptr, nullptr);
    hb_draw_funcs_set_close_path_func(value, hb_close_path, nullptr, nullptr);
    hb_draw_funcs_make_immutable(value);
    return value;
  }();
  return functions;
}

std::vector<unsigned char> read_file(const char* path) {
  std::ifstream input(path, std::ios::binary);
  return {std::istreambuf_iterator<char>{input}, {}};
}

uint16_t read_u16(const std::vector<unsigned char>& value, size_t offset) {
  return static_cast<uint16_t>((value.at(offset) << 8) | value.at(offset + 1));
}

uint32_t read_u32(const std::vector<unsigned char>& value, size_t offset) {
  return (static_cast<uint32_t>(value.at(offset)) << 24) |
         (static_cast<uint32_t>(value.at(offset + 1)) << 16) |
         (static_cast<uint32_t>(value.at(offset + 2)) << 8) |
         static_cast<uint32_t>(value.at(offset + 3));
}

size_t table_record(const std::vector<unsigned char>& font,
                    std::string_view tag) {
  if (tag.size() != 4 || font.size() < 12) return font.size();
  const size_t count = read_u16(font, 4);
  for (size_t index = 0; index < count; ++index) {
    const size_t offset = 12 + index * 16;
    if (offset + 16 > font.size()) return font.size();
    if (std::equal(tag.begin(), tag.end(), font.begin() + offset)) return offset;
  }
  return font.size();
}

std::vector<unsigned char> without_table(
    const std::vector<unsigned char>& font, std::string_view tag) {
  auto result = font;
  const size_t record = table_record(result, tag);
  if (record == result.size()) return {};
  result[record] = 'X';
  return result;
}

std::vector<unsigned char> with_axis_tag(
    const std::vector<unsigned char>& font, std::string_view current,
    std::string_view replacement) {
  auto result = font;
  if (current.size() != 4 || replacement.size() != 4) return {};
  const size_t record = table_record(result, "fvar");
  if (record == result.size()) return {};
  const size_t table = read_u32(result, record + 8);
  if (table + 12 > result.size()) return {};
  const size_t axes_offset = read_u16(result, table + 4);
  const size_t axis_count = read_u16(result, table + 8);
  const size_t axis_size = read_u16(result, table + 10);
  for (size_t index = 0; index < axis_count; ++index) {
    const size_t axis = table + axes_offset + index * axis_size;
    if (axis + 4 > result.size()) return {};
    if (std::equal(current.begin(), current.end(), result.begin() + axis)) {
      std::copy(replacement.begin(), replacement.end(), result.begin() + axis);
      return result;
    }
  }
  return {};
}

Font make_font(hb_face_t* face) {
  Font font{hb_font_create(face)};
  if (!font) return {};
  const unsigned upem = hb_face_get_upem(face);
  hb_ot_font_set_funcs(font.get());
  hb_font_set_scale(font.get(), static_cast<int>(upem),
                    static_cast<int>(upem));
  hb_font_set_ppem(font.get(), upem, upem);
  return font;
}

bool shape_direct(hb_face_t* face, const uint16_t* text, size_t length,
                  int32_t target_width, LineData* output) {
  if (!output) return false;
  Font font = make_font(face);
  Buffer buffer{hb_buffer_create()};
  if (!font || !buffer) return false;
  hb_buffer_add_utf16(buffer.get(), text, static_cast<int>(length), 0,
                      static_cast<int>(length));
  if (!hb_buffer_allocation_successful(buffer.get())) return false;
  hb_buffer_set_direction(buffer.get(), HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer.get(), HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer.get(), hb_language_from_string("ar", -1));
  hb_buffer_set_cluster_level(buffer.get(),
                              HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
  if (target_width > 0) hb_buffer_set_justify(buffer.get(), target_width);
  hb_shape(font.get(), buffer.get(), nullptr, 0);
  if (!hb_buffer_allocation_successful(buffer.get())) return false;

  unsigned info_count = 0;
  unsigned position_count = 0;
  const auto* infos = hb_buffer_get_glyph_infos(buffer.get(), &info_count);
  const auto* positions =
      hb_buffer_get_glyph_positions(buffer.get(), &position_count);
  if (!infos || !positions || info_count == 0 ||
      info_count != position_count ||
      hb_buffer_get_content_type(buffer.get()) != HB_BUFFER_CONTENT_TYPE_GLYPHS) {
    return false;
  }

  output->width = 0;
  output->glyphs.clear();
  output->glyphs.reserve(info_count);
  for (unsigned index = 0; index < info_count; ++index) {
    output->glyphs.push_back({
        infos[index].codepoint,
        infos[index].cluster,
        positions[index].x_advance,
        positions[index].y_advance,
        positions[index].x_offset,
        positions[index].y_offset,
        infos[index].lefttatweel,
        infos[index].righttatweel,
    });
    output->width += positions[index].x_advance;
  }
  return true;
}

bool shape_api(dk_engine_t* engine, const uint16_t* text, size_t length,
               int32_t target_width, LineData* output) {
  if (!output) return false;
  const dk_line_request_v1_t request{
      DK_LINE_REQUEST_V1_SIZE, 0, text, length, target_width};
  dk_line_t* raw = nullptr;
  if (dk_engine_shape_line_v1(engine, &request, &raw) != DK_STATUS_OK ||
      !raw) {
    return false;
  }
  Line line{raw, dk_line_destroy};
  dk_line_metrics_v1_t metrics{};
  metrics.struct_size = DK_LINE_METRICS_V1_SIZE;
  if (dk_line_get_metrics_v1(line.get(), &metrics) != DK_STATUS_OK ||
      metrics.glyph_count == 0 ||
      metrics.glyph_count > std::numeric_limits<size_t>::max()) {
    return false;
  }
  output->width = metrics.width;
  output->glyphs.clear();
  output->glyphs.reserve(static_cast<size_t>(metrics.glyph_count));
  for (size_t index = 0; index < metrics.glyph_count; ++index) {
    dk_glyph_v1_t glyph{};
    glyph.struct_size = DK_GLYPH_V1_SIZE;
    if (dk_line_get_glyph_v1(line.get(), index, &glyph) != DK_STATUS_OK) {
      return false;
    }
    output->glyphs.push_back({
        glyph.glyph_id, glyph.cluster, glyph.x_advance, glyph.y_advance,
        glyph.x_offset, glyph.y_offset, glyph.left_tatweel,
        glyph.right_tatweel});
  }
  return true;
}

bool equal_glyph(const GlyphData& left, const GlyphData& right) {
  return left.glyph_id == right.glyph_id && left.cluster == right.cluster &&
         left.x_advance == right.x_advance &&
         left.y_advance == right.y_advance && left.x_offset == right.x_offset &&
         left.y_offset == right.y_offset &&
         left.left_tatweel == right.left_tatweel &&
         left.right_tatweel == right.right_tatweel;
}

bool equal_line(const LineData& left, const LineData& right) {
  return left.width == right.width &&
         left.glyphs.size() == right.glyphs.size() &&
         std::equal(left.glyphs.begin(), left.glyphs.end(),
                    right.glyphs.begin(), equal_glyph);
}

bool axis_indexes(hb_face_t* face, unsigned* axis_count, unsigned* left,
                  unsigned* right) {
  if (!axis_count || !left || !right) return false;
  *axis_count = hb_ot_var_get_axis_count(face);
  if (*axis_count != 2) return false;
  hb_ot_var_axis_info_t left_info{};
  hb_ot_var_axis_info_t right_info{};
  if (*axis_count < 2 ||
      !hb_ot_var_find_axis_info(face, HB_TAG('L', 'T', 'A', 'T'),
                                &left_info) ||
      !hb_ot_var_find_axis_info(face, HB_TAG('R', 'T', 'A', 'T'),
                                &right_info)) {
    return false;
  }
  *left = left_info.axis_index;
  *right = right_info.axis_index;
  return *left == 0 && *right == 1;
}

PathDigest draw_direct(hb_face_t* face, unsigned axis_count,
                       unsigned left_axis, unsigned right_axis,
                       uint32_t glyph_id, double left_tatweel,
                       double right_tatweel) {
  PathDigest result;
  Font font = make_font(face);
  if (!font) return {};
  std::vector<int> coordinates(axis_count, 0);
  coordinates[left_axis] =
      static_cast<int>(std::lround(left_tatweel * 16384.0));
  coordinates[right_axis] =
      static_cast<int>(std::lround(right_tatweel * 16384.0));
  hb_font_set_var_coords_normalized(font.get(), coordinates.data(), axis_count);
  hb_font_draw_glyph(font.get(), glyph_id, direct_draw_functions(), &result);
  return result;
}

bool draw_api(dk_engine_t* engine, uint32_t glyph_id, double left_tatweel,
              double right_tatweel, PathDigest* output,
              uint32_t sink_size = DK_PATH_SINK_V1_SIZE) {
  if (!output) return false;
  const dk_glyph_variant_v1_t variant{
      DK_GLYPH_VARIANT_V1_SIZE, glyph_id, 0, left_tatweel, right_tatweel};
  auto sink = path_sink(output, sink_size);
  return dk_engine_emit_glyph_outline_v1(engine, &variant, &sink) ==
         DK_STATUS_OK;
}

bool repeat_shape(dk_engine_t* engine, const uint16_t* text, size_t length,
                  int32_t target_width, const LineData& expected) {
  for (int iteration = 0; iteration < 20; ++iteration) {
    LineData actual;
    if (!shape_api(engine, text, length, target_width, &actual) ||
        !equal_line(actual, expected)) {
      return false;
    }
    const auto dynamic = std::find_if(
        actual.glyphs.begin(), actual.glyphs.end(), [](const auto& glyph) {
          return std::abs(glyph.left_tatweel) +
                     std::abs(glyph.right_tatweel) > 1e-9;
        });
    PathDigest outline;
    if (dynamic == actual.glyphs.end() ||
        !draw_api(engine, dynamic->glyph_id, dynamic->left_tatweel,
                  dynamic->right_tatweel, &outline) ||
        outline.commands == 0) {
      return false;
    }
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  CHECK(argc == 2, "expected a DigitalKhatt font path");
  CHECK(dk_engine_abi_version() == DK_ENGINE_ABI_VERSION, "ABI version");
  for (int status = DK_STATUS_OK; status <= DK_STATUS_CANCELLED; ++status) {
    CHECK(*dk_status_message(static_cast<dk_status_t>(status)), "status text");
  }

  const auto font_data = read_file(argv[1]);
  CHECK(!font_data.empty() &&
            font_data.size() <= std::numeric_limits<unsigned>::max(),
        "read font");
  Blob blob{hb_blob_create(reinterpret_cast<const char*>(font_data.data()),
                           static_cast<unsigned>(font_data.size()),
                           HB_MEMORY_MODE_READONLY, nullptr, nullptr)};
  Face face{blob ? hb_face_create(blob.get(), 0) : nullptr};
  CHECK(face && hb_face_get_glyph_count(face.get()) > 0, "create direct face");

  dk_engine_t* file_raw = nullptr;
  auto status = dk_engine_create_from_file(argv[1], &file_raw);
  CHECK(status == DK_STATUS_OK && file_raw, "create file-backed engine");
  Engine engine{file_raw, dk_engine_destroy};
  CHECK(dk_engine_upem(engine.get()) == hb_face_get_upem(face.get()),
        "font UPEM");

  constexpr char16_t source[] = u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
  const std::vector<uint16_t> text(std::begin(source), std::end(source) - 1);
  const auto* text_data = text.data();
  const size_t text_length = text.size();

  LineData natural_api;
  LineData natural_direct;
  CHECK(shape_api(engine.get(), text_data, text_length, 0, &natural_api) &&
            shape_direct(face.get(), text_data, text_length, 0,
                         &natural_direct) &&
            equal_line(natural_api, natural_direct),
        "natural line matches direct HarfBuzz");
  CHECK(natural_api.width > 0 &&
            natural_api.width < std::numeric_limits<int32_t>::max() &&
            !natural_api.glyphs.empty(),
        "usable natural line");

  const int64_t upem = dk_engine_upem(engine.get());
  const int64_t stretch_delta = std::max<int64_t>(1, upem * 2);
  const int64_t shrink_delta =
      std::min(stretch_delta, std::max<int64_t>(1, natural_api.width / 2));
  const auto stretch_target = static_cast<int32_t>(std::min<int64_t>(
      natural_api.width + stretch_delta,
      std::numeric_limits<int32_t>::max()));
  const auto shrink_target =
      static_cast<int32_t>(natural_api.width - shrink_delta);
  const auto saturation_target = static_cast<int32_t>(std::min<int64_t>(
      natural_api.width + upem * 100,
      std::numeric_limits<int32_t>::max()));

  LineData stretch_api;
  LineData stretch_direct;
  LineData shrink_api;
  LineData shrink_direct;
  LineData saturated_api;
  LineData saturated_direct;
  CHECK(shape_api(engine.get(), text_data, text_length, stretch_target,
                  &stretch_api) &&
            shape_direct(face.get(), text_data, text_length, stretch_target,
                         &stretch_direct) &&
            equal_line(stretch_api, stretch_direct),
        "stretched line matches direct HarfBuzz");
  CHECK(shape_api(engine.get(), text_data, text_length, shrink_target,
                  &shrink_api) &&
            shape_direct(face.get(), text_data, text_length, shrink_target,
                         &shrink_direct) &&
            equal_line(shrink_api, shrink_direct),
        "shrunk line matches direct HarfBuzz");
  CHECK(shape_api(engine.get(), text_data, text_length, saturation_target,
                  &saturated_api) &&
            shape_direct(face.get(), text_data, text_length, saturation_target,
                         &saturated_direct) &&
            equal_line(saturated_api, saturated_direct),
        "saturated line matches direct HarfBuzz");

  const auto dynamic = std::find_if(
      stretch_api.glyphs.begin(), stretch_api.glyphs.end(), [](const auto& glyph) {
        return std::abs(glyph.left_tatweel) +
                   std::abs(glyph.right_tatweel) > 1e-9;
      });
  CHECK(dynamic != stretch_api.glyphs.end(), "dynamic glyph variant");
  for (const auto& glyph : stretch_api.glyphs) {
    CHECK(std::isfinite(glyph.left_tatweel) &&
              std::isfinite(glyph.right_tatweel) &&
              std::abs(glyph.left_tatweel) <= 1.0 &&
              std::abs(glyph.right_tatweel) <= 1.0,
          "normalized tatweel range");
  }

  unsigned axis_count = 0;
  unsigned left_axis = 0;
  unsigned right_axis = 0;
  CHECK(axis_indexes(face.get(), &axis_count, &left_axis, &right_axis),
        "find direct tatweel axes");
  PathDigest api_base;
  PathDigest api_variant;
  CHECK(draw_api(engine.get(), dynamic->glyph_id, 0, 0, &api_base) &&
            draw_api(engine.get(), dynamic->glyph_id, dynamic->left_tatweel,
                     dynamic->right_tatweel, &api_variant),
        "draw API outlines");
  const auto direct_base = draw_direct(face.get(), axis_count, left_axis,
                                       right_axis, dynamic->glyph_id, 0, 0);
  const auto direct_variant = draw_direct(
      face.get(), axis_count, left_axis, right_axis, dynamic->glyph_id,
      dynamic->left_tatweel, dynamic->right_tatweel);
  CHECK(api_base.commands > 0 && api_base == direct_base,
        "base outline matches direct HarfBuzz");
  CHECK(api_variant.commands > 0 && api_variant == direct_variant,
        "variant outline matches direct HarfBuzz");
  CHECK(!(api_base == api_variant), "dynamic outline differs from base");

  dk_engine_t* memory_raw = nullptr;
  status = dk_engine_create_from_memory(font_data.data(), font_data.size(),
                                        &memory_raw);
  CHECK(status == DK_STATUS_OK && memory_raw, "create memory-backed engine");
  Engine memory_engine{memory_raw, dk_engine_destroy};
  LineData memory_line;
  CHECK(shape_api(memory_engine.get(), text_data, text_length, stretch_target,
                  &memory_line) &&
            equal_line(memory_line, stretch_api),
        "file and memory engines agree");

  std::vector<std::future<bool>> workers;
  for (int worker = 0; worker < 4; ++worker) {
    workers.push_back(std::async(std::launch::async, [&] {
      return repeat_shape(engine.get(), text_data, text_length, stretch_target,
                          stretch_api);
    }));
  }
  for (auto& worker : workers) CHECK(worker.get(), "concurrent shaping");

  dk_engine_t* invalid = reinterpret_cast<dk_engine_t*>(uintptr_t{1});
  status = dk_engine_create_from_file("/path/that/does/not/exist", &invalid);
  CHECK(status == DK_STATUS_IO_ERROR && !invalid,
        "missing file reports I/O error and clears output");

  const uint32_t junk = 0;
  status = dk_engine_create_from_memory(&junk, sizeof(junk), &invalid);
  CHECK((status == DK_STATUS_INVALID_FONT ||
         status == DK_STATUS_UNSUPPORTED_FONT) &&
            !invalid,
        "reject invalid font");

  for (const std::string_view tag :
       {"CFF2", "GSUB", "GPOS", "GDEF", "JTST", "fvar", "HVAR"}) {
    const auto missing_table = without_table(font_data, tag);
    CHECK(!missing_table.empty(),
          std::string{"prepare font without "} + std::string{tag});
    status = dk_engine_create_from_memory(
        missing_table.data(), missing_table.size(), &invalid);
    CHECK(status == DK_STATUS_UNSUPPORTED_FONT && !invalid,
          std::string{"reject font without "} + std::string{tag});
  }

  const auto wrong_left_axis = with_axis_tag(font_data, "LTAT", "XXXX");
  CHECK(!wrong_left_axis.empty(), "prepare wrong-LTAT font");
  status = dk_engine_create_from_memory(wrong_left_axis.data(),
                                        wrong_left_axis.size(), &invalid);
  CHECK(status == DK_STATUS_UNSUPPORTED_FONT && !invalid,
        "reject font without LTAT axis");

  const auto wrong_right_axis = with_axis_tag(font_data, "RTAT", "XXXX");
  CHECK(!wrong_right_axis.empty(), "prepare wrong-RTAT font");
  status = dk_engine_create_from_memory(wrong_right_axis.data(),
                                        wrong_right_axis.size(), &invalid);
  CHECK(status == DK_STATUS_UNSUPPORTED_FONT && !invalid,
        "reject font without RTAT axis");

  PathDigest invalid_draw;
  auto sink = path_sink(&invalid_draw, DK_PATH_SINK_V1_SIZE);
  dk_glyph_variant_v1_t invalid_variant{
      DK_GLYPH_VARIANT_V1_SIZE, dynamic->glyph_id, 0, 1.0001, 0};
  CHECK(dk_engine_emit_glyph_outline_v1(engine.get(), &invalid_variant,
                                        &sink) == DK_STATUS_INVALID_ARGUMENT,
        "reject out-of-range tatweel");
  invalid_variant = {DK_GLYPH_VARIANT_V1_SIZE, UINT32_MAX, 0, 0, 0};
  CHECK(dk_engine_emit_glyph_outline_v1(engine.get(), &invalid_variant,
                                        &sink) == DK_STATUS_INVALID_ARGUMENT,
        "reject out-of-range glyph ID");
  invalid_variant = {DK_GLYPH_VARIANT_V1_SIZE, dynamic->glyph_id, 0, 0, 0};
  sink.struct_size = DK_PATH_SINK_V1_SIZE - 1;
  CHECK(dk_engine_emit_glyph_outline_v1(engine.get(), &invalid_variant,
                                        &sink) == DK_STATUS_INVALID_ARGUMENT,
        "reject undersized path sink");

  PathDigest blank_api;
  bool found_blank = false;
  for (uint32_t glyph_id = 0; glyph_id < hb_face_get_glyph_count(face.get());
       ++glyph_id) {
    const auto blank_direct =
        draw_direct(face.get(), axis_count, left_axis, right_axis, glyph_id, 0, 0);
    if (blank_direct.commands != 0) continue;
    CHECK(draw_api(engine.get(), glyph_id, 0, 0, &blank_api) &&
              blank_api.commands == 0,
          "blank glyph is a successful empty outline");
    found_blank = true;
    break;
  }
  CHECK(found_blank, "font contains a blank glyph");

  struct ExtendedSink {
    dk_path_sink_v1_t v1;
    void* future;
  };
  PathDigest extended_draw;
  ExtendedSink extended{path_sink(&extended_draw, sizeof(ExtendedSink)),
                        nullptr};
  const dk_glyph_variant_v1_t base_variant{
      DK_GLYPH_VARIANT_V1_SIZE, dynamic->glyph_id, 0, 0, 0};
  CHECK(dk_engine_emit_glyph_outline_v1(engine.get(), &base_variant,
                                        &extended.v1) == DK_STATUS_OK &&
            extended_draw == api_base,
        "accept extended path sink");

  CancelState cancel_state;
  dk_path_sink_v1_t cancel_sink{
      DK_PATH_SINK_V1_SIZE, &cancel_state, cancel_move_to, cancel_line_to,
      cancel_quadratic_to, cancel_cubic_to, cancel_close_path};
  CHECK(dk_engine_emit_glyph_outline_v1(engine.get(), &base_variant,
                                        &cancel_sink) == DK_STATUS_CANCELLED &&
            cancel_state.calls == 1,
        "path callback cancels outline emission");

  struct ExtendedMetrics {
    dk_line_metrics_v1_t v1{};
    uint64_t canary = UINT64_C(0x39b5192f84f7d6a1);
  };
  struct ExtendedGlyph {
    dk_glyph_v1_t v1{};
    uint64_t canary = UINT64_C(0xc42d73e8a9165fb0);
  };
  const dk_line_request_v1_t natural_request{
      DK_LINE_REQUEST_V1_SIZE, 0, text_data, text_length, 0};
  dk_line_t* access_raw = nullptr;
  CHECK(dk_engine_shape_line_v1(engine.get(), &natural_request, &access_raw) ==
                DK_STATUS_OK &&
            access_raw,
        "shape accessor fixture");
  Line access_line{access_raw, dk_line_destroy};
  ExtendedMetrics extended_metrics;
  extended_metrics.v1.struct_size = sizeof(extended_metrics);
  const auto metrics_canary = extended_metrics.canary;
  CHECK(dk_line_get_metrics_v1(access_line.get(), &extended_metrics.v1) ==
                DK_STATUS_OK &&
            extended_metrics.v1.struct_size == sizeof(extended_metrics) &&
            extended_metrics.canary == metrics_canary &&
            extended_metrics.v1.glyph_count == natural_api.glyphs.size(),
        "extended metrics output");
  ExtendedGlyph extended_glyph;
  extended_glyph.v1.struct_size = sizeof(extended_glyph);
  const auto glyph_canary = extended_glyph.canary;
  CHECK(dk_line_get_glyph_v1(access_line.get(), 0, &extended_glyph.v1) ==
                DK_STATUS_OK &&
            extended_glyph.v1.struct_size == sizeof(extended_glyph) &&
            extended_glyph.canary == glyph_canary,
        "extended glyph output");
  dk_line_metrics_v1_t short_metrics{};
  short_metrics.struct_size = DK_LINE_METRICS_V1_SIZE - 1;
  short_metrics.flags = UINT32_MAX;
  CHECK(dk_line_get_metrics_v1(access_line.get(), &short_metrics) ==
                DK_STATUS_INVALID_ARGUMENT &&
            short_metrics.flags == 0,
        "short metrics output");
  dk_glyph_v1_t short_glyph{};
  short_glyph.struct_size = DK_GLYPH_V1_SIZE - 1;
  short_glyph.glyph_id = UINT32_MAX;
  CHECK(dk_line_get_glyph_v1(access_line.get(), 0, &short_glyph) ==
                DK_STATUS_INVALID_ARGUMENT &&
            short_glyph.glyph_id == 0,
        "short glyph output");

  dk_line_metrics_v1_t null_metrics{};
  null_metrics.struct_size = DK_LINE_METRICS_V1_SIZE;
  CHECK(dk_line_get_metrics_v1(nullptr, &null_metrics) ==
                DK_STATUS_INVALID_ARGUMENT &&
            null_metrics.glyph_count == 0,
        "null line reports zero glyphs");

  dk_line_t* cleared_line = reinterpret_cast<dk_line_t*>(uintptr_t{1});
  const dk_line_request_v1_t invalid_request{
      DK_LINE_REQUEST_V1_SIZE, 0, nullptr, text_length, 0};
  status = dk_engine_shape_line_v1(engine.get(), &invalid_request,
                                   &cleared_line);
  CHECK(status == DK_STATUS_INVALID_ARGUMENT && !cleared_line,
        "invalid shaping clears output");
  struct ExtendedRequest {
    dk_line_request_v1_t v1;
    uint64_t future;
  };
  const ExtendedRequest extended_request{
      {sizeof(ExtendedRequest), 0, text_data, text_length, 0},
      UINT64_C(0x0123456789abcdef)};
  cleared_line = nullptr;
  status = dk_engine_shape_line_v1(engine.get(), &extended_request.v1,
                                   &cleared_line);
  CHECK(status == DK_STATUS_OK && cleared_line,
        "accept extended line request");
  dk_line_destroy(cleared_line);

  invalid = reinterpret_cast<dk_engine_t*>(uintptr_t{1});
  status = dk_engine_create_from_memory(nullptr, 0, &invalid);
  CHECK(status == DK_STATUS_INVALID_ARGUMENT && !invalid,
        "invalid creation clears output");

  std::cout << "abi=" << dk_engine_abi_version() << '\n';
  std::cout << "upem=" << dk_engine_upem(engine.get()) << '\n';
  std::cout << "glyphs=" << stretch_api.glyphs.size() << '\n';
  std::cout << "natural_width=" << natural_api.width << '\n';
  std::cout << "stretched_width=" << stretch_api.width << '\n';
  std::cout << "base_outline_hash=" << std::hex << api_base.value << '\n';
  std::cout << "variant_outline_hash=" << api_variant.value << std::dec << '\n';
  return 0;
}
