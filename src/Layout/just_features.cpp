#include "OtLayout.h"
#include  <algorithm>
#include "hb-buffer.hh"
#include <qregularexpression.h>

using namespace std;

static const int PAGE_WIDTH = 17000;
static const int INTERLINE = 1800;
static const int TOP = 200;
static const int MARGIN = 400;
static const int FONTSIZE = 1000;
static const int SPACEWIDTH = 100;

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
  vector<TextFontFeatures> globalFeatures = {};
  map<int, vector<TextFontFeatures>> fontFeatures = {}; /* FontFeatures by character index in the line */
  double simpleSpacing;
  double ayaSpacing;
  double fontSizeRatio;
};

struct JustInfo {
  map<int, vector<TextFontFeatures>> fontFeatures = {};
  double desiredWidth;
  double textLineWidth;
  vector<LayoutResult> layoutResult;
  hb_font_t* font;
};


static const QString rightNoJoinLetters = "ادذرزوؤأٱإءة";
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

      if ((lineText.at(i - 1) >= 0x0660 && lineText.at(i - 1) <= 0x0669) || (lineText.at(i + 1) == 0x06DD)) {
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
        auto& subWord = currentWord->subwords.back();
        subWord.baseText += qchar;
        subWord.baseIndexes.push_back(i - currentWord->startIndex);
        if (i < lineText.size() - 1 && rightNoJoinLetters.contains(qchar)) {
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

static boolean applyAlternatesSubWords(const LineTextInfo& lineTextInfo, JustInfo& justInfo, QString chars, int nbLevels) {

  const auto& wordInfos = lineTextInfo.wordInfos;
  const auto& lineText = lineTextInfo.lineText;

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

        auto tempResult{ justInfo.fontFeatures };

        vector<TextFontFeatures> prevFeatures;

        auto prevFeaturesIter = tempResult.find(indexInLine);

        if (prevFeaturesIter != tempResult.end()) {

          prevFeatures = prevFeaturesIter->second;

          auto cv02 = std::find_if(prevFeatures.begin(), prevFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv02"; });

          if (cv02 != prevFeatures.end() && cv02->value > 0) {
            continue;
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

        auto appliedResult = tryApplyFeatures(wordIndex, lineTextInfo, justInfo, tempResult);

        if (appliedResult == AppliedResult::Overflow) {
          return true;
        }

        break;

      }
    }
  }
  return false;
}

static QString rightKash = QString("بتثنيئ") + "جحخ" + "سش" + "صض" + "طظ" + "عغ" + "فق" + "م" + "ه";
static QString leftKash = QString("ئبتثني") + "جحخ" + "طظ" + "عغ" + "فق" + "ةلم" + "رز";
static QString mediLeftAsendant = "ل";

static auto regexBeh = vector<QRegularExpression>{ QRegularExpression("^.+(?<k1>[بتثنيسشصض][بتثنيم]).+$") };
static auto regexFinaAscendant = vector<QRegularExpression>{ QRegularExpression(QString("^.*(?<k1>[%1][آادذٱأإكلهة])$").arg(rightKash)) };
static auto regexOtherKashidas = vector<QRegularExpression>{
  QRegularExpression(QString(".*(?<k1>[%1][رز])").arg(rightKash)),
  QRegularExpression(QString(".*(?<k1>[%1](?:[%2]|[%3]))").arg(rightKash).arg(mediLeftAsendant).arg(leftKash.replace("رز", ""))),
};
static auto regexKaf = vector<QRegularExpression>{ QRegularExpression("^.*(?<k1>[ك].).*$") };
static auto regexSecondKashidaNotSameSubWord = vector<QRegularExpression>{
  QRegularExpression(QString("^.+(?<k1>[بتثنيسشصض][بتثنيم]).+$")),
  QRegularExpression(QString("^.*(?<k1>[%1][آادذٱأإكلهة])$").arg(rightKash)),
  QRegularExpression(QString(".*(?<k1>[%1][رز])").arg(rightKash)),
  QRegularExpression(QString(".*(?<k1>[%1](?:[%2]|[%3]))").arg(rightKash).arg(mediLeftAsendant).arg(leftKash.replace("رز", ""))),
};
static auto regexSecondKashidaSameSubWord = vector<QRegularExpression>{
  QRegularExpression(QString("^.+(?<k1>[بتثنيسشصض][بتثنيم]).+")),
  QRegularExpression(QString("(?<k1>[%1][آادذٱأإكلهة])$").arg(rightKash)),
  QRegularExpression(QString("(?<k1>[%1][رز])").arg(rightKash)),
  QRegularExpression(QString("(?<k1>[%1](?:[%2]|[%3]))").arg(rightKash).arg(mediLeftAsendant).arg(leftKash.replace("رز", ""))),
};

static boolean applyKashidasSubWords(
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

          auto& subWordInfo = wordInfo.subwords[subWordIndex];
          auto firstMatchIndex = subWordInfo.baseIndexes[firstSubWordMatchIndex];
          auto secondMatchIndex = subWordInfo.baseIndexes[secondSubWordMacthIndex];
          auto firstIndexInLine = wordInfo.startIndex + firstMatchIndex;
          auto secondIndexInLine = wordInfo.startIndex + secondMatchIndex;

          map<int, vector<TextFontFeatures>> tempResult{ justInfo.fontFeatures };

          auto firstPrevFeatures = tempResult[firstIndexInLine];
          auto secondPrevFeatures = tempResult[secondIndexInLine];

          if (type == StretchType::Kaf) {

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
          }
          else {
            auto ff = std::find_if(secondPrevFeatures.begin(), secondPrevFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
            if (ff != secondPrevFeatures.end()) continue;

            auto chark3 = lineText[firstIndexInLine];
            auto chark4 = lineText[secondIndexInLine];

            if (
              chark4 == U'ق' &&
              subWordInfo.baseIndexes.back() == secondMatchIndex
              ) {
              continue;
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
              continue;
            }
            else if (
              QString("ئبتثنيى").contains(chark3) &&
              subWordInfo.baseIndexes[0] != firstMatchIndex &&
              QString("رز").contains(chark4)
              ) {
              continue;
            }

            vector<TextFontFeatures> secondNewFeatures;

            vector<Appliedfeature> firstAppliedFeatures{ Appliedfeature{.feature = {.name = "cv01", .value = 1 }, .calcNewValue = [](int prev, int curr) { return min(prev + curr, 6); } } };

            if (QString("بتثنيئ").contains(chark3)) {
              firstAppliedFeatures.push_back({ .feature = {.name = "cv10", .value = 1 } });
            }

            // decomposition

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

            auto firstNewFeatures = mergeFeatures(
              firstPrevFeatures,
              firstAppliedFeatures
            );

            int cv01Value = 0;

            ff = std::find_if(firstNewFeatures.begin(), firstNewFeatures.end(), [](const TextFontFeatures& x) { return x.name == "cv01"; });
            if (ff != firstNewFeatures.end()) {
              cv01Value = ff->value;
            }

            auto cv02Value = type == StretchType::FinaAscendant ? cv01Value : 2 * cv01Value;
            secondNewFeatures.push_back({ .name = "cv02", .value = cv02Value });


            tempResult.insert_or_assign(firstIndexInLine, firstNewFeatures);
            tempResult.insert_or_assign(secondIndexInLine, secondNewFeatures);
          }
          auto appliedResult = tryApplyFeatures(
            wordIndex,
            lineTextInfo,
            justInfo,
            tempResult
          );

          if (appliedResult == AppliedResult::Positive) {
            wordLayout.appliedKashidas.insert_or_assign(type, SubWordCharIndex{ subWordIndex,firstSubWordMatchIndex });
          }
          else if (appliedResult == AppliedResult::Overflow) {
            return true;
          }

          done = true;

          break;
        }
      }
    }
  }
  return false;
}


