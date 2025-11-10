#include "tajweed.h"

static QRegularExpression makeRe(const QString& pat) {
  QRegularExpression re(pat,
                        QRegularExpression::UseUnicodePropertiesOption | QRegularExpression::OptimizeOnFirstUsageOption);
  if (!re.isValid()) {
    QString errorString = re.errorString();     // errorString == "missing )"
    int errorOffset = re.patternErrorOffset();  // errorOffset == 22
    int tt = 5;
  }
  Q_ASSERT(re.isValid());
  return re;
}

static QString rightNoJoinLetters = QString("ادذرزوؤأٱإءة");
static QString dualJoinLetters = QString("بتثجحخسشصضطظعغفقكلمنهيئى");
static QString bases = rightNoJoinLetters + dualJoinLetters;
static QString IkfaaLetters = QString("صذثكجشقسدطزفتضظ");

static QChar fathatan(0x064B), dammatan(0x064C), kasratan(0x064D);
static QChar fatha(0x064E), damma(0x064F), kasra(0x0650), shadda(0x0651);
static QString sukuns = QString() + QChar(0x0652) + QChar(0x06E1);

static QChar openfathatan(0x08F0), opendammatan(0x08F1), openkasratan(0x08F2);
static QString tanween = QString() + fathatan + dammatan + kasratan;
static QString opentanween = QString() + openfathatan + opendammatan + openkasratan;
static QString alltanween = tanween + opentanween;
static QString fdk = QString() + fatha + damma + kasra;
static QString fdkt = fdk + alltanween;
static QString harakat = fdkt + sukuns + QString(shadda);

static QString prefereWaslIndoPak = QString() + QChar(0x08D5) + QChar(0x0617) + QChar(0x08D7);
static QString mandatoryWaqfIndoPak = QString() + QChar(0x08DE) + QChar(0x08DF) + QChar(0x08DD) + QChar(0x08DB);
static QString prefereWaqfIndoPak = QString() + QChar(0x0615) + QChar(0x08D6);
static QChar takhallus(0x0614);
static QChar disputedEndofAyah(0x08E2);

static QString prefereWasl = QString(QChar(0x06D6)) + prefereWaslIndoPak;
static QString prefereWaqf = QString(QChar(0x06D7)) + prefereWaqfIndoPak;
static QString mandatoryWaqf = QString(QChar(0x06D8)) + mandatoryWaqfIndoPak;
static QChar forbiddenWaqf(0x06D9), permissibleWaqf(0x06DA), waqfInOneOfTwo(0x06DB);

static QString waqfMarks = prefereWasl + prefereWaqf + mandatoryWaqf + QString(forbiddenWaqf) + QString(permissibleWaqf) + QString(waqfInOneOfTwo) + QString(disputedEndofAyah);

static QChar maddah(0x0653), maddawaajib(0x089C);
static QString maddClass = QStringLiteral("[") + QString(maddah) + QString(maddawaajib) + QStringLiteral("]");
static QChar ziaditHarf(0x06DF), ziaditHarfWasl(0x06E0), meemiqlab(0x06E2), lowmeemiqlab(0x06ED);
static QChar daggerAlef(0x0670), smallWaw(0x06E5), smallYeh(0x06E6), invertedDamma(0x0657), subAlef(0x0656);
static QString smallMadd = QString() + daggerAlef + smallWaw + smallYeh + invertedDamma + subAlef;
static QChar smallHighYeh(0x06E7), smallHighWaw(0x08F3), highCircle(0x06EC), lowCircle(0x065C);
static QChar hamzaabove(0x0654), hamzabelow(0x0655);
static QChar smallHighSeen(0x06DC), smallLowSeen(0x06E3), smallHighNoon(0x06E8);
static QChar cgi(0x034F);

