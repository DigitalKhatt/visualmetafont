#include <digitalkhatt/justify/FeatureJustifier.h>

#include <digitalkhatt/core/Regex16.h>
#include <digitalkhatt/core/tajweed/tajweed_service.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "hb-buffer.hh"
#include "hb-font.hh"

namespace digitalkhatt::justify {
namespace {

using digitalkhatt::makeRegex16;
using digitalkhatt::Regex16;
using digitalkhatt::Regex16Match;
using digitalkhatt::replaceAll;
using digitalkhatt::TextString;
using digitalkhatt::TextView;
using std::map;
using std::max;
using std::min;
using std::set;
using std::vector;

bool contains(std::u16string_view text, char16_t character) {
  return text.find(character) != std::u16string_view::npos;
}

std::string shrinkFeatureName(int index) {
  std::ostringstream stream;
  stream << "sk" << std::setw(2) << std::setfill('0') << index;
  return stream.str();
}

// Drops every occurrence of any character in charsToDrop, mirroring
// QString::remove(QRegularExpression("[...]")) / QString::remove(QChar) on a
// plain character class (no quantifiers involved).
TextString removeChars(TextView text, TextView charsToDrop) {
  TextString result;
  result.reserve(text.size());
  for (char16_t ch : text) {
    if (charsToDrop.find(ch) == TextView::npos) result.push_back(ch);
  }
  return result;
}

// Drops every occurrence of the literal substring, mirroring QString::replace(substr, "").
TextString removeSubstring(TextView text, TextView substr) {
  TextString result{text};
  size_t pos = result.find(substr);
  while (pos != TextString::npos) {
    result.erase(pos, substr.size());
    pos = result.find(substr, pos);
  }
  return result;
}

// Mirrors QString::arg(...): each call substitutes every occurrence of the
// next %N marker (so a repeated marker like "%1 ... %1" gets the same value
// everywhere), letting the kashida/alternate patterns below be assembled from
// the same named character-class variables the original Qt code used.
TextString formatPattern(TextView tmpl, TextView a1) {
  return replaceAll(TextString(tmpl), u"%1", a1);
}
TextString formatPattern(TextView tmpl, TextView a1, TextView a2) {
  return replaceAll(formatPattern(tmpl, a1), u"%2", a2);
}
TextString formatPattern(TextView tmpl, TextView a1, TextView a2, TextView a3) {
  return replaceAll(formatPattern(tmpl, a1, a2), u"%3", a3);
}

static const int FONTSIZE = 1000;

enum class SpaceType {
  Simple = 1,
  Aya,
};

enum class StretchType {
  None = 0,
  Beh = 1,
  FinaAscendant = 2,
  OtherKashidas = 3,
  Kaf = 4,
  SecondKashidaNotSameSubWord = 5,
  SecondKashidaSameSubWord = 6,
  BehNonGreedy = 7,
};

enum class AppliedResult {
  NoChange,
  Positive,
  Overflow,
  Forbiden,
};

struct SubWordCharIndex {
  int subWordIndex;
  int characterIndexInSubWord;
};

struct LayoutResult {
  double parWidth;
  map<StretchType, SubWordCharIndex> appliedKashidas;
};

struct SubWordInfo {
  vector<int> baseIndexes;
  TextString baseText;
};

struct WordInfo {
  TextString text;
  TextString baseText;
  int startIndex;
  int endIndex;
  vector<int> baseIndexes;
  vector<SubWordInfo> subwords = {{}};
};

struct LineTextInfo {
  TextString lineText;
  vector<int> ayaSpaceIndexes;
  vector<int> simpleSpaceIndexes;
  map<int, SpaceType> spaces;
  vector<WordInfo> wordInfos;
};

struct TextFontFeatures {
  std::string name;
  int value;
};

struct JustResultByLine {
  float sclxAxis = 0;
  vector<TextFontFeatures> globalFeatures = {};
  map<int, vector<TextFontFeatures>> fontFeatures = {}; /* FontFeatures by character index in the line */
  double simpleSpacing;
  double ayaSpacing;
  double xScale;
  bool isShrink = false;
  double addedSpaceAfterShrink = 0.0;
};

struct JustInfo {
  map<int, vector<TextFontFeatures>> fontFeatures = {};
  double desiredWidth;
  double textLineWidth;
  vector<LayoutResult> layoutResult;
  hb_font_t* font;
};

static const TextString rightNoJoinLetters = u"آاٱأإدذرزوؤءة";
static const TextString dualJoinLetters = u"بتثجحخسشصضطظعغفقكلمنهيئى";

static set<char16_t> bases{};

void initBases() {
  for (int i = 0; i < static_cast<int>(dualJoinLetters.size()); i++) {
    bases.insert(dualJoinLetters.at(i));
  }
  for (int i = 0; i < static_cast<int>(rightNoJoinLetters.size()); i++) {
    bases.insert(rightNoJoinLetters.at(i));
  }
}

static hb_segment_properties_t savedprops{
    HB_DIRECTION_RTL,
    HB_SCRIPT_ARABIC,
    hb_language_from_string("ar", strlen("ar")),
    0,
    0};

static LineTextInfo analyzeLineForJust(TextString lineText) {
  if (bases.size() == 0) {
    initBases();
  }

  LineTextInfo lineTextInfo = {
      .lineText = lineText,
      .ayaSpaceIndexes = {},
      .simpleSpaceIndexes = {},
      .spaces = {},
      .wordInfos = {}};

  lineTextInfo.wordInfos.push_back({});
  WordInfo* currentWord = &lineTextInfo.wordInfos.back();
  currentWord->startIndex = 0;
  currentWord->endIndex = -1;

  for (int i = 0; i < static_cast<int>(lineText.size()); i++) {
    char16_t qchar = lineText.at(i);
    if (qchar == ' ') {
      if ((i > 0 && lineText.at(i - 1) >= 0x0660 && lineText.at(i - 1) <= 0x0669) ||
          (i + 1 < static_cast<int>(lineText.size()) && lineText.at(i + 1) == 0x06DD)) {
        lineTextInfo.ayaSpaceIndexes.push_back(i);
        lineTextInfo.spaces.insert({i, SpaceType::Aya});
      } else {
        lineTextInfo.simpleSpaceIndexes.push_back(i);
        lineTextInfo.spaces.insert({i, SpaceType::Simple});
      }
      lineTextInfo.wordInfos.push_back({});
      currentWord = &lineTextInfo.wordInfos.back();
      currentWord->startIndex = i + 1;
      currentWord->endIndex = i;
    } else {
      currentWord->text += qchar;
      if (bases.find(qchar) != bases.end()) {
        currentWord->baseText += qchar;
        currentWord->baseIndexes.push_back(i - currentWord->startIndex);
        if (qchar == U'ء') {
          currentWord->subwords.push_back({.baseIndexes = {}, .baseText = u""});
        }
        auto& subWord = currentWord->subwords.back();
        subWord.baseText += qchar;
        subWord.baseIndexes.push_back(i - currentWord->startIndex);
        if (i < static_cast<int>(lineText.size()) - 1 && qchar != U'ء' && contains(rightNoJoinLetters, qchar)) {
          currentWord->subwords.push_back({.baseIndexes = {}, .baseText = u""});
        }
      }
      currentWord->endIndex++;
    }
  }

  return lineTextInfo;
}

static hb_buffer_t* shape(TextString text, hb_font_t* font, vector<hb_feature_t> features) {
  hb_buffer_t* buffer = buffer = hb_buffer_create();

  hb_buffer_set_segment_properties(buffer, &savedprops);
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
  buffer->justContext = nullptr;
  buffer->useCallback = true;
  buffer->justifyLine = false;

  hb_buffer_add_utf16(buffer, reinterpret_cast<const uint16_t*>(text.data()), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));

