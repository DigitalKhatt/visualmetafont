#include "../../src/otlayout/qurantext/quran.h"

#include <iostream>

int main() {
  const std::u16string ordinary = u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
  for (int surah = 1; surah <= 114; ++surah) {
    auto expected = ordinary;
    const bool shadda = surah == 95 || surah == 97;
    if (shadda) expected.insert(1, 1, u'\u0651');
    const auto actual = madinaBasmalaText(surah);
    if (actual != expected || actual.size() != (shadda ? 39u : 38u)) {
      std::cerr << "Incorrect source basmala for surah " << surah << '\n';
      return 1;
    }
  }
  std::cout << "Madina source basmala text and UTF-16 lengths passed for all 114 surahs\n";
}