static QString marks = harakat + waqfMarks + QString(maddah) + QString(maddawaajib) + QString(ziaditHarf) + QString(ziaditHarfWasl) + QString(meemiqlab) + QString(lowmeemiqlab) + smallMadd + QString(smallHighYeh) + QString(smallHighWaw) + QString(highCircle) + QString(lowCircle) + QString(hamzaabove) + QString(hamzabelow) + QString(smallHighSeen) + QString(smallLowSeen) + QString(smallHighNoon) + QString(cgi) + QString(takhallus);

// Helper to build a character class like "[...]" safely
static inline QString cls(const QString& s) { return QStringLiteral("[") + s + QStringLiteral("]"); }
static QString maddClassRegex = cls(QString() + maddah + maddawaajib);

static QString ayaCond = QStringLiteral("\\s?[۩]?\\s?۝");
static QString waqfCond = QStringLiteral("(?:") + ayaCond + QStringLiteral("|") + cls(prefereWasl + prefereWaqf + mandatoryWaqf + QString(permissibleWaqf)) + QStringLiteral("|$)");
static QString endWordCond = QStringLiteral("(?:\\s|$)");
static QString endMarksCondOpt = QStringLiteral("(?:") + cls(waqfMarks + QString(takhallus) + QString(disputedEndofAyah)).mid(0) + QStringLiteral("*)");

static QString elevationChars = QString("طقصخغضظ");

// build loweringChars and digits at file scope
static QString loweringChars = [] {
  QString s;
  for (QChar ch : bases)
    if (!elevationChars.contains(ch)) s.append(ch);
  return s;
}();

static QString digits = [] {
  QString s;
  for (ushort d = 0x0660; d <= 0x0669; ++d) s.append(QChar(d));
  return s;
}();

static QString beforeAyaCond = QStringLiteral("[") + digits + QStringLiteral("۞][") + waqfMarks + QStringLiteral("]?\\s");

static QString expand(QString tmpl, const QHash<QString, QString>& v) {
  // simple ${name} replacer
  int i = 0;
  while ((i = tmpl.indexOf("${", i)) != -1) {
    int j = tmpl.indexOf('}', i + 2);
    if (j == -1) break;
    const QString key = tmpl.mid(i + 2, j - (i + 2));
    tmpl.replace(i, j - i + 1, v.value(key));
  }
  return tmpl;
}
/*
static QRegularExpression makeRe(const QString& pat) {
  QRegularExpression re(pat,
      QRegularExpression::UseUnicodePropertiesOption
    | QRegularExpression::OptimizeOnFirstUsageOption);
  Q_ASSERT_X(re.isValid(), "TajweedService",
             qPrintable(QString("PCRE2 error: %1\nPattern:\n%2")
                        .arg(re.errorString(), pat)));
  return re;
}*/

static QString uToX(QString s) {
  QRegularExpression re(R"(\\u([0-9A-Fa-f]{4}))");
  int pos = 0;
  while (true) {
    QRegularExpressionMatch m = re.match(s, pos);
    if (!m.hasMatch()) break;
    const QString repl = QStringLiteral("\\x{") + m.captured(1) + QChar('}');
    s.replace(m.capturedStart(), m.capturedLength(), repl);
    pos = m.capturedStart() + repl.size();
  }
  return s;
}

static QString deU(QString s) {
  QRegularExpression re(R"(\\u([0-9A-Fa-f]{4}))");
  int pos = 0;
  while (true) {
    QRegularExpressionMatch m = re.match(s, pos);
    if (!m.hasMatch()) break;
    bool ok = false;
    ushort code = m.captured(1).toUShort(&ok, 16);
    const QString repl = ok ? QString(QChar(code)) : m.captured(0);
    s.replace(m.capturedStart(), m.capturedLength(), repl);
    pos = m.capturedStart() + repl.size();
  }
  return s;
}

// ====== RAW STRING TEMPLATES (Paste your TS pattern text verbatim) ======