  hb_shape(font, buffer, features.data(), features.size());

  return buffer;
}

static double getWidth(const TextString& text, hb_font_t* font, const vector<hb_feature_t>& features) {
  auto buffer = shape(text, font, features);

  double totalWidth = 0.0;

  unsigned int glyph_count;

  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  for (int i = 0; i < static_cast<int>(glyph_count); i++) {
    totalWidth += glyph_pos[i].x_advance;
  }

  hb_buffer_destroy(buffer);

  return totalWidth;
}

static double getWordWidth(const WordInfo& wordInfo, const map<int, vector<TextFontFeatures>>& justResults, hb_font_t* font) {
  vector<hb_feature_t> features{};

  for (int i = wordInfo.startIndex; i <= wordInfo.endIndex; i++) {
    auto justInfo = justResults.find(i);
    if (justInfo != justResults.end()) {
      for (auto& feat : justInfo->second) {
        features.push_back({hb_tag_from_string(feat.name.c_str(), static_cast<int>(feat.name.size())),
                            (uint32_t)feat.value,
                            (unsigned int)(i - wordInfo.startIndex),
                            (unsigned int)(i - wordInfo.startIndex + 1)});
      }
    }
  }

  return getWidth(wordInfo.text, font, features);
}

static AppliedResult tryApplyFeatures(int wordIndex, const LineTextInfo& lineTextInfo, JustInfo& justInfo, const map<int, vector<TextFontFeatures>>& newFeatures) {
  auto& layout = justInfo.layoutResult[wordIndex];

  const auto& wordInfo = lineTextInfo.wordInfos[wordIndex];

  const auto& wordNewWidth = getWordWidth(wordInfo, newFeatures, justInfo.font);
  auto diff = wordNewWidth - layout.parWidth;
  if (wordNewWidth != layout.parWidth && justInfo.textLineWidth + diff < justInfo.desiredWidth) {
    justInfo.textLineWidth += diff;
    layout.parWidth = wordNewWidth;
    justInfo.fontFeatures = newFeatures;
    return AppliedResult::Positive;
  } else if (diff == 0) {
    return AppliedResult::NoChange;
  } else {
    return AppliedResult::Overflow;
  }
}

struct SubWordsMatch {
  vector<int> subWordIndexes;
  vector<vector<Regex16Match>> matches;
};

static SubWordsMatch matchSubWords(const WordInfo& wordInfo, const vector<Regex16>& regExprs) {
  SubWordsMatch result;

  for (int subIndex = 0; subIndex < static_cast<int>(wordInfo.subwords.size()); subIndex++) {
    const auto& subWord = wordInfo.subwords[subIndex];

    result.matches.push_back({});

    auto& subWordMatches = result.matches.back();

    for (const auto& regExpr : regExprs) {
      int offset = 0;
      while (offset <= static_cast<int>(subWord.baseText.size())) {
        auto match = regExpr.match(subWord.baseText, offset);
        if (!match.hasMatch()) break;
        subWordMatches.push_back(match);
        const int nextOffset = match.end();
        offset = nextOffset > offset ? nextOffset : offset + 1;
      }
    }

    if (subWordMatches.size() > 0) {
      result.subWordIndexes.push_back(subIndex);
    }
  }

  return result;
}

struct Appliedfeature {
  TextFontFeatures feature;
  int (*calcNewValue)(int, int);
};

static vector<TextFontFeatures> mergeFeatures(const vector<TextFontFeatures>& prevFeatures, const vector<Appliedfeature>& newFeatures) {
  vector<TextFontFeatures> mergedFeatures{prevFeatures};

  if (newFeatures.size() > 0) {
    for (auto& newFeature : newFeatures) {
      auto exist = std::find_if(mergedFeatures.begin(), mergedFeatures.end(), [&newFeature](const TextFontFeatures& x) { return x.name == newFeature.feature.name; });

      if (exist != mergedFeatures.end()) {
        exist->value = newFeature.calcNewValue != nullptr ? newFeature.calcNewValue(exist->value, newFeature.feature.value) : newFeature.feature.value;
      } else {
        auto cloneNewFeature = TextFontFeatures{
            .name = newFeature.feature.name,
            .value = newFeature.calcNewValue != nullptr ? newFeature.calcNewValue(0, newFeature.feature.value) : newFeature.feature.value};
        mergedFeatures.push_back(cloneNewFeature);
      }
    }
  }

  return mergedFeatures;
}

static AppliedResult applyAlternate(const LineTextInfo& lineTextInfo, JustInfo& justInfo, int wordIndex, int indexInLine) {
  const auto& lineText = lineTextInfo.lineText;
  auto tempResult{justInfo.fontFeatures};

  vector<TextFontFeatures> prevFeatures;

  AppliedResult appliedResult = AppliedResult::Forbiden;

  auto prevFeaturesIter = tempResult.find(indexInLine);

  if (prevFeaturesIter != tempResult.end()) {
    prevFeatures = prevFeaturesIter->second;

    auto cv02 = std::find_if(prevFeatures.begin(), prevFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv02"; });

    if (cv02 != prevFeatures.end() && cv02->value > 0) {
      return appliedResult;
    }
  }

  auto newFeatures = mergeFeatures(prevFeatures, {Appliedfeature{.feature = {
                                                                     .name = "cv01", .value = 1},
                                                                 .calcNewValue = [](int prev, int curr) {
                                                                   return min(prev + curr, 12);
                                                                 }}});

  tempResult.insert_or_assign(indexInLine, newFeatures);

  int fathaIndex = -1;
  if (indexInLine + 1 < static_cast<int>(lineText.size()) && lineText[indexInLine + 1] == U'\u064E') {
    fathaIndex = indexInLine + 1;
  } else if (indexInLine + 2 < static_cast<int>(lineText.size()) && lineText[indexInLine + 1] == U'\u0651' && lineText[indexInLine + 2] == U'\u064E') {
    fathaIndex = indexInLine + 2;
  }

  if (fathaIndex != -1) {
    double cv01Value = 0;
    auto ff = std::find_if(newFeatures.begin(), newFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
    if (ff != newFeatures.end()) {
      cv01Value = ff->value;
    }

    tempResult.insert_or_assign(fathaIndex, vector<TextFontFeatures>{TextFontFeatures{.name = "cv01", .value = 1 + (int)floor(cv01Value / 3)}});
  }

  appliedResult = tryApplyFeatures(wordIndex, lineTextInfo, justInfo, tempResult);

  return appliedResult;
}

static bool applyAlternatesSubWords(const LineTextInfo& lineTextInfo, JustInfo& justInfo, TextString chars, int nbLevels) {
  const auto& wordInfos = lineTextInfo.wordInfos;

  vector<SubWordsMatch> matchresult;

  // Call sites only ever pass a handful of distinct `chars` values, so cache
  // the compiled regex per value instead of paying for a fresh PCRE2 compile
  // + JIT (Regex16's copy constructor recompiles too, so this must be looked
  // up by reference, never copied out of the cache).
  static map<TextString, vector<Regex16>> regExprAltCache;
  auto cached = regExprAltCache.find(chars);
  if (cached == regExprAltCache.end()) {
    cached = regExprAltCache.emplace(chars, vector<Regex16>{makeRegex16(formatPattern(u"^.*(?<alt>[%1])$", chars))}).first;
  }
  const vector<Regex16>& regExprAlt = cached->second;

  for (int wordIndex = 0; wordIndex < static_cast<int>(wordInfos.size()); wordIndex++) {
    matchresult.push_back(matchSubWords(wordInfos[wordIndex], regExprAlt));
  }

  for (int level = 1; level <= nbLevels; level++) {
    for (int wordIndex = 0; wordIndex < static_cast<int>(wordInfos.size()); wordIndex++) {
      const auto& wordInfo = wordInfos[wordIndex];
      const auto& subWordsMatch = matchresult[wordIndex];

      for (int i = subWordsMatch.subWordIndexes.size() - 1; i >= 0; i--) {
        auto subWordIndex = subWordsMatch.subWordIndexes[i];
        auto matchIndex = subWordsMatch.matches[subWordIndex][0].start("alt");
        auto indexInLine = wordInfo.startIndex + wordInfo.subwords[subWordIndex].baseIndexes[matchIndex];

        auto appliedResult = applyAlternate(lineTextInfo, justInfo, wordIndex, indexInLine);

        if (appliedResult == AppliedResult::Overflow) {
          return true;
        } else if (appliedResult == AppliedResult::Forbiden) {
          continue;
        } else {
          break;
        }
      }
    }
  }
  return false;
}

static const TextString rightKashExp = TextString(u"بتثنيئ") + u"جحخ" + u"سش" + u"صض" + u"طظ" + u"عغ" + u"فق" + u"م" + u"ه";
static const TextString leftKash = TextString(u"ئبتثني") + u"جحخ" + u"طظ" + u"عغ" + u"فق" + u"ةلم" + u"رز";
static const TextString mediLeftAsendant = u"ل";
static const TextString finalAscendant = u"آادذٱأإكلهة";
static const TextString leftKashNoRaZay = removeSubstring(leftKash, u"رز");

static const auto regexBeh = vector<Regex16>{makeRegex16(u"^.+(?<k1>[بتثنيسشصض][بتثنيم]).+$")};
static const auto regexBehNonGreedy = vector<Regex16>{makeRegex16(u"^.+?(?<k1>[بتثنيسشصض][بتثنيم]).+$")};
static const auto regexFinaAscendant = vector<Regex16>{
    makeRegex16(formatPattern(u"^.*(?<k1>[%1][%2])$", rightKashExp + u"ل", finalAscendant))};
static const auto regexOtherKashidas = vector<Regex16>{
    makeRegex16(formatPattern(u".*(?<k1>[%1][رز])", rightKashExp)),
    makeRegex16(formatPattern(u".*(?<k1>[%1](?:[%2]|[%3]))", rightKashExp, mediLeftAsendant, leftKashNoRaZay)),
};
static const auto regexKaf = vector<Regex16>{makeRegex16(u"^.*(?<k1>[ك].).*$")};
static const auto regexSecondKashidaNotSameSubWord = vector<Regex16>{
    makeRegex16(u"^.+(?<k1>[بتثنيسشصض][بتثنيم]).+$"),
    makeRegex16(formatPattern(u"^.*(?<k1>[%1][آادذٱأإكلهة])$", rightKashExp)),
    makeRegex16(formatPattern(u".*(?<k1>[%1][رز])", rightKashExp)),
    makeRegex16(formatPattern(u".*(?<k1>[%1](?:[%2]|[%3]))", rightKashExp, mediLeftAsendant, leftKashNoRaZay)),
};
static const auto regexSecondKashidaSameSubWord = vector<Regex16>{
    makeRegex16(u"^.+(?<k1>[بتثنيسشصض][بتثنيم]).+"),
    makeRegex16(formatPattern(u"(?<k1>[%1][آادذٱأإكلهة])$", rightKashExp)),
    makeRegex16(formatPattern(u"(?<k1>[%1][رز])", rightKashExp)),
    makeRegex16(formatPattern(u"(?<k1>[%1](?:[%2]|[%3]))", rightKashExp, mediLeftAsendant, leftKashNoRaZay)),
};

static void DealWithDecomposition(
    int firstMatchIndex,
    int secondMatchIndex,
    const WordInfo& wordInfo,
    const SubWordInfo& subWordInfo,
    const LineTextInfo& lineTextInfo,
    vector<TextFontFeatures>& secondNewFeatures,
    vector<Appliedfeature>& firstAppliedFeatures) {
  auto firstIndexInLine = wordInfo.startIndex + firstMatchIndex;
  auto secondIndexInLine = wordInfo.startIndex + secondMatchIndex;
  auto& lineText = lineTextInfo.lineText;

  auto chark3 = lineText[firstIndexInLine];
  auto chark4 = lineText[secondIndexInLine];

  if (
      contains(u"ه", chark3) &&
      contains(u"م", chark4) &&
      subWordInfo.baseIndexes.back() == secondMatchIndex) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv11", .value = 1}});
    secondNewFeatures.push_back({.name = "cv11", .value = 1});
  } else if (
      contains(u"بتثنيئ", chark3) &&
      subWordInfo.baseIndexes[0] == firstMatchIndex &&
      contains(u"جحخ", chark4)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv12", .value = 1}});
    secondNewFeatures.push_back({.name = "cv12", .value = 1});
  } else if (
      contains(u"م", chark3) &&
      subWordInfo.baseIndexes[0] == firstMatchIndex &&
      contains(u"جحخ", chark4)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv13", .value = 1}});
    secondNewFeatures.push_back({.name = "cv13", .value = 1});
  } else if (
      contains(u"فق", chark3) &&
      subWordInfo.baseIndexes[0] == firstMatchIndex &&
      contains(u"جحخ", chark4)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv14", .value = 1}});
    secondNewFeatures.push_back({.name = "cv14", .value = 1});
  } else if (
      contains(u"ل", chark3) &&
      subWordInfo.baseIndexes[0] == firstMatchIndex &&
      contains(u"جحخ", chark4)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv15", .value = 1}});
    secondNewFeatures.push_back({.name = "cv15", .value = 1});
  } else if (
      contains(u"عغ", chark3) &&
      subWordInfo.baseIndexes[0] == firstMatchIndex &&
      (contains(u"آادذٱأإل", chark4) ||
       (contains(u"بتثنيئ", chark4) && subWordInfo.baseText.size() > 2 &&
        contains(u"سش", subWordInfo.baseText[2])))) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv16", .value = 1}});
    secondNewFeatures.push_back({.name = "cv16", .value = 1});
  } else if (contains(u"جحخ", chark3)) {
    if (
        contains(u"آادذٱأإل", chark4) ||
        (contains(u"هة", chark4) &&
         subWordInfo.baseIndexes.back() == secondMatchIndex) ||
        (contains(u"بتثنيئ", chark4) &&
         subWordInfo.baseIndexes.size() > 1 &&
         subWordInfo.baseIndexes.end()[-2] == secondMatchIndex &&
         contains(u"رزن", subWordInfo.baseText.back()))) {
      firstAppliedFeatures.push_back({.feature = {.name = "cv16", .value = 1}});
      secondNewFeatures.push_back({.name = "cv16", .value = 1});
    } else if (subWordInfo.baseIndexes[0] == firstMatchIndex && contains(u"م", chark4)) {
      firstAppliedFeatures.push_back({.feature = {.name = "cv18", .value = 1}});
      secondNewFeatures.push_back({.name = "cv18", .value = 1});
    }
  } else if (contains(u"سشصض", chark3) && contains(u"رز", chark4)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv17", .value = 1}});
    secondNewFeatures.push_back({.name = "cv17", .value = 1});
  } else if (contains(u"ل", chark3) && subWordInfo.baseIndexes[0] == firstMatchIndex && contains(u"د", chark4)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv19", .value = 1}});
    secondNewFeatures.push_back({.name = "cv19", .value = 1});
  }
}

static AppliedResult applyKashida(
    const LineTextInfo& lineTextInfo,
    JustInfo& justInfo,
    int wordIndex,
    int subWordIndex,
    int firstSubWordMatchIndex,
    int secondSubWordMacthIndex) {
  auto& wordInfos = lineTextInfo.wordInfos;
  auto& lineText = lineTextInfo.lineText;
  auto& wordInfo = wordInfos[wordIndex];
  auto& subWordInfo = wordInfo.subwords[subWordIndex];
  auto firstMatchIndex = subWordInfo.baseIndexes[firstSubWordMatchIndex];
  auto secondMatchIndex = subWordInfo.baseIndexes[secondSubWordMacthIndex];
  auto firstIndexInLine = wordInfo.startIndex + firstMatchIndex;
  auto secondIndexInLine = wordInfo.startIndex + secondMatchIndex;

  auto chark3 = lineText[firstIndexInLine];
  auto chark4 = lineText[secondIndexInLine];

  map<int, vector<TextFontFeatures>> tempResult{justInfo.fontFeatures};

  auto firstPrevFeatures = tempResult[firstIndexInLine];
  auto secondPrevFeatures = tempResult[secondIndexInLine];

  AppliedResult appliedResult = AppliedResult::Forbiden;

  auto ff = std::find_if(secondPrevFeatures.begin(), secondPrevFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
  if (ff != secondPrevFeatures.end()) return appliedResult;

  if (
      chark4 == U'ق' &&
      subWordInfo.baseIndexes.back() == secondMatchIndex) {
    return appliedResult;
  } else if (
      chark3 == U'ل' &&
      (chark4 == U'ك' ||
       // chark4 == U'د' ||
       // chark4 == U'ذ' ||
       chark4 == U'ة' ||
       (chark4 == U'ه' &&
        subWordInfo.baseIndexes.back() == secondMatchIndex))) {
    return appliedResult;
  } else if (
      contains(u"ئبتثنيى", chark3) &&
      subWordInfo.baseIndexes[0] != firstMatchIndex &&
      contains(u"رز", chark4)) {
    return appliedResult;
  }

  vector<TextFontFeatures> secondNewFeatures;

  vector<Appliedfeature> firstAppliedFeatures{Appliedfeature{.feature = {.name = "cv01", .value = 1}, .calcNewValue = [](int prev, int curr) { return min(prev + curr, 6); }}};

  if (contains(u"بتثنيئ", chark3)) {
    firstAppliedFeatures.push_back({.feature = {.name = "cv10", .value = 1}});
  }

  // decomposition

  DealWithDecomposition(firstMatchIndex, secondMatchIndex, wordInfo, subWordInfo, lineTextInfo, secondNewFeatures, firstAppliedFeatures);

  auto firstNewFeatures = mergeFeatures(
      firstPrevFeatures,
      firstAppliedFeatures);

  int cv01Value = 0;

  ff = std::find_if(firstNewFeatures.begin(), firstNewFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
  if (ff != firstNewFeatures.end()) {
    cv01Value = ff->value;
  }

  int cv02Value;

  if (contains(finalAscendant, chark4) && subWordInfo.baseIndexes.back() == secondMatchIndex) {
    cv02Value = cv01Value;
  } else {
    cv02Value = 2 * cv01Value;
  }

  secondNewFeatures.push_back({.name = "cv02", .value = cv02Value});

  tempResult.insert_or_assign(firstIndexInLine, firstNewFeatures);
  tempResult.insert_or_assign(secondIndexInLine, secondNewFeatures);

  appliedResult = tryApplyFeatures(wordIndex, lineTextInfo, justInfo, tempResult);

  return appliedResult;
}

static AppliedResult applyKaf(
    const LineTextInfo& lineTextInfo,
    JustInfo& justInfo,
    int wordIndex,
    int subWordIndex,
    int firstSubWordMatchIndex,
    int secondSubWordMacthIndex) {
  auto& wordInfos = lineTextInfo.wordInfos;
  auto& lineText = lineTextInfo.lineText;
  auto& wordInfo = wordInfos[wordIndex];
  auto& subWordInfo = wordInfo.subwords[subWordIndex];
  auto firstMatchIndex = subWordInfo.baseIndexes[firstSubWordMatchIndex];
  auto secondMatchIndex = subWordInfo.baseIndexes[secondSubWordMacthIndex];

  auto firstIndexInLine = wordInfo.startIndex + firstMatchIndex;
  auto secondIndexInLine = wordInfo.startIndex + secondMatchIndex;

  auto chark4 = lineText[secondIndexInLine];

  map<int, vector<TextFontFeatures>> tempResult{justInfo.fontFeatures};

  auto firstPrevFeatures = tempResult[firstIndexInLine];
  auto secondPrevFeatures = tempResult[secondIndexInLine];

  if (chark4 == U'ن') {
    for (const auto& feature : secondPrevFeatures) {
      if (feature.name == "cv01" && feature.value > 0) {
        return AppliedResult::Forbiden;
      }
    }
  }

  vector<Appliedfeature> firstAppliedFeatures{{.feature = {.name = "cv03", .value = 1}, .calcNewValue = [](int prev, int curr) { return 1; }}};

  tempResult.insert_or_assign(firstIndexInLine, mergeFeatures(firstPrevFeatures, firstAppliedFeatures));

  vector<Appliedfeature> secondAppliedFeatures{{.feature = {.name = "cv03", .value = 1}, .calcNewValue = [](int prev, int curr) { return 1; }}};

  auto firstNewFeatures = mergeFeatures(secondPrevFeatures, secondAppliedFeatures);

  tempResult.insert_or_assign(secondIndexInLine, firstNewFeatures);

  int fathaIndex = -1;

  if (firstIndexInLine + 1 < static_cast<int>(lineText.size()) &&
      lineText[firstIndexInLine + 1] == U'\u064E') {
    fathaIndex = firstIndexInLine + 1;
  } else if (
      firstIndexInLine + 2 < static_cast<int>(lineText.size()) &&
      lineText[firstIndexInLine + 1] == U'\u0651' &&
      lineText[firstIndexInLine + 2] == U'\u064E') {
    fathaIndex = firstIndexInLine + 2;
  }

  if (fathaIndex != -1) {
    int cv01Value = 0;
    auto ff = std::find_if(firstNewFeatures.begin(), firstNewFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });

    if (ff != firstNewFeatures.end()) {
      cv01Value = ff->value;
    }
    tempResult.insert_or_assign(fathaIndex, vector<TextFontFeatures>{{.name = "cv01", .value = 1 + (int)floor(cv01Value / 3)}});
  }

  auto appliedResult = tryApplyFeatures(
      wordIndex,
      lineTextInfo,
      justInfo,
      tempResult);

  return appliedResult;
}

static bool applyKashidasSubWords(
    const LineTextInfo& lineTextInfo,
    JustInfo& justInfo,
    StretchType type,
    int nbLevels) {
  auto& wordInfos = lineTextInfo.wordInfos;
  vector<SubWordsMatch> matchresult;

  vector<Regex16> regExprs;
  switch (type) {
    case StretchType::Beh:
      regExprs = regexBeh;
      break;
    case StretchType::BehNonGreedy:
      regExprs = regexBehNonGreedy;
      type = StretchType::Beh;
      break;
    case StretchType::FinaAscendant:
      regExprs = regexFinaAscendant;
      break;
    case StretchType::OtherKashidas:
      regExprs = regexOtherKashidas;
      break;
    case StretchType::Kaf:
      regExprs = regexKaf;
      break;
    case StretchType::SecondKashidaNotSameSubWord:
      regExprs = regexSecondKashidaNotSameSubWord;
      break;
    case StretchType::SecondKashidaSameSubWord:
      regExprs = regexSecondKashidaSameSubWord;
      break;
    case StretchType::None:
      return false;
  }

  for (int wordIndex = 0; wordIndex < static_cast<int>(wordInfos.size()); wordIndex++) {
    matchresult.push_back(matchSubWords(wordInfos[wordIndex], regExprs));
  }

  for (int level = 1; level <= nbLevels; level++) {
    for (int wordIndex = 0; wordIndex < static_cast<int>(wordInfos.size()); wordIndex++) {
      auto& subWordsMatch = matchresult[wordIndex];
      auto& wordLayout = justInfo.layoutResult[wordIndex];

      auto type1Applied = wordLayout.appliedKashidas.find(StretchType::Beh);
      auto type2Applied = wordLayout.appliedKashidas.find(StretchType::FinaAscendant);
      auto type3Applied = wordLayout.appliedKashidas.find(StretchType::OtherKashidas);
      auto type5Applied = wordLayout.appliedKashidas.find(StretchType::SecondKashidaNotSameSubWord);

      if (type == StretchType::Beh && (type2Applied != wordLayout.appliedKashidas.end() || type3Applied != wordLayout.appliedKashidas.end())) continue;
      if (type == StretchType::FinaAscendant && (type1Applied != wordLayout.appliedKashidas.end() || type3Applied != wordLayout.appliedKashidas.end()))
        continue;
      if (type == StretchType::OtherKashidas && (type1Applied != wordLayout.appliedKashidas.end() || type2Applied != wordLayout.appliedKashidas.end()))
        continue;

      auto done = false;

      for (
          int i = subWordsMatch.subWordIndexes.size() - 1;
          i >= 0 && !done;
          i--) {
        auto subWordIndex = subWordsMatch.subWordIndexes[i];

        for (auto match : subWordsMatch.matches[subWordIndex]) {
          auto firstSubWordMatchIndex = match.start("k1");

          if (firstSubWordMatchIndex == -1) continue;
          auto secondSubWordMacthIndex = firstSubWordMatchIndex + 1;

          if (type == StretchType::SecondKashidaNotSameSubWord) {
            auto type123 = type1Applied != wordLayout.appliedKashidas.end()   ? type1Applied
                           : type2Applied != wordLayout.appliedKashidas.end() ? type2Applied
                           : type3Applied != wordLayout.appliedKashidas.end() ? type3Applied
                                                                              : wordLayout.appliedKashidas.end();
            if (type123 != wordLayout.appliedKashidas.end() && type123->second.subWordIndex == subWordIndex) continue;
          } else if (type == StretchType::SecondKashidaSameSubWord) {
            auto type123 = type1Applied != wordLayout.appliedKashidas.end()   ? type1Applied
                           : type2Applied != wordLayout.appliedKashidas.end() ? type2Applied
                           : type3Applied != wordLayout.appliedKashidas.end() ? type3Applied
                                                                              : wordLayout.appliedKashidas.end();

            if (
                type123 != wordLayout.appliedKashidas.end() &&
                type123->second.subWordIndex == subWordIndex &&
                type123->second.characterIndexInSubWord == firstSubWordMatchIndex)
              continue;
            if (
                type5Applied != wordLayout.appliedKashidas.end() &&
                type5Applied->second.subWordIndex == subWordIndex &&
                type5Applied->second.characterIndexInSubWord == firstSubWordMatchIndex)
              continue;
          }

          AppliedResult appliedResult = AppliedResult::Forbiden;

          if (type == StretchType::Kaf) {
            appliedResult = applyKaf(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
          } else {
            appliedResult = applyKashida(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
          }

          if (appliedResult == AppliedResult::Positive) {
            wordLayout.appliedKashidas.insert_or_assign(type, SubWordCharIndex{subWordIndex, firstSubWordMatchIndex});
          } else if (appliedResult == AppliedResult::Overflow) {
            return true;
          } else if (appliedResult == AppliedResult::Forbiden) {
            continue;
          }

          done = true;

          break;
        }
      }
    }
  }
  return false;
}

static const Regex16 DecomRegExpr = makeRegex16(u"^(حم|لح).+$");

[[maybe_unused]] static bool applyDecomposition(
    const LineTextInfo& lineTextInfo,
    JustInfo& justInfo, const Regex16& regExpr,
    int nbLevels) {
  auto& wordInfos = lineTextInfo.wordInfos;

  for (int wordIndex = 0; wordIndex < static_cast<int>(wordInfos.size()); wordIndex++) {
    const auto& wordInfo = wordInfos[wordIndex];
    for (int subWordIndex = 0; subWordIndex < static_cast<int>(wordInfo.subwords.size()); subWordIndex++) {
      const auto& subWord = wordInfo.subwords[subWordIndex];
      auto match = regExpr.match(subWord.baseText);
      if (!match.hasMatch()) continue;
      auto matchIndex = match.lastCapturedIndex();
      auto firstSubWordMatchIndex = match.start(matchIndex);
      auto secondSubWordMacthIndex = firstSubWordMatchIndex + 1;
      auto appliedResult = applyKashida(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
      if (appliedResult == AppliedResult::Overflow) return true;
    }
  }
  return false;
}

static const TextString rightChars = dualJoinLetters;
static const TextString leftChars = dualJoinLetters + removeChars(rightNoJoinLetters, u"ء");
static const TextString rightKash = removeChars(rightChars, u"لك");
static const TextString leftKashidaFina = removeChars(leftChars, u"وهصضطظ");
static const TextString leftKashidaMedi = removeChars(leftKashidaFina, u"ه");
static const TextString jhk = u"جحخ";

static const TextString altFinPat = u"^.*([بتثفكنصضسشقيئى])$";
static const Regex16 altFinaPrio1Reg = makeRegex16(altFinPat);
static const TextString finalKashidaEndWord = formatPattern(u"^.*([%1][آاٱأإملهة])$", rightKash);
static const TextString finalKashida = formatPattern(u"^.*([%1][دذآاٱأإملهة])$", rightKash);
static const TextString hahKashida = formatPattern(u"^.*([%1][%2]).*$|^.*([%1][هة])$", jhk, leftKashidaMedi);
static const Regex16 regHahFinaAscenKashida = makeRegex16(hahKashida + u"|" + finalKashidaEndWord);
static const TextString behBehPat = u"^.+([بتثنيسشصض][بتثنيم]).+$";
static const TextString rehPat = formatPattern(u".*([%1][رز])", rightKash);
static const TextString otherPat = formatPattern(u".*([%1](?:[%2]|[%3]))", rightKash, mediLeftAsendant, leftKashidaMedi);
static const TextString kafPat = u"^.*([ك].).*$";
static const TextString patternAlt =
    altFinPat + u"|" + hahKashida + u"|" + finalKashida + u"|" + behBehPat + u"|" + rehPat + u"|" + otherPat + u"|" + kafPat;
static const Regex16 regExprAlt = makeRegex16(patternAlt);

static bool applySimpleJust(const LineTextInfo& lineTextInfo,
                            JustInfo& justInfo,
                            bool firstWordIncluded,
                            bool wordByWord,
                            int nbLevelAlt,
                            int nbLevelKashida) {
  auto& wordInfos = lineTextInfo.wordInfos;
  struct SubWordMatch {
    int subWordIndex;
    Regex16Match match;
    int type;
  };

  vector<SubWordMatch> matchresult;

  auto firstWordIndex = firstWordIncluded ? 0 : 1;

  for (int wordIndex = 0; wordIndex < static_cast<int>(wordInfos.size()); wordIndex++) {
    auto& wordInfo = wordInfos[wordIndex];
    SubWordMatch result{.subWordIndex = -1, .match = {}, .type = 0};
    if (wordInfo.baseText.empty() || wordIndex < firstWordIndex) {
      matchresult.push_back(result);
      continue;
    }

    auto lastIndex = wordInfo.subwords.size() - 1;
    auto& subWord = wordInfo.subwords[lastIndex];
    auto match = altFinaPrio1Reg.match(subWord.baseText);
    if (match.hasMatch()) {
      result.subWordIndex = lastIndex;
      result.match = match;
      result.type = 1;
    } else if (!contains(u"يئى", wordInfo.baseText.back())) {
      match = regHahFinaAscenKashida.match(subWord.baseText);
      if (match.hasMatch()) {
        result.subWordIndex = lastIndex;
        result.match = match;
        result.type = 2;
      } else {
        for (int subIndex = lastIndex; subIndex >= 0; subIndex--) {
          auto& subWord = wordInfo.subwords[subIndex];
          auto match = regExprAlt.match(subWord.baseText);
          if (match.hasMatch()) {
            result.subWordIndex = subIndex;
            result.match = match;
            result.type = 3;
            break;
          }
        }
      }
    }

    matchresult.push_back(result);
  }

  auto stretchedWords = std::map<int, bool>();

  for (int level = 1; level <= max(nbLevelAlt, nbLevelKashida); level++) {
    for (int wordIndex = wordInfos.size() - 1; wordIndex >= firstWordIndex; wordIndex--) {
      if (stretchedWords.find(wordIndex + 1) != stretchedWords.end()) continue;

      AppliedResult appliedResult = AppliedResult::NoChange;

      auto& wordInfo = wordInfos[wordIndex];
      auto& subWordsMatch = matchresult[wordIndex];

      auto& match = subWordsMatch.match;

      if (!match.hasMatch()) continue;

      auto subWordIndex = subWordsMatch.subWordIndex;

      auto matchIndex = match.lastCapturedIndex();

      if (subWordsMatch.type == 1 || (subWordsMatch.type == 3 && (matchIndex == 1))) {
        // Alternates
        if (level <= nbLevelAlt) {
          auto baseIndex = match.start(matchIndex);
          auto indexInLine = wordInfo.startIndex + wordInfo.subwords[subWordIndex].baseIndexes[baseIndex];
          appliedResult = applyAlternate(lineTextInfo, justInfo, wordIndex, indexInLine);
        }
      } else if (level <= nbLevelKashida) {
        auto firstSubWordMatchIndex = match.start(matchIndex);
        auto secondSubWordMacthIndex = firstSubWordMatchIndex + 1;

        if (matchIndex == 8 && subWordsMatch.type == 3) {
          // Kaf
          appliedResult = applyKaf(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
        } else {
          // Kashidas
          appliedResult = applyKashida(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
        }
      }

      if (appliedResult == AppliedResult::Overflow) {
        return true;
      } else if (appliedResult == AppliedResult::Positive) {
        if (wordByWord) {
          stretchedWords.insert({wordIndex, true});
        }
      }
    }
  }
  return false;
}

static void applyExperimentalJust(const LineTextInfo& lineTextInfo, JustInfo& justInfo) {
  applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Beh, 2) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"بتثكن", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 3) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 2) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئ", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Kaf, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Beh, 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"بتثكن", 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئ", 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"بتثكن", 2) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئبتثكن", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Beh, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئبتثكن", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::SecondKashidaNotSameSubWord, 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::SecondKashidaSameSubWord, 2);
}
static void applyExperimental2Just(const LineTextInfo& lineTextInfo, JustInfo& justInfo) {
  applyKashidasSubWords(lineTextInfo, justInfo, StretchType::BehNonGreedy, 2) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"بتثكن", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Kaf, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 2) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئ", 2) ||
      // applyDecomposition(lineTextInfo, justInfo, DecomRegExpr, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::BehNonGreedy, 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"بتثكن", 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئ", 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"بتثكن", 2) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئبتثكن", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::BehNonGreedy, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 1) ||
      applyAlternatesSubWords(lineTextInfo, justInfo, u"ىصضسشفقيئبتثكن", 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::SecondKashidaNotSameSubWord, 2) ||
      applyKashidasSubWords(lineTextInfo, justInfo, StretchType::SecondKashidaSameSubWord, 2);
}
static void stretchLine(const LineTextInfo& lineTextInfo, JustInfo& justInfo, JustType justType) {
  if (justType == JustType::Madina) {
    applySimpleJust(lineTextInfo, justInfo, true, false, 2, 2);
  } else if (justType == JustType::IndoPak) {
    applySimpleJust(lineTextInfo, justInfo, false, true, 2, 2);
  } else if (justType == JustType::Experimental2) {
    applyExperimental2Just(lineTextInfo, justInfo);
  } else {
    applyExperimentalJust(lineTextInfo, justInfo);
  }
}

static JustResultByLine justifyLine(const LineTextInfo& lineTextInfo, hb_font_t* font, double fontSizeLineWidthRatio,
                                    int spaceWidth,
                                    JustOption justOption,
                                    FeatureJustificationLayout& layout) {
  auto desiredWidth = FONTSIZE / fontSizeLineWidthRatio;

  auto lineText = lineTextInfo.lineText;

  vector<LayoutResult> layOutResult{};

  for (int wordIndex = 0; wordIndex < static_cast<int>(lineTextInfo.wordInfos.size()); wordIndex++) {
    auto& wordInfo = lineTextInfo.wordInfos[wordIndex];

    auto parWidth = getWidth(wordInfo.text, font, {});

    layOutResult.push_back({parWidth, {}});
  }

  auto currentLineWidth = getWidth(lineText, font, {});

  auto diff = desiredWidth - currentLineWidth;

  JustResultByLine result;

  result.xScale = 1;
  result.simpleSpacing = spaceWidth;
  result.ayaSpacing = spaceWidth;

  JustInfo justInfo{.fontFeatures = {}, .desiredWidth = desiredWidth, .textLineWidth = currentLineWidth, .layoutResult = layOutResult, .font = font};

  if (diff > 0) {
    // stretch

    // double maxStretchBySpace = std::min(100.0, spaceWidth * 1);
    // double maxStretchByAyaSpace = std::min(200.0, spaceWidth * 2);
    double maxStretchBySpace = std::max(250 - spaceWidth, 0);
    double maxStretchByAyaSpace = std::max(250 - spaceWidth, 0);

    double maxStretch = maxStretchBySpace * lineTextInfo.simpleSpaceIndexes.size() + maxStretchByAyaSpace * lineTextInfo.ayaSpaceIndexes.size();

    auto stretch = min(desiredWidth - currentLineWidth, maxStretch);
    auto spaceRatio = maxStretch != 0 ? stretch / maxStretch : 0;
    auto stretchBySpace = spaceRatio * maxStretchBySpace;
    auto stretchByByAyaSpace = spaceRatio * maxStretchByAyaSpace;

    double simpleSpaceWidth = spaceWidth + stretchBySpace;
    double ayaSpaceWidth = spaceWidth + stretchByByAyaSpace;

    currentLineWidth += stretch;

    justInfo.textLineWidth = currentLineWidth;

    // stretching

    if (desiredWidth > currentLineWidth) {
      stretchLine(lineTextInfo, justInfo, justOption.justType);
      currentLineWidth = justInfo.textLineWidth;
    }

    if (desiredWidth > currentLineWidth) {
      // full justify with space
      auto addToSpace = (desiredWidth - currentLineWidth) / lineTextInfo.spaces.size();
      simpleSpaceWidth += addToSpace;
      ayaSpaceWidth += addToSpace;
    }

    result.simpleSpacing = simpleSpaceWidth;
    result.ayaSpacing = ayaSpaceWidth;

  } else {
    // shrink
    if (justOption.justStyle == JustStyle::SCLX) {
      float xScale = desiredWidth / currentLineWidth;
      auto ff = layout.createFont(font->x_scale / 1000, false);
      hb_font_set_variation(ff, HB_TAG('S', 'C', 'L', 'X'), xScale * 100);
      auto newCurrentLineWidth = getWidth(lineText, ff, {});

      hb_font_destroy(ff);
      if (newCurrentLineWidth < currentLineWidth) {
        result.sclxAxis = xScale * 100;
        result.xScale = desiredWidth / newCurrentLineWidth;
      }

    } else {
      if (justOption.shrinkType == ShrinkType::Test) {
        for (int i = 0; i < 20; i++) {
          result.globalFeatures.push_back({.name = shrinkFeatureName(i + 1), .value = 1});
        }

        result.isShrink = true;
      } else if (justOption.shrinkType == ShrinkType::Standard) {
        vector<hb_feature_t> features;

        int shrinkFeatureIndex = 1;
        while (currentLineWidth > desiredWidth && shrinkFeatureIndex <= 20) {
          auto featureName = shrinkFeatureName(shrinkFeatureIndex);
          features.push_back({hb_tag_from_string(featureName.c_str(), static_cast<int>(featureName.size())),
                              (uint32_t)1,
                              (unsigned int)0,
                              (unsigned int)-1});
          auto newCurrentLineWidth = getWidth(lineText, font, features);
          if (newCurrentLineWidth < currentLineWidth) {
            currentLineWidth = newCurrentLineWidth;
            result.globalFeatures.push_back({.name = featureName, .value = 1});
          }
          shrinkFeatureIndex++;
        }
        if (currentLineWidth < desiredWidth) {
          result.addedSpaceAfterShrink = (desiredWidth - currentLineWidth) / lineTextInfo.spaces.size();
          result.xScale = 1;
        } else {
          result.xScale = desiredWidth / currentLineWidth;
        }
        result.isShrink = true;
      } else {
        result.xScale = desiredWidth / currentLineWidth;
      }
    }
  }
  result.fontFeatures = justInfo.fontFeatures;

  return result;
}
static std::map<std::string, int> tajweedNameToColor = {
    {"green", 0x00A650FF}, {"tafkim", 0x006694FF}, {"lgray", 0xB4B4B4FF}, {"lkalkala", 0x00ADEFFF}, {"red1", 0xC38A08FF}, {"red2", 0xF47216FF}, {"red3", 0xEC008CFF}, {"red4", 0x8C0000FF}};

static LineLayoutInfo shapeLine(FeatureJustificationLayout& layout, int lineWidth, int pageWidth,
                                const LineTextInfo& lineTextInfo, const JustResultByLine& justResult, bool tajweedColor, double emScale, hb_font_t* font,
                                LineJustification justification, int& currentyPos, digitalkhatt::TajweedMap& tajweedResult) {
  vector<hb_feature_t> features{};

  for (auto& feat : justResult.globalFeatures) {
    features.push_back({hb_tag_from_string(feat.name.c_str(), static_cast<int>(feat.name.size())),
                        (uint32_t)feat.value,
                        (unsigned int)0,
                        (unsigned int)-1});
  }

  for (auto& wordInfo : lineTextInfo.wordInfos) {
    for (int i = wordInfo.startIndex; i <= wordInfo.endIndex; i++) {
      auto justInfo = justResult.fontFeatures.find(i);
      if (justInfo != justResult.fontFeatures.end()) {
        for (auto& feat : justInfo->second) {
          features.push_back({hb_tag_from_string(feat.name.c_str(), static_cast<int>(feat.name.size())),
                              (uint32_t)feat.value,
                              (unsigned int)(i),
                              (unsigned int)(i + 1)});
        }
      }
    }
  }

  auto buffer = shape(lineTextInfo.lineText, font, features);

  unsigned int glyph_count;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  LineLayoutInfo lineLayout;
  int currentlineWidth = 0;

  for (int i = glyph_count - 1; i >= 0; i--) {
    GlyphLayoutInfo glyphLayout;

    glyphLayout.codepoint = glyph_info[i].codepoint;
    glyphLayout.lefttatweel = glyph_info[i].lefttatweel;    // normalToParameter(glyph_info[i].codepoint, glyph_info[i].lefttatweel, true);
    glyphLayout.righttatweel = glyph_info[i].righttatweel;  // normalToParameter(glyph_info[i].codepoint, glyph_info[i].righttatweel, false);
    glyphLayout.cluster = glyph_info[i].cluster;
    glyphLayout.x_advance = glyph_pos[i].x_advance;
    glyphLayout.y_advance = glyph_pos[i].y_advance;
    glyphLayout.x_offset = glyph_pos[i].x_offset;
    glyphLayout.y_offset = glyph_pos[i].y_offset;
    glyphLayout.lookup_index = glyph_pos[i].lookup_index;
    if (tajweedColor) {
      auto color = tajweedResult.find(glyphLayout.cluster);
      if (color != tajweedResult.end()) {
        glyphLayout.color = tajweedNameToColor[color->second];
      } else if (lineTextInfo.lineText[glyphLayout.cluster] == u'\u034F') {
        auto nextIndex = i - 1;
        auto nextCluster = glyph_info[nextIndex].cluster;
        auto currCluster = glyphLayout.cluster + 1;
        if (nextCluster > static_cast<uint32_t>(currCluster)) {
          auto color = tajweedResult.find(currCluster);
          if (color != tajweedResult.end()) {
            // happens only in لِيَسُ͏ࣳٓـٔ͏ُوا۟ page 282 line 14
            // console.log(`cgi***************************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
            glyphLayout.color = tajweedNameToColor[color->second];
          }
          /* debug
          if (nextCluster - currCluster > 1 && lineText[glyph.Cluster + 2] !== "\u034Fu") {
            console.log(`nextCluster - currCluster > 1**********************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
          }*/
        } /*else {
          // happens only for فَٱدَّٰرَٰ͏ْٔتُمْ page 11 line 5 no tajweed coloring so it is OK
          //console.log(`nextCluster<=currCluster**********************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
        }  */
      }
    } else {
      glyphLayout.color = 0;
    }
    glyphLayout.subtable_index = glyph_pos[i].subtable_index;
    glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;

    glyphLayout.beginsajda = false;
    glyphLayout.endsajda = false;

    if (!justResult.isShrink) {
      auto space = lineTextInfo.spaces.find(glyphLayout.cluster);

      if (space != lineTextInfo.spaces.end()) {
        if (space->second == SpaceType::Aya) {
          glyphLayout.x_advance = justResult.ayaSpacing * emScale;
        } else if (space->second == SpaceType::Simple) {
          glyphLayout.x_advance = justResult.simpleSpacing * emScale;
        }
      }
    } else if (justResult.addedSpaceAfterShrink != 0) {
      auto space = lineTextInfo.spaces.find(glyphLayout.cluster);

      if (space != lineTextInfo.spaces.end()) {
        glyphLayout.x_advance += justResult.addedSpaceAfterShrink;
      }
    }

    currentlineWidth += glyphLayout.x_advance;

    lineLayout.glyphs.push_back(glyphLayout);
  }

  lineLayout.desiredLineWidth = lineWidth;
  lineLayout.currentLineWidth = currentlineWidth;

  lineLayout.overfull = lineWidth != 0 ? currentlineWidth - lineWidth : 0;

  if (justification == LineJustification::Distribute) {
    lineLayout.xstartposition = 0;
  } else {
    lineLayout.xstartposition = (pageWidth - currentlineWidth) / 2;
  }

  lineLayout.ystartposition = currentyPos;
  lineLayout.fontSize = emScale;

  currentyPos = currentyPos + (layout.interLineSpacing() << layout.scaleBy());

  hb_buffer_destroy(buffer);

  return lineLayout;
}

}  // namespace

