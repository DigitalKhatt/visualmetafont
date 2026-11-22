#include "digitalkhatt/project/FontProfile.h"

#include <iostream>
#include <stdexcept>

namespace digitalkhatt::project {

ScopedFontProfile::ScopedFontProfile(MPFont& font, FontProfile profile) : font_(font) {
  if (profile == FontProfile::ProjectDefault) return;

  const auto previous = font_.stringVariable("fontProfile");
  if (previous.empty())
    throw std::runtime_error("This font project does not declare a shaping profile");
  if (previous != "justification" && previous != "portable" && previous != "live")
    throw std::runtime_error("Unknown project shaping profile: " + previous);

  restore_ = "fontProfile := \"" + previous + "\"; boolean isJustificationProfile, isPortableProfile;";
  restore_ += font_.boolVariable("isJustificationProfile")
                  ? "isJustificationProfile := true;" : "isJustificationProfile := false;";
  restore_ += font_.boolVariable("isPortableProfile")
                  ? "isPortableProfile := true;" : "isPortableProfile := false;";
  restore_ += detail::updateProfileShrink;
  font_.execute(profile == FontProfile::Portable ? "fontProfile := \"portable\";"
                                                 : "fontProfile := \"justification\";");
}

ScopedFontProfile::~ScopedFontProfile() {
  if (restore_.empty()) return;
  try {
    font_.execute(restore_);
  } catch (const std::exception& error) {
    std::cerr << "Could not restore the project shaping profile: " << error.what() << '\n';
  }
}

}  // namespace digitalkhatt::project
