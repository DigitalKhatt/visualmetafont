// The CLI provides the same font, corpus and justification setup. Its main is
// renamed and never executed; no reports or PDFs are created by this probe.
#define main unused_mushaf_main
#include "generate_mushaf.cpp"
#undef main
#include "digitalkhatt/bench_before/OptimizeLayout.h"
#include "digitalkhatt/bench_after/OptimizeLayout.h"
#include "digitalkhatt/bench_after/GlyphCollisionGeometry.h"
#include "benchstats.h"

namespace before = digitalkhatt::bench_before;
namespace after = digitalkhatt::bench_after;
using Clock = std::chrono::steady_clock;
using GeometryCache = std::unordered_map<const GlyphVis*, geometry::GeometrySet>;

template<class Glyph>
std::vector<std::vector<Glyph>> prepare(OtLayout& layout,
    std::vector<LineLayoutInfo>& page, bool legacy, double scale, GeometryCache& cache) {
  auto classes = layout.glyphClasses();
  const auto& marks = classes.at("marks");
  const auto& top = classes.at("topmarks");
  const auto& dots = classes.at("topdotmarks");
  const auto& waqf = classes.at("waqfmarks");
  std::vector<std::vector<Glyph>> result;
  result.reserve(page.size());
  for (int l=0;l<static_cast<int>(page.size());++l) {
    auto& line = page[l];
    auto& glyphs = result.emplace_back();
    if (!legacy && line.type == LineType::Sura) continue;
    glyphs.reserve(line.glyphs.size());
    const double xscale = legacy || line.type == LineType::Line ? line.xscale : 1.0;
    double x = -line.xstartposition;
    const double y = -(line.ystartposition-(OtLayout::TopSpace<<OtLayout::SCALEBY));
    Glyph* base = nullptr;
    for (int i=0;i<static_cast<int>(line.glyphs.size());++i) {
      auto& positioned = line.glyphs[i];
      auto* outline = layout.getGlyph(positioned);
      const auto& name = layout.glyphNamePerCode.at(positioned.codepoint);
      auto geom = cache.find(outline);
      if (geom == cache.end()) {
        auto cubics = geometry::getGlyphCubic(outline->copiedPath);
        auto parts = legacy
          ? (marks.contains(name) ? geometry::buildPolyFromCubics(cubics,geometry::CUBIC_FLATNESS_TOLERANCE)
                                  : geometry::buildConvexPartsFromCubics(cubics,geometry::CUBIC_FLATNESS_TOLERANCE))
          : after::buildGlyphCollisionGeometry(cubics,marks.contains(name));
        if (legacy) parts = parts.scaled(scale,scale);
        geom = cache.emplace(outline,std::move(parts)).first;
      }
      x -= positioned.x_advance*xscale;
      if (legacy) x = static_cast<int>(x); // Original Qt adapter accumulated an int.
      auto& g = glyphs.emplace_back();
      g.isMark=marks.contains(name); g.isTopMark=top.contains(name)||dots.contains(name)||waqf.contains(name);
      g.glyphName=name; g.lineIndex=l; g.glyphIndex=i; g.lineY=y;
      g.baseX=x+positioned.x_offset*xscale; g.baseY=y+positioned.y_offset;
      g.glyphLayout=&positioned;
      g.metrics={outline->width,outline->height,outline->bbox.llx,outline->bbox.urx};
      if (legacy && line.fontSize*xscale==1 && line.fontSize==1) g.geom=&geom->second;
      else g.geomScaled=geom->second.scaled(line.fontSize*xscale,line.fontSize);
      g.prevBase=base;
      if (!g.isMark) { if(base) base->nextBase=&g; base=&g; }
    }
  }
  return result;
}

struct Timing {
  double adapter=0,core=0,exportOffsets=0;
  uint64_t pages=0,glyphs=0,marks=0,parts=0;
  BenchStats stats;
};

template<class Glyph,class Params,class Solve>
Timing measure(OtLayout& layout, const std::vector<std::vector<LineLayoutInfo>>& corpus,
    Params params, Solve solve, BenchStats& counters, bool legacy, double scale) {
  counters={}; GeometryCache cache; Timing timing;
  auto classes=layout.glyphClasses();
  if (!classes.contains("bowlbases")) classes["bowlbases"]={"hah.isol","hah.fina","ain.fina"};
  for (const auto& original:corpus) {
    auto t=Clock::now();
    auto page=original;
    auto glyphs=prepare<Glyph>(layout,page,legacy,scale,cache);
    timing.adapter+=secondsSince(t);
    t=Clock::now(); solve(glyphs,classes,params); timing.core+=secondsSince(t);
    t=Clock::now();
    for (size_t l=0;l<glyphs.size();++l) for (size_t i=0;i<glyphs[l].size();++i) {
      auto& g=glyphs[l][i]; auto& p=page[l].glyphs[i];
      if constexpr (std::is_same_v<Glyph, after::GlyphInstance>)
        digitalkhatt::bench_after::applySolvedGlyphOffsets(p,g,
            page[l].type == LineType::Line ? page[l].xscale : 1.0);
      else { p.x_offset+=g.dx; p.y_offset+=g.dy; }
    }
    timing.exportOffsets+=secondsSince(t);
    ++timing.pages;
    for(const auto& line:glyphs) for(const auto& g:line) { ++timing.glyphs; timing.marks+=g.isMark; timing.parts+=g.geom ? g.geom->size() : g.geomScaled.size(); }
  }
  timing.stats=counters; return timing;
}

