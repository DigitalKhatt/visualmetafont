#define NOMINMAX
#include "OtLayout.h"
#include  <algorithm>
#include "hb-buffer.hh"
#include <qregularexpression.h>

#include "hb-font.hh"
#include "tajweed.h"


using namespace std;

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
  SecondKashidaSameSubWord = 6
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
  map<StretchType, SubWordCharIndex > appliedKashidas;
};


struct SubWordInfo {
  vector<int> baseIndexes;
  QString baseText;
};


struct WordInfo {
  QString text;
  QString baseText;
  int startIndex;
  int endIndex;
  vector<int> baseIndexes;
  vector<SubWordInfo>  subwords = { {} };
};

struct LineTextInfo {
  QString lineText;
  vector<int> ayaSpaceIndexes;
  vector<int> simpleSpaceIndexes;
  map<int, SpaceType> spaces;
  vector<WordInfo> wordInfos;
};

struct TextFontFeatures {
  QString name;
  int value;
};

struct JustResultByLine {
  float sclxAxis = 0;
  vector<TextFontFeatures> globalFeatures = {};
  map<int, vector<TextFontFeatures>> fontFeatures = {}; /* FontFeatures by character index in the line */
  double simpleSpacing;
  double ayaSpacing;
  double xScale;
};

struct JustInfo {
  map<int, vector<TextFontFeatures>> fontFeatures = {};
  double desiredWidth;
  double textLineWidth;
  vector<LayoutResult> layoutResult;
  hb_font_t* font;
};


static const QString rightNoJoinLetters = "آاٱأإدذرزوؤءة";
static const QString dualJoinLetters = "بتثجحخسشصضطظعغفقكلمنهيئى";

static set<QChar> bases{};

void initBases() {
  for (int i = 0; i < dualJoinLetters.size(); i++) {
    bases.insert(dualJoinLetters.at(i));
  }
  for (int i = 0; i < rightNoJoinLetters.size(); i++) {
    bases.insert(rightNoJoinLetters.at(i));
  }
}

static hb_segment_properties_t savedprops{
  HB_DIRECTION_RTL,
  HB_SCRIPT_ARABIC,
  hb_language_from_string("ar", strlen("ar")),
  0,
  0
};

static LineTextInfo analyzeLineForJust(QString lineText) {

  if (bases.size() == 0) {
    initBases();
  }

  LineTextInfo lineTextInfo = {
    .lineText = lineText,
    .ayaSpaceIndexes = {},
    .simpleSpaceIndexes = {},
    .spaces = {},
    .wordInfos = {}
  };



  lineTextInfo.wordInfos.push_back({});
  WordInfo* currentWord = &lineTextInfo.wordInfos.back();
  currentWord->startIndex = 0;
  currentWord->endIndex = -1;

  for (int i = 0; i < lineText.size(); i++) {
    QChar qchar = lineText.at(i);
    if (qchar == ' ') {

      if ((i > 0 && lineText.at(i - 1) >= 0x0660 && lineText.at(i - 1) <= 0x0669) || (lineText.at(i + 1) == 0x06DD)) {
        lineTextInfo.ayaSpaceIndexes.push_back(i);
        lineTextInfo.spaces.insert({ i, SpaceType::Aya });
      }
      else {
        lineTextInfo.simpleSpaceIndexes.push_back(i);
        lineTextInfo.spaces.insert({ i, SpaceType::Simple });
      }
      lineTextInfo.wordInfos.push_back({});
      currentWord = &lineTextInfo.wordInfos.back();
      currentWord->startIndex = i + 1;
      currentWord->endIndex = i;
    }
    else {
      currentWord->text += qchar;
      if (bases.find(qchar) != bases.end()) {
        currentWord->baseText += qchar;
        currentWord->baseIndexes.push_back(i - currentWord->startIndex);
        if (qchar == U'ء') {
          currentWord->subwords.push_back({ .baseIndexes = {}, .baseText = "" });
        }
        auto& subWord = currentWord->subwords.back();
        subWord.baseText += qchar;
        subWord.baseIndexes.push_back(i - currentWord->startIndex);
        if (i < lineText.size() - 1 && qchar != U'ء' && rightNoJoinLetters.contains(qchar)) {
          currentWord->subwords.push_back({ .baseIndexes = {}, .baseText = "" });
        }

      }
      currentWord->endIndex++;
    }
  }


  return lineTextInfo;
}

static hb_buffer_t* shape(QString text, hb_font_t* font, vector< hb_feature_t> features) {
  hb_buffer_t* buffer = buffer = hb_buffer_create();


  hb_buffer_set_segment_properties(buffer, &savedprops);
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
  buffer->justContext = nullptr;
  buffer->useCallback = true;
  buffer->justifyLine = false;

  hb_buffer_add_utf16(buffer, text.utf16(), text.size(), 0, text.size());

  hb_shape(font, buffer, features.data(), features.size());

  return buffer;
}

static double getWidth(const QString& text, hb_font_t* font, const vector< hb_feature_t>& features) {

  auto buffer = shape(text, font, features);

  double totalWidth = 0.0;

  unsigned int glyph_count;

  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  for (int i = 0; i < glyph_count; i++) {
    totalWidth += glyph_pos[i].x_advance;
  }

  hb_buffer_destroy(buffer);

  return totalWidth;
}

