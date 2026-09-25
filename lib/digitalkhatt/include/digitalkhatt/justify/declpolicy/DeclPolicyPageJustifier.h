#pragma once

#include <string>
#include <vector>

#include <hb.h>

#include "digitalkhatt/core/digitalkahtt_types.h"
#include "digitalkhatt/justify/FeatureJustifier.h"

namespace digitalkhatt::justify {

// The declarative justification engine.  Where FeatureJustifier carries the
// hand-written regex policies (Madina, IndoPak, Experimental, Experimental2),
// this one runs nothing but the declarative policy compiled from
// table(justdfa): the rules that select opportunities and the actions that
// stage changes both come from the font's feature file, and the engine itself
// holds no per-font knowledge.
//
// It shares line analysis, shaping and measurement with FeatureJustifier, so
// a line that no policy touches lays out identically through either.
class DeclPolicyPageJustifier {
 public:
  explicit DeclPolicyPageJustifier(FeatureJustificationLayout& layout)
      : layout_(layout) {}

  std::vector<LineLayoutInfo> justifyPage(
      double emScale,
      int pageWidth,
      const std::vector<LineToJustify>& lines,
      bool newFace,
      bool tajweedColor,
      hb_buffer_cluster_level_t clusterLevel,
      JustOption justOption,
      const std::string& mushafLayout) const;

 private:
  FeatureJustificationLayout& layout_;
};

}  // namespace digitalkhatt::justify
