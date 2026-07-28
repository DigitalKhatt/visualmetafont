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

namespace {

std::string readFile(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream)
    throw std::runtime_error("Could not open " + path.string());
  return {std::istreambuf_iterator<char>{stream}, {}};
}

void registerGlyphSources(MPFont& font, const fs::path& glyphsPath) {
  const auto glyphs = readFile(glyphsPath);
  // Keep this loader in sync with QuranShaper::registerGlyphSources.  Besides
  // registering the source used for tatweel alternates, every glyph must be
  // executed once to materialize its MetaPost picture and components.
  const std::string parameterReset =
      "params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;";
  std::size_t count = 0;
  std::size_t pos = 0;
  while (pos < glyphs.size()) {
    const auto beginPos = glyphs.find("beginchar", pos);
    const auto defPos = glyphs.find("defchar", pos);
    const bool isDef =
        defPos != std::string::npos &&
        (beginPos == std::string::npos || defPos < beginPos);
    const auto blockStart = isDef ? defPos : beginPos;
    if (blockStart == std::string::npos)
      break;

    const std::string_view endMarker =
        isDef ? "enddefchar;" : "endchar;";
    const auto endPos = glyphs.find(endMarker, blockStart);
    if (endPos == std::string::npos)
      throw std::runtime_error("Unterminated glyph definition near byte " +
                               std::to_string(blockStart));
    const auto blockEnd = endPos + endMarker.size();
    const auto source = glyphs.substr(blockStart, blockEnd - blockStart);

    const auto parenOpen = source.find('(');
    const auto parenClose = source.find(')', parenOpen);
    const auto comma1 = source.find(',', parenOpen);
    const auto comma2 = source.find(',', comma1 + 1);
    if (parenOpen == std::string::npos || parenClose == std::string::npos ||
        comma1 == std::string::npos || comma2 == std::string::npos ||
        comma2 > parenClose)
      throw std::runtime_error("Invalid glyph header near byte " +
                               std::to_string(blockStart));

    const auto glyphName =
        source.substr(parenOpen + 1, comma1 - parenOpen - 1);
    const auto unicode =
        std::stoi(source.substr(comma1 + 1, comma2 - comma1 - 1));
    font.registerGlyphSource(glyphName, source,
                             isDef ? "defchar" : "beginchar", unicode);
    font.execute(parameterReset + source);

    ++count;
    pos = blockEnd;
  }
  if (count == 0)
    throw std::runtime_error("No glyph definitions found in " +
                             glyphsPath.string());
  std::cout << "Registered " << count << " glyph sources\n";
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
         "  --glyphs PATH           Glyph source file (default: glyphs.mp)\n"
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
    if (outputFile.empty())
      outputFile = projectDirectory / "output" /
                   (projectFile.stem().string() + ".otf");
    else
      outputFile = fs::absolute(outputFile);
    if (glyphsFile.empty())
      glyphsFile = projectDirectory / "glyphs.mp";
    else
      glyphsFile = fs::absolute(glyphsFile);

    std::string source{"MPGUI:=1;"};
    source += readFile(resourceDirectory / "mfplain.mp");
    source += readFile(resourceDirectory / "mpost.mp");
    source += readFile(resourceDirectory / "vmf.mp");
    source += readFile(projectFile);

    MPFont font;
    std::cout << "Loading " << projectFile << '\n';
    font.initialize(std::move(source), projectFile);
    registerGlyphSources(font, glyphsFile);

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
