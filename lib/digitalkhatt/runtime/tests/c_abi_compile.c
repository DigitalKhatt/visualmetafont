#include <digitalkhatt/engine.h>

_Static_assert(sizeof(((dk_line_request_v1_t*)0)->text_length) ==
                   sizeof(uint64_t),
               "line text length must be fixed-width");
_Static_assert(sizeof(((dk_line_metrics_v1_t*)0)->glyph_count) ==
                   sizeof(uint64_t),
               "line glyph count must be fixed-width");
_Static_assert(sizeof(((dk_page_line_input_v1_t*)0)->text_length) ==
                   sizeof(uint64_t),
               "page text length must be fixed-width");
_Static_assert(sizeof(((dk_page_line_v1_t*)0)->glyph_count) ==
                   sizeof(uint64_t),
               "page glyph count must be fixed-width");

int main(void) {
  dk_engine_t* engine = NULL;
  dk_line_t* line = NULL;
  dk_line_request_v1_t request = {0};
  dk_line_metrics_v1_t metrics = {0};
  dk_glyph_v1_t glyph = {0};
  dk_glyph_variant_v1_t variant = {0};
  dk_path_sink_v1_t sink = {0};
  dk_page_t* page = NULL;
  dk_page_options_v1_t page_options = {0};
  dk_page_line_input_v1_t page_input = {0};
  dk_page_line_v1_t page_line = {0};
  dk_page_glyph_v1_t page_glyph = {0};

  request.struct_size = DK_LINE_REQUEST_V1_SIZE;
  metrics.struct_size = DK_LINE_METRICS_V1_SIZE;
  glyph.struct_size = DK_GLYPH_V1_SIZE;
  variant.struct_size = DK_GLYPH_VARIANT_V1_SIZE;
  sink.struct_size = DK_PATH_SINK_V1_SIZE;
  page_options.struct_size = DK_PAGE_OPTIONS_V1_SIZE;
  page_options.profile = DK_PAGE_PROFILE_MADINAH_1441_V1;
  page_options.line_stride = (uint32_t)sizeof(page_input);
  page_input.struct_size = DK_PAGE_LINE_INPUT_V1_SIZE;
  page_line.struct_size = DK_PAGE_LINE_V1_SIZE;
  page_glyph.struct_size = DK_PAGE_GLYPH_V1_SIZE;
  (void)dk_engine_create_from_file;
  (void)dk_engine_create_from_memory;
  (void)dk_engine_destroy;
  (void)dk_engine_upem;
  (void)dk_engine_shape_line_v1;
  (void)dk_line_destroy;
  (void)dk_line_get_metrics_v1;
  (void)dk_line_get_glyph_v1;
  (void)dk_engine_shape_page_utf16_v1;
  (void)dk_page_destroy;
  (void)dk_page_line_count;
  (void)dk_page_get_line_v1;
  (void)dk_page_get_glyph_v1;
  (void)dk_engine_emit_glyph_outline_v1;
  (void)engine;
  (void)line;
  (void)request;
  (void)metrics;
  (void)glyph;
  (void)variant;
  (void)sink;
  (void)page;
  (void)page_options;
  (void)page_input;
  (void)page_line;
  (void)page_glyph;
  return dk_engine_abi_version() == DK_ENGINE_ABI_VERSION ? 0 : 1;
}
