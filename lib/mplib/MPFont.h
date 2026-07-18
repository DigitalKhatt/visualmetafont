#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

extern "C" {
#include "w2c/config.h"
#include "AddedFiles/newmp.h"
#include "mplib.h"
#include "mplibps.h"
#include "mpmp.h"
#include "mppsout.h"
#include "mpstrings.h"
}

struct MPAxis {
  std::string name;
  std::uint32_t axisTag = 0;
  float minValue = 0;
  float defaultValue = 0;
  float maxValue = 0;
  std::string equivExpr;
};

struct MPFontGlyphInfo {
  mp_edge_object* currentPicture = nullptr;
  std::map<std::string, mp_edge_object*> controlledPictures;
};

class MPFont {
 public:
  MPFont() = default;
  ~MPFont();
  MPFont(const MPFont&) = delete;
  MPFont& operator=(const MPFont&) = delete;

  bool initialize(std::string source, std::filesystem::path projectFile);
  std::string execute(std::string_view command);

  double numericVariable(std::string_view name) const;
  double internalNumericVariable(std::string_view name) const;
  bool boolVariable(std::string_view name) const;
  bool pairVariable(std::string_view name, double& x, double& y) const;
  std::string stringVariable(std::string_view name) const;

  std::vector<mp_edge_object*> edges() const;
  mp_edge_object* edge(int charCode) const;
  MPFontGlyphInfo glyphInfo(int charCode) const;
  mp_graphic_object* copyBody(const mp_graphic_object* body) const;

  void setControlledPictureNames(std::vector<std::string> names);
  void clearGlyphSources();
  void registerGlyphSource(std::string name, std::string source,
                           std::string beginMacro, int unicode);
  bool hasGlyph(std::string_view name) const;
  void generateAlternate(std::string_view name, double leftTatweel,
                         double rightTatweel, double third, double fourth,
                         double fifth, double scaleX,
                         std::string_view source = {}, int alternateCode = 983040);

  const std::filesystem::path& projectFile() const { return projectFile_; }
  const std::filesystem::path& projectDirectory() const { return projectDirectory_; }
  const std::string& fontName() const { return fontName_; }
  const std::vector<MPAxis>& axes() const { return axes_; }
  std::string familyName() const;
  std::string copyright() const;
  std::string log() const;
  MP instance() const { return mp_; }

 private:
  struct GlyphSource {
    std::string source;
    std::string beginMacro;
    int unicode = -1;
  };

  static void shipoutBackend(MP mp, void* edge);
  static char* findFile(MP mp, const char* name, const char* mode, int type);
  void shipout(void* edge);
  void clear();
  void readAxes();

  MP mp_ = nullptr;
  std::unordered_map<int, MPFontGlyphInfo> edges_;
  std::vector<std::string> controlledPictureNames_;
  std::unordered_map<std::string, GlyphSource> glyphSources_;
  std::filesystem::path projectFile_;
  std::filesystem::path projectDirectory_;
  std::string fontName_;
  std::vector<MPAxis> axes_;
};