static double getWordWidth(const WordInfo& wordInfo, const map<int, vector<TextFontFeatures>>& justResults, hb_font_t* font) {

  vector< hb_feature_t> features{};

  for (int i = wordInfo.startIndex; i <= wordInfo.endIndex; i++) {

    auto justInfo = justResults.find(i);
    if (justInfo != justResults.end()) {

      for (auto& feat : justInfo->second) {
        features.push_back({
          hb_tag_from_string(feat.name.toStdString().c_str(),feat.name.size()),
          (uint32_t)feat.value,
          (unsigned int)(i - wordInfo.startIndex),
          (unsigned int)(i - wordInfo.startIndex + 1)
          });
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
  }
  else if (diff == 0) {
    return AppliedResult::NoChange;
  }
  else {
    return AppliedResult::Overflow;
  }
}

struct SubWordsMatch {
  vector<int> subWordIndexes;
  vector<vector<QRegularExpressionMatch>> matches;
};

static SubWordsMatch matchSubWords(const WordInfo& wordInfo, const vector<QRegularExpression>& regExprs) {

  SubWordsMatch result;


  for (int subIndex = 0; subIndex < wordInfo.subwords.size(); subIndex++) {
    const auto& subWord = wordInfo.subwords[subIndex];

    result.matches.push_back({});

    auto& subWordMatches = result.matches.back();

    for (const auto& regExpr : regExprs) {
      auto matches = regExpr.globalMatch(subWord.baseText);
      while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        subWordMatches.push_back(match);
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
  int(*calcNewValue)(int, int);
};

static vector<TextFontFeatures> mergeFeatures(const vector<TextFontFeatures>& prevFeatures, const vector<Appliedfeature>& newFeatures) {

  vector<TextFontFeatures> mergedFeatures{ prevFeatures };

  if (newFeatures.size() > 0) {
    for (auto& newFeature : newFeatures) {

      auto exist = std::find_if(mergedFeatures.begin(), mergedFeatures.end(), [&newFeature](const TextFontFeatures& x) { return x.name == newFeature.feature.name; });

      if (exist != mergedFeatures.end()) {
        exist->value = newFeature.calcNewValue != nullptr ? newFeature.calcNewValue(exist->value, newFeature.feature.value) : newFeature.feature.value;
      }
      else {
        auto cloneNewFeature = TextFontFeatures{
          .name = newFeature.feature.name,
          .value = newFeature.calcNewValue != nullptr ? newFeature.calcNewValue(0, newFeature.feature.value) : newFeature.feature.value
        };
        mergedFeatures.push_back(cloneNewFeature);
      }
    }
  }

  return mergedFeatures;
}

static AppliedResult applyAlternate(const LineTextInfo& lineTextInfo, JustInfo& justInfo, int wordIndex, int indexInLine) {
  const auto& lineText = lineTextInfo.lineText;
  auto tempResult{ justInfo.fontFeatures };

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

  auto newFeatures = mergeFeatures(prevFeatures, {
    Appliedfeature{.feature = {
      .name = "cv01", .value = 1 },
    .calcNewValue = [](int prev, int curr) {
        return min(prev + curr, 12);
      }}
    });

  tempResult.insert_or_assign(indexInLine, newFeatures);

  int fathaIndex = -1;
  auto tt = U'\u064E';

  if (indexInLine + 1 < lineText.size() && lineText[indexInLine + 1] == U'\u064E') {
    fathaIndex = indexInLine + 1;
  }
  else if (indexInLine + 2 < lineText.size() && lineText[indexInLine + 1] == U'\u0651' && lineText[indexInLine + 2] == U'\u064E') {
    fathaIndex = indexInLine + 2;
  }

  if (fathaIndex != -1) {
    double cv01Value = 0;
    auto ff = std::find_if(newFeatures.begin(), newFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
    if (ff != newFeatures.end()) {
      cv01Value = ff->value;
    }

    tempResult.insert_or_assign(fathaIndex, vector<TextFontFeatures>{ TextFontFeatures{ .name = "cv01", .value = 1 + (int)floor(cv01Value / 3) } });
  }

  appliedResult = tryApplyFeatures(wordIndex, lineTextInfo, justInfo, tempResult);

  return appliedResult;
}

static bool applyAlternatesSubWords(const LineTextInfo& lineTextInfo, JustInfo& justInfo, QString chars, int nbLevels) {

  const auto& wordInfos = lineTextInfo.wordInfos;

  vector<SubWordsMatch> matchresult;

  auto patternAlt = "^.*(?<alt>[${" + chars + "}])$";
  vector<QRegularExpression> regExprAlt{ QRegularExpression{patternAlt} };


  for (int wordIndex = 0; wordIndex < wordInfos.size(); wordIndex++) {
    matchresult.push_back(matchSubWords(wordInfos[wordIndex], regExprAlt));
  }

  for (int level = 1; level <= nbLevels; level++) {
    for (int wordIndex = 0; wordIndex < wordInfos.size(); wordIndex++) {
      const auto& wordInfo = wordInfos[wordIndex];
      const auto& subWordsMatch = matchresult[wordIndex];

      for (int i = subWordsMatch.subWordIndexes.size() - 1; i >= 0; i--) {
        auto subWordIndex = subWordsMatch.subWordIndexes[i];
        auto matchIndex = subWordsMatch.matches[subWordIndex][0].capturedStart("alt");
        auto indexInLine = wordInfo.startIndex + wordInfo.subwords[subWordIndex].baseIndexes[matchIndex];

        auto appliedResult = applyAlternate(lineTextInfo, justInfo, wordIndex, indexInLine);

        if (appliedResult == AppliedResult::Overflow) {
          return true;
        }
        else if (appliedResult == AppliedResult::Forbiden) {
          continue;
        }
        else {
          break;
        }
      }
    }
  }
  return false;
}

static QString rightKashExp = QString("بتثنيئ") + "جحخ" + "سش" + "صض" + "طظ" + "عغ" + "فق" + "م" + "ه";
static QString leftKash = QString("ئبتثني") + "جحخ" + "طظ" + "عغ" + "فق" + "ةلم" + "رز";
static QString mediLeftAsendant = "ل";
static const QString finalAscendant = "آادذٱأإكلهة";
static auto regexBeh = vector<QRegularExpression>{ QRegularExpression("^.+(?<k1>[بتثنيسشصض][بتثنيم]).+$") };
static auto regexFinaAscendant = vector<QRegularExpression>{ QRegularExpression(QString("^.*(?<k1>[%1][%2])$").arg(rightKashExp).arg(finalAscendant)) };
static auto regexOtherKashidas = vector<QRegularExpression>{
  QRegularExpression(QString(".*(?<k1>[%1][رز])").arg(rightKashExp)),
  QRegularExpression(QString(".*(?<k1>[%1](?:[%2]|[%3]))").arg(rightKashExp).arg(mediLeftAsendant).arg(leftKash.replace("رز", ""))),
};
static auto regexKaf = vector<QRegularExpression>{ QRegularExpression("^.*(?<k1>[ك].).*$") };
static auto regexSecondKashidaNotSameSubWord = vector<QRegularExpression>{
  QRegularExpression(QString("^.+(?<k1>[بتثنيسشصض][بتثنيم]).+$")),
  QRegularExpression(QString("^.*(?<k1>[%1][آادذٱأإكلهة])$").arg(rightKashExp)),
  QRegularExpression(QString(".*(?<k1>[%1][رز])").arg(rightKashExp)),
  QRegularExpression(QString(".*(?<k1>[%1](?:[%2]|[%3]))").arg(rightKashExp).arg(mediLeftAsendant).arg(leftKash.replace("رز", ""))),
};
static auto regexSecondKashidaSameSubWord = vector<QRegularExpression>{
  QRegularExpression(QString("^.+(?<k1>[بتثنيسشصض][بتثنيم]).+")),
  QRegularExpression(QString("(?<k1>[%1][آادذٱأإكلهة])$").arg(rightKashExp)),
  QRegularExpression(QString("(?<k1>[%1][رز])").arg(rightKashExp)),
  QRegularExpression(QString("(?<k1>[%1](?:[%2]|[%3]))").arg(rightKashExp).arg(mediLeftAsendant).arg(leftKash.replace("رز", ""))),
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
    QString("ه").contains(chark3) &&
    QString("م").contains(chark4) &&
    subWordInfo.baseIndexes.back() == secondMatchIndex
    ) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv11", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv11", .value = 1 });
  }
  else if (
    QString("بتثنيئ").contains(chark3) &&
    subWordInfo.baseIndexes[0] == firstMatchIndex &&
    QString("جحخ").contains(chark4)
    ) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv12", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv12", .value = 1 });
  }
  else if (
    QString("م").contains(chark3) &&
    subWordInfo.baseIndexes[0] == firstMatchIndex &&
    QString("جحخ").contains(chark4)
    ) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv13", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv13", .value = 1 });
  }
  else if (
    QString("فق").contains(chark3) &&
    subWordInfo.baseIndexes[0] == firstMatchIndex &&
    QString("جحخ").contains(chark4)
    ) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv14", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv14", .value = 1 });
  }
  else if (
    QString("ل").contains(chark3) &&
    subWordInfo.baseIndexes[0] == firstMatchIndex &&
    QString("جحخ").contains(chark4)
    ) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv15", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv15", .value = 1 });
  }
  else if (
    QString("عغ").contains(chark3) &&
    subWordInfo.baseIndexes[0] == firstMatchIndex &&
    (QString("آادذٱأإل").contains(chark4) ||
      (QString("بتثنيئ").contains(chark4) && subWordInfo.baseText.size() > 2 &&
        QString("سش").contains(subWordInfo.baseText[2])))
    ) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv16", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv16", .value = 1 });
  }
  else if (QString("جحخ").contains(chark3)) {
    if (
      QString("آادذٱأإل").contains(chark4) ||
      (QString("هة").contains(chark4) &&
        subWordInfo.baseIndexes.back() == secondMatchIndex) ||
      (QString("بتثنيئ").contains(chark4) &&
        subWordInfo.baseIndexes.size() > 1 &&
        subWordInfo.baseIndexes.end()[-2] == secondMatchIndex &&
        QString("رزن").contains(subWordInfo.baseText.back()))
      ) {
      firstAppliedFeatures.push_back({ .feature = {.name = "cv16", .value = 1 } });
      secondNewFeatures.push_back({ .name = "cv16", .value = 1 });
    }
    else if (
      subWordInfo.baseIndexes[0] == firstMatchIndex &&
      QString("م").contains(chark4)
      ) {
      firstAppliedFeatures.push_back({ .feature = {.name = "cv18", .value = 1 } });
      secondNewFeatures.push_back({ .name = "cv18", .value = 1 });
    }
  }
  else if (QString("سشصض").contains(chark3) && QString("رز").contains(chark4)) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv17", .value = 1 } });
    secondNewFeatures.push_back({ .name = "cv17", .value = 1 });
  }

}

