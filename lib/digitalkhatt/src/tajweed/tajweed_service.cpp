// tajweed_service.cpp
//
// Faithful C++ port of tajweed.service.ts using PCRE2-16 + std::u16string.
// See tajweed_service.h for build requirements (PCRE2 >= 10.43, 16-bit,
// UCP/UTF, variable-length lookbehind support).

#define PCRE2_CODE_UNIT_WIDTH 16
#include <pcre2.h>

#include "digitalkhatt/core/tajweed/tajweed_service.h"

#include <cstring>
#include <initializer_list>
#include <stdexcept>

namespace digitalkhatt::tajweed {

// --- small string helpers matching the JS operations used in the original -

// JS String.prototype.replace(needle, replacement) — replaces only the
// FIRST occurrence; returns the input unchanged if `needle` isn't found.
std::u16string replaceFirst(const std::u16string& haystack,
                            const std::u16string& needle,
                            const std::u16string& replacement) {
  size_t pos = haystack.find(needle);
  if (pos == std::u16string::npos) return haystack;
  std::u16string result = haystack;
  result.replace(pos, needle.size(), replacement);
  return result;
}

// JS `set.includes(ch)` for a single-character haystack membership test.
bool contains(const std::u16string& set, char16_t c) {
  return set.find(c) != std::u16string::npos;
}
bool contains(const std::u16string& set, std::optional<char16_t> c) {
  return c.has_value() && contains(set, *c);
}

std::u16string toU16(const char* s) {
  return std::u16string(s, s + std::strlen(s));
}

// --- PCRE2-16 wrapper --------------------------------------------------

// Wraps one compiled pattern and provides:
//  - a global-match iterator equivalent to a JS RegExp with the 'g' flag
//    (manual lastIndex bookkeeping, incl. the +1 nudge on zero-length
//    matches that JS engines do automatically), and
//  - named-group access equivalent to `match.indices.groups[name]`
//    (start/end pair, or "not present" if the group didn't participate),
//    plus a convenience accessor equivalent to `match.groups[name][i]`.
class CompiledTajweedRegex {
 public:
  explicit CompiledTajweedRegex(const std::u16string& pattern) {
    int errorcode = 0;
    PCRE2_SIZE erroroffset = 0;
    code_ = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(pattern.c_str()),
        pattern.size(),
        PCRE2_UTF | PCRE2_UCP | PCRE2_DOLLAR_ENDONLY,
        &errorcode,
        &erroroffset,
        nullptr);
    if (!code_) {
      PCRE2_UCHAR errbuf[256];
      pcre2_get_error_message(errorcode, errbuf, 256);
      std::string msg;
      for (PCRE2_UCHAR* p = errbuf; *p; ++p) msg += static_cast<char>(*p);
      throw std::runtime_error(
          "pcre2_compile failed at offset " + std::to_string(erroroffset) +
          ": " + msg);
    }
    matchData_ = pcre2_match_data_create_from_pattern(code_, nullptr);
  }

  ~CompiledTajweedRegex() {
    if (matchData_) pcre2_match_data_free(matchData_);
    if (code_) pcre2_code_free(code_);
  }

  CompiledTajweedRegex(const CompiledTajweedRegex&) = delete;
  CompiledTajweedRegex& operator=(const CompiledTajweedRegex&) = delete;

  class Match {
   public:
    Match(const CompiledTajweedRegex& owner, const std::u16string& subject,
          PCRE2_SIZE* ovector, uint32_t ovectorCount)
        : owner_(&owner), subject_(&subject), ovector_(ovector), ovectorCount_(ovectorCount) {}

    size_t start() const { return ovector_[0]; }
    size_t end() const { return ovector_[1]; }

    // Equivalent to `match.indices.groups[name]`: nullopt if the named
    // group doesn't exist in this pattern, or didn't participate.
    std::optional<std::pair<size_t, size_t>> group(const char* name) const {
      std::u16string name16 = toU16(name);
      int idx = pcre2_substring_number_from_name(
          owner_->code_, reinterpret_cast<PCRE2_SPTR>(name16.c_str()));
      if (idx <= 0 || static_cast<uint32_t>(idx) >= ovectorCount_) return std::nullopt;
      size_t s = ovector_[2 * idx];
      size_t e = ovector_[2 * idx + 1];
      if (s == PCRE2_UNSET || e == PCRE2_UNSET) return std::nullopt;
      return std::make_pair(s, e);
    }

    // Equivalent to `match.groups[name]?.[idx]` (undefined if the group
    // is absent, or idx is beyond the captured span).
    std::optional<char16_t> charInGroup(const char* name, size_t idx) const {
      auto g = group(name);
      if (!g) return std::nullopt;
      size_t pos = g->first + idx;
      if (pos >= g->second) return std::nullopt;
      return (*subject_)[pos];
    }

