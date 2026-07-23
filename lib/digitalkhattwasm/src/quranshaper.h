/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#pragma once

#include "GlazeJson.h"

#include "GlyphVis.h"
#include "MPFont.h"
#include "OtLayout.h"
#include "automedina/automedina.h"
#include "digitalkhatt/core/Regex16.h"
#include "qdatastream_reader.h"
#include <charconv>
#include <digitalkhatt/geometry/geometry.h>
#include <digitalkhatt/layout/OptimizeLayout.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <math.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <emscripten.h>
#include <emscripten/bind.h>

struct PageResult {
  std::vector<LineLayoutInfo> page;
  std::vector<digitalkhatt::TextString> originalPage;
};

namespace {

std::string initializationStage = "not started";

std::string getInitializationStage() { return initializationStage; }

std::string parentDirectory(std::string_view path) {
  const auto separator = path.find_last_of("/\\");
  return separator == std::string_view::npos
             ? std::string{}
             : std::string{path.substr(0, separator)};
}

std::string siblingPath(std::string_view path, std::string_view sibling) {
  const auto directory = parentDirectory(path);
  return directory.empty() ? std::string{sibling}
                           : directory + "/" + std::string{sibling};
}

// Mirrors the sura/bism and sajda-verse patterns already used by
// OtLayout::pageBreak (OtLayout.cpp) so the same PCRE2-16 engine and
// mark-skipping convention are used here.
constexpr digitalkhatt::TextView surapattern =
    u"(?m)^(سُورَةُ .*|بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ|بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ)$";
constexpr digitalkhatt::TextView sajdapatterns =
    u"(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ "
    u"لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ "
    u"سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)";

std::string utf16ToUtf8(digitalkhatt::TextView input) {
  std::string result;
  result.reserve(input.size());
  for (std::size_t i = 0; i < input.size(); ++i) {
    char32_t codepoint = input[i];
    if (codepoint >= 0xd800 && codepoint <= 0xdbff && i + 1 < input.size()) {
      char16_t low = input[i + 1];
      if (low >= 0xdc00 && low <= 0xdfff) {
        codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (low - 0xdc00);
        ++i;
      }
    }
    if (codepoint < 0x80) {
      result.push_back(static_cast<char>(codepoint));
    } else if (codepoint < 0x800) {
      result.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
      result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else if (codepoint < 0x10000) {
      result.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
      result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    } else {
      result.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
      result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
  }
  return result;
}

} // namespace

class QuranShaper {
public:
  QuranShaper() : QuranShaper("madina.mp") {}

  explicit QuranShaper(const std::string &fontFile) {

    initializationStage = "MetaPost initialization";
    int status = initilizeMetapost(fontFile);

    if (status == 0) {
      initializationStage = "OtLayout construction";
      try {
        layout = new OtLayout(&mpFont, true);
      } catch (const std::exception &e) {
        throw std::runtime_error(std::string{"OtLayout construction: "} +
                                 e.what());
      }

      layout->useNormAxisValues = false;

      initializationStage = "feature loading";
      try {
        loadLookupFile(siblingPath(fontFile, "features.fea"));
      } catch (const std::exception &e) {
        throw std::runtime_error(std::string{"feature loading: "} + e.what());
      }
    }
    initializationStage = "complete";
  }

  int initilizeMetapost(const std::string &fontFile = "madina.mp") {
    try {
      if (!loadFontFile(fontFile)) {
        std::cout << "Could not load font file\n";
        return 1;
      }
      std::cout << "Metapost initilized with status 0\n";
      return 0;
    } catch (const std::exception &e) {
      std::cout << "Could not initialize MetaPost library instance!\n"
                << e.what() << '\n';
      throw std::runtime_error(std::string{"MetaPost initialization: "} +
                               e.what());
    } catch (...) {
      std::cout << "Could not initialize MetaPost library instance! (unknown "
                   "exception)\n";
      throw std::runtime_error("MetaPost initialization: unknown exception");
    }
  }

  // Mirrors Font::loadFile (visualmetafont/src/metafont/font.cpp): prepend
  // the shared MetaPost infrastructure (mfplain.mp, mpost.mp, vmf.mp) to the
  // font's own source, initialize MPFont with the combined script, then
  // parse the sibling glyphs.mp file to register each glyph's source so
  // OtLayout::getAlternate()/MPFont::generateAlternate() can regenerate it
  // with different tatweel/scale parameters.
  bool loadFontFile(const std::string &fileName) {
    initializationStage = "reading font assets";
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
      return false;

    std::ifstream rsmfplain("mfplain.mp", std::ios::binary);
    std::ifstream rsmpost("mpost.mp", std::ios::binary);
    std::ifstream rsvmf("vmf.mp", std::ios::binary);

    std::string initMF = "MPGUI:=1;";

    if (!rsmfplain) {
      std::cout << "mfplain.mp file not opened\n";
      return false;
    }
    initMF += std::string{std::istreambuf_iterator<char>{rsmfplain}, {}};

    if (!rsmpost) {
      std::cout << "mpost.mp file not opened\n";
      return false;
    }
    initMF += std::string{std::istreambuf_iterator<char>{rsmpost}, {}};

    if (!rsvmf) {
      std::cout << "vmf.mp file not opened\n";
      return false;
    }
    initMF += std::string{std::istreambuf_iterator<char>{rsvmf}, {}};

    initMF += std::string{std::istreambuf_iterator<char>{file}, {}};

    initializationStage = "MPFont initialize";
    mpFont.initialize(initMF, fileName);

    const std::string glyphsPath = siblingPath(fileName, "glyphs.mp");

    std::ifstream glyphsFile(glyphsPath, std::ios::binary);
    if (!glyphsFile)
      return false;

    std::string code{std::istreambuf_iterator<char>{glyphsFile}, {}};
    initializationStage = "registering glyph sources";
    registerGlyphSources(code);

    return true;
  }

  // Splits glyphs.mp into its "beginchar(...)...endchar;" /
  // "defchar(...)...enddefchar;" blocks -- same pattern Font::loadFile
  // matches with a QRegularExpression -- and registers each block verbatim
  // as that glyph's source. glyphs.mp is machine-written by Font::saveFile
  // (font.cpp) in exactly this "macro(name,unicode,width,height,depth);"
  // header form, so a plain substring scan is enough; no MetaPost/glyph
  // grammar parsing needed here.
  void registerGlyphSources(const std::string &code) {
    const std::string parameterReset =
        "params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;";
    std::size_t pos = 0;
    while (pos < code.size()) {
      std::size_t beginPos = code.find("beginchar", pos);
      std::size_t defPos = code.find("defchar", pos);
      bool isDef = defPos != std::string::npos &&
                   (beginPos == std::string::npos || defPos < beginPos);
      std::size_t blockStart = isDef ? defPos : beginPos;
      if (blockStart == std::string::npos)
        break;

      const std::string endMarker = isDef ? "enddefchar;" : "endchar;";
      std::size_t endPos = code.find(endMarker, blockStart);
      if (endPos == std::string::npos)
        break;
      std::size_t blockEnd = endPos + endMarker.size();

      std::string block = code.substr(blockStart, blockEnd - blockStart);

      std::size_t parenOpen = block.find('(');
      std::size_t parenClose = block.find(')', parenOpen);
      std::string args =
          block.substr(parenOpen + 1, parenClose - parenOpen - 1);
      std::size_t comma1 = args.find(',');
      std::size_t comma2 = args.find(',', comma1 + 1);
      std::string glyphName = args.substr(0, comma1);

      int unicode = std::stoi(args.substr(comma1 + 1, comma2 - comma1 - 1));

      initializationStage = "registering glyph source " + glyphName;
      mpFont.registerGlyphSource(glyphName, block,
                                 isDef ? "defchar" : "beginchar", unicode);

      initializationStage = "executing glyph source " + glyphName;
      executeMetapost(parameterReset + block);

      pos = blockEnd;
    }
  }

  int executeMetapost(std::string code) {
    try {
      mpFont.execute(code);
      return 0;
    } catch (const std::exception &e) {
      std::cout << "Could not execute MetaPost glyph source!\n"
                << e.what() << '\n';
      return 1;
    }
  }

  void initLayout() { layout = new OtLayout(&mpFont, true); }

  void initLookup(std::string fileName) { loadLookupFile(fileName); }

  ~QuranShaper() { delete layout; }

  std::string getGlyphName(int codechar) {

    return layout->glyphNamePerCode.at(codechar);
  }

  int getGlyphCode(const std::string &name) {
    return layout->glyphCodePerName.at(name);
  }

  int getTexNbPages() {
    if (texPages.empty()) {
      readTexPages();
    }

    return static_cast<int>(texPages.size());
  }

  std::vector<SuraLocation> getSuraLocations(bool tex) {
    if (tex) {
      if (texSuraLocations.empty()) {
        readTexPages();
      }

      return texSuraLocations;
    } else {
      if (medinaSuraLocations.empty()) {
        readMedinaPages();
      }

      return medinaSuraLocations;
    }
  }

  void drawPath(std::string glyphName, emscripten::val ctx) {

    if (!mpFont.instance()) {
      std::cout << "cannot initilize mp";
    }

    for (mp_edge_object *p : mpFont.edges()) {
      if (p->charname == glyphName) {
        mp_graphic_object *body = p->body;

        if (body) {
          edgetoHTML5Path(body, ctx);
        }

        return;
      }
    }

    std::cout << "no char";
  }

  double shapeText(std::string text, int lineWidth, float fontScalePerc,
                   bool applyJustification, bool tajweedColor,
                   bool fontExpansion, emscripten::val ctx) {

    layout->applyJustification = applyJustification;

    std::vector<std::string> stdLines;
    std::size_t start = 0;
    while (start <= text.size()) {
      std::size_t nl = text.find('\n', start);
      std::size_t end = (nl == std::string::npos) ? text.size() : nl;
      if (end > start)
        stdLines.push_back(text.substr(start, end - start));
      if (nl == std::string::npos)
        break;
      start = nl + 1;
    }

    auto justification = LineJustification::Distribute;

    if (lineWidth == 0) {
      justification = LineJustification::Center;
    }

    double fontScale = (1 << OtLayout::SCALEBY) * fontScalePerc;

    double scale = 72. / ((4800 << OtLayout::SCALEBY) * fontScalePerc);

    lineWidth = lineWidth / scale;

    auto page = layout->justifyPage(fontScale, lineWidth, lineWidth,
                                    std::move(stdLines), justification, true,
                                    tajweedColor);

    int currentyPos = 0;
    int margin = 0;
    int InterLineSpacing = layout->InterLineSpacing << OtLayout::SCALEBY;

    double maxWidth = 0;

    for (int lineIndex = 0; lineIndex < page.size(); ++lineIndex) {

      auto &line = page[lineIndex];

      int currentxPos = 0; // lineWidth + margin - line.xstartposition;

      Point lastPos{currentxPos, currentyPos};

      ctx.call<void>("save");

      ctx.call<void>("scale", scale, scale);

      ctx.call<void>("transform", 1, 0, 0, -1, lastPos.x(), lastPos.y());

      for (int glyphIndex = 0; glyphIndex < line.glyphs.size(); glyphIndex++) {

        auto &glyph = line.glyphs[glyphIndex];

        if (glyph.color) {
          auto color = glyph.color;

          ctx.set("fillStyle",
                  emscripten::val("rgb(" +
                                  std::to_string(((color >> 24) & 0xff)) + "," +
                                  std::to_string(((color >> 16) & 0xff)) + "," +
                                  std::to_string(((color >> 8) & 0xff)) + ")"));
        }

        Point pos;
        currentxPos -= glyph.x_advance;
        pos.setX(currentxPos + (glyph.x_offset));
        pos.setY(currentyPos - (glyph.y_offset));

        Point diff = pos - lastPos;
        lastPos = pos;

        ctx.call<void>("translate", diff.x(), -diff.y());

        ctx.call<void>("save");
        ctx.call<void>("scale", fontScale, fontScale);

        displayGlyph(glyph.codepoint, glyph.lefttatweel, glyph.righttatweel,
                     ctx);
        ctx.call<void>("restore");

        if (glyph.color) {
          ctx.set("fillStyle", "rgb(0,0,0)");
        }
      }

      if (currentxPos < maxWidth)
        maxWidth = currentxPos;

      currentyPos += InterLineSpacing;

      ctx.call<void>("restore");
    }

    clearAlternates();

    return -maxWidth * scale;
  }

  PageResult shapePage(int pageIndex, float fontScalePerc,
                       bool applyJustification, int lineIndex, bool texFormat,
                       bool tajweedColor, bool changeSize) {

    // if (cachedPages.find(pageNumber) != cachedPages.end()) {
    //	std::cout << "Cached page number " << pageNumber << "\n";
    //	return cachedPages.at(pageNumber);
    // }

    int lineWidth = pageWidth;

    layout->applyJustification = applyJustification;

    std::vector<digitalkhatt::TextString> lines;

    if (texFormat) {
      if (texPages.empty()) {
        readTexPages();
      }

      if (static_cast<int>(texPages.size()) <= pageIndex) {
        std::cout << "Out of range Tex pageNumber " << pageIndex << '\n';
        return PageResult{};
      }

      lines = texPages[pageIndex];
    } else {
      if (medinaPages.empty()) {
        readMedinaPages();
      }

      if (static_cast<int>(medinaPages.size()) <= pageIndex) {
        std::cout << "Out of range Medina pageNumber " << pageIndex << '\n';
        return PageResult{};
      }

      lines = medinaPages[pageIndex];
    }

    double fontScale = (1 << OtLayout::SCALEBY) * fontScalePerc;

    if (lineIndex >= 0) {
      lines = {lines[lineIndex]};
    }

    auto justification = LineJustification::Distribute;
    int beginsura = OtLayout::TopSpace << OtLayout::SCALEBY;

    if (pageIndex == 0 || pageIndex == 1) {
      justification = LineJustification::Center;
      beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3))
                  << OtLayout::SCALEBY;
      if (lineIndex > 0) {
        double ratio = pageIndex == 0 ? 0.9 : 0.9;
        double diameter = pageWidth * ratio; // 0.9;
        if (pageIndex == 0) {
          diameter = pageWidth * ratio; // 0.9;
        }
        // 22.5 = 180 / 8
        double startangle = pageIndex == 0 ? 30 : 30;
        double endangle = 22.5;

        double degree = (startangle + (lineIndex - 1) *
                                          (180 - (startangle + endangle)) / 6) *
                        M_PI / 180;
        lineWidth = diameter * std::sin(degree);

      } else {
        lineWidth = 0;
      }

    } else if (pageIndex > 580) {
      auto ratio = getWidthRatio(pageIndex, lineIndex, texFormat);
      if (ratio < 1) {
        lineWidth = pageWidth * ratio;
        justification = LineJustification::Center;
      }
    }

    std::vector<std::string> stdLines;
    stdLines.reserve(lines.size());
    for (const auto &line : lines)
      stdLines.push_back(utf16ToUtf8(line));
    auto page = layout->justifyPage(fontScale, lineWidth, pageWidth,
                                    std::move(stdLines), justification, false,
                                    tajweedColor);

    if (pageIndex == 0 && lineIndex == 0) {
      page[0].type = LineType::Sura;
    } else if (pageIndex == 1 && lineIndex == 0) {
      page[0].type = LineType::Sura;
    } else if (pageIndex == 1 && lineIndex == 1) {
      page[0].type = LineType::Bism;
    } else {
      for (int i = 0; i < page.size(); ++i) {

        auto &currentLine = page[i];

        // check if suran name or bism
        auto match = surabism.match(lines[i]);
        if (match.hasMatch()) {

          auto temp = layout->justifyPage(
              fontScale, 0, pageWidth,
              std::vector<std::string>{utf16ToUtf8(lines[i])},
              LineJustification::Center, false, tajweedColor);

          digitalkhatt::TextString captured = lines[i].substr(
              static_cast<std::size_t>(match.start()),
              static_cast<std::size_t>(match.end() - match.start()));

          if (captured.starts_with(u"سُ")) {
            temp[0].type = LineType::Sura;
          } else {
            temp[0].type = LineType::Bism;
          }

          page[i] = temp[0];
        } else {
          // check if sajda
          match = sajdaRe.match(lines[i]);
          if (match.hasMatch()) {

            // sajdamatched++;

            int captureIndex = match.lastCapturedIndex();

            int startOffset = match.start(captureIndex); // startOffset == 6
            int endOffset = match.end(captureIndex) - 1; // endOffset == 9

            while (endOffset >= 0) {
              auto category = hb_unicode_general_category(
                  hb_unicode_funcs_get_default(), lines[i][endOffset]);
              if (category != HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK &&
                  category != HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK &&
                  category != HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK)
                break;
              --endOffset;
            }

            bool beginDone = false;

            auto &glyphs = currentLine.glyphs;

            for (auto &glyphLayout : glyphs) {

              if (glyphLayout.cluster == startOffset && !beginDone) {
                glyphLayout.beginsajda = true;
                beginDone = true;
                ;
                // beginsajda++;

              } else if (glyphLayout.cluster == endOffset) {
                glyphLayout.endsajda = true;
                // endsajda++;
                break;
              }
            }
          }
        }
        if (/*i == 0 &&*/ (pageIndex == 0 || pageIndex == 1)) {
          // page[i].type = LineType::Sura;
          page[i].ystartposition =
              (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1))
              << OtLayout::SCALEBY;
        }
        /*else {
          page[i].ystartposition = beginsura;
          beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;

        }*/
      }
    }

