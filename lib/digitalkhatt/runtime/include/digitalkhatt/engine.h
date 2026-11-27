#ifndef DIGITALKHATT_ENGINE_H
#define DIGITALKHATT_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(DIGITALKHATT_ENGINE_SHARED)
  #if defined(DIGITALKHATT_ENGINE_BUILDING)
    #define DK_ENGINE_API __declspec(dllexport)
  #else
    #define DK_ENGINE_API __declspec(dllimport)
  #endif
#elif defined(__GNUC__) || defined(__clang__)
  #define DK_ENGINE_API __attribute__((visibility("default")))
#else
  #define DK_ENGINE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DK_ENGINE_ABI_VERSION 1u

typedef struct dk_engine dk_engine_t;
typedef struct dk_line dk_line_t;

typedef uint32_t dk_status_t;
enum {
  DK_STATUS_OK = 0u,
  DK_STATUS_INVALID_ARGUMENT = 1u,
  DK_STATUS_OUT_OF_MEMORY = 2u,
  DK_STATUS_IO_ERROR = 3u,
  DK_STATUS_INVALID_FONT = 4u,
  DK_STATUS_UNSUPPORTED_FONT = 5u,
  DK_STATUS_SHAPING_FAILED = 6u,
  DK_STATUS_INTERNAL_ERROR = 7u,
  DK_STATUS_CANCELLED = 8u
};

typedef struct dk_line_request_v1 {
  /* Set to DK_LINE_REQUEST_V1_SIZE. Later fields, if any, are append-only. */
  uint32_t struct_size;
  /* Reserved. Must be zero. */
  uint32_t flags;
  const uint16_t* text;
  uint64_t text_length;
  /* Font units. Zero disables the font's JTST target-width pass. */
  int32_t target_width;
} dk_line_request_v1_t;

#define DK_LINE_REQUEST_V1_SIZE                                           \
  ((uint32_t)(offsetof(dk_line_request_v1_t, target_width) +              \
              sizeof(((dk_line_request_v1_t*)0)->target_width)))

typedef struct dk_line_metrics_v1 {
  /* Set to DK_LINE_METRICS_V1_SIZE before calling dk_line_get_metrics_v1(). */
  uint32_t struct_size;
  /* Reserved. Zero in revision 1. */
  uint32_t flags;
  int64_t width;
  uint64_t glyph_count;
} dk_line_metrics_v1_t;

#define DK_LINE_METRICS_V1_SIZE                                           \
  ((uint32_t)(offsetof(dk_line_metrics_v1_t, glyph_count) +               \
              sizeof(((dk_line_metrics_v1_t*)0)->glyph_count)))

typedef struct dk_glyph_v1 {
  /* Set to DK_GLYPH_V1_SIZE before calling dk_line_get_glyph_v1(). */
  uint32_t struct_size;
  uint32_t glyph_id;
  uint32_t cluster;
  /* Reserved. Zero in revision 1. */
  uint32_t flags;
  int32_t x_advance;
  int32_t y_advance;
  int32_t x_offset;
  int32_t y_offset;
  /* Normalized LTAT/RTAT coordinates in the inclusive range [-1, 1]. */
  double left_tatweel;
  double right_tatweel;
} dk_glyph_v1_t;

#define DK_GLYPH_V1_SIZE                                                  \
  ((uint32_t)(offsetof(dk_glyph_v1_t, right_tatweel) +                    \
              sizeof(((dk_glyph_v1_t*)0)->right_tatweel)))

typedef struct dk_glyph_variant_v1 {
  /* Set to DK_GLYPH_VARIANT_V1_SIZE. Later fields, if any, are append-only. */
  uint32_t struct_size;
  uint32_t glyph_id;
  /* Reserved. Must be zero. */
  uint32_t flags;
  /* Normalized LTAT/RTAT coordinates in the inclusive range [-1, 1]. */
  double left_tatweel;
  double right_tatweel;
} dk_glyph_variant_v1_t;

#define DK_GLYPH_VARIANT_V1_SIZE                                          \
  ((uint32_t)(offsetof(dk_glyph_variant_v1_t, right_tatweel) +            \
              sizeof(((dk_glyph_variant_v1_t*)0)->right_tatweel)))

typedef struct dk_path_sink_v1 {
  /* Set to DK_PATH_SINK_V1_SIZE. Later fields, if any, are append-only. */
  uint32_t struct_size;
  void* user_data;
  dk_status_t (*move_to)(void* user_data, float x, float y);
  dk_status_t (*line_to)(void* user_data, float x, float y);
  dk_status_t (*quadratic_to)(void* user_data, float control_x,
                              float control_y, float x, float y);
  dk_status_t (*cubic_to)(void* user_data, float control1_x,
                          float control1_y, float control2_x,
                          float control2_y, float x, float y);
  dk_status_t (*close_path)(void* user_data);
} dk_path_sink_v1_t;

#define DK_PATH_SINK_V1_SIZE                                              \
  ((uint32_t)(offsetof(dk_path_sink_v1_t, close_path) +                   \
              sizeof(((dk_path_sink_v1_t*)0)->close_path)))

DK_ENGINE_API uint32_t dk_engine_abi_version(void);
DK_ENGINE_API const char* dk_status_message(dk_status_t status);

/*
 * The engine copies memory-backed font data and owns file-backed data. A
 * created engine is immutable: shaping and outline emission may run
 * concurrently, but destruction must wait until all calls have returned.
 */
DK_ENGINE_API dk_status_t dk_engine_create_from_file(const char* font_path,
                                                      dk_engine_t** out_engine);
DK_ENGINE_API dk_status_t dk_engine_create_from_memory(const void* font_data,
                                                        size_t font_size,
                                                        dk_engine_t** out_engine);
DK_ENGINE_API void dk_engine_destroy(dk_engine_t* engine);
DK_ENGINE_API uint32_t dk_engine_upem(const dk_engine_t* engine);

/*
 * Shapes one independent RTL Arabic UTF-16 line. This low-level operation
 * optionally runs the font's JTST target-width pass; it does not apply a
 * Mushaf page profile or FeatureJustifier page policy. Input text is copied
 * before return. Calls are independent and may run concurrently.
 */
DK_ENGINE_API dk_status_t dk_engine_shape_line_v1(
    const dk_engine_t* engine,
    const dk_line_request_v1_t* request,
    dk_line_t** out_line);
DK_ENGINE_API void dk_line_destroy(dk_line_t* line);
DK_ENGINE_API dk_status_t dk_line_get_metrics_v1(
    const dk_line_t* line,
    dk_line_metrics_v1_t* out_metrics);
DK_ENGINE_API dk_status_t dk_line_get_glyph_v1(
    const dk_line_t* line,
    size_t glyph_index,
    dk_glyph_v1_t* out_glyph);

/*
 * Emits the foreground CFF2 outline selected by a glyph ID and normalized
 * LTAT/RTAT coordinates. A valid glyph with no outline emits no commands and
 * returns DK_STATUS_OK. A callback may return DK_STATUS_CANCELLED or another
 * error; remaining commands are skipped and that status is returned.
 * Callbacks must not throw across the C boundary.
 */
DK_ENGINE_API dk_status_t dk_engine_emit_glyph_outline_v1(
    const dk_engine_t* engine,
    const dk_glyph_variant_v1_t* variant,
    const dk_path_sink_v1_t* sink);

#ifdef __cplusplus
}
#endif

#endif
