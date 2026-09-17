#include <digitalkhatt/engine.h>

_Static_assert(sizeof(((dk_line_request_v1_t*)0)->text_length) ==
                   sizeof(uint64_t),
               "line text length must be fixed-width");
_Static_assert(sizeof(((dk_line_metrics_v1_t*)0)->glyph_count) ==
                   sizeof(uint64_t),
               "line glyph count must be fixed-width");

int main(void) {
  dk_engine_t* engine = NULL;
  dk_line_t* line = NULL;
  dk_line_request_v1_t request = {0};
  dk_line_metrics_v1_t metrics = {0};
  dk_glyph_v1_t glyph = {0};
  dk_glyph_variant_v1_t variant = {0};
  dk_path_sink_v1_t sink = {0};

  request.struct_size = DK_LINE_REQUEST_V1_SIZE;
  metrics.struct_size = DK_LINE_METRICS_V1_SIZE;
  glyph.struct_size = DK_GLYPH_V1_SIZE;
  variant.struct_size = DK_GLYPH_VARIANT_V1_SIZE;
  sink.struct_size = DK_PATH_SINK_V1_SIZE;
  (void)dk_engine_create_from_file;
  (void)dk_engine_create_from_memory;
  (void)dk_engine_destroy;
  (void)dk_engine_upem;
  (void)dk_engine_shape_line_v1;
  (void)dk_line_destroy;
  (void)dk_line_get_metrics_v1;
  (void)dk_line_get_glyph_v1;
  (void)dk_engine_emit_glyph_outline_v1;
  (void)engine;
  (void)line;
  (void)request;
  (void)metrics;
  (void)glyph;
  (void)variant;
  (void)sink;
  return dk_engine_abi_version() == DK_ENGINE_ABI_VERSION ? 0 : 1;
}