static AppliedResult applyKashida(
  const LineTextInfo& lineTextInfo,
  JustInfo& justInfo,
  int wordIndex,
  int subWordIndex,
  int firstSubWordMatchIndex,
  int secondSubWordMacthIndex
) {

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

  map<int, vector<TextFontFeatures>> tempResult{ justInfo.fontFeatures };

  auto firstPrevFeatures = tempResult[firstIndexInLine];
  auto secondPrevFeatures = tempResult[secondIndexInLine];

  AppliedResult appliedResult = AppliedResult::Forbiden;

  auto ff = std::find_if(secondPrevFeatures.begin(), secondPrevFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
  if (ff != secondPrevFeatures.end()) return appliedResult;

  if (
    chark4 == U'ق' &&
    subWordInfo.baseIndexes.back() == secondMatchIndex
    ) {
    return appliedResult;
  }
  else if (
    chark3 == U'ل' &&
    (chark4 == U'ك' ||
      chark4 == U'د' ||
      chark4 == U'ذ' ||
      chark4 == U'ة' ||
      (chark4 == U'ه' &&
        subWordInfo.baseIndexes.back() == secondMatchIndex))
    ) {
    return appliedResult;
  }
  else if (
    QString("ئبتثنيى").contains(chark3) &&
    subWordInfo.baseIndexes[0] != firstMatchIndex &&
    QString("رز").contains(chark4)
    ) {
    return appliedResult;
  }

  vector<TextFontFeatures> secondNewFeatures;

  vector<Appliedfeature> firstAppliedFeatures{ Appliedfeature{.feature = {.name = "cv01", .value = 1 }, .calcNewValue = [](int prev, int curr) { return min(prev + curr, 6); } } };

  if (QString("بتثنيئ").contains(chark3)) {
    firstAppliedFeatures.push_back({ .feature = {.name = "cv10", .value = 1 } });
  }

  // decomposition

  DealWithDecomposition(firstMatchIndex, secondMatchIndex, wordInfo, subWordInfo, lineTextInfo, secondNewFeatures, firstAppliedFeatures);

  auto firstNewFeatures = mergeFeatures(
    firstPrevFeatures,
    firstAppliedFeatures
  );

  int cv01Value = 0;

  ff = std::find_if(firstNewFeatures.begin(), firstNewFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
  if (ff != firstNewFeatures.end()) {
    cv01Value = ff->value;
  }

  int cv02Value;

  if (finalAscendant.contains(chark4) && subWordInfo.baseIndexes.back() == secondMatchIndex) {
    cv02Value = cv01Value;
  }
  else {
    cv02Value = 2 * cv01Value;
  }

  secondNewFeatures.push_back({ .name = "cv02", .value = cv02Value });


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
  int secondSubWordMacthIndex
) {

  auto& wordInfos = lineTextInfo.wordInfos;
  auto& lineText = lineTextInfo.lineText;
  auto& wordInfo = wordInfos[wordIndex];
  auto& subWordInfo = wordInfo.subwords[subWordIndex];
  auto firstMatchIndex = subWordInfo.baseIndexes[firstSubWordMatchIndex];
  auto secondMatchIndex = subWordInfo.baseIndexes[secondSubWordMacthIndex];

  auto firstIndexInLine = wordInfo.startIndex + firstMatchIndex;
  auto secondIndexInLine = wordInfo.startIndex + secondMatchIndex;

  map<int, vector<TextFontFeatures>> tempResult{ justInfo.fontFeatures };

  auto firstPrevFeatures = tempResult[firstIndexInLine];
  auto secondPrevFeatures = tempResult[secondIndexInLine];

  vector<Appliedfeature> firstAppliedFeatures{ {.feature = {.name = "cv03", .value = 1 }, .calcNewValue = [](int prev, int curr) {return 1; } } };

  tempResult.insert_or_assign(firstIndexInLine, mergeFeatures(firstPrevFeatures, firstAppliedFeatures));

  vector<Appliedfeature> secondAppliedFeatures{ {.feature = {.name = "cv03", .value = 1 }, .calcNewValue = [](int prev, int curr) {return 1; } } };

  auto firstNewFeatures = mergeFeatures(secondPrevFeatures, secondAppliedFeatures);

  tempResult.insert_or_assign(secondIndexInLine, firstNewFeatures);

  int fathaIndex = -1;

  if (firstIndexInLine + 1 < lineText.size() &&
    lineText[firstIndexInLine + 1] == U'\u064E') {
    fathaIndex = firstIndexInLine + 1;
  }
  else if (
    firstIndexInLine + 2 < lineText.size() &&
    lineText[firstIndexInLine + 1] == U'\u0651' &&
    lineText[firstIndexInLine + 2] == U'\u064E'
    ) {
    fathaIndex = firstIndexInLine + 2;
  }

  if (fathaIndex != -1) {
    int cv01Value = 0;
    auto ff = std::find_if(firstNewFeatures.begin(), firstNewFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });

    if (ff != firstNewFeatures.end()) {
      cv01Value = ff->value;
    }
    tempResult.insert_or_assign(fathaIndex, vector<TextFontFeatures>{{.name = "cv01", .value = 1 + (int)floor(cv01Value / 3) }});

  }

  auto appliedResult = tryApplyFeatures(
    wordIndex,
    lineTextInfo,
    justInfo,
    tempResult
  );

  return appliedResult;
}

static bool applyKashidasSubWords(
  const LineTextInfo& lineTextInfo,
  JustInfo& justInfo,
  StretchType type,
  int nbLevels
) {

  auto& wordInfos = lineTextInfo.wordInfos;
  auto& lineText = lineTextInfo.lineText;

  vector<SubWordsMatch> matchresult;

  vector<QRegularExpression> regExprs;
  switch (type) {
  case StretchType::Beh:
    regExprs = regexBeh;
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
  }

  for (int wordIndex = 0; wordIndex < wordInfos.size(); wordIndex++) {
    matchresult.push_back(matchSubWords(wordInfos[wordIndex], regExprs));
  }

  for (int level = 1; level <= nbLevels; level++) {
    for (int wordIndex = 0; wordIndex < wordInfos.size(); wordIndex++) {
      auto& wordInfo = wordInfos[wordIndex];
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
        i--
        ) {
        auto subWordIndex = subWordsMatch.subWordIndexes[i];

        for (auto match : subWordsMatch.matches[subWordIndex]) {

          auto firstSubWordMatchIndex = match.capturedStart("k1");

          if (firstSubWordMatchIndex == -1) continue;
          auto secondSubWordMacthIndex = firstSubWordMatchIndex + 1;

          if (type == StretchType::SecondKashidaNotSameSubWord) {
            auto type123 = type1Applied != wordLayout.appliedKashidas.end() ? type1Applied
              : type2Applied != wordLayout.appliedKashidas.end() ? type2Applied :
              type3Applied != wordLayout.appliedKashidas.end() ? type3Applied : wordLayout.appliedKashidas.end();
            if (type123 != wordLayout.appliedKashidas.end() && type123->second.subWordIndex == subWordIndex) continue;
          }
          else if (type == StretchType::SecondKashidaSameSubWord) {
            auto type123 = type1Applied != wordLayout.appliedKashidas.end() ? type1Applied
              : type2Applied != wordLayout.appliedKashidas.end() ? type2Applied :
              type3Applied != wordLayout.appliedKashidas.end() ? type3Applied : wordLayout.appliedKashidas.end();


            if (
              type123 != wordLayout.appliedKashidas.end() &&
              type123->second.subWordIndex == subWordIndex &&
              type123->second.characterIndexInSubWord == firstSubWordMatchIndex
              )
              continue;
            if (
              type5Applied != wordLayout.appliedKashidas.end() &&
              type5Applied->second.subWordIndex == subWordIndex &&
              type5Applied->second.characterIndexInSubWord == firstSubWordMatchIndex
              )
              continue;
          }

          AppliedResult appliedResult = AppliedResult::Forbiden;

          if (type == StretchType::Kaf) {
            appliedResult = applyKaf(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
          }
          else {
            appliedResult = applyKashida(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
          }

          if (appliedResult == AppliedResult::Positive) {
            wordLayout.appliedKashidas.insert_or_assign(type, SubWordCharIndex{ subWordIndex,firstSubWordMatchIndex });
          }
          else if (appliedResult == AppliedResult::Overflow) {
            return true;
          }
          else if (appliedResult == AppliedResult::Forbiden) {
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

//static const QString rightNoJoinLetters = "آاٱأإدذرزوؤءة";
//static const QString dualJoinLetters = "بتثجحخسشصضطظعغفقكلمنهيئى";

static const QString rightChars = dualJoinLetters;
static const QString leftChars = dualJoinLetters + QString(rightNoJoinLetters).remove("ء");
static const QString rightKash = QString(rightChars).remove(QRegularExpression("[لك]"));
static const QString leftKashidaFina = QString(leftChars).remove(QRegularExpression("[وهصضطظ]"));
static const QString leftKashidaMedi = QString(leftKashidaFina).remove("ه");
static const QString jhk = "جحخ";

static const QString altFinPat = "^.*([بتثفكنصضسشقيئى])$";
static const QRegularExpression altFinaPrio1Reg(altFinPat);
static const QString finalKashidaEndWord = QString("^.*([%1][آاٱأإملهة])$").arg(rightKash);
static const QString finalKashida = QString("^.*([%1][دذآاٱأإملهة])$").arg(rightKash);
static const QString hahKashida = QString("^.*([%1][%2]).*$|^.*([%1][هة])$").arg(jhk).arg(leftKashidaMedi);
static const QRegularExpression regHahFinaAscenKashida(hahKashida + "|" + finalKashidaEndWord);
static const QString behBehPat = "^.+([بتثنيسشصض][بتثنيم]).+$";
static const QString rehPat = QString(".*([%1][رز])").arg(rightKash);
static const QString otherPat = QString(".*([%1](?:[%2]|[%3]))").arg(rightKash).arg(mediLeftAsendant).arg(leftKashidaMedi);
static const QString kafPat = "^.*([ك].).*$";
static const QString patternAlt = altFinPat + "|" + hahKashida + "|" + finalKashida + "|" + behBehPat + "|" + rehPat + "|" + otherPat + "|" + kafPat;
static const QRegularExpression regExprAlt(patternAlt);


static bool  applySimpleJust(const LineTextInfo& lineTextInfo,
  JustInfo& justInfo,
  bool firstWordIncluded,
  bool wordByWord,
  int nbLevelAlt,
  int nbLevelKashida)
{

  auto& wordInfos = lineTextInfo.wordInfos;
  auto& lineText = lineTextInfo.lineText;

  struct SubWordMatch {
    int subWordIndex;
    QRegularExpressionMatch match;
    int type;
  };

  vector<SubWordMatch> matchresult;

  auto firstWordIndex = firstWordIncluded ? 0 : 1;

  for (int wordIndex = 0; wordIndex < wordInfos.size(); wordIndex++) {
    auto& wordInfo = wordInfos[wordIndex];
    SubWordMatch result{ .subWordIndex = -1, .match = {}, .type = 0 };
    if (wordInfo.baseText.isEmpty() || wordIndex < firstWordIndex) {
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
    }
    else if (!QString("يئى").contains(wordInfo.baseText.back())) {
      match = regHahFinaAscenKashida.match(subWord.baseText);
      if (match.hasMatch()) {
        result.subWordIndex = lastIndex;
        result.match = match;
        result.type = 2;
      }
      else {
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

      AppliedResult appliedResult;

      auto& wordInfo = wordInfos[wordIndex];
      auto& subWordsMatch = matchresult[wordIndex];

      auto& match = subWordsMatch.match;


      if (!match.hasMatch()) continue;

      auto subWordIndex = subWordsMatch.subWordIndex;

      auto matchIndex = match.lastCapturedIndex();

      if (subWordsMatch.type == 1 || (subWordsMatch.type == 3 && (matchIndex == 1))) {
        // Alternates
        if (level <= nbLevelAlt) {
          auto baseIndex = match.capturedStart(matchIndex);
          auto indexInLine = wordInfo.startIndex + wordInfo.subwords[subWordIndex].baseIndexes[baseIndex];
          appliedResult = applyAlternate(lineTextInfo, justInfo, wordIndex, indexInLine);
        }
      }
      else if (level <= nbLevelKashida) {
        auto firstSubWordMatchIndex = match.capturedStart(matchIndex);
        auto secondSubWordMacthIndex = firstSubWordMatchIndex + 1;

        if (matchIndex == 8 && subWordsMatch.type == 3) {
          //Kaf
          appliedResult = applyKaf(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
        }
        else {
          // Kashidas
          appliedResult = applyKashida(lineTextInfo, justInfo, wordIndex, subWordIndex, firstSubWordMatchIndex, secondSubWordMacthIndex);
        }
      }

      if (appliedResult == AppliedResult::Overflow) {
        return true;
      }
      else if (appliedResult == AppliedResult::Positive) {
        if (wordByWord) {
          stretchedWords.insert({ wordIndex, true });
        }
      }
    }
  }
  return false;
}

static void applyExperimentalJust(const LineTextInfo& lineTextInfo, JustInfo& justInfo) {
  applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Beh, 2) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "بتثكن", 2) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 3) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 2) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "ىصضسشفقيئ", 2) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Kaf, 1) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Beh, 1) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "بتثكن", 1) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 1) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "ىصضسشفقيئ", 1) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "بتثكن", 2) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "ىصضسشفقيئبتثكن", 2) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::Beh, 1) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::FinaAscendant, 1) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::OtherKashidas, 1) ||
    applyAlternatesSubWords(lineTextInfo, justInfo, "ىصضسشفقيئبتثكن", 2) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::SecondKashidaNotSameSubWord, 2) ||
    applyKashidasSubWords(lineTextInfo, justInfo, StretchType::SecondKashidaSameSubWord, 2);
}
static void stretchLine(const LineTextInfo& lineTextInfo, JustInfo& justInfo, JustType justType) {

  if (justType == JustType::Madina) {
    applySimpleJust(lineTextInfo, justInfo, true, false, 2, 2);
  }
  else if (justType == JustType::IndoPak) {
    applySimpleJust(lineTextInfo, justInfo, false, true, 2, 2);
  }
  else {
    applyExperimentalJust(lineTextInfo, justInfo);
  }

}