static void stretchLine(const LineTextInfo& lineTextInfo, JustInfo& justInfo) {

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

static JustResultByLine justifyLine(const LineTextInfo& lineTextInfo, hb_font_t* font, double fontSizeLineWidthRatio) {

  auto desiredWidth = FONTSIZE / fontSizeLineWidthRatio;

  auto defaultSpaceWidth = SPACEWIDTH;

  auto lineText = lineTextInfo.lineText;

  vector<LayoutResult> layOutResult{};

  for (int wordIndex = 0; wordIndex < lineTextInfo.wordInfos.size(); wordIndex++) {
    auto& wordInfo = lineTextInfo.wordInfos[wordIndex];

    auto parWidth = getWidth(wordInfo.text, font, {});

    layOutResult.push_back({ parWidth,{} });

  }

  auto currentLineWidth = getWidth(lineText, font, {});

  auto diff = desiredWidth - currentLineWidth;

  double fontSizeRatio = 1;
  double simpleSpacing = SPACEWIDTH;
  double ayaSpacing = SPACEWIDTH;

  JustInfo justInfo{ .fontFeatures = {},.desiredWidth = desiredWidth,.textLineWidth = currentLineWidth, .layoutResult = layOutResult, .font = font };

  if (diff > 0) {
    // stretch   

    double maxStretchBySpace = defaultSpaceWidth * 1;
    double maxStretchByAyaSpace = defaultSpaceWidth * 2;

    double maxStretch = maxStretchBySpace * lineTextInfo.simpleSpaceIndexes.size() + maxStretchByAyaSpace * lineTextInfo.ayaSpaceIndexes.size();

    auto stretch = min(desiredWidth - currentLineWidth, maxStretch);
    auto spaceRatio = maxStretch != 0 ? stretch / maxStretch : 0;
    auto stretchBySpace = spaceRatio * maxStretchBySpace;
    auto stretchByByAyaSpace = spaceRatio * maxStretchByAyaSpace;

    double simpleSpaceWidth = defaultSpaceWidth + stretchBySpace;
    double ayaSpaceWidth = defaultSpaceWidth + stretchByByAyaSpace;

    currentLineWidth += stretch;

    justInfo.textLineWidth = currentLineWidth;

    // stretching

    if (desiredWidth > currentLineWidth) {


      stretchLine(lineTextInfo, justInfo);
      currentLineWidth = justInfo.textLineWidth;
    }



    if (desiredWidth > currentLineWidth) {
      // full justify with space
      auto addToSpace = (desiredWidth - currentLineWidth) / lineTextInfo.spaces.size();
      simpleSpaceWidth += addToSpace;
      ayaSpaceWidth += addToSpace;
    }

    simpleSpacing = simpleSpaceWidth;
    ayaSpacing = ayaSpaceWidth;


  }
  else {
    //shrink
    fontSizeRatio = desiredWidth / currentLineWidth;

  }

  return { .fontFeatures = justInfo.fontFeatures,.simpleSpacing = simpleSpacing, .ayaSpacing = ayaSpacing, .fontSizeRatio = fontSizeRatio };

}

static LineLayoutInfo shapeLine(OtLayout* layout, int lineWidth, int pageWidth,
  const LineTextInfo& lineTextInfo, const JustResultByLine& justResult, bool tajweedColor, double emScale, hb_font_t* font,
  LineJustification justification, int& currentyPos) {

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
    glyphLayout.color = glyph_pos[i].lookup_index >= layout->tajweedcolorindex ? glyph_pos[i].base_codepoint : 0;
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

QList<LineLayoutInfo> OtLayout::justifyPageUsingFeatures(double emScale, int pageWidth, const QVector<LineToJustify>& lines, bool newFace, bool tajweedColor, bool changeSize, hb_buffer_cluster_level_t  cluster_level) {

  QList<LineLayoutInfo> page;

  hb_font_t* defaultShapefont = this->createFont(emScale, newFace);
  hb_font_t* justifyFont = this->createFont(1, false);

  auto maxFontSizeRatioWithoutOverflow = 1.0;
  // get max font size  for the page

  vector<LineTextInfo> linesTextInfo;



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

      if (desiredWidth < currentLineWidth) {
        maxFontSizeRatioWithoutOverflow = min(
          desiredWidth / currentLineWidth,
          maxFontSizeRatioWithoutOverflow,
          );
      }
    }
  }


  int currentyPos = TopSpace << OtLayout::SCALEBY;

  for (auto& line : lines) {
    auto lineTextInfo = analyzeLineForJust(line.text);

    auto fontSizeLineWidthRatio = line.width != 0 ? (double)FONTSIZE * emScale / line.width : 1;

    auto justResultByLine = justifyLine(lineTextInfo, justifyFont, fontSizeLineWidthRatio * maxFontSizeRatioWithoutOverflow);
    justResultByLine.fontSizeRatio = line.width != 0 ? justResultByLine.fontSizeRatio * maxFontSizeRatioWithoutOverflow : 1;
    hb_font_t* shapeFont = defaultShapefont;
    auto newEmScale = emScale;
    if (justResultByLine.fontSizeRatio != 1) {
      newEmScale = emScale * justResultByLine.fontSizeRatio;
      shapeFont = this->createFont(newEmScale, false);
    }

    auto lineLayoutInfo = shapeLine(this, line.width, pageWidth, lineTextInfo, justResultByLine, tajweedColor, newEmScale, shapeFont, line.lineJustification, currentyPos);

    if (justResultByLine.fontSizeRatio != 1) {
      hb_font_destroy(shapeFont);
    }


    page.append(lineLayoutInfo);
  }

  hb_font_destroy(defaultShapefont);
  hb_font_destroy(justifyFont);

  return page;

}