std::vector<LineLayoutInfo> FeatureJustifier::justifyPageUsingFeatures(
    double emScale, int pageWidth, const std::vector<LineToJustify>& lines,
    bool newFace, bool tajweedColor,
    hb_buffer_cluster_level_t clusterLevel, JustOption justOption,
    const std::string& mushafLayout) const {
  vector<LineLayoutInfo> page;

  hb_font_t* defaultShapefont = layout_.createFont(emScale, newFace);
  hb_font_t* justifyFont = layout_.createFont(1, false);

  vector<LineTextInfo> linesTextInfo;
  vector<double> fontSizeRatios;

  int spaceWidth = getWidth(u" ", justifyFont, {});

  auto minRatio = 1000.0;
  auto maxRatio = 0.00000001;

  digitalkhatt::PageTajweedResult tajweedResults(lines.size());

  if (tajweedColor) {
    // digitalkhatt::TajweedService tajweedService;
    digitalkhatt::tajweed::TajweedService tajweedService;
    tajweedResults = tajweedService.applyTajweedByPage(lines, mushafLayout.find("indopak") != std::string::npos);
  }

  for (auto lineIdx = 0; lineIdx < static_cast<int>(lines.size()); lineIdx++) {
    auto& line = lines[lineIdx];

    if (line.width != 0) {
      auto fontSizeLineWidthRatio = (double)FONTSIZE * emScale / line.width;

      auto lineTextInfo = analyzeLineForJust(line.text);
      linesTextInfo.push_back(lineTextInfo);
      auto lineWidthUPEM = FONTSIZE / fontSizeLineWidthRatio;

      auto lineWidthRatio = 1;

      auto desiredWidth = lineWidthRatio * lineWidthUPEM;

      auto currentLineWidth = getWidth(lineTextInfo.lineText, justifyFont, {});

      auto ratio = desiredWidth / currentLineWidth;
      fontSizeRatios.emplace_back(ratio);

      minRatio = min(ratio, minRatio);
      maxRatio = max(ratio, maxRatio);
    } else {
      fontSizeRatios.emplace_back(1);
    }
  }

  int currentyPos = layout_.topSpace() << layout_.scaleBy();

  double fontRatio;

  if (justOption.justStyle == JustStyle::SameSizeByPage) {
    fontRatio = std::min(minRatio, 1.0);
  } else if (justOption.justStyle == JustStyle::FontSizeXScale) {
    double maxThresholdRatio = 1.2;
    double minThresholdRatio = 0.95;
    double maxStretchRatio = 0.02;
    double maxShrinkRatio = 0.02;
    if (maxRatio > maxThresholdRatio && minRatio < minThresholdRatio) {
      // very inconsistent, keep font size
      fontRatio = 1;
    } else if (maxRatio <= maxThresholdRatio && minRatio >= minThresholdRatio) {
      // within tolerance threshold, keep font size
      fontRatio = 1;
    } else if (maxRatio > maxThresholdRatio) {
      double diff = std::min(maxRatio - maxThresholdRatio, maxStretchRatio);
      diff = std::min(diff, minRatio - minThresholdRatio);
      fontRatio = 1 + diff;
    } else {
      double diff = std::min(minThresholdRatio - minRatio, maxShrinkRatio);
      diff = std::min(diff, maxThresholdRatio - maxRatio);
      fontRatio = 1 - diff;
    }
  } else {
    fontRatio = 1;
  }

  for (auto lineIdx = 0; lineIdx < static_cast<int>(lines.size()); lineIdx++) {
    auto& line = lines[lineIdx];

    auto lineTextInfo = analyzeLineForJust(line.text);

    auto fontSizeLineWidthRatio = line.width != 0 ? (double)FONTSIZE * emScale / line.width : 1;

    // auto defaultFontRatio = justOption.justStyle == JustStyle::SameSizeByPage ? 1.0 : 1.0;  // (minRatio + maxRatio) / 2

    JustResultByLine justResultByLine;

    if (line.lineType == LineType::Bism) {
      justResultByLine.globalFeatures.push_back({line.basm2 ? "bism" : "basm", 1});
      justResultByLine.ayaSpacing = spaceWidth;
      justResultByLine.simpleSpacing = spaceWidth;
      justResultByLine.xScale = 1;
    } else if (line.lineType == LineType::Sura) {
      justResultByLine.ayaSpacing = spaceWidth;
      justResultByLine.simpleSpacing = spaceWidth;
      justResultByLine.xScale = 1;
    } else {
      justResultByLine = justifyLine(lineTextInfo, justifyFont, fontSizeLineWidthRatio * fontRatio, spaceWidth, justOption, layout_);
    }

    hb_font_t* shapeFont = defaultShapefont;
    auto newEmScale = emScale;
    auto newFont = false;
    if (fontRatio != 1) {
      newFont = true;
      newEmScale = emScale * fontRatio;
      shapeFont = layout_.createFont(newEmScale, false);
    } else if (justOption.justStyle == JustStyle::FontSize && justResultByLine.xScale != 1) {
      newFont = true;
      newEmScale = emScale * justResultByLine.xScale;
      justResultByLine.xScale = 1;
      shapeFont = layout_.createFont(newEmScale, false);
    }
    if (justResultByLine.sclxAxis != 0) {
      if (!newFont) {
        newFont = true;
        shapeFont = layout_.createFont(emScale, false);
      }
      hb_font_set_variation(shapeFont, HB_TAG('S', 'C', 'L', 'X'), justResultByLine.sclxAxis);
    }

    auto lineLayoutInfo = shapeLine(layout_, line.width, pageWidth, lineTextInfo, justResultByLine, tajweedColor, newEmScale, shapeFont, line.lineJustification, currentyPos, tajweedResults[lineIdx]);
    lineLayoutInfo.type = line.lineType;

    if (newFont) {
      hb_font_destroy(shapeFont);
    }
    if (justOption.justStyle == JustStyle::SCLX) {
      lineLayoutInfo.fontSize = lineLayoutInfo.fontSize * justResultByLine.xScale;
      lineLayoutInfo.xscale = 1;
      lineLayoutInfo.xscaleparameter = static_cast<double>(justResultByLine.sclxAxis);
    } else if (lineLayoutInfo.type == LineType::Line) {
      if (justOption.justStyle == JustStyle::XScale) {
        lineLayoutInfo.xscale = justResultByLine.xScale;
      } else {
        lineLayoutInfo.xscale = 1;
      }
    }

    page.push_back(lineLayoutInfo);
  }

  hb_font_destroy(defaultShapefont);
  hb_font_destroy(justifyFont);

  return page;
}

}  // namespace digitalkhatt::justify
