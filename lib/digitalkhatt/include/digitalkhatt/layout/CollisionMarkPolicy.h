#pragma once

#include <algorithm>
#include "digitalkhatt/layout/CollisionReport.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt::layout {

inline bool isAdditionalCollisionMark(const std::string& name, const OptParams& params) {
  return !isCollisionSpace(name) &&
      std::find(params.additionalCollisionMarks.begin(), params.additionalCollisionMarks.end(), name) !=
          params.additionalCollisionMarks.end();
}

}  // namespace digitalkhatt::layout