// TAFKHIM (this is your 'pattern' built in TS; now as one raw string)
static const char* TAFKHIM_TMPL =
    "(?<kalkala1>[طقدجب][${sukuns}])"
    "|(?<kalkala2>[طقدجب]${shadda}?)(?=[${marks}]*${waqfCond})"
    "|(?<tafkhim1>[${elevationChars}]${shadda}?[${fdk}${sukuns}]?)"
    "|(?<=\\sا${kasra})(?<tafkhim_reh1>ر[${sukuns}])"
    "|(?<=${kasra}|${kasra}[${loweringChars}][${sukuns}]|[ي][${sukuns}]?|\\u0650\\u0637\\u0652)ر(?<tafkhim2>[${marks}]*)(?<tafkhim2_1>${waqfCond})"
    "|(?<tafkhim3>ر)(?<tafkhim4>[${marks}]*)(?<tafkhim4_1>${waqfCond})"
    "|ر(?=${shadda}?[${kasras}])"
    "|(?<=${kasra})ر[${sukuns}](?![${elevationChars}]${fatha})"
    "|(?<tafkhim5>ر[${fdk}${sukuns}${shadda}]*)"
    "|(?<=^|[${fatha}${damma}${mandatoryWaqf}${prefereWaqf}${permissibleWaqf}${prefereWasl}](?:[${forbiddenWaqf}]|ا${ziaditHarf})?[ياى]?\\s?|${beforeAyaCond}|وا${ziaditHarf}?\\s|${takhallus}\\s|\\u0670\\u089C)(?:[ٱ]|\\u0627\\u034f?\\u0653|\\u0627\\u064E?)ل(?<tafkhim6>ل[${marks}]*)ه[${marks}]*م?[${marks}]*${endWordCond}"
    "|\\u0627\\u0670\\u089Cل(?<tafkhim6_2>ل[${marks}]*)ه[${marks}]*م?[${marks}]*${endWordCond}"
    "|(?<gray3>[${bases}][${ziaditHarf}])"
    "|(?<=[و][${sukuns}]?${maddClass}?|[و]${cgi}?\\u0654${cgi}?[${damma}${dammatan}]|[و][${damma}]|[${damma}${sukuns}][و][${fatha}])(?<gray3_indopak_1>[ا])(?=${endMarksCondOpt}(?:${endWordCond}|(?:${ayaCond})))"
    "|(?<=[${kasra}])(?<gray3_indopak_2>[ا])(?=[${bases}])(?!\\u0644[${marks}]*\\u0644[${marks}]*\\u0647[${marks}]*${endWordCond})";