static JustResultByLine justifyLine(const LineTextInfo& lineTextInfo, hb_font_t* font, double fontSizeLineWidthRatio,
  double spaceWidth,
  JustType justType,
  JustStyle justStyle,
  OtLayout* layout
) {

  auto desiredWidth = FONTSIZE / fontSizeLineWidthRatio;

  auto lineText = lineTextInfo.lineText;

  vector<LayoutResult> layOutResult{};

  for (int wordIndex = 0; wordIndex < lineTextInfo.wordInfos.size(); wordIndex++) {
    auto& wordInfo = lineTextInfo.wordInfos[wordIndex];

    auto parWidth = getWidth(wordInfo.text, font, {});

    layOutResult.push_back({ parWidth,{} });

  }

  auto currentLineWidth = getWidth(lineText, font, {});

  auto diff = desiredWidth - currentLineWidth;

  JustResultByLine result;

  result.xScale = 1;
  result.simpleSpacing = spaceWidth;
  result.ayaSpacing = spaceWidth;

  JustInfo justInfo{ .fontFeatures = {},.desiredWidth = desiredWidth,.textLineWidth = currentLineWidth, .layoutResult = layOutResult, .font = font };



  if (diff > 0) {
    // stretch   

    double maxStretchBySpace = std::min(100.0, spaceWidth * 1);
    double maxStretchByAyaSpace = std::min(200.0, spaceWidth * 2);
    //let maxStretchBySpace = Math.max(200 - spaceWidth,0);
    //let maxStretchByAyaSpace = Math.max(300 - spaceWidth,0);

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


      stretchLine(lineTextInfo, justInfo, justType);
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


  }
  else {
    //shrink
    if (justStyle == JustStyle::SCLX) {
      float xScale = desiredWidth / currentLineWidth;
      auto ff = layout->createFont(font->x_scale / 1000, false);
      hb_font_set_variation(ff, HB_TAG('S', 'C', 'L', 'X'), xScale * 100);
      auto newCurrentLineWidth = getWidth(lineText, ff, {});

      hb_font_destroy(ff);
      if (newCurrentLineWidth < currentLineWidth) {
        result.sclxAxis = xScale * 100;
        result.xScale = desiredWidth / newCurrentLineWidth;
        std::cout << "result.xScale=" << result.xScale << " result.sclxAxis=" << result.sclxAxis << " currentLineWidth=" << currentLineWidth << " newCurrentLineWidth = " << newCurrentLineWidth << std::endl;
      }

    }
    else {
      //result.globalFeatures = {{.name = "cv04", .value = 5}};
      //result.globalFeatures = {{.name = "sk01", .value = 1}};
      result.xScale = desiredWidth / currentLineWidth;
    }


  }
  result.fontFeatures = justInfo.fontFeatures;

  return result;

}
static QMap<QString, int> tajweedNameToColor = {
  {"green",0x00A650FF}, {"tafkim", 0x006694FF}, {"lgray", 0xB4B4B4FF},
  {"lkalkala",0x00ADEFFF}, {"red1", 0xC38A08FF}, {"red2", 0xF47216FF},
  {"red3",0xEC008CFF}, {"red4", 0x8C0000FF}
};

