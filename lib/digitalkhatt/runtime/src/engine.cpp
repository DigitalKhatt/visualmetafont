#include "digitalkhatt/engine.h"

#include <hb.h>
#include <hb-ot.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <utility>
#include <vector>

namespace {

template <typename T, void (*Destroy)(T*)>
struct HbDeleter {
  void operator()(T* value) const {
    if (value) Destroy(value);
  }
};

template <typename T, void (*Destroy)(T*)>
using HbPtr = std::unique_ptr<T, HbDeleter<T, Destroy>>;

using Blob = HbPtr<hb_blob_t, hb_blob_destroy>;
using Face = HbPtr<hb_face_t, hb_face_destroy>;
using Font = HbPtr<hb_font_t, hb_font_destroy>;
using Buffer = HbPtr<hb_buffer_t, hb_buffer_destroy>;

constexpr hb_tag_t kRequiredTables[] = {
    HB_TAG('C', 'F', 'F', '2'),
    HB_TAG('G', 'S', 'U', 'B'),
    HB_TAG('G', 'P', 'O', 'S'),
    HB_TAG('G', 'D', 'E', 'F'),
    HB_TAG('J', 'T', 'S', 'T'),
    HB_TAG('f', 'v', 'a', 'r'),
    HB_TAG('H', 'V', 'A', 'R'),
};
constexpr hb_tag_t kLeftTatweelAxis = HB_TAG('L', 'T', 'A', 'T');
constexpr hb_tag_t kRightTatweelAxis = HB_TAG('R', 'T', 'A', 'T');

bool valid_tatweel_axis(const hb_ot_var_axis_info_t& axis) {
  return std::isfinite(axis.min_value) && std::isfinite(axis.default_value) &&
         std::isfinite(axis.max_value) && axis.min_value < 0.0f &&
         axis.default_value == 0.0f && axis.max_value > 0.0f;
}

bool has_runtime_axes(hb_face_t* face) {
  if (hb_ot_var_get_axis_count(face) != 2) return false;

  hb_ot_var_axis_info_t left{};
  hb_ot_var_axis_info_t right{};
  return hb_ot_var_find_axis_info(face, kLeftTatweelAxis, &left) &&
         hb_ot_var_find_axis_info(face, kRightTatweelAxis, &right) &&
         left.axis_index == 0 && right.axis_index == 1 &&
         valid_tatweel_axis(left) && valid_tatweel_axis(right);
}

bool has_table(hb_face_t* face, hb_tag_t tag) {
  Blob table{hb_face_reference_table(face, tag)};
  return table && hb_blob_get_length(table.get()) != 0;
}

Font create_font(hb_face_t* face, uint32_t upem) {
  Font font{hb_font_create(face)};
  if (!font) return {};
  hb_ot_font_set_funcs(font.get());
  hb_font_set_scale(font.get(), static_cast<int>(upem),
                    static_cast<int>(upem));
  hb_font_set_ppem(font.get(), upem, upem);
  return font;
}

struct DrawContext {
  const dk_path_sink_v1_t* sink;
  dk_status_t status = DK_STATUS_OK;
};

template <typename Callback, typename... Args>
void emit_path_command(DrawContext& context, Callback callback,
                       Args... arguments) {
  if (context.status == DK_STATUS_OK) {
    context.status = callback(context.sink->user_data, arguments...);
  }
}

void move_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float x, float y,
             void*) {
  auto& context = *static_cast<DrawContext*>(data);
  emit_path_command(context, context.sink->move_to, x, y);
}

void line_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float x, float y,
             void*) {
  auto& context = *static_cast<DrawContext*>(data);
  emit_path_command(context, context.sink->line_to, x, y);
}

void quadratic_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float cx,
                  float cy, float x, float y, void*) {
  auto& context = *static_cast<DrawContext*>(data);
  emit_path_command(context, context.sink->quadratic_to, cx, cy, x, y);
}

void cubic_to(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float c1x,
              float c1y, float c2x, float c2y, float x, float y, void*) {
  auto& context = *static_cast<DrawContext*>(data);
  emit_path_command(context, context.sink->cubic_to, c1x, c1y, c2x, c2y, x,
                    y);
}

void close_path(hb_draw_funcs_t*, void* data, hb_draw_state_t*, void*) {
  auto& context = *static_cast<DrawContext*>(data);
  emit_path_command(context, context.sink->close_path);
}