    // Equivalent to `match.groups[name].at(-1)` (last captured
    // character), or nullopt if the group is absent or captured an
    // empty span.
    std::optional<char16_t> lastCharInGroup(const char* name) const {
      auto g = group(name);
      if (!g || g->second <= g->first) return std::nullopt;
      return (*subject_)[g->second - 1];
    }

    // First group among `names` that participated (mirrors JS
    // `groups.a || groups.b || ...`).
    std::optional<std::pair<size_t, size_t>> firstGroup(
        std::initializer_list<const char*> names) const {
      for (const char* n : names) {
        if (auto g = group(n)) return g;
      }
      return std::nullopt;
    }

   private:
    const CompiledTajweedRegex* owner_;
    const std::u16string* subject_;
    PCRE2_SIZE* ovector_;
    uint32_t ovectorCount_;
  };

  class GlobalIterator {
   public:
    GlobalIterator(CompiledTajweedRegex& owner, const std::u16string& subject)
        : owner_(&owner), subject_(&subject) {}

    std::optional<Match> next() {
      if (offset_ > subject_->size()) return std::nullopt;
      int rc = pcre2_match(
          owner_->code_,
          reinterpret_cast<PCRE2_SPTR>(subject_->c_str()),
          subject_->size(),
          offset_,
          0,
          owner_->matchData_,
          nullptr);
      if (rc < 0) return std::nullopt;  // no more matches (PCRE2_ERROR_NOMATCH)

      PCRE2_SIZE* ov = pcre2_get_ovector_pointer(owner_->matchData_);
      uint32_t ovCount = pcre2_get_ovector_count(owner_->matchData_);
      size_t matchStart = ov[0];
      size_t matchEnd = ov[1];

      // Mirrors the JS engine's automatic lastIndex handling for a
      // global regex: advance past the match, or by one code unit for
      // a zero-length match (to avoid looping forever).
      offset_ = (matchEnd == matchStart) ? matchEnd + 1 : matchEnd;
      (void)matchStart;

      return Match(*owner_, *subject_, ov, ovCount);
    }

    // Mirrors the single `OthersRE.lastIndex--` in the original source
    // (deliberately re-examines the previous code unit on the next
    // iteration).
    void rewindOne() {
      if (offset_ > 0) offset_--;
    }

   private:
    CompiledTajweedRegex* owner_;
    const std::u16string* subject_;
    PCRE2_SIZE offset_ = 0;
  };

  GlobalIterator globalIterator(const std::u16string& subject) {
    return GlobalIterator(*this, subject);
  }

 private:
  friend class Match;
  pcre2_code* code_ = nullptr;
  pcre2_match_data* matchData_ = nullptr;
};

// JS `bases.replace('ا', '')` — remove the first occurrence of a single
// character.
inline int I(size_t x) { return static_cast<int>(x); }

std::u16string removeFirstChar(const std::u16string& s, char16_t c) {
  std::u16string result = s;
  size_t pos = result.find(c);
  if (pos != std::u16string::npos) result.erase(pos, 1);
  return result;
}

// =========================================================================
// Pattern construction (translated 1:1 from the TS constructor)
// =========================================================================

