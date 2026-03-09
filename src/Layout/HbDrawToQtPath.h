#pragma once
#include <hb.h>

#include <QPainterPath>
#include <QPointF>

class HbDrawToQtPath {
 public:
  HbDrawToQtPath();
  ~HbDrawToQtPath();

  QPainterPath drawGlyphPath(hb_font_t* font, hb_codepoint_t gid) const;

  HbDrawToQtPath(const HbDrawToQtPath&) = delete;
  HbDrawToQtPath& operator=(const HbDrawToQtPath&) = delete;

 private:
  hb_draw_funcs_t* m_funcs = nullptr;

  struct Ctx {
    QPainterPath* path = nullptr;
    QPointF current;
    bool hasCurrent = false;
  };

  static void move_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                      float to_x, float to_y, void*);
  static void line_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                      float to_x, float to_y, void*);
  static void quadratic_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                           float c1_x, float c1_y, float to_x, float to_y, void*);
  static void cubic_to(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*,
                       float c1_x, float c1_y, float c2_x, float c2_y, float to_x, float to_y, void*);
  static void close_path(hb_draw_funcs_t*, void* draw_data, hb_draw_state_t*, void*);
};