    // cachedPages.insert({ pageNumber, page });
    // PageResult resutl;
    // resutl.page = page;

    return {page, lines};
  }

  void displayGlyph(int glyphIndex, double leftTatweel, double righttatweel,
                    emscripten::val ctx) {

    GlyphParameters parameters;
    parameters.lefttatweel = leftTatweel;
    parameters.righttatweel = righttatweel;

    GlyphVis *glyph = layout->getGlyph(glyphIndex, parameters);

    if (glyph) {
      generateGlyph(*glyph, ctx);
    }
  }

  void clearAlternates() { layout->clearAlternates(); }

  MPFont mpFont;
  OtLayout *layout;

protected:
  std::vector<std::vector<digitalkhatt::TextString>> texPages;
  std::vector<std::vector<digitalkhatt::TextString>> medinaPages;

  std::vector<SuraLocation> texSuraLocations;
  std::vector<SuraLocation> medinaSuraLocations;

  digitalkhatt::Regex16 surabism{surapattern};
  digitalkhatt::Regex16 sajdaRe{sajdapatterns};

  int pageWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  std::unordered_map<int, std::vector<LineLayoutInfo>> cachedPages;

  void readTexPages() {
    QDataStreamReader in(readBinaryFile("texpages.dat"));

    in.readDouble(); // EMSCALE, unused here

    texPages = in.readList<std::vector<digitalkhatt::TextString>>(
        [](QDataStreamReader &reader) {
          return reader.readList<digitalkhatt::TextString>(
              [](QDataStreamReader &r) { return r.readString(); });
        });

    in.readList<digitalkhatt::TextString>([](QDataStreamReader &r) {
      return r.readString();
    }); // suraNamebyPage, unused

    texSuraLocations = in.readList<SuraLocation>([](QDataStreamReader &r) {
      SuraLocation location;
      location.name = r.readString();
      location.pageNumber = r.readInt32();
      location.x = r.readInt32();
      location.y = r.readInt32();
      return location;
    });
  }

  void readMedinaPages() {
    QDataStreamReader in(readBinaryFile("medinapages.dat"));

    in.readDouble(); // EMSCALE, unused here

    medinaPages = in.readList<std::vector<digitalkhatt::TextString>>(
        [](QDataStreamReader &reader) {
          return reader.readList<digitalkhatt::TextString>(
              [](QDataStreamReader &r) { return r.readString(); });
        });

    in.readList<digitalkhatt::TextString>([](QDataStreamReader &r) {
      return r.readString();
    }); // suraNamebyPage, unused

    medinaSuraLocations = in.readList<SuraLocation>([](QDataStreamReader &r) {
      SuraLocation location;
      location.name = r.readString();
      location.pageNumber = r.readInt32();
      location.x = r.readInt32();
      location.y = r.readInt32();
      return location;
    });
  }

  void loadLookupFile(std::string fileName) {

    layout->parseFeatureFile(fileName);

    const auto parametersPath = siblingPath(fileName, "parameters.json");
    std::ifstream parametersStream(parametersPath, std::ios::binary);
    // std::ifstream parametersStream("parameters.json", std::ios::binary);

    if (parametersStream) {
      std::string buffer{std::istreambuf_iterator<char>{parametersStream}, {}};
      ParameterJsonObject parameters;
      if (glz::read_json(parameters, buffer)) {
        std::cout << "Problem reading file." << "parameters.json";
      } else {
        layout->readParameters(parameters);
      }

      parametersStream.close();
    }
  }

  void filltoHTML5Path(mp_gr_knot h, emscripten::val ctx) {
    mp_gr_knot p, q;

    ctx.call<void>("moveTo", h->x_coord, h->y_coord);

    p = h;
    do {
      q = p->next;

      ctx.call<void>("bezierCurveTo", p->right_x, p->right_y, q->left_x,
                     q->left_y, q->x_coord, q->y_coord);

      p = q;
    } while (p != h);
    if (h->data.types.left_type != mp_endpoint) {
      ctx.call<void>("closePath");
    }
  }

  void getImageStream(GlyphVis &glyph, emscripten::val ctx) {
    {
      mp_graphic_object *body = glyph.mpPath();
      if (body) {
        do {
          switch (body->type) {
          case mp_fill_code: {
            auto fillobject = (mp_fill_object *)body;
            ctx.call<void>("beginPath");

            filltoHTML5Path(fillobject->path_p, ctx);
            if (fillobject->color_model == mp_rgb_model) {
              ctx.set("fillStyle",
                      emscripten::val(
                          "rgb(" +
                          std::to_string(fillobject->color.a_val * 255) + "," +
                          std::to_string(fillobject->color.b_val * 255) + "," +
                          std::to_string(fillobject->color.c_val * 255) + ")"));
              // out << "\tctx.fillStyle = 'rgb(" << fillobject->color.a_val *
              // 255 << "," << fillobject->color.b_val * 255 << "," <<
              // fillobject->color.c_val * 255 << ")';\n";
            }
            // out << "\tctx.fill();\n";
            ctx.call<void>("fill");
            ctx.set("fillStyle", emscripten::val("rgb(0,0,0)"));
            // out << "\tctx.fillStyle = 'rgb(0,0,0)';\n";

            break;
          }
          default:
            break;
          }

        } while (body = body->next);
      }
    }
  }

  void edgetoHTML5Path(mp_graphic_object *body, emscripten::val ctx) {

    if (body) {

      ctx.call<void>("beginPath");
      do {
        switch (body->type) {
        case mp_fill_code: {
          filltoHTML5Path(((mp_fill_object *)body)->path_p, ctx);

          break;
        }
        default:
          break;
        }

      } while (body = body->next);

      ctx.call<void>("fill");
    }
  }

  void generateGlyph(GlyphVis &glyph, emscripten::val ctx) {

    if (glyph.name == "endofaya") { //||  glyph->name == "rubelhizb" glyph->name
                                    //== "placeofsajdah" ||
      getImageStream(glyph, ctx);
    } else if (glyph.charcode >= Automedina::AyaNumberCode &&
               glyph.charcode <= Automedina::AyaNumberCode + 286) {
      int ayaNumber = (glyph.charcode - Automedina::AyaNumberCode) + 1;

      int digitheight = 120;

      // out << "\tglyphs['endofaya'](ctx);\n";
      auto ayaGlyph = &layout->glyphs["endofaya"];
      generateGlyph(*ayaGlyph, ctx);

      // out << "\tctx.save();\n";
      ctx.call<void>("save");

      if (ayaNumber < 10) {
        auto &onesglyph =
            layout->glyphs[layout->glyphNamePerCode[1632 + ayaNumber]];

        auto position =
            layout->glyphs["endofaya"].width / 2 - (onesglyph.width) / 2;

        // out << "\tctx.translate(" << position << "," << digitheight <<
        // ");\n";
        ctx.call<void>("translate", position, digitheight);

        // out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";
        auto tempGlyph = &layout->glyphs[onesglyph.name];
        generateGlyph(*tempGlyph, ctx);

      } else if (ayaNumber < 100) {
        int onesdigit = ayaNumber % 10;
        int tensdigit = ayaNumber / 10;

        auto &onesglyph =
            layout->glyphs[layout->glyphNamePerCode[1632 + onesdigit]];
        auto &tensglyph =
            layout->glyphs[layout->glyphNamePerCode[1632 + tensdigit]];

        auto position = layout->glyphs["endofaya"].width / 2 -
                        (onesglyph.width + tensglyph.width + 40) / 2;

        // out << "\tctx.translate(" << position << "," << digitheight <<
        // ");\n";
        ctx.call<void>("translate", position, digitheight);

        // out << "\tglyphs['" << tensglyph.name << "'](ctx);\n";
        auto tempGlyph = &layout->glyphs[tensglyph.name];
        generateGlyph(*tempGlyph, ctx);

        // out << "\tctx.translate(" << tensglyph.width + 40 << "," << 0 <<
        // ");\n";
        ctx.call<void>("translate", tensglyph.width + 40, 0);
        // out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";
        tempGlyph = &layout->glyphs[onesglyph.name];
        generateGlyph(*tempGlyph, ctx);

      } else {
        int onesdigit = ayaNumber % 10;
        int tensdigit = (ayaNumber / 10) % 10;
        int hundredsdigit = ayaNumber / 100;

        auto &onesglyph =
            layout->glyphs[layout->glyphNamePerCode[1632 + onesdigit]];
        auto &tensglyph =
            layout->glyphs[layout->glyphNamePerCode[1632 + tensdigit]];
        auto &hundredsglyph =
            layout->glyphs[layout->glyphNamePerCode[1632 + hundredsdigit]];

        auto position =
            layout->glyphs["endofaya"].width / 2 -
            (onesglyph.width + tensglyph.width + hundredsglyph.width + 80) / 2;

        // out << "\tctx.translate(" << position << "," << digitheight <<
        // ");\n";
        ctx.call<void>("translate", position, digitheight);

        // out << "\tglyphs['" << hundredsglyph.name << "'](ctx);\n";
        auto tempGlyph = &layout->glyphs[hundredsglyph.name];
        generateGlyph(*tempGlyph, ctx);

        // out << "\tctx.translate(" << hundredsglyph.width + 40 << "," << 0 <<
        // ");\n";
        ctx.call<void>("translate", hundredsglyph.width + 40, 0);
        // out << "\tglyphs['" << tensglyph.name << "'](ctx);\n";
        tempGlyph = &layout->glyphs[tensglyph.name];
        generateGlyph(*tempGlyph, ctx);

        // out << "\tctx.translate(" << tensglyph.width + 40 << "," << 0 <<
        // ");\n";
        ctx.call<void>("translate", tensglyph.width + 40, 0);
        // out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";
        tempGlyph = &layout->glyphs[onesglyph.name];
        generateGlyph(*tempGlyph, ctx);
      }

      ctx.call<void>("restore");
    } else {
      edgetoHTML5Path(glyph.copiedPath, ctx);
    }
  }

