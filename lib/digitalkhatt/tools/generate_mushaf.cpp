#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <glaze/glaze.hpp>
#include <sqlite3.h>
#include "MPFont.h"
#include "Layout/GlyphVis.h"
#include "Layout/MushafLayout.h"
#include "Layout/MushafRunOptions.h"
#include "Layout/PlacementPipeline.h"
#include "digitalkhatt/layout/ViolationReportContext.h"
#include "digitalkhatt/pdf/MushafPdfWriter.h"
#include "digitalkhatt/pdf/ViolationReportWriter.h"

namespace fs = std::filesystem;
using namespace digitalkhatt;
using namespace digitalkhatt::layout;
using Report = digitalkhatt::pdf::ViolationReportWriter;

namespace mushaf_tool {
std::string readFile(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>(in), {}};
}
template<class T> void readJson(T& value, const fs::path& path) {
  auto data = readFile(path);
  // Accept older GUI snapshots without keeping the removed solver option.
  // Other unknown fields still fail the normal strict configuration reader.
  if constexpr (std::is_same_v<T, MushafRunOptions> || std::is_same_v<T, OptParams>) {
    if (data.find("preserveMarkSemantics") != std::string::npos ||
        data.find("waqfBaseVicinity") != std::string::npos ||
        data.find("waqfInterLineGap") != std::string::npos ||
        data.find("waqfStackGap") != std::string::npos ||
        data.find("waqfHeightPreferenceBand") != std::string::npos) {
      glz::generic_i64 document;
      if (const auto error = glz::read_json(document, data))
        throw std::runtime_error("Invalid JSON in " + path.string() + ": " + glz::format_error(error, data));
      auto child = [](glz::generic_i64* object, const char* key) -> glz::generic_i64* {
        if (!object || !object->is_object()) return nullptr;
        auto& fields = object->get_object();
        const auto it = fields.find(key);
        return it == fields.end() ? nullptr : &it->second;
      };
      auto* xpbd = &document;
      if constexpr (std::is_same_v<T, MushafRunOptions>) xpbd = child(xpbd, "xpbd");
      auto* toggles = child(xpbd, "toggles");
      bool changed = false;
      if (toggles && toggles->is_object()) {
        changed |= toggles->get_object().erase("preserveMarkSemantics") != 0;
        changed |= toggles->get_object().erase("waqfBaseVicinity") != 0;
      }
      if (xpbd && xpbd->is_object()) {
        for (const auto* key : {"waqfBaseVicinityMarginFactor", "waqfInterLineGap", "waqfStackGap", "waqfHeightPreferenceBand"})
          changed |= xpbd->get_object().erase(key) != 0;
      }
      auto* compliance = child(xpbd, "compliance");
      if (compliance && compliance->is_object()) {
        changed |= compliance->get_object().erase("waqfBaseVicinityLeft") != 0;
        changed |= compliance->get_object().erase("waqfBaseVicinityRight") != 0;
      }
      if (changed) {
        data.clear();
        if (glz::write_json(document, data)) throw std::runtime_error("Cannot normalize " + path.string());
      }
    }
  }
  if (const auto error = glz::read_json(value, data))
    throw std::runtime_error("Invalid JSON in " + path.string() + ": " + glz::format_error(error, data));
  if constexpr (std::is_same_v<T, MushafRunOptions>) {
    // Older snapshots limited only the summary. Carry that setting into the
    // shared report selection unless the new field is explicitly present.
    if (data.find("\"summaryLimit\"") != std::string::npos && data.find("\"reportMaxFindings\"") == std::string::npos)
      value.xpbd.reportMaxFindings = value.summaryLimit;
  }
}
template<class T> void writeJson(const T& value, const fs::path& path) {
  std::string text;
  if (glz::write<glz::opts{.prettify=true}>(value, text)) throw std::runtime_error("Cannot serialize run data");
  std::ofstream out(path); out << text << '\n';
  if (!out) throw std::runtime_error("Cannot write " + path.string());
}
TextString utf16(std::string_view text) {
  auto* buffer = hb_buffer_create();
  hb_buffer_add_utf8(buffer, text.data(), static_cast<int>(text.size()), 0, text.size());
  unsigned count = 0; auto* info = hb_buffer_get_glyph_infos(buffer, &count);
  TextString result;
  for (unsigned i = 0; i < count; ++i) {
    auto cp = info[i].codepoint;
    if (cp <= 0xffff) result.push_back(static_cast<char16_t>(cp));
    else { cp -= 0x10000; result.push_back(0xd800+(cp>>10)); result.push_back(0xdc00+(cp&1023)); }
  }
  hb_buffer_destroy(buffer); return result;
}
std::string utf8(TextView text) {
  std::string result;
  for (size_t i = 0; i < text.size(); ++i) {
    uint32_t c = text[i];
    if (c >= 0xd800 && c <= 0xdbff && i+1<text.size()) c = 0x10000+((c-0xd800)<<10)+(text[++i]-0xdc00);
    if (c<128) result += char(c);
    else if (c<2048) { result += char(0xc0|(c>>6)); result += char(0x80|(c&63)); }
    else if (c<65536) { result += char(0xe0|(c>>12)); result += char(0x80|((c>>6)&63)); result += char(0x80|(c&63)); }
    else { result += char(0xf0|(c>>18)); result += char(0x80|((c>>12)&63)); result += char(0x80|((c>>6)&63)); result += char(0x80|(c&63)); }
  }
  return result;
}
std::string checksum(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Cannot checksum " + path.string());
  uint64_t hash=14695981039346656037ULL; char buffer[65536];
  while (in.read(buffer,sizeof(buffer)) || in.gcount())
    for (std::streamsize i=0;i<in.gcount();++i) { hash ^= static_cast<unsigned char>(buffer[i]); hash *= 1099511628211ULL; }
  std::ostringstream out; out << "fnv1a64:" << std::hex << std::setfill('0') << std::setw(16) << hash;
  return out.str();
}
void initializeFont(MPFont& font, const fs::path& project, const fs::path& resources) {
  font.initialize("MPGUI:=1;" + readFile(resources/"mfplain.mp") + readFile(resources/"mpost.mp") +
      readFile(resources/"vmf.mp") + readFile(project), project);
  const auto glyphs=readFile(project.parent_path()/"glyphs.mp");
  size_t pos=0, count=0;
  while (pos<glyphs.size()) {
    const auto begin=glyphs.find("beginchar",pos), def=glyphs.find("defchar",pos);
    const bool isDef=def!=std::string::npos && (begin==std::string::npos || def<begin);
    const auto start=isDef?def:begin; if (start==std::string::npos) break;
    const std::string endMarker=isDef?"enddefchar;":"endchar;";
    const auto end=glyphs.find(endMarker,start);
    if (end==std::string::npos) throw std::runtime_error("Unterminated glyph definition");
    const auto block=glyphs.substr(start,end+endMarker.size()-start);
    const auto paren=block.find('('), comma1=block.find(',',paren), comma2=block.find(',',comma1+1);
    if (paren==std::string::npos || comma1==std::string::npos || comma2==std::string::npos) throw std::runtime_error("Invalid glyph header");
    font.registerGlyphSource(block.substr(paren+1,comma1-paren-1),block,isDef?"defchar":"beginchar",std::stoi(block.substr(comma1+1,comma2-comma1-1)));
    font.execute("params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;"+block);
    ++count; pos=end+endMarker.size();
  }
  if (!count) throw std::runtime_error("No glyph sources registered");
  std::cout << "Registered " << count << " glyph sources\n";
}
std::string textColumn(const std::string& layout) {
  return layout.starts_with("indopak") ? "dk_indopak" : layout=="qpc_v1_layout" ? "dk_v1" : "dk_v2";
}
std::vector<TextString> loadCorpus(const MushafRunOptions& options) {
  // Restrict identifiers before interpolating them; SQLite values use binds.
  for (char c:options.layout) if (!std::isalnum(static_cast<unsigned char>(c)) && c!='_') throw std::runtime_error("Invalid layout table name");
  sqlite3* raw=nullptr;
  if (sqlite3_open_v2(options.database.c_str(),&raw,SQLITE_OPEN_READONLY,nullptr)!=SQLITE_OK) {
    if (raw) sqlite3_close(raw);
    throw std::runtime_error("Cannot open Quran database");
  }
  std::unique_ptr<sqlite3,decltype(&sqlite3_close)> db(raw,&sqlite3_close);
  const auto column=textColumn(options.layout);
  const std::string query="SELECT l.page,l.line,l.type,w."+column+" FROM \""+options.layout+"\" l LEFT JOIN words w ON l.type='ayah' AND l.range_start<=w.word_number_all AND l.range_end>=w.word_number_all ORDER BY l.page,l.line,w.word_number_all";
  sqlite3_stmt* stmt=nullptr;
  if (sqlite3_prepare_v2(db.get(),query.c_str(),-1,&stmt,nullptr)!=SQLITE_OK) throw std::runtime_error(sqlite3_errmsg(db.get()));
  std::unique_ptr<sqlite3_stmt,decltype(&sqlite3_finalize)> statement(stmt,&sqlite3_finalize);
  std::vector<MushafWordRow> rows; int step;
  const auto text=[&](int i) { const auto* p=sqlite3_column_text(stmt,i); return p?std::string(reinterpret_cast<const char*>(p)):std::string{}; };
  while ((step=sqlite3_step(stmt))==SQLITE_ROW) rows.push_back({sqlite3_column_int(stmt,0),sqlite3_column_int(stmt,1),text(2),utf16(text(3))});
  if (step!=SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(db.get()));
  if (rows.empty() || rows.front().page!=1) throw std::runtime_error("Corpus must start at page 1");
  auto pages=assembleMushafText(rows,column);
  if (static_cast<int>(pages.size())!=rows.back().page) throw std::runtime_error("Corpus contains missing page numbers");
  return pages;
}

