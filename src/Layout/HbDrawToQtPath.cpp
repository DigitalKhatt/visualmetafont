#include "HbDrawToQtPath.h"

HbDrawToQtPath::HbDrawToQtPath() {
  m_funcs = hb_draw_funcs_create();

  // We store path coordinates in Qt/PDF coordinates (y-down),
  // so we flip y here: (x, -y).
  hb_draw_funcs_set_move_to_func(m_funcs, &HbDrawToQtPath::move_to, nullptr, nullptr);
  hb_draw_funcs_set_line_to_func(m_funcs, &HbDrawToQtPath::line_to, nullptr, nullptr);
  hb_draw_funcs_set_quadratic_to_func(m_funcs, &HbDrawToQtPath::quadratic_to, nullptr, nullptr);
  hb_draw_funcs_set_cubic_to_func(m_funcs, &HbDrawToQtPath::cubic_to, nullptr, nullptr);
  hb_draw_funcs_set_close_path_func(m_funcs, &HbDrawToQtPath::close_path, nullptr, nullptr);

  hb_draw_funcs_make_immutable(m_funcs);
}

HbDrawToQtPath::~HbDrawToQtPath() {
  if (m_funcs) hb_draw_funcs_destroy(m_funcs);
  m_funcs = nullptr;
}

static inline QPointF qpt(float x, float y) {
  return QPointF((double)x, (double)(-y));  // flip Y
}

void HbDrawToQtPath::move_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                             float to_x, float to_y, void*) {
  auto* ctx = reinterpret_cast<Ctx*>(draw_data);
  if (!ctx || !ctx->path) return;
  ctx->current = qpt(to_x, to_y);
  ctx->path->moveTo(ctx->current);
  ctx->hasCurrent = true;
}

void HbDrawToQtPath::line_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                             float to_x, float to_y, void*) {
  auto* ctx = reinterpret_cast<Ctx*>(draw_data);
  if (!ctx || !ctx->path || !ctx->hasCurrent) return;
  ctx->current = qpt(to_x, to_y);
  ctx->path->lineTo(ctx->current);
}

void HbDrawToQtPath::quadratic_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                                  float c1_x, float c1_y, float to_x, float to_y, void*) {
  auto* ctx = reinterpret_cast<Ctx*>(draw_data);
  if (!ctx || !ctx->path || !ctx->hasCurrent) return;
  ctx->path->quadTo(qpt(c1_x, c1_y), qpt(to_x, to_y));
  ctx->current = qpt(to_x, to_y);
}

void HbDrawToQtPath::cubic_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                              float c1_x, float c1_y, float c2_x, float c2_y, float to_x, float to_y, void*) {
  auto* ctx = reinterpret_cast<Ctx*>(draw_data);
  if (!ctx || !ctx->path || !ctx->hasCurrent) return;
  ctx->path->cubicTo(qpt(c1_x, c1_y), qpt(c2_x, c2_y), qpt(to_x, to_y));
  ctx->current = qpt(to_x, to_y);
}

void HbDrawToQtPath::close_path(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*, void*) {
  auto* ctx = reinterpret_cast<Ctx*>(draw_data);
  if (!ctx || !ctx->path) return;
  ctx->path->closeSubpath();
}

QPainterPath HbDrawToQtPath::drawGlyphPath(hb_font_t* font, hb_codepoint_t gid) const {
  QPainterPath path;
  Ctx ctx;
  ctx.path = &path;

  // If outlines are missing, path will remain empty.
  hb_font_draw_glyph(font, gid, m_funcs, &ctx);

  return path;
}