protected:
  std::map<int, double> lineWidths = {
      {601 * 3, 1},     {601 * 4, 1},    {601 * 7, 1},      {601 * 8, 1},
      {601 * 9, 1},     {601 * 10, 1},   {601 * 13, 1},     {601 * 14, 1},
      {601 * 15, 1},    {602 * 5, 0.63}, {602 * 11, 0.9},   {602 * 15, 0.53},
      {603 * 10, 0.66}, {603 * 13, 1},   {603 * 15, 0.60},  {604 * 3, 1},
      {604 * 4, 0.55},  {604 * 7, 1},    {604 * 8, 1},      {604 * 9, 0.55},
      {604 * 12, 1},    {604 * 13, 1},   {604 * 14, 0.675}, {604 * 15, 0.5},
  };

  std::map<int, double> madinaLineWidths = {
      {586 * 1, 0.81},
      {593 * 2, 0.81},
      {594 * 5, 0.63},
      {600 * 10, 0.63},
  };

  double getWidthRatio(int pageIndex, int lineIndex, bool texFormat) {

    if (texFormat) {
      int pageDiff = static_cast<int>(texPages.size()) - 604;
      pageIndex = pageIndex - pageDiff;
    }

    int key = (pageIndex + 1) * (lineIndex + 1);
    double ratio = 1;
    if (lineWidths.contains(key)) {
      ratio = lineWidths.at(key);
    } else {
      if (!texFormat && madinaLineWidths.contains(key)) {
        ratio = madinaLineWidths.at(key);
      }
    }

    return ratio;
  }
};