static const char* OTHERS_MAD_TMPL =
    "(?<=[${bases}][${marks}]{0,5})(?<gray1>ٱ)(?!\\u0644[${marks}]*\\u0644[${marks}]*\\u0647[${marks}]*${endWordCond})"
    "|(?<=[اٱ])(?<gray2>ل(?![${marks}]*ل[${marks}]*ه[${marks}]*${endWordCond}))(?=[${bases}])"
    "|(?<gray4>[و])(?=${daggerAlef})|(?<gray4_1>[ى])(?=${daggerAlef}[${bases}])"
    "|(?<=${maddClass})(?<gray7>[وي])(?=${cgi}?${hamzaabove}${cgi}?[${marks}]*(?:ا[${ziaditHarf}]?)?${endWordCond})"
    "|(?<=${maddClass})(?<gray8>ل)(?=\\u0630\\u0651)"
    "|(?<tanween1>ن${meemiqlab}${cgi}?[${sukuns}]?)"
    "|(?<!${beforeAyaCond}|^)(?<tanween2>[من]${shadda}(?:[${fdkt}]|(?=${daggerAlef})))(?!${maddawaajib})"
    "|(?<tanween3>[${meemiqlab}${lowmeemiqlab}])(?=(?:ا[${ziaditHarf}]?)?(?<tanween3_a>${ayaCond})?)"
    "|(?<tanween4>م[${sukuns}]?)(?=\\sب)"
    "|(?<tanween5>ن[${bases}])"
    "|(?<tanween6>[ن${opentanween}${tanween}][${sukuns}]?[${bases}]?\\u06DF?${endMarksCondOpt}\\s(?<tanween7>[ينمو](?:[${shadda}]?[${fdkt}]|[${shadda}](?=${daggerAlef}))))"
    "|(?<tanween8>[ن${opentanween}${tanween}][${sukuns}]?[${bases}]?${endMarksCondOpt}\\s?[لر][${shadda}])"
    "|(?<tanween9>[${opentanween}${tanween}][${sukuns}]?[${bases}]?${endMarksCondOpt}\\s?[${IkfaaLetters}])"
    "|(?<tanween9_noon>[ن][${sukuns}]?[${bases}]?\\s?[${IkfaaLetters}])"
    "|(?<=[${fdk}])(?<gray6>[${bases}])(?<gray6_sukuns>[${sukuns}]?)(?=\\s?(?<gray6_1>[${bases}]${shadda}))"
    "|(?<tanween10>ـۨ[${sukuns}]?)"
    "|(?<!\\s|^)(?<madd5>(?:[يو${daggerAlef}${subAlef}][${sukuns}]?|[ا]))(?=[و\\u0649]?[${bases}][${harakat}][${marks}]?${waqfCond}|\\u0647\\u0650\\u06DB)(?!ا[${ziaditHarf}])"
    "|(?<madd4_1>[ى]${daggerAlef}${maddClass})${endWordCond}(?=(?<madd4_1_aya>${ayaCond})?)"
    "|(?<madd4_4>${daggerAlef}${maddClass})[ي][\\u06D9]?${endWordCond}(?=(?<madd4_4_aya>${ayaCond})?)"
    "|(?<=[ى])${daggerAlef}[${waqfMarks}]?${endWordCond}"
    "|(?<madd1>[او${smallMadd}]${cgi}?[${sukuns}]?${maddClass})(?=[${bases}][${shadda}${sukuns}]|[${bases}][${bases}])(?!وا)"
    "|(?<madd4_2>[اويى${smallMadd}]${cgi}?[${sukuns}]?${maddClass})(?=(?:ا[${ziaditHarf}])?(?<madd4_2_a>${waqfCond})?)"
    "|(?<madd5_1>ـ[${smallHighYeh}])(?=[و\\u0649]?[${bases}][${harakat}][${marks}]?${waqfCond}|\\u0647\\u0650\\u06DB)(?!ا[${ziaditHarf}])"
    "|(?<madd2_1>ـ[${smallHighYeh}])(?![${harakat}])"
    "|(?<madd2>[${smallMadd}])(?!${cgi}?${hamzaabove}|[${fdkt}]|${ayaCond})"
    "|[او${smallMadd}]${cgi}?[${sukuns}]?${maddClass}${ayaCond}"
    "|(?<madd3>[نكعصلمسق][${shadda}]?[${fatha}]?${maddClass})"
    "|(?<madd4_3>ࣳٓ)";

// IndoPak variant is made by replacing these two sub-parts, like in your TS:
static const char* INDO_REPLACE_SRC1 =
    R"REGEX(|(?<gray4>[و])(?=${daggerAlef})|(?<gray4_1>[ى])(?=${daggerAlef}[${bases}]))REGEX";

static const char* INDO_REPLACE_DST1 =
    R"REGEX(|(?<=${daggerAlef}${maddah}?)(?<gray4>[و])(?=[${bases}])|(?<gray4_1>[ى])(?=[${bases}])|(?<gray4_2>[و](?=[${basesNoAlef}])) )REGEX";

static const char* INDO_REPLACE_SRC2 =
    R"REGEX((?<=[${bases}][${marks}]*)(?<gray1>ٱ)(?!\u0644[${marks}]*\u0644[${marks}]*\u0647[${marks}]*${endWordCond}))REGEX";

