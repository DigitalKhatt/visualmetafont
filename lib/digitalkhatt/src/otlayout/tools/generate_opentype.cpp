#include "digitalkhatt/project/FontProfile.h"
#include "digitalkhatt/project/GlyphSources.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Layout/OtLayout.h"
#include "Layout/to_opentype.h"
#include "MPFont.h"

namespace fs = std::filesystem;
using digitalkhatt::project::FontProfile;
using digitalkhatt::project::ScopedFontProfile;

namespace {

std::string readFile(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream)
    throw std::runtime_error("Could not open " + path.string());
  return {std::istreambuf_iterator<char>{stream}, {}};
}


void usage(const char* program) {
  std::cerr
      << "Usage: " << program
      << " [options] font.mp\n"
         "Generate the same CFF2 font as VisualMetaFont's "
         "\"Generate OpenType CFF2 Standard\" action.\n\n"
         "Options:\n"
         "  -o, --output PATH       Output OTF path\n"
         "  --features PATH         Feature file (default: features.fea)\n"
         "  --resources DIR         Directory containing mfplain.mp, "
         "mpost.mp, vmf.mp\n"
         "  --glyphs PATH           Override glyph source file (default: project manifest)\n"
         "  --profile PROFILE       justification or portable (default: project setting)\n"
         "  --extended              Generate the extended online-shaper font\n"
         "  --no-variable           Disable OpenType variable axes\n"
         "  --disable-lookup NAME   Disable a lookup; may be repeated\n"
         "  -h, --help              Show this help\n";
}

}  // namespace

int main(int argc, char** argv) {
  try {
    fs::path projectFile;
    fs::path outputFile;
    fs::path glyphsFile;
    fs::path resourceDirectory{DIGITALKHATT_METAFONT_RESOURCES};
    std::string featuresFile{"features.fea"};
    std::vector<std::string> disabledLookups;
    FontProfile profile = FontProfile::ProjectDefault;
    bool extended = false;
    bool variable = true;

    for (int i = 1; i < argc; ++i) {
      const std::string_view argument{argv[i]};
      auto value = [&](std::string_view option) -> std::string {
        if (++i >= argc)
          throw std::runtime_error("Missing value after " +
                                   std::string(option));
        return argv[i];
      };
      if (argument == "-h" || argument == "--help") {
        usage(argv[0]);
        return 0;
      } else if (argument == "-o" || argument == "--output") {
        outputFile = value(argument);
      } else if (argument == "--features") {
        featuresFile = value(argument);
      } else if (argument == "--resources") {
        resourceDirectory = value(argument);
      } else if (argument == "--glyphs") {
        glyphsFile = value(argument);
      } else if (argument == "--profile") {
        const auto selected = value(argument);
        if (selected == "justification")
          profile = FontProfile::Justification;
        else if (selected == "portable")
          profile = FontProfile::Portable;
        else
          throw std::runtime_error("Unknown profile '" + selected + "' (expected justification or portable)");
      } else if (argument == "--disable-lookup") {
        disabledLookups.push_back(value(argument));
      } else if (argument == "--extended") {
        extended = true;
      } else if (argument == "--no-variable") {
        variable = false;
      } else if (!argument.empty() && argument.front() == '-') {
        throw std::runtime_error("Unknown option " + std::string(argument));
      } else if (projectFile.empty()) {
        projectFile = argument;
      } else {
        throw std::runtime_error("More than one font project was provided");
      }
    }

    if (projectFile.empty()) {
      usage(argv[0]);
      return 2;
    }
    projectFile = fs::absolute(projectFile);
    const auto projectDirectory = projectFile.parent_path();
    if (!glyphsFile.empty()) glyphsFile = fs::absolute(glyphsFile);

    std::string source{"MPGUI:=1;"};
    source += readFile(resourceDirectory / "mfplain.mp");
    source += readFile(resourceDirectory / "mpost.mp");
    source += readFile(resourceDirectory / "vmf.mp");
    source += readFile(projectFile);

    MPFont font;
    std::cout << "Loading " << projectFile << '\n';
    font.initialize(std::move(source), projectFile);
    const ScopedFontProfile profileOverride(font, profile);
    if (outputFile.empty()) {
      const auto suffix = font.stringVariable("fontProfile") == "portable" ? "-portable.otf" : ".otf";
      outputFile = projectDirectory / "output" / (projectFile.stem().string() + suffix);
    } else {
      outputFile = fs::absolute(outputFile);
    }
    const auto sources = digitalkhatt::project::ProjectGlyphSources::load(projectFile,
        glyphsFile.empty() ? std::nullopt : std::optional<fs::path>{glyphsFile});
    std::cout << "Registered " << sources.registerWith(font) << " glyph sources\n";

    OtLayout layout(&font, extended, extended ? true : variable);
    for (const auto& lookup : disabledLookups)
      layout.setLookupDisabled(lookup, true);
    layout.toOpenType->isCff2 = true;

    fs::create_directories(outputFile.parent_path());
    std::cout << "Generating " << outputFile << '\n';
    if (!layout.toOpenType->GenerateFile(outputFile, featuresFile))
      throw std::runtime_error("OpenType generation failed");
    std::cout << "Generated " << outputFile << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