// Browser-facing Mushaf API backed directly by OtLayout's full-page
// LineToJustify overload. This deliberately stays separate from QuranShaper's
// legacy, line-at-a-time browser contract: it mirrors the shaping portion of
// LayoutWindow::shapeMushaf/generateMushaf and returns native line/glyph data
// for a web renderer.
class OtLayoutMushaf : public QuranShaper {
public:
  OtLayoutMushaf() = default;
  explicit OtLayoutMushaf(const std::string &fontFile) try
      : QuranShaper(fontFile) {
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string{"QuranShaper construction: "} +
                             e.what());
  }

  PageResult shapeMushafPage(int pageIndex, float fontScalePerc,
                             bool tajweedColor, bool applyForce,
                             const std::vector<digitalkhatt::TextString> &lines,
                             const std::vector<double> &widthRatios,
                             const std::vector<int> &lineTypes) {
    if (pageIndex < 0 || lines.empty() || widthRatios.size() != lines.size() ||
        lineTypes.size() != lines.size()) {
      return {};
    }

    std::vector<LineToJustify> linesToJustify;
    linesToJustify.reserve(lines.size());

    for (int lineIndex = 0; lineIndex < static_cast<int>(lines.size());
         ++lineIndex) {
      int width = pageWidth;
      auto justification = LineJustification::Distribute;
      auto lineType = static_cast<LineType>(lineTypes[lineIndex]);

      bool basmalaOnFirstPages =
          (pageIndex == 0 || pageIndex == 1) && lineIndex == 1;

      if (basmalaOnFirstPages) {
        lineType = LineType::Bism;
      } else if (lineType == LineType::Sura || lineType == LineType::Bism) {
        width = 0;
        justification = LineJustification::Center;
      }


      const double widthRatio = widthRatios[lineIndex];
      if (widthRatio < 1.0) {
        width = static_cast<int>(pageWidth * widthRatio);
        justification = LineJustification::Center;
      }

      linesToJustify.push_back({lines[lineIndex], width, justification,
                                lineType, basmalaOnFirstPages});
    }

    const double emScale = (1 << OtLayout::SCALEBY) * fontScalePerc;
    auto page = layout->justifyPage(
        emScale, pageWidth, linesToJustify, pageIndex == 0, tajweedColor,
        HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES,
        {JustType::Experimental2, JustStyle::XScale, ShrinkType::Standard},
        "qpc_v2_layout");

    if (pageIndex == 0 || pageIndex == 1) {
      for (std::size_t i = 1; i < page.size(); ++i) {
        page[i].ystartposition += 3000 << OtLayout::SCALEBY;
      }
    }

    PageResult result{std::move(page), lines};
    if (applyForce) {
      optimizePage(result.page, emScale);
    }
    return result;
  }

  int mushafPageWidth() const { return pageWidth; }
  int scaleBy() const { return OtLayout::SCALEBY; }