namespace {

// --- module-level constants (character sets used by the patterns) ------

const std::u16string rightNoJoinLetters = u"ادذرزوؤأٱإءة";
const std::u16string dualJoinLetters = u"بتثجحخسشصضطظعغفقكلمنهيئى";
const std::u16string bases = rightNoJoinLetters + dualJoinLetters;
const std::u16string IkfaaLetters = u"صذثكجشقسدطزفتضظ";

const std::u16string fathatan = u"\u064B";
const std::u16string dammatan = u"\u064C";
const std::u16string kasratan = u"\u064D";
const std::u16string fatha = u"\u064E";
const std::u16string damma = u"\u064F";
const std::u16string kasra = u"\u0650";
const std::u16string shadda = u"\u0651";
const std::u16string sukuns = u"\u0652\u06E1";
const std::u16string openfathatan = u"\u08F0";
const std::u16string opendammatan = u"\u08F1";
const std::u16string openkasratan = u"\u08F2";
const std::u16string tanween = fathatan + dammatan + kasratan;
const std::u16string opentanween = openfathatan + opendammatan + openkasratan;
const std::u16string alltanween = tanween + opentanween;
const std::u16string fdk = fatha + damma + kasra;
const std::u16string fdkt = fdk + alltanween;
const std::u16string harakat = fdkt + sukuns + shadda;

const std::u16string prefereWaslIndoPak = u"\u08D5\u0617\u08D7";
const std::u16string mandatoryWaqfIndoPak = u"\u08DE\u08DF\u08DD\u08DB";
const std::u16string prefereWaqfIndoPak = u"\u0615\u08D6";
const std::u16string takhallus = u"\u0614";
const std::u16string disputedEndofAyah = u"\u08E2";

const std::u16string prefereWasl = u"\u06D6" + prefereWaslIndoPak;
const std::u16string prefereWaqf = u"\u06D7" + prefereWaqfIndoPak;
const std::u16string mandatoryWaqf = u"\u06D8" + mandatoryWaqfIndoPak;
const std::u16string forbiddenWaqf = u"\u06D9";
const std::u16string permissibleWaqf = u"\u06DA";
const std::u16string waqfInOneOfTwo = u"\u06DB";

const std::u16string waqfMarks = prefereWasl + prefereWaqf + mandatoryWaqf +
                                 forbiddenWaqf + permissibleWaqf + waqfInOneOfTwo +
                                 disputedEndofAyah;

const std::u16string maddah = u"\u0653";
const std::u16string maddawaajib = u"\u089C";
const std::u16string maddClass = u"[" + maddah + maddawaajib + u"]";
const std::u16string ziaditHarf = u"\u06DF";
const std::u16string ziaditHarfWasl = u"\u06E0";
const std::u16string meemiqlab = u"\u06E2";
const std::u16string lowmeemiqlab = u"\u06ED";
const std::u16string daggerAlef = u"\u0670";
const std::u16string smallWaw = u"\u06E5";
const std::u16string smallYeh = u"\u06E6";
const std::u16string invertedDamma = u"\u0657";
const std::u16string subAlef = u"\u0656";
const std::u16string smallMadd = daggerAlef + smallWaw + smallYeh + invertedDamma + subAlef;
const std::u16string smallHighYeh = u"\u06E7";
const std::u16string smallHighWaw = u"\u08F3";
const std::u16string highCircle = u"\u06EC";
const std::u16string lowCircle = u"\u065C";
const std::u16string hamzaabove = u"\u0654";
const std::u16string hamzabelow = u"\u0655";
const std::u16string smallHighSeen = u"\u06DC";
const std::u16string smallLowSeen = u"\u06E3";
const std::u16string smallHighNoon = u"\u06E8";
const std::u16string cgi = u"\u034F";

const std::u16string marks = harakat + waqfMarks + maddah + maddawaajib + ziaditHarf +
                             ziaditHarfWasl + meemiqlab + lowmeemiqlab + smallMadd +
                             smallHighYeh + smallHighWaw + highCircle + lowCircle +
                             hamzaabove + hamzabelow + smallHighSeen + smallLowSeen +
                             smallHighNoon + cgi + takhallus;

const std::u16string ayaCond = u"\\s?[\u06E9]?\\s?\u06DD";
// included waqfInOneOfTwo : occurs 6 times and has an effect only in the second
// occurrence (page 2) as in the Tajweed Mushaf
const std::u16string waqfCond =
    u"(?:" + ayaCond + u"|[" + (prefereWasl + prefereWaqf + mandatoryWaqf + permissibleWaqf) +
    u"]|$)";
const std::u16string endWordCond = u"(?:\\s|$)";
const std::u16string endMarksCondOpt =
    u"(?:[" + (waqfMarks + takhallus + disputedEndofAyah) + u"]*)";

const std::u16string elevationChars = u"طقصخغضظ";

std::u16string makeLoweringChars() {
  std::u16string result;
  for (size_t i = 0; i < bases.size(); i++) {
    if (elevationChars.find(bases[i]) == std::u16string::npos) {
      result += bases[i];
    }
  }
  return result;
}

std::u16string makeDigits() {
  std::u16string result;
  for (char16_t digit = 1632; digit <= 1641; digit++) {
    result += digit;
  }
  return result;
}

const std::u16string loweringChars = makeLoweringChars();
const std::u16string digits = makeDigits();

const std::u16string beforeAyaCond = u"[" + digits + u"\u06DE][" + waqfMarks + u"]?\\s";

}  // namespace

