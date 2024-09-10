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
#include "qstring.h"
#include "Subtable.h"
#include "Lookup.h"
#include "font.hpp"
#include "GlyphVis.h"
#include <algorithm>
#include "qregularexpression.h"
#include "metafont.h"
#include "qdebug.h"


using namespace std;


QSet<quint16> Automedina::regexptoUnicode(QString regexp) {

  QSet<quint16> unicodes;

  QRegularExpression re(regexp);
  for (auto& glyph : m_layout->glyphs) {
    if (re.match(glyph.name).hasMatch()) {
      unicodes.insert(glyph.charcode);
      /*
      auto t = m_layout->nojustalternatePaths.find(glyph.charcode);
      if (t != m_layout->nojustalternatePaths.end()) {
        auto& second = t->second;
        for (auto& addedGlyph : second) {
          unicodes.insert(addedGlyph.second->charcode);
        }
      }*/
    }
  }

  return unicodes;
}
QSet<quint16> Automedina::classtoUnicode(QString className, bool includeExpandables) {

  if (cachedClasstoUnicode.contains(className)) {
    return cachedClasstoUnicode[className];
  }

  QSet<quint16> unicodes;

  if (!classes.contains(className)) {
    if (m_layout->glyphs.contains(className)) {
      auto& glyph = m_layout->glyphs[className];
      unicodes.insert(glyph.charcode);

      if (includeExpandables) {
        auto set = m_layout->getSubsts(glyph.charcode);
        unicodes.unite(set);      
      }
    }
    else {
      bool ok;
      quint16 uniode = className.toUInt(&ok, 16);
      if (!ok) {
        QRegularExpression re(className);
        for (auto& glyph : m_layout->glyphs) {
          if (re.match(glyph.name).hasMatch()) {
            unicodes.insert(glyph.charcode);
          }
        }
      }
      else {
        unicodes.insert(uniode);
      }

    }
  }
  else {
    for (auto name : classes[className]) {
      unicodes.unite(classtoUnicode(name, true));
    }
  }

  cachedClasstoUnicode[className] = unicodes;
  return unicodes;
}



QSet<QString> Automedina::classtoGlyphName(QString className) {

  QSet<QString> names;
  //TODO use classtoUnicode
  if (!classes.contains(className)) {
    if (m_layout->glyphs.contains(className)) {
      auto& glyph = m_layout->glyphs[className];
      names.insert(glyph.name);
    }
    else {
      QRegularExpression re(className);
      for (auto& glyph : m_layout->glyphs) {
        if (re.match(glyph.name).hasMatch()) {
          names.insert(glyph.name);
        }
      }
    }
  }
  else {

    for (auto name : classes[className]) {
      names.unite(classtoGlyphName(name));
    }
  }

  return names;

}