private:
  void optimizePage(std::vector<LineLayoutInfo> &page, double emScale) {
    using namespace geometry;

    std::unordered_map<GlyphVis *, GeometrySet> glyphToPolys;
    const auto &classes = layout->glyphClasses();
    const auto &marks = digitalkhatt::layout::classesOrEmpty(classes, "marks");
    const auto &topmarks =
        digitalkhatt::layout::classesOrEmpty(classes, "topmarks");
    const auto &lowmarks =
        digitalkhatt::layout::classesOrEmpty(classes, "lowmarks");
    const auto &waqfmarks =
        digitalkhatt::layout::classesOrEmpty(classes, "waqfmarks");
    const auto &topdotmarks =
        digitalkhatt::layout::classesOrEmpty(classes, "topdotmarks");
    const auto &downdotmarks =
        digitalkhatt::layout::classesOrEmpty(classes, "downdotmarks");

    auto isTopMark = [&](const std::string &name) {
      return topmarks.contains(name) || waqfmarks.contains(name) ||
             topdotmarks.contains(name);
    };

    std::vector<std::vector<digitalkhatt::layout::GlyphInstance>> pageGlyphs;
    pageGlyphs.reserve(page.size());
    for (int lineIndex = 0; lineIndex < static_cast<int>(page.size());
         ++lineIndex) {
      auto &line = page[lineIndex];
      auto &lineGlyphs = pageGlyphs.emplace_back();
      lineGlyphs.reserve(line.glyphs.size());

      const auto xScale = line.fontSize * line.xscale;
      const auto yScale = line.fontSize;
      int currentX = -line.xstartposition;
      int currentY =
          -(line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY));
      digitalkhatt::layout::GlyphInstance *currentBase = nullptr;
      digitalkhatt::layout::GlyphInstance *previousBase = nullptr;

      for (int glyphIndex = 0;
           glyphIndex < static_cast<int>(line.glyphs.size()); ++glyphIndex) {
        auto &glyphLayout = line.glyphs[glyphIndex];
        const auto &glyphName = layout->glyphNamePerCode[glyphLayout.codepoint];
        auto *glyphVis = layout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});

        auto glyphToPoly = glyphToPolys.find(glyphVis);
        if (glyphToPoly == glyphToPolys.end()) {
          auto cubics = getGlyphCubic(glyphVis->copiedPath);
          auto geometry =
              marks.contains(glyphName)
                  ? buildPolyFromCubics(cubics, CUBIC_FLATNESS_TOLERANCE)
                  : buildConvexPartsFromCubics(cubics,
                                               CUBIC_FLATNESS_TOLERANCE);
          glyphToPoly =
              glyphToPolys.emplace(glyphVis, geometry.scaled(emScale, emScale))
                  .first;
        }

        currentX -= glyphLayout.x_advance * line.xscale;
        auto &glyph = lineGlyphs.emplace_back();
        glyph.isMark = marks.contains(glyphName);
        glyph.isTopMark = isTopMark(glyphName);
        glyph.lineY = currentY;
        glyph.baseX = currentX + glyphLayout.x_offset * line.xscale;
        glyph.baseY = currentY + glyphLayout.y_offset;
        glyph.glyphLayout = &glyphLayout;
        glyph.metrics = {glyphVis->width, glyphVis->height, glyphVis->bbox.llx,
                         glyphVis->bbox.urx};
        glyph.glyphName = glyphName;
        glyph.lineIndex = lineIndex;
        glyph.glyphIndex = glyphIndex;
        if (xScale == 1 && yScale == 1) {
          glyph.geom = &glyphToPoly->second;
        } else {
          glyph.geomScaled = glyphToPoly->second.scaled(xScale, yScale);
        }
        glyph.prevBase = currentBase;
        if (!glyph.isMark) {
          previousBase = currentBase;
          currentBase = &glyph;
          if (previousBase)
            previousBase->nextBase = currentBase;
        }
      }
    }

    auto solverClasses = classes;
    if (!solverClasses.contains("bowlbases")) {
      solverClasses["bowlbases"] = {"hah.isol", "hah.fina", "ain.fina"};
    }
    digitalkhatt::layout::OptParams solverParams;
    digitalkhatt::layout::optimizePage(pageGlyphs, solverClasses, solverParams);

    for (int lineIndex = 0; lineIndex < static_cast<int>(page.size());
         ++lineIndex) {
      for (int glyphIndex = 0;
           glyphIndex < static_cast<int>(page[lineIndex].glyphs.size());
           ++glyphIndex) {
        auto &glyphLayout = page[lineIndex].glyphs[glyphIndex];
        const auto &glyph = pageGlyphs[lineIndex][glyphIndex];
        glyphLayout.x_offset += glyph.dx;
        glyphLayout.y_offset += glyph.dy;
      }
    }
  }
};