hb_draw_funcs_t* draw_functions() {
  static hb_draw_funcs_t* functions = [] {
    hb_draw_funcs_t* value = hb_draw_funcs_create();
    hb_draw_funcs_set_move_to_func(value, move_to, nullptr, nullptr);
    hb_draw_funcs_set_line_to_func(value, line_to, nullptr, nullptr);
    hb_draw_funcs_set_quadratic_to_func(value, quadratic_to, nullptr, nullptr);
    hb_draw_funcs_set_cubic_to_func(value, cubic_to, nullptr, nullptr);
    hb_draw_funcs_set_close_path_func(value, close_path, nullptr, nullptr);
    hb_draw_funcs_make_immutable(value);
    return value;
  }();
  return functions;
}

bool valid_sink(const dk_path_sink_v1_t* sink) {
  return sink && sink->struct_size >= DK_PATH_SINK_V1_SIZE &&
         sink->move_to && sink->line_to && sink->quadratic_to &&
         sink->cubic_to && sink->close_path;
}

}  // namespace

struct dk_engine {
  Face face;
  uint32_t upem;
  uint32_t glyph_count;

  dk_engine(Face font_face, uint32_t units_per_em,
            uint32_t font_glyph_count)
      : face(std::move(font_face)),
        upem(units_per_em),
        glyph_count(font_glyph_count) {}
};

struct GlyphResult {
  uint32_t glyph_id = 0;
  uint32_t cluster = 0;
  int32_t x_advance = 0;
  int32_t y_advance = 0;
  int32_t x_offset = 0;
  int32_t y_offset = 0;
  double left_tatweel = 0;
  double right_tatweel = 0;
};

struct dk_line {
  std::vector<GlyphResult> glyphs;
  int64_t width = 0;
};

namespace {

template <typename T>
bool prepare_output(T* output, uint32_t required_size) {
  if (!output) return false;
  const uint32_t supplied_size = output->struct_size;
  const size_t clear_size =
      supplied_size < required_size ? supplied_size : required_size;
  if (clear_size != 0) std::memset(output, 0, clear_size);
  if (supplied_size >= sizeof(output->struct_size)) {
    output->struct_size = supplied_size;
  }
  return supplied_size >= required_size;
}

dk_status_t finish_engine(Blob blob, dk_engine_t** out_engine) {
  if (!blob || hb_blob_get_length(blob.get()) == 0) {
    return DK_STATUS_INVALID_FONT;
  }

  Face face{hb_face_create(blob.get(), 0)};
  const uint32_t glyph_count = face ? hb_face_get_glyph_count(face.get()) : 0;
  if (!face || glyph_count == 0) return DK_STATUS_INVALID_FONT;

  const uint32_t upem = hb_face_get_upem(face.get());
  if (upem == 0 ||
      upem > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
    return DK_STATUS_INVALID_FONT;
  }
  for (const hb_tag_t tag : kRequiredTables) {
    if (!has_table(face.get(), tag)) return DK_STATUS_UNSUPPORTED_FONT;
  }
  if (!has_runtime_axes(face.get())) return DK_STATUS_UNSUPPORTED_FONT;
  hb_face_make_immutable(face.get());

  auto engine =
      std::make_unique<dk_engine>(std::move(face), upem, glyph_count);
  *out_engine = engine.release();
  return DK_STATUS_OK;
}

}  // namespace

