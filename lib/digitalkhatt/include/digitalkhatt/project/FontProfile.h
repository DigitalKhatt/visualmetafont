#pragma once

#include <stdexcept>
#include <string>

#include "MPFont.h"

namespace digitalkhatt::project {

enum class FontProfile { ProjectDefault, Justification, Portable };

namespace detail {
inline constexpr char updateProfileShrink[] =
    " if known shrinkInJustification: isShrink := shrinkInJustification and isJustificationProfile; fi;";
}

// Called by fonts that expose profile conditionals in their feature files.
inline FontProfile initializeFontProfile(MPFont& font) {
  const auto name = font.stringVariable("fontProfile");
  const bool portable = name == "portable";
  if (!portable && !name.empty() && name != "justification" && name != "live")
    throw std::runtime_error("Unknown project shaping profile: " + name);

  font.execute(std::string{"boolean isJustificationProfile, isPortableProfile; isPortableProfile := "} +
               (portable ? "true;" : "false;") + " isJustificationProfile := not isPortableProfile;" +
               detail::updateProfileShrink);
  return portable ? FontProfile::Portable : FontProfile::Justification;
}

// OpenType export can share the editor's MetaPost interpreter. Restore its
// profile and derived feature conditions when the export layout is destroyed.
class ScopedFontProfile {
 public:
  ScopedFontProfile(MPFont& font, FontProfile profile);
  ~ScopedFontProfile();
  ScopedFontProfile(const ScopedFontProfile&) = delete;
  ScopedFontProfile& operator=(const ScopedFontProfile&) = delete;

 private:
  MPFont& font_;
  std::string restore_;
};

}  // namespace digitalkhatt::project
