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

#include "automedina.h"

#include <algorithm>
#include <charconv>

#include "GlyphVis.h"
#include "Lookup.h"
#include "Subtable.h"
#include "digitalkhatt/core/Regex16.h"
#include "metafont.h"

using namespace std;

namespace {

digitalkhatt::TextString toTextString(std::string_view text) {
  return {text.begin(), text.end()};
}

std::optional<std::uint16_t> parseHexUInt16(std::string_view text) {
  std::uint16_t value = 0;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value, 16);
  if (error != std::errc{} || end != text.data() + text.size()) return std::nullopt;
  return value;
}

}  // namespace

Automedina::~Automedina() {}  // not inline
std::unordered_set<std::uint16_t> Automedina::regexptoUnicode(const std::string& regexp) {
  std::unordered_set<std::uint16_t> unicodes;

  const auto re = digitalkhatt::makeRegex16(toTextString(regexp));

  for (const auto& [name, code] : m_layout->glyphCodePerName) {
    if (re.match(toTextString(name)).hasMatch()) {
      unicodes.insert(code);
    }
  }

  return unicodes;
}
std::unordered_set<std::uint16_t> Automedina::classtoUnicode(const std::string& exprName, bool includeExpandables) {
  if (auto cached = cachedClasstoUnicode.find(exprName); cached != cachedClasstoUnicode.end()) {
    return cached->second;
  }

  std::unordered_set<std::uint16_t> unicodes;

  if (!classes.contains(exprName)) {
    if (m_layout->glyphCodePerName.contains(exprName)) {
      auto charcode = m_layout->glyphCodePerName[exprName];
      unicodes.insert(charcode);

      if (includeExpandables) {
        auto set = m_layout->getSubsts(charcode);
        unicodes.insert(set.cbegin(), set.cend());
      }
    } else {
      if (const auto unicode = parseHexUInt16(exprName)) {
        unicodes.insert(*unicode);
      } else {
        const auto matches = regexptoUnicode(exprName);
        unicodes.insert(matches.cbegin(), matches.cend());
      }
    }
  } else {
    for (const auto& name : classes[exprName]) {
      const auto classUnicodes = classtoUnicode(name);
      unicodes.insert(classUnicodes.cbegin(), classUnicodes.cend());
    }
  }

  cachedClasstoUnicode[exprName] = unicodes;
  return unicodes;
}

void Automedina::generateAyas(std::string_view ayaName, bool colored) {
  const std::string ayaNameString(ayaName);
  for (int ayaNumber = 1; ayaNumber <= 286; ++ayaNumber) {
    const std::string number = std::to_string(ayaNumber);
    const std::string glyphName = ayaNameString + number;

    std::string setColored;
    if (colored) {
      setColored = "coloredglyph:=\"" + ayaNameString + ".colored" + number + '"';
    }

    std::string data = "beginchar(" + glyphName + ",-1,-1,2,-1);\n"
                       "%%beginbody\n"
                       "genAyaNumber(" + ayaNameString + ", " + number + ",3000);" + setColored + ";endchar;";
    m_layout->font->execute(data);
    addedGlyphs[glyphName] = data;

    if (colored) {
      const std::string coloredGlyphName = ayaNameString + ".colored" + number;
      data = "beginchar(" + coloredGlyphName + ",-1,-1,5,-1);\n"
             "%%beginbody\n"
             "genAyaNumber(" + ayaNameString + ".colored, " + number + ",3000);endchar;";
      m_layout->font->execute(data);
      addedGlyphs[coloredGlyphName] = data;
    }
  }
}
