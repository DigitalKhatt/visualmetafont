#include "digitalkhatt/pdf/MushafPdfWriter.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <unordered_map>
#include <hb-ot.h>
#include <glaze/glaze.hpp>
#include "Layout/GlyphVis.h"
#include "automedina/automedina.h"
#include "MPFont.h"
#include "PDFWriter.h"
#include "PDFPage.h"
#include "PDFRectangle.h"
#include "PDFFormXObject.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFPageInput.h"
#include "PDFUsedFont.h"
#include "PageContentContext.h"
#include "XObjectContentContext.h"
#include "DictionaryContext.h"
#include "ObjectsContext.h"
#include "DocumentContextExtenderAdapter.h"
#include "PDFTextString.h"

namespace digitalkhatt::pdf {
namespace {
using PDFHummus::eSuccess;

std::string readFile(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>(in), {}};
}
std::filesystem::path labelFont() {
  for (const auto& p : {"/System/Library/Fonts/Supplemental/Arial.ttf", "/Library/Fonts/Arial.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "C:/Windows/Fonts/arial.ttf"})
    if (std::filesystem::is_regular_file(p)) return p;
  return {};
}
std::string utf16Bytes(TextView text) {
  std::string out{"\xfe\xff", 2};
  for (char16_t c : text) { out += char(c >> 8); out += char(c & 255); }
  return out;
}
std::string actualText(TextView text) {
  std::ostringstream out;
  out << "/Span << /ActualText <FEFF" << std::hex << std::setfill('0');
  for (char16_t c : text) out << std::setw(4) << unsigned(c);
  out << "> >> BDC\n";
  return out.str();
}
std::string edgePath(const mp_graphic_object* body) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(3);
  for (auto* object = body; object; object = object->next) {
    if (object->type != mp_fill_code && object->type != mp_stroked_code) continue;
    auto* first = reinterpret_cast<const mp_fill_object*>(object)->path_p;
    if (!first) continue;
    out << first->x_coord << ' ' << first->y_coord << " m\n";
    auto* p = first;
    do {
      auto* next = p->next;
      out << p->right_x << ' ' << p->right_y << ' ' << next->left_x << ' ' << next->left_y
          << ' ' << next->x_coord << ' ' << next->y_coord << " c\n";
      p = next;
    } while (p != first);
    if (first->data.types.left_type != mp_endpoint) out << "h\n";
  }
  return out.str();
}
std::string coloredEdges(const mp_graphic_object* body) {
  std::string foreground;
  std::ostringstream out;
  out << std::fixed << std::setprecision(6);
  for (auto* object = body; object; object = object->next) {
    if (object->type != mp_fill_code) continue;
    const auto* fill = reinterpret_cast<const mp_fill_object*>(object);
    // Generate this fill alone; colors are separated, while foreground paths
    // share a fill operation to preserve holes, as in the editor writer.
    auto one = *fill;
    one.next = nullptr;
    const auto path = edgePath(reinterpret_cast<const mp_graphic_object*>(&one));
    if (fill->color_model == mp_rgb_model)
      out << fill->color.a_val << ' ' << fill->color.b_val << ' ' << fill->color.c_val << " rg\n" << path << "f\n";
    else foreground += path;
  }
  out << "0 0 0 rg\n" << foreground << "f\n";
  return out.str();
}

class CatalogExtender : public DocumentContextExtenderAdapter {
 public:
  ObjectIDType outlines, labels;
  CatalogExtender(ObjectIDType a, ObjectIDType b) : outlines(a), labels(b) {}
  PDFHummus::EStatusCode OnCatalogWrite(CatalogInformation*, DictionaryContext* dictionary,
      ObjectsContext* objects, PDFHummus::DocumentContext*) override {
    dictionary->WriteKey("Outlines"); objects->WriteIndirectObjectReference(outlines);
    dictionary->WriteKey("PageMode"); objects->WriteName("UseOutlines");
    dictionary->WriteKey("PageLabels"); objects->WriteIndirectObjectReference(labels);
    return eSuccess;
  }
};

// Drawing callbacks for the editor's bundled surah icon font, without QRawFont.
class IconPath {
 public:
  hb_draw_funcs_t* funcs = hb_draw_funcs_create();
  struct Draw { std::ostringstream out; double x = 0, y = 0; };
  IconPath() {
    hb_draw_funcs_set_move_to_func(funcs, [](hb_draw_funcs_t*, void* d, hb_draw_state_t*, float x, float y, void*) {
      auto& v = *static_cast<Draw*>(d); v.out << x << ' ' << y << " m\n"; v.x=x; v.y=y;
    }, nullptr, nullptr);
    hb_draw_funcs_set_line_to_func(funcs, [](hb_draw_funcs_t*, void* d, hb_draw_state_t*, float x, float y, void*) {
      auto& v = *static_cast<Draw*>(d); v.out << x << ' ' << y << " l\n"; v.x=x; v.y=y;
    }, nullptr, nullptr);
    hb_draw_funcs_set_quadratic_to_func(funcs, [](hb_draw_funcs_t*, void* d, hb_draw_state_t*, float cx, float cy, float x, float y, void*) {
      auto& v = *static_cast<Draw*>(d);
      v.out << v.x+(cx-v.x)*2/3 << ' ' << v.y+(cy-v.y)*2/3 << ' ' << x+(cx-x)*2/3 << ' ' << y+(cy-y)*2/3 << ' ' << x << ' ' << y << " c\n";
      v.x=x; v.y=y;
    }, nullptr, nullptr);
    hb_draw_funcs_set_cubic_to_func(funcs, [](hb_draw_funcs_t*, void* d, hb_draw_state_t*, float x1, float y1, float x2, float y2, float x, float y, void*) {
      auto& v = *static_cast<Draw*>(d); v.out << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << ' ' << x << ' ' << y << " c\n"; v.x=x; v.y=y;
    }, nullptr, nullptr);
    hb_draw_funcs_set_close_path_func(funcs, [](hb_draw_funcs_t*, void* d, hb_draw_state_t*, void*) { static_cast<Draw*>(d)->out << "h\n"; }, nullptr, nullptr);
    hb_draw_funcs_make_immutable(funcs);
  }
  ~IconPath() { hb_draw_funcs_destroy(funcs); }
  std::string draw(hb_font_t* font, hb_codepoint_t glyph) {
    Draw d; d.out << std::fixed << std::setprecision(3);
    hb_font_draw_glyph(font, glyph, funcs, &d);
    return d.out.str() + "f\n";
  }
};
}  // namespace

struct MushafPdfWriter::Impl {
  OtLayout& layout;
  Options options;
  PDFWriter writer;
  bool started = false;
  double width, height, unit;
  ObjectIDType frame = 0;
  ObjectIDType searchFont = 0;
  struct Form { ObjectIDType id = 0; double width = 0; };
  std::unordered_map<const GlyphVis*, Form> forms;
  std::unordered_map<int, Form> icons;
  std::map<std::string, int> surahCodes;
  hb_blob_t* iconBlob = nullptr;
  hb_face_t* iconFace = nullptr;
  hb_font_t* iconFont = nullptr;
  IconPath iconDraw;
  struct Bookmark { TextString title; ObjectIDType page; double y; };
  std::vector<Bookmark> bookmarks;
  int firstMushafPage = 1;

