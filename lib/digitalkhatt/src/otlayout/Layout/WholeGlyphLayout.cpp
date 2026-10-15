#include "digitalkhatt/layout/WholeGlyphLayout.h"
#include "Layout/OtLayout.h"
#include "Layout/GlyphVis.h"
#include <bit>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>

extern "C" {
#include "mplibps.h"
}

namespace digitalkhatt {
namespace {
struct Bytes {
  std::vector<uint8_t> data;
  void u8(unsigned v) {
    if (v>255) throw std::runtime_error("DKLY value exceeds uint8");
    data.push_back(static_cast<uint8_t>(v));
  }
  void u16(unsigned v) {
    if (v>65535) throw std::runtime_error("DKLY value exceeds uint16");
    u8(v>>8); u8(v&255);
  }
  void u32(uint32_t v) { u16(v>>16); u16(v&65535); }
  void i16(int v) {
    if (v<std::numeric_limits<int16_t>::min() || v>std::numeric_limits<int16_t>::max())
      throw std::runtime_error("DKLY placement exceeds int16");
    u16(static_cast<uint16_t>(v));
  }
  void f32(double v) {
    float f=static_cast<float>(v);
    if (!std::isfinite(f)) throw std::runtime_error("Non-finite DKLY coordinate");
    u32(std::bit_cast<uint32_t>(f));
  }
  void fixed(double v) {
    // Match the existing GUI serializer's float32 roundf conversion.
    float f=std::round(static_cast<float>(v)*65536.f);
    if (!std::isfinite(f) || double(f)<INT32_MIN || double(f)>INT32_MAX)
      throw std::runtime_error("Invalid DKLY line scale");
    u32(static_cast<uint32_t>(static_cast<int32_t>(f)));
  }
  void append(const Bytes& other) { data.insert(data.end(),other.data.begin(),other.data.end()); }
};
using Key=std::pair<unsigned,std::vector<double>>;
Key key(const GlyphLayoutInfo& glyph) {
  if (glyph.codepoint<0 || glyph.codepoint>65535) throw std::runtime_error("Invalid DKLY glyph code");
  Key result{static_cast<unsigned>(glyph.codepoint),{}};
  for (GlyphAxisId axis=0;axis<glyph.parameters.size();++axis) {
    double v=glyph.parameters.value(axis);
    if (!std::isfinite(v)) throw std::runtime_error("Non-finite glyph parameter");
    result.second.push_back(v);
  }
  return result;
}
void outline(const GlyphVis& glyph,Bytes& out) {
  Bytes contours; unsigned count=0;
  for (auto* object=glyph.mpPath();object;object=object->next) {
    if (object->type!=mp_fill_code) continue;
    auto* start=reinterpret_cast<mp_fill_object*>(object)->path_p;
    if (!start) continue;
    ++count;
    contours.f32(start->x_coord);contours.f32(start->y_coord);
    Bytes cubics;unsigned n=0;auto* p=start;
    do {
      ++n;auto* q=p->next;
      cubics.f32(p->right_x);cubics.f32(p->right_y);
      cubics.f32(q->left_x);cubics.f32(q->left_y);
      cubics.f32(q->x_coord);cubics.f32(q->y_coord);p=q;
    } while(p!=start);
    contours.u8(n);contours.append(cubics);
  }
  out.u8(count);out.append(contours);
}
unsigned color(uint32_t rgba) {
  static const std::map<uint32_t,unsigned> colors{
    {0x00A650FF,1},{0x006694FF,2},{0xB4B4B4FF,3},{0x00ADEFFF,4},
    {0xC38A08FF,5},{0xF47216FF,6},{0xEC008CFF,7},{0x8C0000FF,8}};
  auto it=colors.find(rgba);return it==colors.end()?0:it->second;
}
}
std::vector<uint8_t> wholeGlyphLayoutBinary(OtLayout& layout,const LayoutPages& pages) {
  if (pages.pages.size()!=pages.originalPages.size()) throw std::runtime_error("DKLY text/page count differs");
  struct Variant { const GlyphLayoutInfo* placement;unsigned id=0; };
  std::map<Key,Variant> variants;
  for (const auto& page:pages.pages) for (const auto& line:page)
    if (line.type!=LineType::Sura) for (const auto& g:line.glyphs) variants.try_emplace(key(g),Variant{&g});
  if (variants.empty()) throw std::runtime_error("No body glyphs for DKLY export");
  Bytes out;out.u32(0x444B4C59);out.u16(4);
  out.u32(layout.pageLayout().pageWidth << OtLayout::SCALEBY);
  out.u32(layout.interLineSpacing() << OtLayout::SCALEBY);
  out.u16(variants.size());
  unsigned next=1;
  for (auto& [k,v]:variants) {
    v.id=next++;auto* glyph=layout.getGlyph(*v.placement);
    if (!glyph) throw std::runtime_error("Missing GlyphVis for DKLY export");
    out.u16(k.first);outline(*glyph,out);
  }
  out.u16(pages.pages.size());unsigned surah=0;
  for (size_t p=0;p<pages.pages.size();++p) {
    const auto& page=pages.pages[p];const auto& text=pages.originalPages[p];
    if(page.size()!=text.size()) throw std::runtime_error("DKLY text/line count differs");
    out.u8(page.size());
    for (size_t l=0;l<page.size();++l) {
      const auto& line=page[l];out.u8(line.glyphs.size());
      for(const auto& g:line.glyphs) {
        unsigned mask=(g.x_advance?1:0)|(g.x_offset?2:0)|(g.y_offset?4:0)|(g.color?8:0);
        out.u16(line.type==LineType::Sura?0:variants.at(key(g)).id);
        out.u8(g.cluster);out.u8(mask);
        if(mask&1)out.i16(g.x_advance);
        if(mask&2)out.i16(g.x_offset);
        if(mask&4)out.i16(g.y_offset);
        if(mask&8)out.u8(color(g.color));
      }
      out.u8(static_cast<unsigned>(line.type));
      out.i16((layout.pageLayout().rightMargin << OtLayout::SCALEBY) + line.xstartposition);
      out.fixed(line.xscale);out.fixed(line.fontSize);out.u16(text[l].size());
      if(line.type==LineType::Sura)out.u8(surah++);
    }
  }
  return std::move(out.data);
}
}
