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

void Automedina::prepareAnchorResolution() {
  anchorClassCache_.clear();
}

std::optional<std::size_t> Automedina::matchingAnchorClassSize(
    const MarkBaseSubtable& subtable, std::string_view className,
    const std::string& groupName) const {
  const AnchorClassKey key{&subtable, std::string(className), groupName};
  if (const auto cached = anchorClassCache_.find(key); cached != anchorClassCache_.end())
    return cached->second;
  std::optional<std::size_t> size;
  // Group names may be lookup-qualified without changing a feature's classes.
  // Cache the namespace check with membership, not on every glyph access.
  for (auto split = groupName.rfind('_'); split != std::string::npos;) {
    const auto scope = groupName.substr(0, split);
    if (m_layout->lookupsIndexByName.contains(scope)) {
      if (scope != subtable.getLookup()->name)
        return anchorClassCache_.emplace(key, std::nullopt).first->second;
      break;
    }
    if (split == 0) break;
    split = groupName.rfind('_', split - 1);
  }
  if (const auto found = subtable.classes.find(std::get<1>(key)); found != subtable.classes.end()) {
    auto coverage = found->second.markCodes;
    bool unresolved = false;
    if (coverage.empty()) {
      for (const auto& expression : found->second.mark) {
        const auto codes = m_layout->classtoUnicode(expression);
        unresolved |= codes.empty();
        coverage.insert(codes.begin(), codes.end());
      }
    }
    const auto members = m_layout->classtoUnicode(groupName);
    const auto belongs = [&](auto code) {
      if (members.contains(code)) return true;
      const auto* mark = m_layout->getGlyph(code);
      return mark && mark->sourceGlyphCode && members.contains(*mark->sourceGlyphCode);
    };
    if (!unresolved && !coverage.empty() &&
        std::all_of(coverage.begin(), coverage.end(), belongs)) size = members.size();
  }
  return anchorClassCache_.emplace(key, size).first->second;
}

std::optional<Point> Automedina::resolveMarkClassAnchor(const GlyphVis& glyph,
    const MarkBaseSubtable& subtable, std::string_view className, bool lookupScopedOnly,
    std::string_view role) const {
  std::optional<Point> result;
  std::optional<std::pair<std::size_t, bool>> best;
  const auto prefix = subtable.getLookup()->name + "_";
  const std::string suffix = role.empty() ? std::string{} : "_" + std::string(role);
  for (const auto& [key, anchor] : glyph.anchors) {
    if (key.type != GlyphVis::AnchorType::MarkAnchor) continue;
    const bool scoped = key.name.starts_with(prefix);
    if (lookupScopedOnly && !scoped) continue;
    auto groupName = key.name;
    // Incoming groups are explicitly authored with a role suffix. Ordinary
    // recipient group anchors must never become attached-mark origins.
    if (!suffix.empty()) {
      if (!groupName.ends_with(suffix)) continue;
      groupName.resize(groupName.size() - suffix.size());
    }
    if (!classes.contains(groupName) && !m_layout->glyphCodePerName.contains(groupName)) continue;
    const auto size = matchingAnchorClassSize(subtable, className, groupName);
    // Narrowest group wins, then lookup scope, then lexical anchor order.
    if (size) {
      const std::pair rank{*size, !scoped};
      if (!best || rank < *best) {
        result = anchor.anchor;
        best = rank;
      }
    }
  }
  return result;
}

bool Automedina::isLookupDisabled(const std::string& lookupName) const {
  return m_layout->disabledLookups.contains(lookupName);
}

Automedina::SubstEquivGlyphMap& Automedina::substEquivGlyphMap() {
  return m_layout->substEquivGlyphs;
}

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