  Impl(OtLayout& l, const Options& o) : layout(l), options(o),
      width(std::round(o.pageWidthMM * 72 / 25.4)), height(std::round(o.pageHeightMM * 72 / 25.4)),
      unit(72.0 / (4800 * (1 << OtLayout::SCALEBY))) {}
  ~Impl() {
    if (iconFont) hb_font_destroy(iconFont);
    if (iconFace) hb_face_destroy(iconFace);
    if (iconBlob) hb_blob_destroy(iconBlob);
  }
  static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }

  Form icon(int code) {
    if (auto found = icons.find(code); found != icons.end()) return found->second;
    hb_codepoint_t glyph = 0;
    require(hb_font_get_nominal_glyph(iconFont, code, &glyph), "Missing surah icon");
    hb_glyph_extents_t ext{};
    require(hb_font_get_glyph_extents(iconFont, glyph, &ext), "Missing surah icon extents");
    const double w = ext.width, h = -ext.height;
    auto* form = writer.StartFormXObject(PDFRectangle(0, -h/2, w, h/2));
    auto* ctx = form->GetContentContext();
    ctx->cm(1, 0, 0, 1, -ext.x_bearing, -ext.y_bearing + h/2);
    ctx->WriteFreeCode(iconDraw.draw(iconFont, glyph));
    const Form result{form->GetObjectID(), w};
    require(writer.EndFormXObjectAndRelease(form) == eSuccess, "Cannot finish surah icon form");
    icons[code] = result;
    return result;
  }
  void use(XObjectContentContext* ctx, PDFFormXObject* parent, Form form, double x, double y) {
    const auto name = parent->GetResourcesDictionary().AddFormXObjectMapping(form.id);
    ctx->WriteFreeCode("q\n"); ctx->cm(1,0,0,1,x,y);
    ctx->WriteFreeCode("/" + name + " Do\nQ\n");
  }
  Form glyph(GlyphVis& g) {
    if (auto found = forms.find(&g); found != forms.end()) return found->second;
    const bool ayah = g.charcode >= Automedina::AyaNumberCode && g.charcode <= Automedina::AyaNumberCode+286;
    Form endForm;
    std::vector<Form> digitForms;
    std::string digits;
    if (ayah) {
      endForm = glyph(layout.glyphs.at("endofaya"));
      digits = std::to_string(g.charcode - Automedina::AyaNumberCode + 1);
      for (char d : digits) digitForms.push_back(glyph(*layout.getGlyph(1632+d-'0')));
    }
    auto* form = writer.StartFormXObject(PDFRectangle(g.bbox.llx, g.bbox.lly, g.bbox.urx, g.bbox.ury));
    auto* ctx = form->GetContentContext();
    if (ayah) {
      auto& end = layout.glyphs.at("endofaya");
      use(ctx, form, endForm, 0, 0);
      double total = 40 * (digits.size()-1);
      for (char d : digits) total += layout.getGlyph(1632 + d-'0')->width;
      double x = static_cast<int>(end.width/2 - total/2);
      size_t digitIndex = 0;
      for (char d : digits) {
        auto* digit = layout.getGlyph(1632 + d-'0');
        use(ctx, form, digitForms[digitIndex++], x, 120);
        x += digit->width + 40;
      }
    } else if (g.name.starts_with("endofaya")) {
      auto* colored = g.getColoredGlyph();
      ctx->WriteFreeCode(colored ? coloredEdges(colored->mpPath()) : edgePath(g.mpPath()) + "f\n");
    } else ctx->WriteFreeCode(edgePath(g.mpPath()) + "f\n");
    const Form result{form->GetObjectID(), g.width};
    require(writer.EndFormXObjectAndRelease(form) == eSuccess, "Cannot finish glyph form");
    forms[&g] = result;
    return result;
  }
  void drawForm(PageContentContext* ctx, PDFPage* page, Form form,
      double sx, double sy, double x, double y) {
    const auto name = page->GetResourcesDictionary().AddFormXObjectMapping(form.id);
    ctx->WriteFreeCode("q\n"); ctx->cm(sx,0,0,sy,x,y);
    ctx->WriteFreeCode("/" + name + " Do\nQ\n");
  }
  void notice() {
    const auto fontFile = labelFont();
    require(!fontFile.empty(), "Cannot find a notice font; pass --no-notice or install Arial/DejaVuSans");
    auto* regular = writer.GetFontForFile(fontFile.string());
    auto boldFile = fontFile.parent_path() / "Arial Bold.ttf";
    auto* bold = writer.GetFontForFile(std::filesystem::is_regular_file(boldFile) ? boldFile.string() : fontFile.string());
    require(regular && bold, "Cannot load notice font");
    auto* page = new PDFPage(); page->SetMediaBox(PDFRectangle(0,0,width,height));
    auto* ctx = writer.StartPageContentContext(page);
    const auto title = bold->CalculateTextDimensions("Important Notice",16);
    double y = height-title.height-20;
    ctx->WriteText((width-title.width)/2,y,"Important Notice", AbstractContentContext::TextOptions(bold,16));
    y -= 10;
    const std::string url = "https://github.com/DigitalKhatt/oldmadinafont";
    PDFRectangle urlLinkBox;
    std::vector<std::string> lines = {
      "This mushaf is an experimental development version produced as part",
      "of the DigitalKhatt project and is not yet considered a final release.",
      "The text, typography, and layout are still undergoing review and refinement.",
      "Consequently, this edition may contain errors and may be updated periodically.",
      "For the latest version, updates, and corrections, or to report an issue,",
      "please consult the official project page:", url, ""};
    const auto now = std::time(nullptr); std::ostringstream date;
    date << "Generation Date: " << std::put_time(std::gmtime(&now), "%Y-%m-%d %H:%M:%S UTC"); lines.push_back(date.str());
    for (const auto& line : lines) {
      const auto bounds = regular->CalculateTextDimensions(line,6); y -= bounds.height+10;
      const double x = (width-bounds.width)/2;
      ctx->WriteText(x,y,line,AbstractContentContext::TextOptions(regular,6,AbstractContentContext::eRGB,line==url ? 0x0000ff : 0));
      if (line == url) urlLinkBox = PDFRectangle(x+bounds.xMin,y+bounds.yMin,x+bounds.xMax,y+bounds.yMax);
    }
    require(writer.EndPageContentContext(ctx) == eSuccess,"Cannot finish notice content");
    // Link annotations write indirect objects immediately. The compressed
    // page stream must be closed first, as in the GUI PDF writer.
    require(writer.AttachURLLinktoCurrentPage(url,urlLinkBox) == eSuccess,"Cannot write notice link");
    const auto result = writer.WritePageReleaseAndReturnPageID(page);
    require(result.first == eSuccess,"Cannot write notice page");
    bookmarks.push_back({u"Important Notice",result.second,height});
  }
  void start() {
    PDFCreationSettings settings(true,true);
    require(writer.StartPDF(options.output.string(), ePDFVersion17, LogConfiguration::DefaultLogConfiguration(), settings)==eSuccess,"Cannot start Mushaf PDF");
    started = true;
    auto& info = writer.GetDocumentContext().GetTrailerInformation().GetInfo();
    info.Title = PDFTextString(options.title); info.Author = PDFTextString("DigitalKhatt Project");
    info.Creator = PDFTextString("DigitalKhatt Generate Mushaf");
    // ActualText around paths alone is ignored by PDF text extractors. A
    // hidden standard-font glyph supplies a text object for each source line.
    auto& objects = writer.GetObjectsContext();
    searchFont = objects.GetInDirectObjectsRegistry().AllocateNewObjectID();
    objects.StartNewIndirectObject(searchFont);
    auto* fontDictionary = objects.StartDictionary();
    fontDictionary->WriteKey("Type"); objects.WriteName("Font");
    fontDictionary->WriteKey("Subtype"); objects.WriteName("Type1");
    fontDictionary->WriteKey("BaseFont"); objects.WriteName("Helvetica");
    objects.EndDictionary(fontDictionary); objects.EndIndirectObject();
    std::unique_ptr<PDFDocumentCopyingContext> copy(writer.CreatePDFCopyingContext((options.resources/"images/surahframe.pdf").string()));
    require(bool(copy),"Cannot open bundled surah frame PDF");
    const auto copied = copy->CreateFormXObjectFromPDFPage(0,EPDFPageBox::ePDFPageBoxMediaBox);
    require(copied.first==eSuccess,"Cannot import surah frame"); frame = copied.second;
    const auto iconsFile = options.resources/"fonts/icomoon.ttf";
    iconBlob = hb_blob_create_from_file(iconsFile.string().c_str()); iconFace = hb_face_create(iconBlob,0); iconFont = hb_font_create(iconFace);
    hb_ot_font_set_funcs(iconFont); hb_font_set_scale(iconFont,1024,1024);
    require(hb_face_get_glyph_count(iconFace)>0,"Cannot load surah icon font");
    require(!glz::read_json(surahCodes,readFile(options.resources/"fonts/surahCodes.json")),"Cannot parse surah icon mapping");
    if (options.notice) notice();
  }
  void append(const std::vector<LineLayoutInfo>& page, const std::vector<TextString>& text,
      int pageNumber, int surahBeforePage) {
    require(started,"Mushaf writer is not open");
    if (firstMushafPage==1 && pageNumber>1 && bookmarks.size()<=1) firstMushafPage=pageNumber;
    // Materialize form streams before opening the page stream.
    for (const auto& line : page) for (const auto& g : line.glyphs) glyph(*layout.getGlyph(g));
    int surah = surahBeforePage;
    for (const auto& line : page) if (line.type==LineType::Sura) { icon(surahCodes.at(std::to_string(++surah))); icon(0xe903); }
    auto* pdfPage = new PDFPage(); pdfPage->SetMediaBox(PDFRectangle(0,0,width,height));
    auto* ctx = writer.StartPageContentContext(pdfPage);
    const auto searchName = pdfPage->GetResourcesDictionary().AddFontMapping(searchFont);
    const auto searchableLine = [&](TextView source, double x, double y, double span) {
      std::ostringstream code;
      code << "q\n" << actualText(source) << "BT /" << searchName
           << " 10 Tf 3 Tr " << std::max(1.0, span / 2.78)
           << " 0 0 1 " << x << ' ' << y << " Tm (.) Tj ET\nEMC\nQ\n";
      ctx->WriteFreeCode(code.str());
    };
    std::vector<std::pair<TextString,double>> newBookmarks;
    surah = surahBeforePage;
    double sajdaX=0, sajdaY=0; bool sajdaBegun=false;
    for (size_t l=0; l<page.size(); ++l) {
      const auto& line=page[l];
      if (line.type==LineType::Sura) {
        const double frameY = height-(line.ystartposition-(1100<<OtLayout::SCALEBY))*unit;
        drawForm(ctx,pdfPage,{frame,256},width/256,-width/256,0,frameY);
        const auto name = icons.at(surahCodes.at(std::to_string(++surah))), word = icons.at(0xe903);
        const double s=1.1;
        const double x=static_cast<int>((17000-(static_cast<int>(name.width*s)+100+static_cast<int>(word.width*s)))/2);
        const double y=height-(line.ystartposition-300)*unit;
        drawForm(ctx,pdfPage,name,unit*s,unit*s,x*unit,y);
        drawForm(ctx,pdfPage,word,unit*s,unit*s,(x+static_cast<int>(name.width*s)+100)*unit,y);
        searchableLine(text.at(l), x*unit, y, (name.width+word.width)*s*unit);
        const auto number = std::to_string(surah);
        newBookmarks.emplace_back(text.at(l)+u" ( "+TextString(number.begin(),number.end())+u" )",frameY);
        continue;
      }
      const double lineScale = line.type==LineType::Line ? line.xscale : 1.0;
      double x=options.textWidth+(options.margin<<OtLayout::SCALEBY)-line.xstartposition;
      const double baseline=height-line.ystartposition*unit;
      searchableLine(text.at(l),
          (x-line.currentLineWidth*lineScale)*unit, baseline,
          line.currentLineWidth*lineScale*unit);
      double priorX=x;
      for (const auto& positioned:line.glyphs) {
        x -= positioned.x_advance;
        auto* g=layout.getGlyph(positioned);
        const double placedX=x+positioned.x_offset;
        if (positioned.beginsajda) { sajdaX=priorX; sajdaY=baseline; sajdaBegun=true; }
        if (positioned.endsajda && sajdaBegun) {
          const double begin=sajdaY==baseline ? sajdaX : options.textWidth+(options.margin<<OtLayout::SCALEBY)-line.xstartposition;
          std::ostringstream commands; commands << "q\n0 0 0 RG\n" << (50<<OtLayout::SCALEBY)*unit << " w\n"
              << begin*unit << ' ' << baseline+(1100<<OtLayout::SCALEBY)*unit << " m\n"
              << placedX*unit << ' ' << baseline+(1100<<OtLayout::SCALEBY)*unit << " l\nS\nQ\n";
          ctx->WriteFreeCode(commands.str()); sajdaBegun=false;
        }
        priorX=placedX;
        std::ostringstream color;
        if (positioned.color) {
          const unsigned c=positioned.color;
          color << ((c>>24)&255)/255.0 << ' ' << ((c>>16)&255)/255.0 << ' ' << ((c>>8)&255)/255.0 << " rg\n";
        } else color << "0 0 0 rg\n";
        ctx->WriteFreeCode(color.str());
        const double right=options.textWidth+(options.margin<<OtLayout::SCALEBY)-line.xstartposition;
        drawForm(ctx,pdfPage,forms.at(g),unit*line.fontSize*lineScale,unit*line.fontSize,
            (right+(placedX-right)*lineScale)*unit,baseline+positioned.y_offset*unit);
      }
    }
    writer.EndPageContentContext(ctx);
    const auto result=writer.WritePageReleaseAndReturnPageID(pdfPage);
    require(result.first==eSuccess,"Cannot write Mushaf page");
    for (auto& [title,y]:newBookmarks) bookmarks.push_back({std::move(title),result.second,y});
  }
  void finish() {
    auto& objects=writer.GetObjectsContext();
    auto allocate=[&] { return objects.GetInDirectObjectsRegistry().AllocateNewObjectID(); };
    const auto root=allocate(), labels=allocate();
    std::vector<ObjectIDType> ids; for (size_t i=0;i<bookmarks.size();++i) ids.push_back(allocate());
    objects.StartNewIndirectObject(root); auto* d=objects.StartDictionary();
    d->WriteKey("Type"); objects.WriteName("Outlines");
    if (!ids.empty()) {
      d->WriteKey("First"); objects.WriteIndirectObjectReference(ids.front()); d->WriteKey("Last"); objects.WriteIndirectObjectReference(ids.back());
      d->WriteKey("Count"); objects.WriteInteger(ids.size());
    }
    objects.EndDictionary(d); objects.EndIndirectObject();
    for (size_t i=0;i<ids.size();++i) {
      objects.StartNewIndirectObject(ids[i]); d=objects.StartDictionary();
      d->WriteKey("Title"); d->WriteHexStringValue(utf16Bytes(bookmarks[i].title));
      d->WriteKey("Parent"); objects.WriteIndirectObjectReference(root);
      if (i) { d->WriteKey("Prev"); objects.WriteIndirectObjectReference(ids[i-1]); }
      if (i+1<ids.size()) { d->WriteKey("Next"); objects.WriteIndirectObjectReference(ids[i+1]); }
      d->WriteKey("Dest"); objects.StartArray(); objects.WriteIndirectObjectReference(bookmarks[i].page);
      objects.WriteName("XYZ"); objects.WriteInteger(0); objects.WriteDouble(bookmarks[i].y); objects.WriteInteger(0); objects.EndArray();
      objects.EndDictionary(d); objects.EndIndirectObject();
    }
    objects.StartNewIndirectObject(labels); d=objects.StartDictionary(); d->WriteKey("Nums"); objects.StartArray();
    const auto label=[&](int index,const char* style,int start) {
      objects.WriteInteger(index); auto* v=objects.StartDictionary(); v->WriteKey("S"); objects.WriteName(style); v->WriteKey("St"); objects.WriteInteger(start); objects.EndDictionary(v);
    };
    if (options.notice) label(0,"r",1);
    label(options.notice?1:0,"D",firstMushafPage);
    objects.EndArray(); objects.EndDictionary(d); objects.EndIndirectObject();
    writer.GetDocumentContext().AddDocumentContextExtender(new CatalogExtender(root,labels));
    require(writer.EndPDF()==eSuccess,"Cannot finalize Mushaf PDF"); started=false;
  }
};

MushafPdfWriter::MushafPdfWriter(OtLayout& layout,const Options& options) : impl_(std::make_unique<Impl>(layout,options)) {}
MushafPdfWriter::~MushafPdfWriter() = default;
void MushafPdfWriter::start() { impl_->start(); }
void MushafPdfWriter::appendPage(const std::vector<LineLayoutInfo>& page,const std::vector<TextString>& text,int number,int surahBefore) { impl_->append(page,text,number,surahBefore); }
void MushafPdfWriter::finish() { impl_->finish(); }
}  // namespace digitalkhatt::pdf