static LineLayoutInfo shapeLine(OtLayout* layout, int lineWidth, int pageWidth,
  const LineTextInfo& lineTextInfo, const JustResultByLine& justResult, bool tajweedColor, double emScale, hb_font_t* font,
  LineJustification justification, int& currentyPos,QMap<int, QString>& tajweedResult) {

  vector< hb_feature_t> features{};

  for (auto& feat : justResult.globalFeatures) {
    features.push_back({
      hb_tag_from_string(feat.name.toStdString().c_str(),feat.name.size()),
      (uint32_t)feat.value,
      (unsigned int)0,
      (unsigned int)-1
      });
  }

  for (auto& wordInfo : lineTextInfo.wordInfos) {
    for (int i = wordInfo.startIndex; i <= wordInfo.endIndex; i++) {

      auto justInfo = justResult.fontFeatures.find(i);
      if (justInfo != justResult.fontFeatures.end()) {

        for (auto& feat : justInfo->second) {
          features.push_back({
            hb_tag_from_string(feat.name.toStdString().c_str(),feat.name.size()),
            (uint32_t)feat.value,
            (unsigned int)(i),
            (unsigned int)(i + 1)
            });
        }
      }

    }
  }

  auto buffer = shape(lineTextInfo.lineText, font, features);

  uint glyph_count;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  LineLayoutInfo lineLayout;
  int currentlineWidth = 0;

  for (int i = glyph_count - 1; i >= 0; i--) {

    GlyphLayoutInfo glyphLayout;

    glyphLayout.codepoint = glyph_info[i].codepoint;
    glyphLayout.lefttatweel = glyph_info[i].lefttatweel; // normalToParameter(glyph_info[i].codepoint, glyph_info[i].lefttatweel, true);
    glyphLayout.righttatweel = glyph_info[i].righttatweel; // normalToParameter(glyph_info[i].codepoint, glyph_info[i].righttatweel, false);
    glyphLayout.cluster = glyph_info[i].cluster;
    glyphLayout.x_advance = glyph_pos[i].x_advance;
    glyphLayout.y_advance = glyph_pos[i].y_advance;
    glyphLayout.x_offset = glyph_pos[i].x_offset;
    glyphLayout.y_offset = glyph_pos[i].y_offset;
    glyphLayout.lookup_index = glyph_pos[i].lookup_index;
    if (tajweedColor) {
      auto color = tajweedResult.find(glyphLayout.cluster);
      if(color != tajweedResult.end()){
        glyphLayout.color = tajweedNameToColor[color.value()];
      } else if (lineTextInfo.lineText[glyphLayout.cluster] == QChar(0x034F)) {
        auto nextIndex = i - 1;
        auto nextCluster = glyph_info[nextIndex].cluster;
        auto currCluster = glyphLayout.cluster + 1;
        if (nextCluster > currCluster) {
          auto color = tajweedResult.find(currCluster);
          if (color != tajweedResult.end()) {
            // happens only in لِيَسُ͏ࣳٓـٔ͏ُوا۟ page 282 line 14  
            //console.log(`cgi***************************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
            glyphLayout.color = tajweedNameToColor[color.value()];
          }
          /* debug
          if (nextCluster - currCluster > 1 && lineText[glyph.Cluster + 2] !== "\u034F") {
            console.log(`nextCluster - currCluster > 1**********************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
          }*/
        } /*else {
          // happens only for فَٱدَّٰرَٰ͏ْٔتُمْ page 11 line 5 no tajweed coloring so it is OK
          //console.log(`nextCluster<=currCluster**********************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
        }  */
      }
    }else{
      glyphLayout.color = 0;
    }    
    glyphLayout.subtable_index = glyph_pos[i].subtable_index;
    glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;

    glyphLayout.beginsajda = false;
    glyphLayout.endsajda = false;

    auto space = lineTextInfo.spaces.find(glyphLayout.cluster);

    if (space != lineTextInfo.spaces.end()) {
      if (space->second == SpaceType::Aya) {
        glyphLayout.x_advance = justResult.ayaSpacing * emScale;
      }
      else if (space->second == SpaceType::Simple) {
        glyphLayout.x_advance = justResult.simpleSpacing * emScale;
      }
    }

    currentlineWidth += glyphLayout.x_advance;

    lineLayout.glyphs.push_back(glyphLayout);

  }

  lineLayout.overfull = lineWidth != 0 ? currentlineWidth - lineWidth : 0;

  if (justification == LineJustification::Distribute) {
    lineLayout.xstartposition = 0;
  }
  else {
    lineLayout.xstartposition = (pageWidth - currentlineWidth) / 2;
  }

  lineLayout.ystartposition = currentyPos;
  lineLayout.fontSize = emScale;

  currentyPos = currentyPos + (layout->InterLineSpacing << OtLayout::SCALEBY);

  hb_buffer_destroy(buffer);

  return lineLayout;

}