static const char* INDO_REPLACE_DST2 =
    R"REGEX((?<=[${bases}][${marks}]*)(?<gray1>ا)(?!\u0644[${marks}]*\u0644[${marks}]*\u0647[${marks}]*${endWordCond})(?=[${bases}][${sukuns}${shadda}]|ل[${bases}]))REGEX";

// ---------- Build maps & compile ----------

static QHash<QString, QString> varsCommon() {
  QHash<QString, QString> v;
  v["sukuns"] = sukuns;
  v["marks"] = marks;
  v["waqfCond"] = waqfCond;
  v["elevationChars"] = elevationChars;
  v["loweringChars"] = loweringChars;
  v["fdk"] = fdk;
  v["kasra"] = QString(kasra);
  v["fatha"] = QString(fatha);
  v["kasras"] = QString(kasra) + QString(kasratan) + QString(openkasratan);
  v["bases"] = bases;
  v["IkfaaLetters"] = IkfaaLetters;
  v["maddClass"] = maddClass;
  v["smallMadd"] = smallMadd;
  v["cgi"] = QString(cgi);
  v["hamzaabove"] = QString(hamzaabove);
  v["daggerAlef"] = QString(daggerAlef);
  v["maddah"] = QString(maddah);
  v["maddawaajib"] = QString(maddawaajib);
  v["takhallus"] = QString(takhallus);
  v["ziaditHarf"] = QString(ziaditHarf);
  v["beforeAyaCond"] = beforeAyaCond;
  v["ayaCond"] = ayaCond;
  v["endWordCond"] = endWordCond;
  v["endMarksCondOpt"] = endMarksCondOpt;
  v["waqfMarks"] = waqfMarks;
  v["shadda"] = QString(shadda);
  v["harakat"] = harakat;
  v["damma"] = QString(damma);
  v["dammatan"] = QString(dammatan);
  v["prefereWasl"] = prefereWasl;
  v["prefereWaqf"] = prefereWaqf;
  v["mandatoryWaqf"] = mandatoryWaqf;
  v["permissibleWaqf"] = QString(permissibleWaqf);
  v["forbiddenWaqf"] = QString(forbiddenWaqf);
  v["fathatan"] = QString(fathatan);
  v["opentanween"] = opentanween;
  v["tanween"] = tanween;
  v["fdkt"] = fdkt;
  v["smallHighYeh"] = QString(smallHighYeh);
  v["meemiqlab"] = QString(meemiqlab);
  v["lowmeemiqlab"] = QString(lowmeemiqlab);
  v["basesNoAlef"] = QString(bases).remove(QChar(u'ا'));
  return v;
}

static QRegularExpression compileTafkhim() {
  auto v = varsCommon();
  QString pat = expand(QString::fromUtf8(TAFKHIM_TMPL), v);
  pat = uToX(pat);
  return makeRe(pat);
}

static QRegularExpression compileOthersMadinah() {
  auto v = varsCommon();
  QString pat = expand(QString::fromUtf8(OTHERS_MAD_TMPL), v);
  pat = uToX(pat);
  return makeRe(pat);
}

static QRegularExpression compileOthersIndopak() {
  auto v = varsCommon();
  QString src1 = uToX(expand(QString::fromUtf8(INDO_REPLACE_SRC1), v));
  QString dst1 = uToX(expand(QString::fromUtf8(INDO_REPLACE_DST1), v));
  QString src2 = uToX(expand(QString::fromUtf8(INDO_REPLACE_SRC2), v));
  QString dst2 = uToX(expand(QString::fromUtf8(INDO_REPLACE_DST2), v));

  QString mad = uToX(expand(QString::fromUtf8(OTHERS_MAD_TMPL), v));
  mad.replace(src1, dst1);
  mad.replace(src2, dst2);
  return makeRe(mad);
}

// Compile once at module load
static QRegularExpression TafkhimRE = compileTafkhim();
static QRegularExpression OthersREMadinah = compileOthersMadinah();
static QRegularExpression OthersREIndoPak = compileOthersIndopak();