int main(int argc,char** argv) {
  try {
    if(argc!=4) throw std::runtime_error("Expected CONFIG OUTPUT ROUNDS");
    MushafRunOptions options; mushaf_tool::readJson(options,argv[1]);
    options.force=true; options.report=false; options.pdf=false;
    options.xpbd.toggles.reportViolations=false;
    auto text=mushaf_tool::loadCorpus(options);
    MPFont font; mushaf_tool::initializeFont(font,options.font,options.resources);
    OtLayout layout(&font,true,true); layout.useNormAxisValues=false;
    layout.setInterLineSpacing(options.lineSpacing);
    for(const auto& name:options.disabledLookups) layout.setLookupDisabled(name,true);
    layout.loadLookupFile(options.features);
    auto engine=parseJustifier(options.justifier); layout.applyJustification=engine!=JustType::None;
    JustOption just{engine,parseStyle(options.style),parseShrink(options.shrink)};
    if(!options.stretchPolicy.empty()||!options.shrinkPolicy.empty()) throw std::runtime_error("Probe requires default declarative policy selection");
    OtLayout::EMSCALE=options.emScale;
    const double scale=(1<<OtLayout::SCALEBY)*options.emScale;
    const int width=options.textWidth<<OtLayout::SCALEBY;
    std::vector<std::vector<LineLayoutInfo>> corpus;
    bool newFace=true; auto started=Clock::now();
    for(int p=1;p<=static_cast<int>(text.size());++p) {
      auto lines=splitMushafLines(text[p-1]);
      auto inputs=mushafLineInputs(lines,p,width,options.layout);
      auto page=layout.justifyPage(scale,width,inputs,newFace,options.tajweed,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES,just,options.layout);
      newFace=false; finishMushafPage(page,lines,p); corpus.push_back(std::move(page));
    }
    std::cout<<"Shaped "<<corpus.size()<<" pages once in "<<secondsSince(started)<<" seconds\n"<<std::flush;
    // Exclude lazy MetaPost outline generation from either solver's timing.
    for(const auto& page:corpus) for(const auto& line:page)
      for(const auto& g:line.glyphs) layout.getGlyph(g);
    const auto paramsJson=glz::write_json(options.xpbd).value();
    before::OptParams oldParams; after::OptParams newParams;
    if(glz::read<glz::opts{.error_on_unknown_keys=false}>(oldParams,paramsJson)||glz::read_json(newParams,paramsJson)) throw std::runtime_error("Cannot map solver parameters");
    std::ofstream csv(fs::path(argv[2])/"timings.csv");
    csv<<"round,variant,adapter_s,core_s,export_s,total_s,pages,glyphs,marks,parts,iterations,gap_candidates,safety_setup_s,safety_projection_s,convergence_s,convergence_checks,final_safety_s,fallback_passes\n";
    int rounds=std::stoi(argv[3]);
    for(int round=0;round<rounds;++round) {
      std::vector<std::string> order={"before","after","before-modern-geometry"};
      std::rotate(order.begin(),order.begin()+(round%order.size()),order.end());
      for(const auto& variant:order) {
        Timing t;
        if(variant.starts_with("before")) {
          t=measure<before::GlyphInstance>(layout,corpus,oldParams,
            [](auto& g,const auto& c,const auto& p){before::optimizePage(g,c,p,nullptr);},before::benchStats,variant=="before",scale);
        } else {
          t=measure<after::GlyphInstance>(layout,corpus,newParams,
            [](auto& g,const auto& c,const auto& p){after::optimizePage(g,c,p,nullptr);},after::benchStats,false,scale);
        }
        auto& s=t.stats;
        csv<<std::setprecision(10)<<round+1<<','<<variant<<','<<t.adapter<<','<<t.core<<','<<t.exportOffsets<<','<<t.adapter+t.core+t.exportOffsets<<','<<t.pages<<','<<t.glyphs<<','<<t.marks<<','<<t.parts<<','<<s.iterations<<','<<s.pairs<<','<<s.safetySetup<<','<<s.safetyProjection<<','<<s.convergence<<','<<s.convergenceChecks<<','<<s.finalSafety<<','<<s.fallbackPasses<<'\n'; csv.flush();
        std::cout<<"Round "<<round+1<<' '<<variant<<": adapter "<<t.adapter<<" s, solver "<<t.core<<" s, iterations "<<s.iterations<<'\n'<<std::flush;
      }
    }
  } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
