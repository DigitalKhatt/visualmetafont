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

#include "GlyphVis.h"
#include "Lookup.h"
#include "Subtable.h"
#include "font.hpp"
#include "metafont.h"
#include "qdebug.h"
#include "qregularexpression.h"
#include "qstring.h"

using namespace std;

Automedina::~Automedina() {}  // not inline
std::unordered_set<std::uint16_t> Automedina::regexptoUnicode(const std::string& regexp) {
  std::unordered_set<std::uint16_t> unicodes;

  QRegularExpression re(QString::fromStdString(regexp));

  for (auto it = m_layout->glyphCodePerName.keyValueBegin(); it != m_layout->glyphCodePerName.keyValueEnd(); ++it) {
    if (re.match(it->first).hasMatch()) {
      unicodes.insert(it->second);
    }
  }

  return unicodes;
}
std::unordered_set<std::uint16_t> Automedina::classtoUnicode(const std::string& exprName, bool includeExpandables) {
  if (auto cached = cachedClasstoUnicode.find(exprName); cached != cachedClasstoUnicode.end()) {
    return cached->second;
  }

  std::unordered_set<std::uint16_t> unicodes;
  const auto qexprName = QString::fromStdString(exprName);

  if (!classes.contains(exprName)) {
    if (m_layout->glyphCodePerName.contains(qexprName)) {
      auto charcode = m_layout->glyphCodePerName[qexprName];
      unicodes.insert(charcode);

      if (includeExpandables) {
        auto set = m_layout->getSubsts(charcode);
        unicodes.insert(set.cbegin(), set.cend());
      }
    } else {
      bool ok;
      std::uint16_t unicode = qexprName.toUShort(&ok, 16);
      if (!ok) {
        const auto matches = regexptoUnicode(exprName);
        unicodes.insert(matches.cbegin(), matches.cend());
      } else {
        unicodes.insert(unicode);
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

QSet<QString> Automedina::classtoGlyphName(QString className) {
  QSet<QString> names;
  // TODO use classtoUnicode
  auto classNameStd = className.toStdString();
  if (!classes.contains(classNameStd)) {
    if (m_layout->glyphCodePerName.contains(className)) {
      names.insert(className);
    } else {
      QRegularExpression re(className);
      for (auto it = m_layout->glyphCodePerName.keyValueBegin(); it != m_layout->glyphCodePerName.keyValueEnd(); ++it) {
        if (re.match(it->first).hasMatch()) {
          names.insert(it->first);
        }
      }
    }
  } else {
    for (auto& name : classes[classNameStd]) {
      names.unite(classtoGlyphName(QString::fromStdString(name)));
    }
  }

  return names;
}

void Automedina::generateAyas(QString ayaName, bool colored) {
  for (int ayaNumber = 1; ayaNumber <= 286; ayaNumber++) {
    QString setcolored;
    if (colored) {
      setcolored = QString("coloredglyph:=\"%1.colored%2\"").arg(ayaName).arg(ayaNumber);
    }
    QString data = QString("beginchar(%1%2,-1,-1,2,-1);\n%%beginbody\ngenAyaNumber(%1, %2,3000);%3;endchar;").arg(ayaName).arg(ayaNumber).arg(setcolored);
    m_layout->font->executeMetaPost(data);
    addedGlyphs[QString("%1%2").arg(ayaName).arg(ayaNumber).toStdString()] = data.toStdString();
    if (colored) {
      data = QString("beginchar(%1.colored%2,-1,-1,5,-1);\n%%beginbody\ngenAyaNumber(%1.colored, %2,3000);endchar;").arg(ayaName).arg(ayaNumber);
      m_layout->font->executeMetaPost(data);
      addedGlyphs[QString("%1.colored%2").arg(ayaName).arg(ayaNumber).toStdString()] = data.toStdString();
    }
  }
}