TajweedService::TajweedService() {
  // التجويد الميسر الحذيفي (https://ar.islamway.net/book/28837/%D8%A7%D9%84%D8%AA%D8%AC%D9%88%D9%8A%D8%AF-%D8%A7%D9%84%D9%85%D9%8A%D8%B3%D8%B1-%D9%84%D8%B9%D8%A7%D9%85-1442-%D9%87)

  // kalkala
  std::u16string pattern = u"(?<kalkala1>[طقدجب][" + (sukuns) + u"])";
  pattern += u"|(?<kalkala2>[طقدجب]" + (shadda) + u"?)(?=[" + (marks) + u"]*" + (waqfCond) + u")";

  // Tafkhim
  pattern += u"|(?<tafkhim1>[" + (elevationChars) + u"]" + (shadda) + u"?[" + (fdk + sukuns) + u"]?)";

  // Tafkhim Reh Indopak Hamzat wasl
  pattern += u"|(?<=\\sا" + (kasra) + u")(?<tafkhim_reh1>ر[" + (sukuns) + u"])";

  // Tafkhim Reh when waqf

  const std::u16string kasras = kasra + kasratan + openkasratan;

  pattern += u"|(?<=" + (kasra) + u"|" + (kasra) + u"[" + (loweringChars) + u"][" + (sukuns) + u"]|[ي][" + (sukuns) + u"]?|\u0650\u0637\u0652)ر(?<tafkhim2>[" + (marks) + u"]*)(?<tafkhim2_1>" + (waqfCond) + u")";  // ترقيق الراء
  pattern += u"|(?<tafkhim3>ر)(?<tafkhim4>[" + (marks) + u"]*)(?<tafkhim4_1>" + (waqfCond) + u")";                                                                                                                   // otherwise tafkhim Reh without marks during waqf

  // Tafkhim Reh when wasl

  pattern += u"|ر(?=" + (shadda) + u"?[" + (kasras) + u"])";                                                // ترقيق الراء
  pattern += u"|(?<=" + (kasra) + u")ر[" + (sukuns) + u"](?![" + (elevationChars) + u"]" + (fatha) + u")";  // ترقيق الراء
  pattern += u"|(?<tafkhim5>ر[" + (fdk + sukuns + shadda) + u"]*)";                                         // otherwise tafkhim Reh with marks

  // Allah Tafkhim lam
  pattern += u"|(?<=^|[" + (fatha + damma + mandatoryWaqf + prefereWaqf + permissibleWaqf + prefereWasl) + u"](?:[" + (forbiddenWaqf) + u"]|ا" + (ziaditHarf) + u")?[ياى]?\\s?|" + (beforeAyaCond) + u"|وا[" + (ziaditHarf) + u"]?\\s|" + (takhallus) + u"\\s|\u0670\u089C)(?:[ٱ]|\u0627\u034f?\u0653|\u0627\u064E?)ل(?<tafkhim6>ل[" + (marks) + u"]*)ه[" + (marks) + u"]*م?[" + (marks) + u"]*" + (endWordCond);  // prettier-ignore
  pattern += u"|\u0627\u0670\u089Cل(?<tafkhim6_2>ل[" + (marks) + u"]*)ه[" + (marks) + u"]*م?[" + (marks) + u"]*" + (endWordCond);                                                                                                                                                                                                                                                                                  // prettier-ignore

  const std::u16string lamlamhehOfAllahSWTSeq = u"\u0644[" + (marks) + u"]*ل[" + (marks) + u"]*ه[" + (marks) + u"]*" + (endWordCond);

  // Never pronounced
  pattern += u"|(?<gray3>[" + (bases) + u"][" + (ziaditHarf) + u"])";                                                                                                                                                                                                                                                      // never pronounced indicated by special chars
  pattern += u"|(?<=[و][" + (sukuns) + u"]?" + (maddClass) + u"?|[و]" + (cgi) + u"?\u0654" + (cgi) + u"?[" + (damma) + (dammatan) + u"]|[و][" + (damma) + u"]|[" + (damma) + (sukuns) + u"][و][" + (fatha) + u"])(?<gray3_indopak_1>[ا])(?=" + (endMarksCondOpt) + u"(?:" + (endWordCond) + u"|(?:" + (ayaCond) + u")))";  // prettier-ignore
  pattern += u"|(?<=[" + (kasra) + u"])(?<gray3_indopak_2>[ا])(?=[" + (bases) + u"])(?!" + (lamlamhehOfAllahSWTSeq) + u")";

  tafkhimRE_ = std::make_unique<CompiledTajweedRegex>(pattern);

  // gray
  // NOTE: the original JS is `(?<=[bases][marks]*)`, an unbounded
  // Kleene star. PCRE2 accepts variable-length lookbehind (10.43+) but
  // still requires a *finite* upper bound on its length -- bare `*`
  // inside a lookbehind is rejected with "length of lookbehind assertion
  // is not limited". {0,6} gives the same practical behavior (real
  // Quranic text never stacks anywhere near 6 combining marks on one
  // letter) while satisfying that requirement.
  const std::u16string greyHamzatWaslInsideWordMadinah = u"(?<=[" + (bases) + u"][" + (marks) + u"]{0,6})(?<gray1>ٱ)(?!" + (lamlamhehOfAllahSWTSeq) + u")";  // همزة الوصل داخل الكلمة
  pattern = greyHamzatWaslInsideWordMadinah;
  pattern += u"|(?<=[اٱ])(?<gray2>ل(?![" + (marks) + u"]*ل[" + (marks) + u"]*ه[" + (marks) + u"]*" + (endWordCond) + u"))(?=[" + (bases) + u"])";  // اللام الشمسية
  const std::u16string greyWawYehMadinah = u"|(?<gray4>[و])(?=" + (daggerAlef) + u")|(?<gray4_1>[ى])(?=" + (daggerAlef) + u"[" + (bases) + u"])";  // waw with dagger alef above or initial or medial alef maksura with dagger alef above
  pattern += greyWawYehMadinah;
  pattern += u"|(?<=" + (maddClass) + u")(?<gray7>[وي])(?=" + (cgi) + u"?" + (hamzaabove) + (cgi) + u"?[" + (marks) + u"]*(?:ا[" + (ziaditHarf) + u"]?)?" + (endWordCond) + u")";  // الهمزة ترسم بغير كرسي (see توضيح للمتخصصين في القراءة par 10 of tajweed mushaf)
  pattern += u"|(?<=" + (maddClass) + u")(?<gray8>ل)(?=\u0630\u0651)";                                                                                                             // silent lam in ءَآلذَّكَرَيْنِ

  // tanween
  pattern += u"|(?<tanween1>ن" + (meemiqlab) + (cgi) + u"?[" + (sukuns) + u"]?)";
  pattern += u"|(?<!" + (beforeAyaCond) + u"|^)(?<tanween2>[من]" + (shadda) + u"(?:[" + (fdkt) + u"]|(?=" + (daggerAlef) + u")))(?!" + (maddawaajib) + u")";
  pattern += u"|(?<tanween3>[" + (meemiqlab + lowmeemiqlab) + u"])(?=(?:ا[" + (ziaditHarf) + u"]?)?(?<tanween3_a>" + (ayaCond) + u")?)";  // prettier-ignore
  pattern += u"|(?<tanween4>م[" + (sukuns) + u"]?)(?=\\sب)";
  pattern += u"|(?<tanween5>ن[" + (bases) + u"])";
  // النون الساكنة والتنوين
  pattern += u"|(?<tanween6>[ن" + (opentanween) + (tanween) + u"][" + (sukuns) + u"]?)[" + (bases) + u"]?\u06DF?" + (endMarksCondOpt) + u"\\s(?<tanween7>[ينمو](?:[" + (shadda) + u"]?[" + (fdkt) + u"]|[" + (shadda) + u"](?=" + (daggerAlef) + u")))";  // إدغام بغنة
  pattern += u"|(?<tanween8>[ن" + (opentanween) + (tanween) + u"][" + (sukuns) + u"]?)[" + (bases) + u"]?" + (endMarksCondOpt) + u"\\s?[لر][" + (shadda) + u"]";                                                                                          // إدغام بغير غنة
  pattern += u"|(?<tanween9>[" + (opentanween) + (tanween) + u"][" + (sukuns) + u"]?)[" + (bases) + u"]?" + (endMarksCondOpt) + u"\\s?[" + (IkfaaLetters) + u"]";                                                                                         // الإخفاء
  pattern += u"|(?<tanween9_noon>[ن][" + (sukuns) + u"]?)[" + (bases) + u"]?\\s?[" + (IkfaaLetters) + u"]";                                                                                                                                               // الإخفاء
  pattern += u"|(?<=[" + (fdk) + u"])(?<gray6>[" + (bases) + u"])(?<gray6_sukuns>[" + (sukuns) + u"]?)(?=\\s?(?<gray6_1>[" + (bases) + u"]" + (shadda) + u"))";                                                                                           // الإدغام الكامل
  pattern += u"|(?<tanween10>ـۨ[" + (sukuns) + u"]?)";

  const std::u16string madJaizAssert = u"(?=[و\u0649]?[" + (bases) + u"][" + (harakat) + u"][" + (marks) + u"]?" + (waqfCond) + u"|\u0647\u0650\u06DB)(?!ا[" + (ziaditHarf) + u"])";

  // madd
  pattern += u"|(?<!\\s|^)(?<madd5>(?:[يو" + (daggerAlef) + (subAlef) + u"][" + (sukuns) + u"]?|[ا]))" + (madJaizAssert);
  pattern += u"|(?<madd4_1>[ى]" + (daggerAlef) + (maddClass) + u")" + (endWordCond) + u"(?=(?<madd4_1_aya>" + (ayaCond) + u")?)";  // final alefmaksura followed by dagger alef that is followed by maddah
  pattern += u"|(?<madd4_4>" + (daggerAlef) + (maddClass) + u")[ي][\u06D9]?" + (endWordCond) + u"(?=(?<madd4_4_aya>" + (ayaCond) + u")?)";
  pattern += u"|(?<=[ى])" + (daggerAlef) + u"[" + (waqfMarks) + u"]?" + (endWordCond);                                                                                                                      // no coloring of dagger alef when final alefmaksura followed by dagger alef that is not followed by maddah
  pattern += u"|(?<madd1>[او" + (smallMadd) + u"]" + (cgi) + u"?[" + (sukuns) + u"]?" + (maddClass) + u")(?=[" + (bases) + u"][" + (shadda) + (sukuns) + u"]|[" + (bases) + u"][" + (bases) + u"])(?!وا)";  // 6 count madd (red4)
  pattern += u"|(?<madd4_2>[اويى" + (smallMadd) + u"]" + (cgi) + u"?[" + (sukuns) + u"]?" + (maddClass) + u")(?=(?:ا[" + (ziaditHarf) + u"])?(?<madd4_2_a>" + (waqfCond) + u")?)";
  pattern += u"|(?<madd5_1>ـ[" + (smallHighYeh) + u"])" + (madJaizAssert);
  pattern += u"|(?<madd2_1>ـ[" + (smallHighYeh) + u"])(?![" + (harakat) + u"])";
  pattern += u"|(?<madd2>[" + (smallMadd) + u"])(?!" + (cgi) + u"?" + (hamzaabove) + u"|[" + (fdkt) + u"]|" + (ayaCond) + u")";
  pattern += u"|[او" + (smallMadd) + u"]" + (cgi) + u"?[" + (sukuns) + u"]?" + (maddClass) + (ayaCond);
  pattern += u"|(?<madd3>[نكعصلمسق][" + (shadda) + u"]?[" + (fatha) + u"]?" + (maddClass) + u")";  // 6 count madd (red4)
  pattern += u"|(?<madd4_3>ࣳٓ)";                                                                     // madd wajeeb 4-5 (red3)

  const std::u16string greyWawYehIndopak = u"|(?<=" + (daggerAlef) + (maddah) + u"?)(?<gray4>[و])(?=[" + (bases) + u"])|(?<gray4_1>[ى])(?=[" + (bases) + u"])|(?<gray4_2>[و](?=[" + (removeFirstChar(bases, u'ا')) + u"]))";
  const std::u16string greyHamzatWaslInsideWordIndoPak = u"(?<=[" + (bases) + u"][" + (marks) + u"]{0,20})(?<gray1>ا)(?!" + (lamlamhehOfAllahSWTSeq) + u")(?=[" + (bases) + u"][" + (sukuns) + (shadda) + u"]|ل[" + (bases) + u"])";  // همزة الوصل داخل الكلمة

  std::u16string patternIndopak = replaceFirst(pattern, greyWawYehMadinah, greyWawYehIndopak);
  patternIndopak = replaceFirst(patternIndopak, greyHamzatWaslInsideWordMadinah, greyHamzatWaslInsideWordIndoPak);

  othersREMadinah_ = std::make_unique<CompiledTajweedRegex>(pattern);
  othersREIndoPak_ = std::make_unique<CompiledTajweedRegex>(patternIndopak);
}