// ---------- processing (same as before) ----------
static inline bool containsChar(const QString& s, QChar ch) { return s.contains(ch); }

// ---------- public API ----------
void TajweedService::applyTajweedForText(
    const QString& text,
    const std::function<void(int, const QString&)>& setTajweed,
    const std::function<void()>& resetIndex,
    bool isIndopak) const {
  // Pass 1: TafkhimRE
  int pos = 0;
  while (true) {
    QRegularExpressionMatch m = TafkhimRE.match(text, pos);
    if (!m.hasMatch()) break;

    auto tagRun = [&](int a, int b, const QString& style) { for (int i=a; i<b; ++i) setTajweed(i, style); };
    auto tag1 = [&](int i, const QString& style) { if (i >= 0) setTajweed(i, style); };
    auto grp = [&](const char* name) -> QPair<int, int> {
      int s = m.capturedStart(name);
      if (s < 0) return {-1, -1};
      return {s, m.capturedEnd(name)};
    };

    QPair<int, int> g;
    if ((g = grp("tafkhim_reh1")).first >= 0) {
      tag1(g.first, "tafkim");
      tag1(g.first + 1, "tafkim");
    } else if ((g = grp("tafkhim1")).first >= 0 || (g = grp("tafkhim5")).first >= 0 || (g = grp("tafkhim6")).first >= 0 || (g = grp("tafkhim6_2")).first >= 0) {
      tagRun(g.first, g.second, "tafkim");
    } else if ((g = grp("tafkhim2")).first >= 0) {
      int firstPos = g.first;
      int e = m.capturedStart("tafkhim2_1");
      if (e >= 0 && text[e] != QChar(u' ')) {
        QChar ch = text[firstPos];
        if (ch == fatha || ch == damma) tag1(firstPos, "tafkim");
      }
    } else if ((g = grp("tafkhim3")).first >= 0) {
      tag1(g.first, "tafkim");
      auto g4 = grp("tafkhim4");
      if (g4.first >= 0) {
        QChar ch = text[g4.first];
        int e = m.capturedStart("tafkhim4_1");
        if (e >= 0) {
          QChar endchar = text[e];
          if (endchar != QChar(u' ')) {
            if (ch == fatha || ch == damma || containsChar(sukuns, ch)) {
              tag1(g4.first, "tafkim");
            } else if (ch == shadda) {
              tag1(g4.first, "tafkim");
              int nextIndex = g4.first + 1;
              if (nextIndex < g4.second) {
                QChar nextch = text[nextIndex];
                if (nextch == fatha || nextch == damma) tag1(nextIndex, "tafkim");
              }
            }
          } else {
            if (containsChar(sukuns, ch) || ch == shadda) tag1(g4.first, "tafkim");
          }
        }
      }
    } else if ((g = grp("kalkala1")).first >= 0) {
      tag1(g.first, "lkalkala");
      tag1(g.first + 1, "lkalkala");
    } else if ((g = grp("kalkala2")).first >= 0) {
      tag1(g.first, "lkalkala");
      if (g.first + 1 < g.second) tag1(g.first + 1, "lkalkala");
    } else if ((g = grp("gray3")).first >= 0) {
      tag1(g.first, "lgray");
      tag1(g.first + 1, "lgray");
    } else if ((g = grp("gray3_indopak_1")).first >= 0 || (g = grp("gray3_indopak_2")).first >= 0) {
      tag1(g.first, "lgray");
    }

    pos = m.capturedEnd();
  }

  resetIndex();

  // Pass 2: Others
  const QRegularExpression& OthersRE = isIndopak ? OthersREIndoPak : OthersREMadinah;
  pos = 0;
  while (true) {
    QRegularExpressionMatch m = OthersRE.match(text, pos);
    if (!m.hasMatch()) break;

    auto grp = [&](const char* name) -> QPair<int, int> {
      int s = m.capturedStart(name);
      if (s < 0) return {-1, -1};
      return {s, m.capturedEnd(name)};
    };
    auto tag1 = [&](int i, const QString& style) { if (i >= 0) setTajweed(i, style); };
    auto tagRun = [&](int a, int b, const QString& style) { for (int i=a;i<b;++i) setTajweed(i, style); };

    QPair<int, int> g;
    if ((g = grp("tanween1")).first >= 0) {
      int p = g.first;
      tag1(p++, "lgray");
      while (p < g.second) tag1(p++, "green");
    } else if ((g = grp("tanween2")).first >= 0) {
      int firstPos = g.first;
      tag1(firstPos, "green");
      tag1(firstPos + 1, "green");
      const QString cap = m.captured("tanween2");
      if (cap.size() >= 3 && alltanween.contains(cap[2])) {
        // JS had OthersRE.lastIndex--; best approximation:
        pos = m.capturedStart() + 1;
        tag1(firstPos + 2, "green");
        continue;
      } else {
        tag1(firstPos + 2, "green");
      }
    } else if ((g = grp("tanween3")).first >= 0) {
      if (m.capturedStart("tanween3_a") < 0) tag1(g.first, "green");
    } else if ((g = grp("tanween4")).first >= 0) {
      tag1(g.first, "green");
      if (g.first + 1 < g.second) tag1(g.first + 1, "green");
    } else if ((g = grp("tanween5")).first >= 0) {
      tag1(g.first, "green");
    } else if ((g = grp("tanween6")).first >= 0) {
      const QString t6 = m.captured("tanween6");
      const QString t7 = m.captured("tanween7");
      if (!(t6.startsWith(u'ن') && t7.startsWith(u'ن'))) {
        tag1(g.first, "lgray");
        if (g.first + 1 < g.second) tag1(g.first + 1, "lgray");
      }
      auto g7 = grp("tanween7");
      int greenPos = g7.first;
      tag1(greenPos, "green");
      tag1(greenPos + 1, "green");
      if (greenPos + 2 < g7.second) tag1(greenPos + 2, "green");
    } else if ((g = grp("tanween8")).first >= 0) {
      tag1(g.first, "lgray");
      if (g.first + 1 < g.second) tag1(g.first + 1, "lgray");
    } else if ((g = grp("tanween9")).first >= 0 || (g = grp("tanween9_noon")).first >= 0) {
      tag1(g.first, "green");
      if (g.first + 1 < g.second) tag1(g.first + 1, "green");
    } else if ((g = grp("tanween10")).first >= 0) {
      tag1(g.first, "green");
      tag1(g.first + 1, "green");
      if (g.first + 2 < g.second) tag1(g.first + 2, "green");
    } else if ((g = grp("gray1")).first >= 0 || (g = grp("gray2")).first >= 0 || (g = grp("gray4")).first >= 0 || (g = grp("gray4_1")).first >= 0 || (g = grp("gray4_2")).first >= 0 || (g = grp("gray5")).first >= 0) {
      tag1(g.first, "lgray");
    } else if ((g = grp("gray6")).first >= 0) {
      const QString gray6_suk = m.captured("gray6_sukuns");
      if (!gray6_suk.isEmpty()) {
        const QChar firstChar = m.captured("gray6").at(0);
        const QChar secondChar = m.captured("gray6_1").at(0);
        if (firstChar != secondChar) {
          if (firstChar != QChar(u'ط')) {
            tag1(g.first, "lgray");
            tag1(g.first + 1, "lgray");
          } else {
            tag1(g.first, "tafkim");
            tag1(g.first + 1, "tafkim");
          }
        }
      } else {
        const QChar a = m.captured("gray6").at(0);
        const QChar b = m.captured("gray6_1").at(0);
        if (!(a == QChar(u'ي') && b == QChar(u'ي')) && a != b)
          tag1(g.first, "lgray");
        else if (a == QChar(u'ي') && b == QChar(u'ي'))
          tag1(g.first, "lgray");
      }
    } else if ((g = grp("gray7")).first >= 0 || (g = grp("gray8")).first >= 0) {
      tag1(g.first, "lgray");
    } else if ((g = grp("madd1")).first >= 0) {
      tag1(g.first, "red4");
      tag1(g.first + 1, "red4");
      if (g.first + 2 < g.second) tag1(g.first + 2, "red4");
    } else if ((g = grp("madd2")).first >= 0) {
      tag1(g.first, "red1");
    } else if ((g = grp("madd2_1")).first >= 0) {
      tag1(g.first, "red1");
      tag1(g.first + 1, "red1");
    } else if ((g = grp("madd5_1")).first >= 0) {
      tag1(g.first, "red2");
      tag1(g.first + 1, "red2");
    } else if ((g = grp("madd3")).first >= 0) {
      tagRun(g.first, g.second, "red4");
    } else if ((g = grp("madd4_1")).first >= 0) {
      if (m.capturedStart("madd4_1_aya") < 0) {
        tag1(g.first, "red3");
        tag1(g.first + 1, "red3");
        if (g.first + 2 < g.second) tag1(g.first + 2, "red3");
      }
    } else if ((g = grp("madd4_4")).first >= 0) {
      if (m.capturedStart("madd4_4_aya") < 0) {
        tag1(g.first, "red3");
        tag1(g.first + 1, "red3");
        tag1(g.first + 2, "red3");
      }
    } else if ((g = grp("madd4_2")).first >= 0) {
      int firstPos = g.first;
      bool skipAya = (m.capturedStart("madd4_2_a") >= 0) && text.at(m.capturedEnd("madd4_2_a") - 1) == QChar(u'۝');
      if (!skipAya) {
        const QChar c0 = m.captured("madd4_2").at(0);
        if (m.capturedStart("madd4_2_a") < 0 || c0 == smallYeh || c0 == smallWaw || c0 == invertedDamma || c0 == subAlef) {
          tag1(firstPos, "red3");
        }
        tag1(firstPos + 1, "red3");
        if (firstPos + 2 < g.second) tag1(firstPos + 2, "red3");
      }
    } else if ((g = grp("madd5")).first >= 0) {
      tag1(g.first, "red2");
      if (g.first + 1 < g.second) tag1(g.first + 1, "red2");
    } else if ((g = grp("madd4_3")).first >= 0) {
      tag1(g.first, "red3");
      tag1(g.first + 1, "red3");
    }

    pos = m.capturedEnd();
  }
}