QList<LineLayoutInfo> OtLayout::justifyPageUsingFeatures(double emScale, int pageWidth, const QVector<LineToJustify>& lines,
  bool newFace, bool tajweedColor,
  hb_buffer_cluster_level_t  cluster_level,
  JustType justType, JustStyle justStyle, QString mushafLayout) {

  QList<LineLayoutInfo> page;

  hb_font_t* defaultShapefont = this->createFont(emScale, newFace);
  hb_font_t* justifyFont = this->createFont(1, false);

  vector<LineTextInfo> linesTextInfo;
  vector<double> fontSizeRatios;

  double spaceWidth = getWidth(" ", justifyFont, {});

  auto minRatio = 1000.0;
  auto maxRatio = 0.00000001;

  QVector<QMap<int, QString>> tajweedResults(lines.size());

  if(tajweedColor){
    TajweedService tajweedService;
    tajweedResults = tajweedService.applyTajweedByPage(lines, mushafLayout.contains("indopak"));
  }

  for (auto lineIdx = 0; lineIdx < lines.size(); lineIdx++) {
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
    }
    else {
      fontSizeRatios.emplace_back(1);
    }
  }

  int currentyPos = TopSpace << OtLayout::SCALEBY;

  for (auto lineIdx = 0; lineIdx < lines.size(); lineIdx++) {
    auto& line = lines[lineIdx];

    auto lineTextInfo = analyzeLineForJust(line.text);

    auto fontSizeLineWidthRatio = line.width != 0 ? (double)FONTSIZE * emScale / line.width : 1;

    auto defaultFontRatio = justStyle == JustStyle::SameSizeByPage ? 1.0 : 1.0; // (minRatio + maxRatio) / 2

    double fontRatio;

    if (justStyle == JustStyle::SameSizeByPage) {
      fontRatio = min(minRatio, defaultFontRatio);
    }
    else {
      fontRatio = 1; // min(fontSizeRatios[lineIdx], defaultFontRatio);
    }

    JustResultByLine justResultByLine;
    

    if (line.lineType == LineType::Bism) {
      justResultByLine.globalFeatures.push_back({ line.basm2 ? "bism" : "basm",1 });
      justResultByLine.ayaSpacing = spaceWidth;
      justResultByLine.simpleSpacing = spaceWidth;
      justResultByLine.xScale = 1;
    }
    else {
      justResultByLine = justifyLine(lineTextInfo, justifyFont, fontSizeLineWidthRatio * fontRatio, spaceWidth, justType, justStyle, this);
    }

    hb_font_t* shapeFont = defaultShapefont;
    auto newEmScale = emScale;
    auto newFont = false;
    if (fontRatio != 1) {
      newFont = true;
      newEmScale = emScale * fontRatio;
      shapeFont = this->createFont(newEmScale, false);
    }
    if (justResultByLine.sclxAxis != 0) {
      if (!newFont) {
        newFont = true;
        shapeFont = this->createFont(emScale, false);
      }
      hb_font_set_variation(shapeFont, HB_TAG('S', 'C', 'L', 'X'), justResultByLine.sclxAxis);
    }

    auto lineLayoutInfo = shapeLine(this, line.width, pageWidth, lineTextInfo, justResultByLine, tajweedColor, newEmScale, shapeFont, line.lineJustification, currentyPos,tajweedResults[lineIdx]);
    lineLayoutInfo.type = line.lineType;

    if (newFont) {
      hb_font_destroy(shapeFont);
    }
    if (justStyle == JustStyle::SCLX) {
      lineLayoutInfo.fontSize = lineLayoutInfo.fontSize * justResultByLine.xScale;
      lineLayoutInfo.xscale = 1;
      lineLayoutInfo.xscaleparameter = justResultByLine.sclxAxis;
    }
    else if (lineLayoutInfo.type == LineType::Line) {
      lineLayoutInfo.xscale = justResultByLine.xScale;
    }

    



    page.append(lineLayoutInfo);
  }

  hb_font_destroy(defaultShapefont);
  hb_font_destroy(justifyFont);

  return page;

}