extern "C" {

uint32_t dk_engine_abi_version(void) { return DK_ENGINE_ABI_VERSION; }

const char* dk_status_message(dk_status_t status) {
  switch (status) {
    case DK_STATUS_OK: return "ok";
    case DK_STATUS_INVALID_ARGUMENT: return "invalid argument";
    case DK_STATUS_OUT_OF_MEMORY: return "out of memory";
    case DK_STATUS_IO_ERROR: return "I/O error";
    case DK_STATUS_INVALID_FONT: return "invalid font";
    case DK_STATUS_UNSUPPORTED_FONT: return "unsupported DigitalKhatt font";
    case DK_STATUS_SHAPING_FAILED: return "shaping failed";
    case DK_STATUS_INTERNAL_ERROR: return "internal error";
    case DK_STATUS_CANCELLED: return "cancelled";
  }
  return "unknown status";
}

dk_status_t dk_engine_create_from_file(const char* font_path,
                                        dk_engine_t** out_engine) {
  if (!out_engine) return DK_STATUS_INVALID_ARGUMENT;
  *out_engine = nullptr;
  if (!font_path || !*font_path) return DK_STATUS_INVALID_ARGUMENT;
  try {
    Blob blob{hb_blob_create_from_file_or_fail(font_path)};
    if (!blob) return DK_STATUS_IO_ERROR;
    return finish_engine(std::move(blob), out_engine);
  } catch (const std::bad_alloc&) {
    return DK_STATUS_OUT_OF_MEMORY;
  } catch (...) {
    return DK_STATUS_INTERNAL_ERROR;
  }
}

dk_status_t dk_engine_create_from_memory(const void* font_data, size_t font_size,
                                          dk_engine_t** out_engine) {
  if (!out_engine) return DK_STATUS_INVALID_ARGUMENT;
  *out_engine = nullptr;
  if (!font_data || font_size == 0 ||
      font_size > std::numeric_limits<unsigned>::max()) {
    return DK_STATUS_INVALID_ARGUMENT;
  }
  try {
    Blob blob{hb_blob_create(static_cast<const char*>(font_data),
                             static_cast<unsigned>(font_size),
                             HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr)};
    if (!blob || hb_blob_get_length(blob.get()) != font_size) {
      return DK_STATUS_OUT_OF_MEMORY;
    }
    return finish_engine(std::move(blob), out_engine);
  } catch (const std::bad_alloc&) {
    return DK_STATUS_OUT_OF_MEMORY;
  } catch (...) {
    return DK_STATUS_INTERNAL_ERROR;
  }
}

void dk_engine_destroy(dk_engine_t* engine) { delete engine; }

uint32_t dk_engine_upem(const dk_engine_t* engine) {
  return engine ? engine->upem : 0;
}

dk_status_t dk_engine_shape_line_v1(const dk_engine_t* engine,
                                    const dk_line_request_v1_t* request,
                                    dk_line_t** out_line) {
  if (!out_line) return DK_STATUS_INVALID_ARGUMENT;
  *out_line = nullptr;
  if (!engine || !request ||
      request->struct_size < DK_LINE_REQUEST_V1_SIZE || request->flags != 0 ||
      !request->text || request->text_length == 0 ||
      request->text_length >
          static_cast<uint64_t>(std::numeric_limits<int>::max()) ||
      request->target_width < 0) {
    return DK_STATUS_INVALID_ARGUMENT;
  }

  try {
    Font font = create_font(engine->face.get(), engine->upem);
    Buffer buffer{hb_buffer_create()};
    if (!font || !buffer) return DK_STATUS_OUT_OF_MEMORY;

    const auto text_length = static_cast<int>(request->text_length);
    hb_buffer_add_utf16(buffer.get(), request->text, text_length, 0,
                        text_length);
    if (!hb_buffer_allocation_successful(buffer.get())) {
      return DK_STATUS_OUT_OF_MEMORY;
    }
    hb_buffer_set_direction(buffer.get(), HB_DIRECTION_RTL);
    hb_buffer_set_script(buffer.get(), HB_SCRIPT_ARABIC);
    hb_buffer_set_language(buffer.get(), hb_language_from_string("ar", -1));
    hb_buffer_set_cluster_level(
        buffer.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    if (request->target_width > 0) {
      hb_buffer_set_justify(buffer.get(), request->target_width);
    }
    hb_shape(font.get(), buffer.get(), nullptr, 0);
    if (!hb_buffer_allocation_successful(buffer.get())) {
      return DK_STATUS_OUT_OF_MEMORY;
    }

    unsigned info_count = 0;
    unsigned position_count = 0;
    const hb_glyph_info_t* infos =
        hb_buffer_get_glyph_infos(buffer.get(), &info_count);
    const hb_glyph_position_t* positions =
        hb_buffer_get_glyph_positions(buffer.get(), &position_count);
    if (!infos || !positions || info_count == 0 ||
        info_count != position_count ||
        hb_buffer_get_content_type(buffer.get()) !=
            HB_BUFFER_CONTENT_TYPE_GLYPHS) {
      return DK_STATUS_SHAPING_FAILED;
    }

    auto line = std::make_unique<dk_line>();
    line->glyphs.reserve(info_count);
    for (unsigned index = 0; index < info_count; ++index) {
      if (!std::isfinite(infos[index].lefttatweel) ||
          !std::isfinite(infos[index].righttatweel) ||
          std::abs(infos[index].lefttatweel) > 1.0 ||
          std::abs(infos[index].righttatweel) > 1.0) {
        return DK_STATUS_SHAPING_FAILED;
      }
      line->glyphs.push_back({
          infos[index].codepoint,
          infos[index].cluster,
          positions[index].x_advance,
          positions[index].y_advance,
          positions[index].x_offset,
          positions[index].y_offset,
          infos[index].lefttatweel,
          infos[index].righttatweel,
      });
      line->width += positions[index].x_advance;
    }
    *out_line = line.release();
    return DK_STATUS_OK;
  } catch (const std::bad_alloc&) {
    return DK_STATUS_OUT_OF_MEMORY;
  } catch (...) {
    return DK_STATUS_INTERNAL_ERROR;
  }
}

void dk_line_destroy(dk_line_t* line) { delete line; }

dk_status_t dk_line_get_metrics_v1(const dk_line_t* line,
                                   dk_line_metrics_v1_t* out_metrics) {
  if (!prepare_output(out_metrics, DK_LINE_METRICS_V1_SIZE)) {
    return DK_STATUS_INVALID_ARGUMENT;
  }
  if (!line) return DK_STATUS_INVALID_ARGUMENT;
  out_metrics->flags = 0;
  out_metrics->width = line->width;
  out_metrics->glyph_count = line->glyphs.size();
  return DK_STATUS_OK;
}

dk_status_t dk_line_get_glyph_v1(const dk_line_t* line, size_t glyph_index,
                                 dk_glyph_v1_t* out_glyph) {
  if (!prepare_output(out_glyph, DK_GLYPH_V1_SIZE)) {
    return DK_STATUS_INVALID_ARGUMENT;
  }
  if (!line || glyph_index >= line->glyphs.size()) {
    return DK_STATUS_INVALID_ARGUMENT;
  }
  const auto& glyph = line->glyphs[glyph_index];
  out_glyph->glyph_id = glyph.glyph_id;
  out_glyph->cluster = glyph.cluster;
  out_glyph->flags = 0;
  out_glyph->x_advance = glyph.x_advance;
  out_glyph->y_advance = glyph.y_advance;
  out_glyph->x_offset = glyph.x_offset;
  out_glyph->y_offset = glyph.y_offset;
  out_glyph->left_tatweel = glyph.left_tatweel;
  out_glyph->right_tatweel = glyph.right_tatweel;
  return DK_STATUS_OK;
}

dk_status_t dk_engine_emit_glyph_outline_v1(
    const dk_engine_t* engine, const dk_glyph_variant_v1_t* variant,
    const dk_path_sink_v1_t* sink) {
  if (!engine || !variant ||
      variant->struct_size < DK_GLYPH_VARIANT_V1_SIZE ||
      variant->flags != 0 || !valid_sink(sink) ||
      !std::isfinite(variant->left_tatweel) ||
      !std::isfinite(variant->right_tatweel) ||
      std::abs(variant->left_tatweel) > 1.0 ||
      std::abs(variant->right_tatweel) > 1.0 ||
      variant->glyph_id >= engine->glyph_count) {
    return DK_STATUS_INVALID_ARGUMENT;
  }

  try {
    Font font = create_font(engine->face.get(), engine->upem);
    if (!font) return DK_STATUS_OUT_OF_MEMORY;
    const int coordinates[] = {
        static_cast<int>(std::lround(variant->left_tatweel * 16384.0)),
        static_cast<int>(std::lround(variant->right_tatweel * 16384.0)),
    };
    hb_font_set_var_coords_normalized(font.get(), coordinates, 2);
    DrawContext context{sink};
    hb_font_draw_glyph(font.get(), variant->glyph_id, draw_functions(),
                       &context);
    return context.status;
  } catch (const std::bad_alloc&) {
    return DK_STATUS_OUT_OF_MEMORY;
  } catch (...) {
    return DK_STATUS_INTERNAL_ERROR;
  }
}

}  // extern "C"
