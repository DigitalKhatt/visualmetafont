#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

SolverContext::SolverContext(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                              const ClassMap& classes)
    : classes{classes}, pageGlyphs{pageGlyphs} {
  marks = classesOrEmpty(classes, "marks");
  topmarks = classesOrEmpty(classes, "topmarks");
  lowmarks = classesOrEmpty(classes, "lowmarks");
  waqfmarks = classesOrEmpty(classes, "waqfmarks");
  topdotmarks = classesOrEmpty(classes, "topdotmarks");
  downdotmarks = classesOrEmpty(classes, "downdotmarks");
  bowlbases = classesOrEmpty(classes, "bowlbases");
}

}  // namespace digitalkhatt::layout
