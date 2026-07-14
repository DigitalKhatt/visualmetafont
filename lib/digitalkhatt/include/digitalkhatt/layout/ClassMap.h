#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace digitalkhatt::layout {

using ClassSet = std::unordered_set<std::string>;
using ClassMap = std::unordered_map<std::string, ClassSet>;

// classes["marks"]-style lookup on a const map. std::unordered_map has no
// const operator[], so this mirrors QHash::operator[] const's behavior of
// returning an empty value instead of inserting when the key is absent.
inline const ClassSet& classesOrEmpty(const ClassMap& classes, const std::string& key) {
  static const ClassSet empty;
  auto it = classes.find(key);
  return it != classes.end() ? it->second : empty;
}

inline bool containsSubstr(const std::string& haystack, const std::string& needle) {
  return haystack.find(needle) != std::string::npos;
}

}  // namespace digitalkhatt::layout