TajweedService::~TajweedService() = default;

std::string static toNarrow(const std::u16string& s) {
  return std::string(s.begin(), s.end());
}

// =========================================================================
// applyTajweedForText — translated 1:1 from the two `while (match = ...)`
// loops in the TS source.
// =========================================================================

void TajweedService::applyTajweedForText(const std::u16string& text,
                                         const SetTajweedFn& setTajweed,
                                         const ResetIndexFn& resetIndex,
                                         bool isIndopak) {
  CompiledTajweedRegex& othersRE = isIndopak ? *othersREIndoPak_ : *othersREMadinah_;

  // --- TafkhimRE pass ----------------------------------------------------
  {
    auto iter = tafkhimRE_->globalIterator(text);
    while (auto matchOpt = iter.next()) {
      const auto& m = *matchOpt;
      std::optional<std::pair<size_t, size_t>> group;

      if ((group = m.group("tafkhim_reh1"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "tafkim");
        setTajweed(I(firstPos + 1), "tafkim");
      } else if ((group = m.firstGroup({"tafkhim1", "tafkhim5", "tafkhim6", "tafkhim6_2"}))) {
        for (size_t p = group->first; p < group->second; p++) {
          setTajweed(I(p), "tafkim");
        }
      } else if ((group = m.group("tafkhim2"))) {
        size_t firstPos = group->first;
        char16_t ch = text[firstPos];
        char16_t endchar = text[m.group("tafkhim2_1").value().first];
        if (endchar != u' ') {
          // not aya mark (always waqf)
          if (ch == fatha[0] || ch == damma[0]) {
            setTajweed(I(firstPos), "tafkim");
          }
        }
      } else if ((group = m.group("tafkhim3"))) {
        setTajweed(I(group->first), "tafkim");
        if ((group = m.group("tafkhim4"))) {
          char16_t ch = text[group->first];
          char16_t endchar = text[m.group("tafkhim4_1").value().first];
          if (endchar != u' ') {
            if (ch == fatha[0] || ch == damma[0] || contains(sukuns, ch)) {
              setTajweed(I(group->first), "tafkim");
            } else if (ch == shadda[0]) {
              setTajweed(I(group->first), "tafkim");
              size_t nextIndex = group->first + 1;
              if (nextIndex < group->second) {
                char16_t nextchar = text[nextIndex];
                if (nextchar == fatha[0] || nextchar == damma[0]) {
                  setTajweed(I(nextIndex), "tafkim");
                }
              }
            }
          } else {
            if (contains(sukuns, ch) || ch == shadda[0]) {
              setTajweed(I(group->first), "tafkim");
            }
          }
        }
      } else if ((group = m.group("kalkala1"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "lkalkala");
        setTajweed(I(firstPos + 1), "lkalkala");
      } else if ((group = m.group("kalkala2"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "lkalkala");
        if (firstPos + 1 < group->second) {
          setTajweed(I(firstPos + 1), "lkalkala");
        }
      } else if ((group = m.group("gray3"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "lgray");
        setTajweed(I(firstPos + 1), "lgray");
      } else if ((group = m.firstGroup({"gray3_indopak_1", "gray3_indopak_2"}))) {
        setTajweed(I(group->first), "lgray");
      }
    }
  }

  resetIndex();

  // --- OthersRE pass -------------------------------------------------
  {
    auto iter = othersRE.globalIterator(text);
    while (auto matchOpt = iter.next()) {
      const auto& m = *matchOpt;
      std::optional<std::pair<size_t, size_t>> group;

      if ((group = m.group("tanween1"))) {
        size_t p = group->first;
        setTajweed(I(p++), "lgray");
        while (p < group->second) {
          setTajweed(I(p++), "green");
        }
      } else if ((group = m.group("tanween2"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "green");
        setTajweed(I(firstPos + 1), "green");
        if (contains(alltanween, m.charInGroup("tanween2", 2))) {
          iter.rewindOne();
        } else {
          setTajweed(I(firstPos + 2), "green");
        }
      } else if ((group = m.group("tanween3"))) {
        if (!m.group("tanween3_a")) {
          setTajweed(I(group->first), "green");
        }
      } else if ((group = m.group("tanween4"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "green");
        if (firstPos + 1 < group->second) {
          setTajweed(I(firstPos + 1), "green");
        }
      } else if ((group = m.group("tanween5"))) {
        setTajweed(I(group->first), "green");
      } else if ((group = m.group("tanween6"))) {
        // dont gray noon
        if (m.charInGroup("tanween6", 0) != char16_t(0x0646) ||
            m.charInGroup("tanween7", 0) != char16_t(0x0646)) {
          size_t firstPos = group->first;
          setTajweed(I(firstPos), "lgray");
          if (firstPos + 1 < group->second) {
            setTajweed(I(firstPos + 1), "lgray");
          }
        }
        auto tanween7Group = m.group("tanween7").value();
        size_t greenPos = tanween7Group.first;
        setTajweed(I(greenPos), "green");
        setTajweed(I(greenPos + 1), "green");
        if (tanween7Group.first + 2 < tanween7Group.second) {
          setTajweed(I(greenPos + 2), "green");
        }
      } else if ((group = m.group("tanween8"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "lgray");
        if (firstPos + 1 < group->second) {
          setTajweed(I(firstPos + 1), "lgray");
        }
      } else if ((group = m.firstGroup({"tanween9", "tanween9_noon"}))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "green");
        if (firstPos + 1 < group->second) {
          setTajweed(I(firstPos + 1), "green");
        }
      } else if ((group = m.group("tanween10"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "green");
        setTajweed(I(firstPos + 1), "green");
        if (firstPos + 2 < group->second) {
          setTajweed(I(firstPos + 2), "green");
        }
      } else if ((group = m.group("gray1"))) {
        setTajweed(I(group->first), "lgray");
      } else if ((group = m.group("gray2"))) {
        setTajweed(I(group->first), "lgray");
      } else if ((group = m.firstGroup({"gray4", "gray4_1", "gray4_2"}))) {
        setTajweed(I(group->first), "lgray");
      } else if ((group = m.group("gray5"))) {
        // NOTE: "gray5" is never produced by any named group in
        // either compiled pattern (same as the original TS source,
        // where `groups.gray5` is always undefined) -- this branch
        // is unreachable dead code, preserved only for fidelity.
        setTajweed(I(group->first), "lgray");
      } else if ((group = m.group("gray6"))) {
        auto sukunsGroup = m.group("gray6_sukuns").value();
        if (sukunsGroup.second != sukunsGroup.first) {
          // Indopak
          // TODO check if we gray same letters different from madinah.
          // With the addition of the sukun, it seems to be less ambiguous
          // to gray out the letter and the sukun. See 9:1.
          size_t firstPos = group->first;
          char16_t firstChar = m.charInGroup("gray6", 0).value();
          char16_t secondChar = m.charInGroup("gray6_1", 0).value();
          if (firstChar != secondChar) {
            if (firstChar != char16_t(0x0637)) {  // ط
              setTajweed(I(firstPos), "lgray");
              setTajweed(I(firstPos + 1), "lgray");
            } else {
              setTajweed(I(firstPos), "tafkim");
              setTajweed(I(firstPos + 1), "tafkim");
            }
          } else {
            // Remove Qalqala
            setTajweed(I(firstPos), std::nullopt);
            setTajweed(I(firstPos + 1), std::nullopt);
          }
        } else {
          // Madinah
          // dont gray same letters unless yeh (only in 68:6)
          char16_t firstChar = m.charInGroup("gray6", 0).value();
          char16_t secondChar = m.charInGroup("gray6_1", 0).value();
          if (firstChar != secondChar ||
              (firstChar == char16_t(0x064A) && secondChar == char16_t(0x064A))) {  // ي
            setTajweed(I(group->first), "lgray");
          }
        }
      } else if ((group = m.firstGroup({"gray7", "gray8"}))) {
        setTajweed(I(group->first), "lgray");
      } else if ((group = m.group("madd1"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "red4");
        setTajweed(I(firstPos + 1), "red4");
        if (firstPos + 2 < group->second) {
          setTajweed(I(firstPos + 2), "red4");
        }
      } else if ((group = m.group("madd2"))) {
        setTajweed(I(group->first), "red1");
      } else if ((group = m.group("madd2_1"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "red1");
        setTajweed(I(firstPos + 1), "red1");
      } else if ((group = m.group("madd5_1"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "red2");
        setTajweed(I(firstPos + 1), "red2");
      } else if ((group = m.group("madd3"))) {
        for (size_t p = group->first; p < group->second; p++) {
          setTajweed(I(p), "red4");
        }
      } else if ((group = m.group("madd4_1"))) {
        if (!m.group("madd4_1_aya")) {
          size_t firstPos = group->first;
          setTajweed(I(firstPos), "red3");
          setTajweed(I(firstPos + 1), "red3");
          if (firstPos + 2 < group->second) {
            setTajweed(I(firstPos + 2), "red3");
          }
        }
      } else if ((group = m.group("madd4_4"))) {
        if (!m.group("madd4_4_aya")) {
          size_t firstPos = group->first;
          setTajweed(I(firstPos), "red3");
          setTajweed(I(firstPos + 1), "red3");
          setTajweed(I(firstPos + 2), "red3");
        }
      } else if ((group = m.group("madd4_2"))) {
        size_t firstPos = group->first;
        auto madd4_2_a = m.group("madd4_2_a");
        if (madd4_2_a && m.lastCharInGroup("madd4_2_a") == char16_t(0x06DD)) continue;  // ۝
        char16_t leadChar = m.charInGroup("madd4_2", 0).value();
        if (!madd4_2_a ||
            leadChar == smallYeh[0] ||
            leadChar == smallWaw[0] ||
            leadChar == invertedDamma[0] ||
            leadChar == subAlef[0]) {
          setTajweed(I(firstPos), "red3");
        }
        setTajweed(I(firstPos + 1), "red3");
        if (firstPos + 2 < group->second) {
          setTajweed(I(firstPos + 2), "red3");
        }
      } else if ((group = m.group("madd5"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "red2");
        if (firstPos + 1 < group->second) {
          setTajweed(I(firstPos + 1), "red2");
        }
      } else if ((group = m.group("madd4_3"))) {
        size_t firstPos = group->first;
        setTajweed(I(firstPos), "red3");
        setTajweed(I(firstPos + 1), "red3");
      }
    }
  }
}

// =========================================================================
// applyTajweedByPage — translated 1:1 from the TS method of the same name.
// =========================================================================

PageTajweedResult TajweedService::applyTajweedByPage(const std::vector<LineToJustify>& lines, bool isIndopak) {
  struct PageLineIndex {
    int lineIndex;
    size_t start;
    size_t end;
  };
  std::vector<PageLineIndex> pageIndexes;

  size_t lastIndex = 0;
  std::u16string text;
  PageTajweedResult result(lines.size());

  for (size_t lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
    const auto& line = lines[lineIndex];
    if (line.lineType == LineType::Sura) continue;
    const std::u16string& lineText = line.text;
    std::u16string addedText = (line.lineType == LineType::Bism) ? u" ۝ " : u" ";
    text += lineText + addedText;
    pageIndexes.push_back(PageLineIndex{static_cast<int>(lineIndex), lastIndex, lastIndex + lineText.size()});
    lastIndex += lineText.size() + addedText.size();
  }

  size_t globalLastIndex = 0;

  SetTajweedFn setTajweed = [&](int pos, std::optional<std::string> tajweed) {
    while (globalLastIndex < pageIndexes.size()) {
      const auto& lineIndexes = pageIndexes[globalLastIndex];
      size_t p = static_cast<size_t>(pos);
      if (p >= lineIndexes.start) {
        if (p < lineIndexes.end) {
          auto& lineMap = result[static_cast<size_t>(lineIndexes.lineIndex)];
          int col = I(p - lineIndexes.start);
          if (tajweed) {
            lineMap[col] = *tajweed;
          } else {
            lineMap.erase(col);
          }
          break;
        } else {
          globalLastIndex++;
        }
      } else {
        break;
      }
    }
  };

  ResetIndexFn resetIndex = [&]() {
    globalLastIndex = 0;
  };

  applyTajweedForText(text, setTajweed, resetIndex, isIndopak);

  return result;
}

}  // namespace digitalkhatt::tajweed