struct Statistics {
  uint64_t pages=0, lines=0, glyphs=0, marks=0, findings=0, hard=0, soft=0, introduced=0, worsened=0;
  uint64_t surahHeaders=0, basmalaLines=0, sajdaStarts=0, sajdaEnds=0;
  uint64_t reportedFindings=0, eligibleFindings=0;
  std::map<std::string,uint64_t> byType, initialByType, introducedByType, worsenedByType;
};
struct RunManifest {
  bool complete=false;
  MushafRunOptions options;
  std::map<std::string,std::string> sources;
  Statistics statistics;
  double seconds=0;
};
Report::GlyphRef glyphRef(const GlyphInstance& g,const std::vector<TextString>& text) {
  Report::GlyphRef result{&g.worldPolys,g.glyphName,g.globalIndex,g.isMark&&g.prevBase?g.prevBase->globalIndex:-1,g.lineIndex+1,g.dx,g.dy};
  if (g.glyphLayout && g.lineIndex>=0 && g.lineIndex<static_cast<int>(text.size())) {
    result.cluster=g.glyphLayout->cluster;
    const auto& line=text[g.lineIndex];
    result.wordNumber=violationWordNumber(line,result.cluster);
    if (result.cluster>=0 && result.cluster<static_cast<int>(line.size())) {
      size_t begin=result.cluster, end=result.cluster;
      while (begin && line[begin-1]!=u' ') --begin;
      while (end<line.size() && line[end]!=u' ') ++end;
      result.wordText=utf8(TextView(line).substr(begin,end-begin));
    }
  }
  return result;
}
void usage() {
  std::cout << "Usage: digitalkhatt_generate_mushaf [options] FONT.mp\n"
      "Generate the same corpus layout as the GUI Generate Mushaf action.\n"
      "The Mushaf PDF is generated by default; --report adds placement reports.\n\n"
      "  --font PATH                Font project (or positional FONT.mp)\n"
      "  --config PATH              Portable GUI/run settings JSON\n"
      "  -o, --output PATH          Mushaf PDF (default mushaf.pdf)\n"
      "  --layout NAME              Database layout; aliases qpc, v1, v2, v4\n"
      "  --justifier NAME           none, harfbuzz, madina, indopak, experimental, experimental2, decl-policy\n"
      "  --style NAME               none, same-size-by-page, xscale, font-size, font-size-xscale\n"
      "  --shrink NAME              none, standard, test\n"
      "  --stretch-policy NAME      Declarative stretch policy override\n"
      "  --shrink-policy NAME       Declarative shrink policy override\n"
      "  --force / --no-force       Enable/disable XPBD (default matches GUI: off)\n"
      "  --report / --no-report     Generate PDFs, CSV and offline web viewer\n"
      "  --xpbd-config PATH         Partial/full OptParams JSON\n"
      "  --soft-targets             Include optional soft residuals\n"
      "  --report-waqf-bounds / --no-report-waqf-bounds  Solver height-bound residuals (default off)\n"
      "  --waqf-left-drift-tolerance N   Allowed left drift, % of waqf width (default 100)\n"
      "  --waqf-right-drift-tolerance N  Allowed right drift, % of waqf width (default 50)\n"
      "  --waqf-previous-line-margin N  Margin below previous baseline, % of spacing (default 20)\n"
      "  --waqf-x-alignment-band N   Inactive horizontal band, % of waqf width each side (default 25)\n"
      "  --report-generic-gap / --no-report-generic-gap  Include/exclude gap findings (default off)\n"
      "  --placement-audit / --no-placement-audit  Include/exclude final side/class/owner audit (default off)\n"
      "  --min-severity N           Minimum residual beyond allowed slack (default 1)\n"
      "  --base-vicinity-mark-tolerance N  Reporting slack in percent of mark width (default 5)\n"
      "  --base-vicinity-dot-tolerance N   Reporting slack in percent of dot width (default 0)\n"
      "  --report-limit N           Highest-ranked findings across all report outputs (default 1000; 0 unlimited)\n"
      "  --report-sort MODE         severity (default) or priority; critical structural findings first\n"
      "  --only-changed / --all-findings  New/worsened plus structural, or all eligible findings\n"
      "  --line-spacing N           Baseline distance in font units\n"
      "  --text-width N             Text width in font units\n"
      "  --em-scale N               GUI font size percentage / 100\n"
      "  --tajweed / --no-tajweed   Tajweed coloring\n"
      "  --disable-lookup NAME      Disable a lookup; repeatable\n"
      "  --pages A[-B]              Inclusive range; default all corpus pages\n"
      "  --summary-limit N          Legacy alias for --report-limit\n"
      "  --database PATH           Quran SQLite database\n"
      "  --features PATH           Font feature file (default features.fea)\n"
      "  --resources DIR           MetaPost resource directory\n"
      "  --pdf-resources DIR       Bundled fonts and images for PDF output\n"
      "  --no-notice               Omit the GUI notice page\n"
      "  --no-pdf                  Skip the Mushaf PDF for parameter experiments\n"
      "  --write-config PATH       Also write resolved settings to PATH\n"
      "  --help                    Show help\n";
}
}  // namespace