QVector<QMap<int, QString>>
TajweedService::applyTajweedByPage(const QVector<LineToJustify>& lines, bool isIndopak) const {
  QVector<QMap<int, QString>> result(lines.size());

  struct Range {
    int lineIndex;
    int start;
    int end;
  };
  QVector<Range> ranges;
  QString concat;
  concat.reserve(4096);

  int lastIndex = 0;
  for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
    auto line = lines[lineIndex];
    if (line.lineType == LineType::Sura) continue;

    const QString& lineText = line.text;
    const QString addedText = (line.lineType == LineType::Bism) ? QStringLiteral(" ۝ ") : QStringLiteral(" ");

    concat += lineText;
    ranges.push_back(Range{lineIndex, lastIndex, lastIndex + lineText.size()});
    lastIndex += lineText.size();

    concat += addedText;
    lastIndex += addedText.size();
  }

  int globalLastIndex = 0;
  auto setTajweed = [&](int pos, const QString& style) {
    while (globalLastIndex < ranges.size()) {
      const auto& r = ranges[globalLastIndex];
      if (pos >= r.start) {
        if (pos < r.end) {
          result[r.lineIndex][pos - r.start] = style;
          break;
        } else {
          ++globalLastIndex;
        }
      } else {
        break;
      }
    }
  };
  auto resetIndex = [&]() { globalLastIndex = 0; };

  applyTajweedForText(concat, setTajweed, resetIndex, isIndopak);
  return result;
}