using namespace mushaf_tool;
int main(int argc,char** argv) {
  std::cout << std::unitbuf;
  try {
    MushafRunOptions options;
    options.database=DIGITALKHATT_QURAN_DATABASE;
    options.resources=DIGITALKHATT_METAFONT_RESOURCES;
    options.pdfResources=DIGITALKHATT_PDF_RESOURCES;
    fs::path output="mushaf.pdf", extraConfig;
    // Config is the base; command-line options override it regardless of order.
    for (int i=1;i<argc;++i) if (std::string_view(argv[i])=="--config") {
      if (++i>=argc) throw std::runtime_error("Missing --config value"); readJson(options,argv[i]);
    }
    for (int i=1;i<argc;++i) {
      const std::string arg=argv[i];
      const auto value=[&]() -> std::string { if (++i>=argc) throw std::runtime_error("Missing value for "+arg); return argv[i]; };
      if (arg=="--help" || arg=="-h") { usage(); return 0; }
      else if (arg=="--config") value();
      else if (arg=="--font") options.font=value();
      else if (arg=="--output" || arg=="-o") output=value();
      else if (arg=="--layout") options.layout=value();
      else if (arg=="--justifier") options.justifier=value();
      else if (arg=="--style") options.style=value();
      else if (arg=="--shrink") options.shrink=value();
      else if (arg=="--stretch-policy") options.stretchPolicy=value();
      else if (arg=="--shrink-policy") options.shrinkPolicy=value();
      else if (arg=="--line-spacing") options.lineSpacing=std::stoi(value());
      else if (arg=="--text-width") options.textWidth=std::stoi(value());
      else if (arg=="--em-scale") options.emScale=std::stod(value());
      else if (arg=="--summary-limit" || arg=="--report-limit") options.xpbd.reportMaxFindings=std::stoi(value());
      else if (arg=="--report-sort") options.xpbd.reportSort=value();
      else if (arg=="--min-severity") options.xpbd.minViolationSeverity=std::stod(value());
      else if (arg=="--base-vicinity-mark-tolerance") options.xpbd.baseVicinityMarkTolerancePercent=std::stod(value());
      else if (arg=="--base-vicinity-dot-tolerance") options.xpbd.baseVicinityDotTolerancePercent=std::stod(value());
      else if (arg=="--waqf-left-drift-tolerance") options.xpbd.waqfLeftDriftTolerancePercent=std::stod(value());
      else if (arg=="--waqf-right-drift-tolerance") options.xpbd.waqfRightDriftTolerancePercent=std::stod(value());
      else if (arg=="--waqf-previous-line-margin") options.xpbd.waqfPreviousLineMarginPercent=std::stod(value());
      else if (arg=="--waqf-x-alignment-band") options.xpbd.waqfHorizontalAlignmentBandPercent=std::stod(value());
      else if (arg=="--report-waqf-bounds") options.xpbd.toggles.reportWaqfBounds=true;
      else if (arg=="--no-report-waqf-bounds") options.xpbd.toggles.reportWaqfBounds=false;
      else if (arg=="--database") options.database=value();
      else if (arg=="--features") options.features=value();
      else if (arg=="--resources") options.resources=value();
      else if (arg=="--pdf-resources") options.pdfResources=value();
      else if (arg=="--disable-lookup") options.disabledLookups.push_back(value());
      else if (arg=="--xpbd-config") readJson(options.xpbd,value());
      else if (arg=="--write-config") extraConfig=value();
      else if (arg=="--force") options.force=true;
      else if (arg=="--no-force") options.force=false;
      else if (arg=="--report") options.report=true;
      else if (arg=="--no-report") options.report=false;
      else if (arg=="--tajweed") options.tajweed=true;
      else if (arg=="--no-tajweed") options.tajweed=false;
      else if (arg=="--soft-targets") options.xpbd.toggles.reportSoftResiduals=true;
      else if (arg=="--report-generic-gap") options.xpbd.toggles.reportGenericGap=true;
      else if (arg=="--no-report-generic-gap") options.xpbd.toggles.reportGenericGap=false;
      else if (arg=="--placement-audit") options.xpbd.toggles.reportPlacementAudit=true;
      else if (arg=="--no-placement-audit") options.xpbd.toggles.reportPlacementAudit=false;
      else if (arg=="--only-changed") options.xpbd.reportOnlyChanged=true;
      else if (arg=="--all-findings") options.xpbd.reportOnlyChanged=false;
      else if (arg=="--no-notice") options.notice=false;
      else if (arg=="--no-pdf") options.pdf=false;
      else if (arg=="--pages") {
        const auto range=value(); const auto dash=range.find('-');
        options.firstPage=std::stoi(range.substr(0,dash));
        options.lastPage=dash==std::string::npos?options.firstPage:std::stoi(range.substr(dash+1));
      } else if (!arg.empty() && arg[0]=='-') throw std::runtime_error("Unknown option: "+arg);
      else if (options.font.empty()) options.font=arg;
      else throw std::runtime_error("Only one font project can be provided");
    }
    if (options.font.empty()) { usage(); return 2; }
    if (options.layout=="qpc" || options.layout=="v2" || options.layout=="v2_layout") options.layout="qpc_v2_layout";
    if (options.layout=="v1" || options.layout=="v1_layout") options.layout="qpc_v1_layout";
    if (options.layout=="v4" || options.layout=="v4_layout") options.layout="qpc_v4_layout";
    if (options.xpbd.reportMaxFindings<0 || options.xpbd.maxIters<0 || options.xpbd.maxIters>10000 ||
        !std::isfinite(options.xpbd.minViolationSeverity) || options.xpbd.minViolationSeverity<0 ||
        !std::isfinite(options.xpbd.baseVicinityMarkTolerancePercent) || options.xpbd.baseVicinityMarkTolerancePercent<0 || options.xpbd.baseVicinityMarkTolerancePercent>100 ||
        !std::isfinite(options.xpbd.baseVicinityDotTolerancePercent) || options.xpbd.baseVicinityDotTolerancePercent<0 || options.xpbd.baseVicinityDotTolerancePercent>100 ||
        !std::isfinite(options.xpbd.waqfLeftDriftTolerancePercent) || options.xpbd.waqfLeftDriftTolerancePercent<0 || options.xpbd.waqfLeftDriftTolerancePercent>1000 ||
        !std::isfinite(options.xpbd.waqfRightDriftTolerancePercent) || options.xpbd.waqfRightDriftTolerancePercent<0 || options.xpbd.waqfRightDriftTolerancePercent>1000 ||
        !std::isfinite(options.xpbd.waqfPreviousLineMarginPercent) || options.xpbd.waqfPreviousLineMarginPercent<0 || options.xpbd.waqfPreviousLineMarginPercent>100 ||
        !std::isfinite(options.xpbd.waqfHorizontalAlignmentBandPercent) || options.xpbd.waqfHorizontalAlignmentBandPercent<0 || options.xpbd.waqfHorizontalAlignmentBandPercent>1000 ||
        !std::isfinite(options.emScale) || options.emScale<=0 || options.emScale>5 ||
        options.textWidth<1 || options.textWidth>100000) throw std::runtime_error("Invalid numeric option");
    if (options.xpbd.reportSort != "severity" && options.xpbd.reportSort != "priority")
      throw std::runtime_error("--report-sort must be severity or priority");
    options.summaryLimit=options.xpbd.reportMaxFindings;
    options.font=fs::absolute(options.font).string();
    for (auto* path:{&options.database,&options.resources,&options.pdfResources}) if (!path->empty()) *path=fs::absolute(*path).string();
    if (options.database.empty()) options.database=DIGITALKHATT_QURAN_DATABASE;
    if (options.resources.empty()) options.resources=DIGITALKHATT_METAFONT_RESOURCES;
    if (options.pdfResources.empty()) options.pdfResources=DIGITALKHATT_PDF_RESOURCES;
    const fs::path project=options.font;
    if (fs::path(options.features).is_relative()) options.features=(project.parent_path()/options.features).string();
    options.xpbd.toggles.reportViolations=options.report;
    const auto pages=loadCorpus(options);
    if (!options.lastPage) options.lastPage=static_cast<int>(pages.size());
    if (options.firstPage<1 || options.lastPage<options.firstPage || options.lastPage>static_cast<int>(pages.size())) throw std::runtime_error("Invalid page range");
    output=fs::absolute(output); fs::create_directories(output.parent_path());
    const auto sidecar=[&](std::string_view suffix) { return output.parent_path()/(output.stem().string()+std::string(suffix)); };
    writeJson(options,sidecar(".settings.json")); if (!extraConfig.empty()) writeJson(options,extraConfig);
    RunManifest manifest; manifest.options=options;
    for (const auto& path:{project,project.parent_path()/"glyphs.mp",fs::path(options.features),project.parent_path()/"parameters.json",fs::path(options.database)})
      if (fs::is_regular_file(path)) manifest.sources[path.string()]=checksum(path);
    for (const auto& entry : fs::directory_iterator(project.parent_path())) {
      const auto extension = entry.path().extension().string();
      if (entry.is_regular_file() && (extension==".mp" || extension==".fea" || extension==".json" ||
          extension==".dylib" || extension==".so" || extension==".dll"))
        manifest.sources[entry.path().string()]=checksum(entry.path());
    }
    for (const auto& path : {fs::absolute(argv[0]), fs::path(DIGITALKHATT_LAYOUT_LIBRARY),
        fs::path(options.pdfResources)/"images/surahframe.pdf", fs::path(options.pdfResources)/"fonts/icomoon.ttf",
        fs::path(options.pdfResources)/"fonts/surahCodes.json"})
      if (fs::is_regular_file(path)) manifest.sources[path.string()]=checksum(path);
    writeJson(manifest,sidecar(".run.json"));
    const auto started=std::chrono::steady_clock::now();
    MPFont font; initializeFont(font,project,options.resources);
    OtLayout layout(&font,true,true); layout.useNormAxisValues=false;
    layout.setInterLineSpacing(options.lineSpacing);
    for (const auto& name:options.disabledLookups) layout.setLookupDisabled(name,true);
    layout.loadLookupFile(options.features);
    const auto engine=parseJustifier(options.justifier);
    layout.applyJustification=engine!=JustType::None;
    JustOption just{engine,parseStyle(options.style),parseShrink(options.shrink)};
    const auto policy=[&](const auto& list,const std::string& name) {
      for (size_t i=0;i<list.size();++i) if (list[i].name==name) return static_cast<int>(i);
      throw std::runtime_error("Unknown declarative policy: "+name);
    };
    if ((!options.stretchPolicy.empty() || !options.shrinkPolicy.empty()) && !layout.compiledJustificationCatalog)
      throw std::runtime_error("The font has no compiled justification catalog");
    if (!options.stretchPolicy.empty()) just.justStretchPolicy=policy(layout.compiledJustificationCatalog->stretchPolicies,options.stretchPolicy);
    if (!options.shrinkPolicy.empty()) just.justShrinkPolicy=policy(layout.compiledJustificationCatalog->shrinkPolicies,options.shrinkPolicy);
    OtLayout::EMSCALE=options.emScale;
    const double scale=(1<<OtLayout::SCALEBY)*options.emScale;
    const int width=options.textWidth<<OtLayout::SCALEBY;
    PlacementPipeline placement(layout,scale);
    std::unique_ptr<digitalkhatt::pdf::MushafPdfWriter> mushaf;
    if (options.pdf) {
      digitalkhatt::pdf::MushafPdfWriter::Options pdfOptions;
      pdfOptions.output=output; pdfOptions.resources=options.pdfResources; pdfOptions.notice=options.notice;
      pdfOptions.pageWidthMM=options.pageWidthMM; pdfOptions.pageHeightMM=options.pageHeightMM; pdfOptions.textWidth=width;
      mushaf=std::make_unique<digitalkhatt::pdf::MushafPdfWriter>(layout,pdfOptions); mushaf->start();
    }
    Report report;
    if (options.report && !report.start(sidecar("_violations.pdf"),sidecar("_violations.csv"),options.xpbd)) throw std::runtime_error("Cannot start violation report");
    const std::vector<std::string> notes={
        "Configuration: "+options.layout+"; "+options.justifier+"; "+options.style+"; spacing "+std::to_string(options.lineSpacing)+"; Force "+(options.force?"on":"off"),
        "Blue dashed: shaped position. Red: hard finding. Amber: review. NEW/WORSE compares the same shaped page.",
        "Placement audit (side, class, owner): "+std::string(options.xpbd.toggles.reportPlacementAudit?"on":"off"),
        "Generic gaps: "+std::string(options.xpbd.toggles.reportGenericGap?"on":"off")+"; soft targets: "+(options.xpbd.toggles.reportSoftResiduals?"on":"off")+"; minimum excess severity "+std::to_string(options.xpbd.minViolationSeverity),
        "Sort: "+options.xpbd.reportSort+"; maximum findings: "+std::to_string(options.xpbd.reportMaxFindings)+" (0 unlimited); only changed (plus structural): "+(options.xpbd.reportOnlyChanged?"on":"off"),
        "BaseVicinity reporting tolerance (% of mark width): marks "+std::to_string(options.xpbd.baseVicinityMarkTolerancePercent)+"; dots "+std::to_string(options.xpbd.baseVicinityDotTolerancePercent),
        "Waqf allowed left/right drift (% of waqf width): "+std::to_string(options.xpbd.waqfLeftDriftTolerancePercent)+"/"+std::to_string(options.xpbd.waqfRightDriftTolerancePercent),
        "Waqf previous-baseline margin (% of line spacing): "+std::to_string(options.xpbd.waqfPreviousLineMarginPercent)+"; solver bound residuals: "+(options.xpbd.toggles.reportWaqfBounds?"on":"off")};
    bool newFace=true; int surah=0;
    for (int p=1;p<=options.lastPage;++p) {
      const auto text=splitMushafLines(pages[p-1]);
      const auto input=mushafLineInputs(text,p,width,options.layout);
      const int surahBefore=surah;
      for (const auto& line:input) if (line.lineType==LineType::Sura) ++surah;
      if (p<options.firstPage) continue;
      auto shaped=layout.justifyPage(scale,width,input,newFace,options.tajweed,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES,just,options.layout);
      newFace=false; finishMushafPage(shaped,text,p);
      if (shaped.size()!=text.size()) throw std::runtime_error("Wrong line count on page "+std::to_string(p));
      auto solved=placement.solve(shaped,options.xpbd,options.force,options.report);
      auto& statistics=manifest.statistics;
      ++statistics.pages; statistics.lines+=shaped.size();
      for (const auto& line : shaped) {
        if (line.type==LineType::Sura) ++statistics.surahHeaders;
        if (line.type==LineType::Bism) ++statistics.basmalaLines;
        for (const auto& g : line.glyphs) {
          if (g.beginsajda) ++statistics.sajdaStarts;
          if (g.endsajda) ++statistics.sajdaEnds;
        }
      }
      for (const auto& line:solved.glyphs) for (const auto& g:line) { ++statistics.glyphs; if (g.isMark) ++statistics.marks; }
      for (const auto& v:solved.initialViolations) ++statistics.initialByType[violationTypeName(v.type)];
      if (options.report) {
        Report::Page page; page.pageNumber=p; page.notes=notes; page.violations=solved.violations;
        for (auto& line:solved.glyphs) {
          for (auto& g:line) page.glyphs.push_back(glyphRef(g,text));
        }
        for (const auto& v:solved.violations) {
          ++statistics.findings; ++statistics.byType[violationTypeName(v.type)];
          if (v.kind==ViolationKind::Hard) ++statistics.hard; else ++statistics.soft;
          if (v.introduced) { ++statistics.introduced; ++statistics.introducedByType[violationTypeName(v.type)]; }
          if (v.worsened) { ++statistics.worsened; ++statistics.worsenedByType[violationTypeName(v.type)]; }
        }
        if (!report.appendPage(page)) throw std::runtime_error("Cannot write violation page "+std::to_string(p));
      }
      if (mushaf) mushaf->appendPage(shaped,text,p,surahBefore);
      std::cout << "Page " << p << '/' << options.lastPage << ": " << shaped.size() << " lines, " << solved.violations.size() << " findings\n";
    }
    if (mushaf) mushaf->finish();
    if (options.report) {
      if (!report.finish()) throw std::runtime_error("Cannot finalize violation report");
      auto summary=report.summaryEntries();
      manifest.statistics.reportedFindings=report.selectedCount();
      manifest.statistics.eligibleFindings=report.eligibleCount();
      auto summaryNotes=notes;
      summaryNotes.push_back("Selected "+std::to_string(summary.size())+" of "+std::to_string(report.eligibleCount())+" eligible findings; same selection as CSV and page overview.");
      if (!report.writeSummary(summary,sidecar("_violations_summary.pdf"),summaryNotes)) throw std::runtime_error("Cannot write violation summary");
      if (!report.writeCompact(summary,sidecar("_violations_compact.pdf"),summaryNotes)) throw std::runtime_error("Cannot write compact violation report");
      if (!report.writeWeb(summary,sidecar("_violations.html"),summaryNotes)) throw std::runtime_error("Cannot write web violation report");
    }
    for (const auto& [path,hash]:manifest.sources) if (checksum(path)!=hash) throw std::runtime_error("Source changed during run: "+path);
    manifest.complete=true;
    manifest.seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    writeJson(manifest,sidecar(".run.json"));
    std::cout << "Completed " << manifest.statistics.pages << " pages in " << manifest.seconds << " seconds.\n";
    if (options.pdf) std::cout << "Mushaf: " << output << '\n';
    if (options.report) std::cout << "Placement review: " << sidecar("_violations.html") << '\n';
    std::cout << "Run manifest: " << sidecar(".run.json") << '\n';
    return 0;
  } catch (const std::exception& error) { std::cerr << "Generate Mushaf: " << error.what() << '\n'; return 1; }
}
