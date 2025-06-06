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

#include "Subtable.h"
#include "font.hpp"
#include "Lookup.h"
#include "OtLayout.h"
#include <QJsonArray>
#include <QJsonObject>
#include "QByteArrayOperator.h"
#include <QDebug>
#include <algorithm>
#include "GlyphVis.h"
#include <hb-ot-layout-common.hh>
#include <iostream>
#include <unordered_map>

using namespace std;

QDataStream& operator<<(QDataStream& s, const QVector<quint16>& v)
{
  for (QVector<quint16>::const_iterator it = v.begin(); it != v.end(); ++it)
    s << *it;
  return s;
}

QDataStream& operator<<(QDataStream& s, const QVector<quint32>& v)
{
  for (QVector<quint32>::const_iterator it = v.begin(); it != v.end(); ++it)
    s << *it;
  return s;
}

Subtable::Subtable(Lookup* lookup)
{
  m_lookup = lookup;
  m_layout = lookup->layout;
  metafont = m_layout->font;

}

quint16 Subtable::getCodeFromName(QString name) {
  //return m_layout->glyphCodePerName[name];
  quint16 uniode;
  if (m_layout->glyphCodePerName.contains(name)) {
    uniode = m_layout->glyphCodePerName[name];
  }
  else {
    bool ok;
    uniode = name.toUInt(&ok, 16);
    if (!ok) {
      printf(QString("glyph name <%1> not found\n").arg(name).toLatin1());
      uniode = 0;
    }
  }

  return uniode;
}
QString Subtable::getNameFromCode(quint16 code) {
  return m_layout->glyphNamePerCode[code];
}

SingleSubtable::SingleSubtable(Lookup* lookup, quint16 format) : Subtable(lookup), format{ format } {
}

bool SingleSubtable::isExtended() {
  if (format == 10 || format == 11) {
    return true;
  }
  return false;
}

void SingleSubtable::readJson(const QJsonObject& json)
{
  subst.clear();
  for (int index = 0; index < json.size(); ++index) {
    QString glyphname = json.keys()[index];
    quint16 unicode = getCodeFromName(glyphname);
    if (!unicode) continue;


    QString name = json[glyphname].toString();

    quint16 value = getCodeFromName(name);
    if (!value) continue;
    subst[unicode] = value;


  }
}
SingleSubtableWithTatweel::SingleSubtableWithTatweel(Lookup* lookup) : SingleSubtable(lookup, 11) {};

void SingleSubtableWithTatweel::generateSubstEquivGlyphs() {

  QMapIterator<quint16, GlyphExpansion>i(expansion);


  while (i.hasNext()) {
    i.next();

    GlyphExpansion expan = i.value();

    if (expan.MinLeftTatweel != 0 || expan.MinRightTatweel != 0) {

      if (expan.MinLeftTatweel > 0 && expan.MinRightTatweel > 0) {
        throw new std::runtime_error("SHOULD NOT");
      }

      GlyphParameters parameters;

      parameters.lefttatweel = (double)expan.MinLeftTatweel;
      parameters.righttatweel = (double)expan.MinRightTatweel;

      auto substGlyph = (uint16_t)subst[i.key()];
      auto substEquivGlyphs = m_layout->getSubstEquivGlyphs(substGlyph);
      while (true) {

        auto oldSize = substEquivGlyphs.size();

        for (auto& glyph : substEquivGlyphs) {
          if ((expan.MinLeftTatweel > 0 && glyph.second->charrt > 0) || (expan.MinRightTatweel > 0 && glyph.second->charlt > 3)) continue;
          if ((glyph.second->charlt > 5 || glyph.second->charrt > 5)) continue;
          //if ((glyph.second->charlt < -0.1 || glyph.second->charrt < -0.1)) continue;

          //if ((expan.MinLeftTatweel > 0 && glyph.second->charlt > 5) || (expan.MinRightTatweel > 0 && glyph.second->charrt > 5)) continue;

          GlyphVis* newglyph = m_layout->getAlternate(glyph.second->charcode, parameters, true, true);
        }
        GlyphVis* glyph = m_layout->getAlternate(substGlyph, parameters, true, true);

        if (!m_lookup->name.contains("jt02")) break;

        substEquivGlyphs = m_layout->getSubstEquivGlyphs(substGlyph);

        auto newSize = substEquivGlyphs.size();
        if (newSize == oldSize) break;
      }
    }


  }

}
QByteArray SingleSubtableWithTatweel::getConvertedOpenTypeTable() {

  QMapIterator<quint16, GlyphExpansion>oldExpaIter(expansion);
  QMapIterator<quint16, quint16>oldSubstIter(subst);

  QMap<quint16, GlyphExpansion > newexpansion;
  QMap<quint16, quint16 > newsubst;

  while (oldSubstIter.hasNext()) {
    oldExpaIter.next();
    oldSubstIter.next();

    GlyphExpansion expan = oldExpaIter.value();

    quint16 glyphCode = oldSubstIter.key();

    quint16 substGlyph = oldSubstIter.value();

    if (expan.MinLeftTatweel != 0 || expan.MinRightTatweel != 0) {
      GlyphParameters parameters;

      parameters.lefttatweel = (double)expan.MinLeftTatweel;
      parameters.righttatweel = (double)expan.MinRightTatweel;

      auto& addedSubstGlyphs = m_layout->getSubstEquivGlyphs(substGlyph);

      auto found = addedSubstGlyphs.find(parameters);

      if (found != addedSubstGlyphs.end()) {

        /*
        auto name = m_layout->glyphNamePerCode[found->second->charcode];

        if (name.contains("meem.fina.basmala")) {
          auto& tt = m_layout->glyphs["meem.fina.basmala"];
          auto& tt2 = m_layout->glyphs[name];
          cout << "meem.fina.basmala=" << tt.width << ";" << name.toStdString() << "=" << tt2.width << std::endl;
        }*/

        newexpansion.insert(glyphCode, oldExpaIter.value());
        newsubst.insert(glyphCode, found->second->charcode);
      }

      auto& addedGlyphs = m_layout->getSubstEquivGlyphs(glyphCode);

      for (auto& addedGlyph : addedGlyphs) {

        GlyphParameters parameters;

        parameters.lefttatweel = addedGlyph.second->charlt + (double)expan.MinLeftTatweel;
        parameters.righttatweel = addedGlyph.second->charrt + (double)expan.MinRightTatweel;

        auto found = addedSubstGlyphs.find(parameters);

        if (found != addedSubstGlyphs.end()) {

          newexpansion.insert(addedGlyph.second->charcode, oldExpaIter.value());
          newsubst.insert(addedGlyph.second->charcode, found->second->charcode);
        }
      }
    }
    else {
      newexpansion.insert(glyphCode, oldExpaIter.value());
      newsubst.insert(glyphCode, substGlyph);
    }
  }


  QByteArray root;
  QByteArray coverage;


  QMapIterator<quint16, GlyphExpansion>i(newexpansion);
  QMapIterator<quint16, quint16>substIter(newsubst);

  quint16 glyphCount = newexpansion.size();
  quint16 coverage_offset = 2 + 2 + 2 + 2 * glyphCount;


  root << (quint16)2;
  root << coverage_offset;
  root << glyphCount;


  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  while (substIter.hasNext()) {
    i.next();
    substIter.next();

    GlyphExpansion expan = i.value();

    quint16 substGlyph = substIter.value();

    root << substGlyph;
    coverage << (quint16)substIter.key();

  }

  root.append(coverage);

  return root;
}
QByteArray SingleSubtableWithTatweel::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;
  QByteArray substituteGlyphIDs;



  quint16 glyphCount = expansion.size();
  quint16 coverage_offset = 2 + 2 + 2 + 10 * glyphCount;


  root << (quint16)format;
  root << coverage_offset;
  root << glyphCount;


  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  QMapIterator<quint16, GlyphExpansion>i(expansion);

  while (i.hasNext()) {
    i.next();

    root << (uint16_t)subst[i.key()];

    GlyphExpansion expan = i.value();

    if (!m_layout->useNormAxisValues) {
      OT::F16DOT16 lefttatweel;
      lefttatweel.set_float(expan.MinLeftTatweel);

      OT::F16DOT16 righttatweel;
      righttatweel.set_float(expan.MinRightTatweel);

      root << (int32_t)lefttatweel.to_int() << (int32_t)righttatweel.to_int();
    }
    else {
      OT::F16DOT16 value;

      ValueLimits limits;

      auto& name = m_layout->glyphNamePerCode.value(subst[i.key()]);

      const auto& find = m_layout->expandableGlyphs.find(name);

      if (find != m_layout->expandableGlyphs.end()) {
        limits = find->second;
      }

      if ((expan.MinLeftTatweel < 0 && expan.MinLeftTatweel < limits.minLeft) || (expan.MinLeftTatweel > 0 && expan.MinLeftTatweel > limits.maxLeft)) {
        std::cout << "MinLeftTatweel error for glyph " + name.toStdString() << std::endl;
        //throw new runtime_error("MinLeftTatweel error for glyph " + name.toStdString());
      }
      else if (expan.MinLeftTatweel < 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(-expan.MinLeftTatweel / m_layout->toOpenType->axisLimits.minLeft);
        }
        else {
          value.set_float(-expan.MinLeftTatweel / limits.minLeft);
        }

        root << (int32_t)value.to_int();
      }
      else if (expan.MinLeftTatweel > 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinLeftTatweel / m_layout->toOpenType->axisLimits.maxLeft);
        }
        else {
          value.set_float(expan.MinLeftTatweel / limits.maxLeft);
        }

        root << (int32_t)value.to_int();
      }
      else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
      }

      if ((expan.MinRightTatweel < 0 && expan.MinRightTatweel < limits.minRight) || (expan.MinRightTatweel > 0 && expan.MinRightTatweel > limits.maxRight)) {
        std::cout << "MinRightTatweel error for glyph " + name.toStdString() << std::endl;
        //throw new runtime_error("MinRightTatweel error for glyph " + name.toStdString());
      }
      else if (expan.MinRightTatweel < 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(-expan.MinRightTatweel / m_layout->toOpenType->axisLimits.minRight);
        }
        else {
          value.set_float(-expan.MinRightTatweel / limits.minRight);
        }
        root << (int32_t)value.to_int();;
      }
      else if (expan.MinRightTatweel > 0.0) {

        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinRightTatweel / m_layout->toOpenType->axisLimits.maxRight);
        }
        else {
          value.set_float(expan.MinRightTatweel / limits.maxRight);
        }
        root << (int32_t)value.to_int();;
      }
      else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }
    }


    coverage << (quint16)i.key();

  }

  root.append(coverage);

  return root;
};


QByteArray FSMSubtable::getOpenTypeTable(bool extended) {

  QByteArray coverage;

  QByteArray classDef;

  QByteArray root;

  QByteArray header;

  QByteArray chainNodes;
  std::vector<uint32_t> chainNodesSizes;

  int numNodes = dfa.states.size();

  if (numNodes == 0) {
    root << (quint16)0;
    return root;
  }

  quint16 glyphCount = dfa.glyphToClass.size();

  classDef << (quint16)2; //Format identifier — format = 2
  classDef << glyphCount; // Number of ClassRangeRecords

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (auto it = dfa.glyphToClass.cbegin(); it != dfa.glyphToClass.cend(); it++) {

    classDef << (quint16)it.key();
    classDef << (quint16)it.key();
    classDef << (quint16)(it.value() + 1); // Class 0 for not used glyphs

    coverage << (quint16)it.key();
  }

  if (dfa.backupStates.size() != (dfa.maxBackup - dfa.minBackup + 1)) {
    throw new std::runtime_error("invalid backupstates number");
  }

  quint16 startOffsets = 2 + 2 + 2 + 1 + 1 + 1 + 1 + dfa.backupStates.size() * 2 + 2 + numNodes * 4;


  quint16 coverageOffset = startOffsets;
  quint16 classDefOffset = coverageOffset + coverage.size();
  quint32 nextOffset = classDefOffset + classDef.size();

  header << (quint16)1;
  header << (quint16)coverageOffset; //  Offset to Coverage;
  header << (quint16)classDefOffset; //  Offset to ClassDef;
  header << (uint8_t)dfa.maxBackup;
  header << (uint8_t)dfa.minBackup;
  header << (uint8_t)dfa.maxLoop;
  header << (uint8_t)0; // reserved
  for (auto it = dfa.backupStates.cbegin(); it != dfa.backupStates.cend(); it++) {
    header << (quint16)*it;
  }
  header << (quint16)numNodes; // Number of ChainNodes

  auto addBackLink = [&m_layout = m_layout, &m_lookup = m_lookup](QByteArray& array, const DFABackTrackInfo& backTrackInfo) {
    array << (uint16_t)backTrackInfo.prevTransIndex;
    array << (uint16_t)backTrackInfo.actions.size();

    uint16_t lookupListIndex = 0;

    for (auto action : backTrackInfo.actions) {
      if (action.type == DFAActionType::LOOKUP) {
        QString fullname = QString::fromStdString(action.name);
        if (m_lookup->isGsubLookup()) {
          if (m_layout->gsublookupsIndexByName.contains(fullname)) {
            lookupListIndex = m_layout->gsublookupsIndexByName[fullname];
          }
          else {
            throw new std::runtime_error("Invalid lookup");
          }
        }
        else {
          if (m_layout->gposlookupsIndexByName.contains(fullname)) {
            lookupListIndex = m_layout->gposlookupsIndexByName[fullname];
          }
          else {
            throw new std::runtime_error("Invalid lookup");
          }
        }
      }
      else if (action.type == DFAActionType::STARTNEWMATCH) {
        lookupListIndex = 0xFFFF;
      }
      else {
        throw new std::runtime_error("Not yet implemented");
      }


      array << (uint8_t)action.idRule; // chainid : Action chain number of this action
      array << (uint8_t)0; // distance : How far back to process
      array << lookupListIndex; // lookup :	Lookup id to execute
    }
    };

  for (auto it = dfa.states.cbegin(); it != dfa.states.cend(); it++) {
    auto& state = *it;

    int numTransitions = it->transtitions.size();

    uint16_t nextChainNodeOffset = 1 /*actionid*/
      + 2 /* OffsetTo<BackLink> */
      + 2 /* numTransitions */
      + numTransitions * 6 /* ClassNode size*/;

    QByteArray chainNode;

    QByteArray offsets;

    chainNode << (uint8_t)state.final;  // actionid : Action identifier for a final node
    if (state.final != 0) {
      QByteArray backlinkArray;
      addBackLink(backlinkArray, state.backtrackfinal);
      chainNode << (uint16_t)nextChainNodeOffset; // OffsetTo<BackLink> backLink
      nextChainNodeOffset += backlinkArray.size();
      offsets.append(backlinkArray);
    }
    else {
      chainNode << (uint16_t)0; // OffsetTo<BackLink> backLink
    }
    chainNode << (uint16_t)numTransitions;  // numTransitions	: Number of transitions


    QByteArray classNodes;


    for (auto itTransi = it->transtitions.cbegin(); itTransi != it->transtitions.cend(); itTransi++) {
      numTransitions++;

      classNodes << (uint16_t)(itTransi->first + 1); // classIndex	: class index value to match
      classNodes << (quint16)(itTransi->second.state); // chainNode	: chainNode index to use on match
      //OffsetTo<BackLinkArray> backLinks
      auto& backtracks = itTransi->second.backtracks;
      if (backtracks.size() > 0) {
        uint16_t numbacklinks = backtracks.size();
        QByteArray backLinksArray;
        QByteArray offsetbackLinksArray;
        backLinksArray << numbacklinks;

        uint16_t nextbackLinksOffset = 2 + 2 * numbacklinks;

        for (auto& backtrack : itTransi->second.backtracks) {
          QByteArray backlinkArray;
          addBackLink(backlinkArray, backtrack);
          backLinksArray << nextbackLinksOffset;
          nextbackLinksOffset += backlinkArray.size();
          offsetbackLinksArray.append(backlinkArray);
        }

        backLinksArray.append(offsetbackLinksArray);

        classNodes << (uint16_t)nextChainNodeOffset; // OffsetTo<BackLink> backLink
        nextChainNodeOffset += backLinksArray.size();
        offsets.append(backLinksArray);

      }
      else {
        classNodes << (uint16_t)0;
      }
    }

    chainNode.append(classNodes);
    chainNode.append(offsets);


    chainNodes.append(chainNode);

    header << nextOffset;
    nextOffset += chainNode.size();

  }

  root.append(header);
  root.append(coverage);
  root.append(classDef);
  root.append(chainNodes);

  return root;
}



QByteArray SingleSubtable::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;

  QMap<quint16, quint16 > newSubst;

  if (!extended) {
    for (auto i = subst.cbegin(), end = subst.cend(); i != end; ++i) {
      auto before = i.key();
      auto after = i.value();
      newSubst.insert(before, after);
      auto& beforeGlyphs = m_layout->getSubstEquivGlyphs(before);
      auto& afterGlyphs = m_layout->getSubstEquivGlyphs(after);

      for (auto& addedGlyph : beforeGlyphs) {
        auto ret = afterGlyphs.find(addedGlyph.first);
        if (ret != afterGlyphs.end()) {
          newSubst.insert(addedGlyph.second->charcode, ret->second->charcode);
        }
      }

    }
  }
  else {
    newSubst = subst;
  }


  quint16 glyphCount = newSubst.size();
  quint16 coverage_offset = 2 + 2 + 2 + 2 * glyphCount;


  root << (quint16)format;
  root << coverage_offset;
  root << glyphCount;


  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  QMapIterator<quint16, quint16>i(newSubst);


  while (i.hasNext()) {
    i.next();

    quint16 substGlyph = i.value();

    root << substGlyph;
    coverage << (quint16)i.key();

  }

  root.append(coverage);

  return root;
};

SingleSubtableWithExpansion::SingleSubtableWithExpansion(Lookup* lookup) : SingleSubtable(lookup, 10) {};
QByteArray SingleSubtableWithExpansion::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;
  QByteArray substituteGlyphIDs;



  quint16 glyphCount = expansion.size();
  quint16 coverage_offset = 2 + 2 + 2 + 24 * glyphCount;


  root << (quint16)format;
  root << coverage_offset;
  root << glyphCount;


  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  QMapIterator<quint16, GlyphExpansion>i(expansion);

  while (i.hasNext()) {
    i.next();

    root << (uint16_t)subst[i.key()];

    GlyphExpansion expan = i.value();

    if (!m_layout->useNormAxisValues) {
      OT::F16DOT16 minLeftTatweel;
      minLeftTatweel.set_float(expan.MinLeftTatweel);
      root << (int32_t)minLeftTatweel.to_int();

      OT::F16DOT16 maxLeftTatweel;
      maxLeftTatweel.set_float(expan.MaxLeftTatweel);
      root << (int32_t)maxLeftTatweel.to_int();

      OT::F16DOT16 minRightTatweel;
      minRightTatweel.set_float(expan.MinRightTatweel);
      root << (int32_t)minRightTatweel.to_int();

      OT::F16DOT16 maxRightTatweel;
      maxRightTatweel.set_float(expan.MaxRightTatweel);
      root << (int32_t)maxRightTatweel.to_int();
    }
    else {
      OT::F16DOT16 value;
      ValueLimits limits;

      auto& name = m_layout->glyphNamePerCode.value(subst[i.key()]);

      const auto& find = m_layout->expandableGlyphs.find(name);

      if (find != m_layout->expandableGlyphs.end()) {
        limits = find->second;
      }


      if ((expan.MinLeftTatweel < 0 && expan.MinLeftTatweel < limits.minLeft) || (expan.MinLeftTatweel > 0 && expan.MinLeftTatweel > limits.maxLeft)) {
        //throw new runtime_error("MinLeftTatweel error for glyph " + name.toStdString());
        value.set_float(0.0);
        root << (int32_t)value.to_int();
      }
      else if (expan.MinLeftTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinLeftTatweel / m_layout->toOpenType->axisLimits.maxLeft);
        }
        else {
          value.set_float(expan.MinLeftTatweel / limits.maxLeft);
        }
        root << (int32_t)value.to_int();;
      }
      else {

        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }

      if ((expan.MaxLeftTatweel < 0 && expan.MaxLeftTatweel < limits.minLeft) || (expan.MaxLeftTatweel > 0 && expan.MaxLeftTatweel > limits.maxLeft)) {
        //throw new runtime_error("MinLeftTatweel error for glyph " + name.toStdString());
        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }
      else if (expan.MaxLeftTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MaxLeftTatweel / m_layout->toOpenType->axisLimits.maxLeft);
        }
        else {
          value.set_float(expan.MaxLeftTatweel / limits.maxLeft);
        }
        root << (int32_t)value.to_int();;
      }
      else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }

      if ((expan.MinRightTatweel < 0 && expan.MinRightTatweel < limits.minRight) || (expan.MinRightTatweel > 0 && expan.MinRightTatweel > limits.maxRight)) {
        throw new runtime_error("MinLeftTatweel error for glyph " + name.toStdString());
      }
      else if (expan.MinRightTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinRightTatweel / m_layout->toOpenType->axisLimits.maxRight);
        }
        else {
          value.set_float(expan.MinRightTatweel / limits.maxRight);
        }
        root << (int32_t)value.to_int();;
      }
      else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }

      if ((expan.MaxRightTatweel < 0 && expan.MaxRightTatweel < limits.minRight) || (expan.MaxRightTatweel > 0 && expan.MaxRightTatweel > limits.maxRight)) {
        //throw new runtime_error("MinLeftTatweel error for glyph " + name.toStdString());
        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }
      else if (expan.MaxRightTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MaxRightTatweel / m_layout->toOpenType->axisLimits.maxRight);
        }
        else {
          value.set_float(expan.MaxRightTatweel / limits.maxRight);
        }
        root << (int32_t)value.to_int();;
      }
      else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();;
      }
    }



    root << (uint16_t)expan.weight;
    uint32_t flags = 0;

    flags = flags | (uint32_t)(expan.startEndLig);
    flags = flags | (expan.stretchIsAbsolute << 3);
    flags = flags | (expan.shrinkIsAbsolute << 4);

    root << flags;

    coverage << (quint16)i.key();

  }

  root.append(coverage);

  return root;
};



SingleAdjustmentSubtable::SingleAdjustmentSubtable(Lookup* lookup, quint16 pformat) : Subtable(lookup), format{ pformat } {}

bool SingleAdjustmentSubtable::isExtended() {
  if (format == 3) {
    return true;
  }
  else {
    return false;
  }
}

void SingleAdjustmentSubtable::readJson(const QJsonObject& json) {
  singlePos.clear();
  for (int index = 0; index < json.size(); ++index) {
    QString className = json.keys()[index];
    auto  unicodes = m_layout->classtoUnicode(className);
    QJsonArray record = json[className].toArray();
    for (auto unicode : unicodes) {
      ValueRecord valueRecord{ (qint16)record[0].toInt(),(qint16)record[1].toInt(),(qint16)record[2].toInt(),(qint16)record[3].toInt() };

      singlePos[unicode] = valueRecord;
    }


  }
}
QByteArray SingleAdjustmentSubtable::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;
  QByteArray valueRecords;
  QByteArray VariationIndexes;

  quint16 glyphCount = singlePos.size();
  quint16 valueFormat;
  uint valueRecordsSize;

  auto isOTVar = m_layout->isOTVar;

  if (isOTVar) {
    valueFormat = 0xFF;
    valueRecordsSize = 16 * glyphCount;
  }
  else {
    valueFormat = 0xF;
    valueRecordsSize = 8 * glyphCount;
  }

  uint currentVariationIndexesOffset = 8 + valueRecordsSize;

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  QMapIterator<quint16, ValueRecord>i(singlePos);

  while (i.hasNext()) {
    i.next();

    ValueRecord record = i.value();

    auto originalCode = i.key();

    if (!extended) {
      auto glyph = m_layout->getGlyph(originalCode);
      if (glyph->name.contains(".added_")) {
        originalCode = m_layout->glyphCodePerName[glyph->originalglyph];
      }
    }

    if (parameters.contains(originalCode)) {
      ValueRecord par = parameters.value(originalCode);
      record = { (qint16)(record.xPlacement + par.xPlacement),(qint16)(record.yPlacement + par.yPlacement) ,(qint16)(record.xAdvance + par.xAdvance) ,record.yAdvance };
    }

    valueRecords << record.xPlacement << record.yPlacement << record.xAdvance << record.yAdvance;

    if (isOTVar) {

      auto glyphName = m_layout->glyphNamePerCode[originalCode];

      auto regionIndexes = m_layout->toOpenType->getGlyphParameters(glyphName);
      int regionIndexesArrayIndex = regionIndexes.second;
      auto glyphParamertersArray = regionIndexes.first;

      if (glyphParamertersArray.size() != 0) {

        DefaultDelta delatX;
        DefaultDelta delatY;
        DefaultDelta delatXadvance;
        DefaultDelta delatYadvance;

        for (auto& parameters : glyphParamertersArray) {
          if (parameters.scalex != 0) {
            delatX.push_back(record.xPlacement * (parameters.scalex / 100) - record.xPlacement);
            delatY.push_back(0);
            delatXadvance.push_back(record.xAdvance * (parameters.scalex / 100) - record.xAdvance);
            delatYadvance.push_back(0);
          }

        }

        valueRecords << (quint16)currentVariationIndexesOffset;
        currentVariationIndexesOffset += 6;
        valueRecords << (quint16)currentVariationIndexesOffset;
        currentVariationIndexesOffset += 6;
        valueRecords << (quint16)currentVariationIndexesOffset;
        currentVariationIndexesOffset += 6;
        valueRecords << (quint16)currentVariationIndexesOffset;
        currentVariationIndexesOffset += 6;

        auto indexesX = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
        auto indexesY = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);
        auto indexesXadvance = m_layout->getDeltaSetEntry(delatXadvance, regionIndexesArrayIndex);
        auto indexesYadvance = m_layout->getDeltaSetEntry(delatYadvance, regionIndexesArrayIndex);

        VariationIndexes << (quint16)indexesX.first;
        VariationIndexes << (quint16)indexesX.second;
        VariationIndexes << (quint16)0x8000;

        VariationIndexes << (quint16)indexesY.first;
        VariationIndexes << (quint16)indexesY.second;
        VariationIndexes << (quint16)0x8000;

        VariationIndexes << (quint16)indexesXadvance.first;
        VariationIndexes << (quint16)indexesXadvance.second;
        VariationIndexes << (quint16)0x8000;

        VariationIndexes << (quint16)indexesYadvance.first;
        VariationIndexes << (quint16)indexesYadvance.second;
        VariationIndexes << (quint16)0x8000;

      }
      else {
        valueRecords << (quint16)0;
        valueRecords << (quint16)0;
        valueRecords << (quint16)0;
        valueRecords << (quint16)0;
      }
    }

    coverage << (quint16)i.key();
  }

  if (valueRecordsSize != valueRecords.size()) {
    throw std::runtime_error("ERROR");
  }

  quint32 coverageOffset = 8 + valueRecords.size() + VariationIndexes.size();

  if (coverageOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name.toStdString() << " Subtable " << name.toStdString() << " Overflows : " << coverageOffset << std::endl;
  }

  root << format;
  root << (quint16)coverageOffset;
  root << valueFormat;
  root << glyphCount;
  root.append(valueRecords);
  root.append(VariationIndexes);
  root.append(coverage);

  return root;
}
void SingleAdjustmentSubtable::readParameters(const QJsonObject& json) {


  if (json["parameters"].isObject()) {
    QJsonObject parametersObject = json["parameters"].toObject();
    for (int ia = 0; ia < parametersObject.size(); ++ia) {
      QString glyphName = parametersObject.keys()[ia];
      QJsonArray pointArray = parametersObject[glyphName].toArray();
      parameters[m_layout->glyphCodePerName[glyphName]] = { (qint16)pointArray[0].toInt(), (qint16)pointArray[1].toInt(),(qint16)pointArray[2].toInt(),(qint16)pointArray[3].toInt() };
    }

  }

}
void SingleAdjustmentSubtable::saveParameters(QJsonObject& json) const {

  if (parameters.size() != 0) {
    QJsonObject parametersObject;
    for (auto parameter = parameters.constBegin(); parameter != parameters.constEnd(); ++parameter) {
      if (!parameter.value().isEmpty()) {
        QJsonArray pointArray;
        pointArray.append(parameter.value().xPlacement);
        pointArray.append(parameter.value().yPlacement);
        pointArray.append(parameter.value().xAdvance);
        pointArray.append(parameter.value().yAdvance);
        parametersObject[m_layout->glyphNamePerCode[parameter.key()]] = pointArray;
      }

    }

    json["parameters"] = parametersObject;
  }
}

MultipleSubtable::MultipleSubtable(Lookup* lookup) : Subtable(lookup) {
}

QByteArray MultipleSubtable::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;
  QByteArray sequencetables;
  QDataStream root_stream(&root, QIODevice::WriteOnly);
  QDataStream coverage_stream(&coverage, QIODevice::WriteOnly);
  QDataStream seqtable_stream(&sequencetables, QIODevice::WriteOnly);



  quint16 total = subst.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root_stream << (quint16)1;
  root_stream << coverage_offset;
  root_stream << total;


  coverage_stream << (quint16)1;
  coverage_stream << (quint16)total;

  QMapIterator<quint16, QVector<quint16> >i(subst);
  quint16 seqtables_size = 0;
  while (i.hasNext()) {
    i.next();



    QVector<quint16> seqtable = i.value();

    root_stream << debutsequence;
    coverage_stream << (quint16)i.key();
    seqtable_stream << (quint16)seqtable.size();
    seqtable_stream << seqtable;

    debutsequence += 2 + 2 * seqtable.size();
  }

  root.append(coverage);
  root.append(sequencetables);


  return root;
};

void MultipleSubtable::readJson(const QJsonObject& json)
{
  subst.clear();
  for (int index = 0; index < json.size(); ++index) {
    QString glyphname = json.keys()[index];
    uint uniode = getCodeFromName(glyphname);
    if (!uniode) continue;


    QJsonArray dest = json[glyphname].toArray();

    for (int id = 0; id < dest.size(); ++id) {
      QString name = dest[id].toString();

      uint value = getCodeFromName(name);

      if (!value) continue;

      subst[uniode].append(value);
    }

  }
}

AlternateSubtable::AlternateSubtable(Lookup* lookup, quint16 format) : Subtable(lookup), format{ format } {}

void AlternateSubtable::generateSubstEquivGlyphs() {

  QMapIterator<quint16, QVector<ExtendedGlyph> >i(alternates);
  while (i.hasNext()) {
    i.next();



    QVector<ExtendedGlyph> seqtable = i.value();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        m_layout->getAlternate(alternateGlyph.code, parameters, true, true);
      }
    }
  }
}

QByteArray AlternateSubtable::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;
  QByteArray sequencetables;
  QDataStream root_stream(&root, QIODevice::WriteOnly);
  QDataStream coverage_stream(&coverage, QIODevice::WriteOnly);
  QDataStream seqtable_stream(&sequencetables, QIODevice::WriteOnly);

  quint16 total = alternates.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root_stream << (quint16)1;
  root_stream << coverage_offset;
  root_stream << total;


  coverage_stream << (quint16)1;
  coverage_stream << (quint16)total;

  QMapIterator<quint16, QVector<ExtendedGlyph> >i(alternates);
  quint16 seqtables_size = 0;
  while (i.hasNext()) {
    i.next();



    QVector<ExtendedGlyph> seqtable = i.value();

    root_stream << debutsequence;
    coverage_stream << (quint16)i.key();
    seqtable_stream << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        auto newGlyph = m_layout->getAlternate(alternateGlyph.code, parameters, true, false);

        seqtable_stream << (quint16)newGlyph->charcode;
      }
      else {
        seqtable_stream << (quint16)alternateGlyph.code;
      }
    }
    //seqtable_stream << seqtable;

    debutsequence += 2 + 2 * seqtable.size();
  }

  root.append(coverage);
  root.append(sequencetables);


  return root;
};

AlternateSubtableWithTatweel::AlternateSubtableWithTatweel(Lookup* lookup) : AlternateSubtable(lookup, 10) {};

void AlternateSubtableWithTatweel::generateSubstEquivGlyphs() {

  QMapIterator<quint16, QVector<ExtendedGlyph> >i(alternates);
  while (i.hasNext()) {
    i.next();



    QVector<ExtendedGlyph> seqtable = i.value();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        m_layout->getAlternate(alternateGlyph.code, parameters, true, true);
      }
    }
  }
}

QByteArray AlternateSubtableWithTatweel::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverage;
  QByteArray sequencetables;
  QDataStream root_stream(&root, QIODevice::WriteOnly);
  QDataStream coverage_stream(&coverage, QIODevice::WriteOnly);
  QDataStream seqtable_stream(&sequencetables, QIODevice::WriteOnly);

  quint16 total = alternates.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root_stream << (quint16)format;
  root_stream << coverage_offset;
  root_stream << total;


  coverage_stream << (quint16)1;
  coverage_stream << (quint16)total;

  QMapIterator<quint16, QVector<ExtendedGlyph> >i(alternates);
  quint16 seqtables_size = 0;
  while (i.hasNext()) {
    i.next();

    QVector<ExtendedGlyph> seqtable = i.value();

    root_stream << debutsequence;
    coverage_stream << (quint16)i.key();

    QByteArray alternatesArray;
    QByteArray tatweelsArray;

    QDataStream alternatesArrayStream(&alternatesArray, QIODevice::WriteOnly);
    QDataStream tatweelsArrayStream(&tatweelsArray, QIODevice::WriteOnly);

    /*
    seqtable_stream << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        auto newGlyph = m_layout->getAlternate(alternateGlyph.code, parameters, true, false);

        seqtable_stream << (quint16)newGlyph->charcode;
      }
      else {
        seqtable_stream << (quint16)alternateGlyph.code;
      }
    }

    debutsequence += 2 + 2 * seqtable.size();*/

    alternatesArrayStream << (quint16)seqtable.size();
    tatweelsArrayStream << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      alternatesArrayStream << (quint16)alternateGlyph.code;
      if (!m_layout->useNormAxisValues) {
        OT::F16DOT16 lefttatweel;
        lefttatweel.set_float(alternateGlyph.lefttatweel);

        OT::F16DOT16 righttatweel;
        righttatweel.set_float(alternateGlyph.righttatweel);

        tatweelsArrayStream << (int32_t)lefttatweel.to_int() << (int32_t)righttatweel.to_int();
      }
      else {
        throw std::runtime_error("Not implemented");
      }
    }

    //seqtable_stream << alternatesArrayStream;
    //seqtable_stream << tatweelsArrayStream;

    sequencetables.append(alternatesArray);
    sequencetables.append(tatweelsArray);

    debutsequence += alternatesArray.size() + tatweelsArray.size();

  }

  root.append(coverage);
  root.append(sequencetables);


  return root;
};

QByteArray AlternateSubtableWithTatweel::getConvertedOpenTypeTable() {

  QByteArray root;
  QByteArray coverage;
  QByteArray sequencetables;
  QDataStream root_stream(&root, QIODevice::WriteOnly);
  QDataStream coverage_stream(&coverage, QIODevice::WriteOnly);
  QDataStream seqtable_stream(&sequencetables, QIODevice::WriteOnly);

  quint16 total = alternates.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root_stream << (quint16)1;
  root_stream << coverage_offset;
  root_stream << total;


  coverage_stream << (quint16)1;
  coverage_stream << (quint16)total;

  QMapIterator<quint16, QVector<ExtendedGlyph> >i(alternates);
  quint16 seqtables_size = 0;
  while (i.hasNext()) {
    i.next();



    QVector<ExtendedGlyph> seqtable = i.value();

    root_stream << debutsequence;
    coverage_stream << (quint16)i.key();
    seqtable_stream << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        auto newGlyph = m_layout->getAlternate(alternateGlyph.code, parameters, true, false);

        seqtable_stream << (quint16)newGlyph->charcode;
      }
      else {
        seqtable_stream << (quint16)alternateGlyph.code;
      }
    }
    //seqtable_stream << seqtable;

    debutsequence += 2 + 2 * seqtable.size();
  }

  root.append(coverage);
  root.append(sequencetables);


  return root;
};


LigatureSubtable::LigatureSubtable(Lookup* lookup) : Subtable(lookup) {
}

QByteArray LigatureSubtable::getOpenTypeTable(bool extended) {

  struct Ligaturetable {
    quint16 ligatureGlyph;
    QVector<quint16> componentGlyphIDs;
  };

  QMap<quint16, QVector<Ligaturetable> > LigatureSets;

  for (auto ligature : ligatures) {
    quint16 ligatureGlyph = ligature.ligatureGlyph;
    auto seq = ligature.componentGlyphIDs;
    LigatureSets[seq.at(0)].append({ ligatureGlyph, seq.mid(1) });
  }

  QByteArray root;
  QByteArray coverage;
  QByteArray LigatureSetTables;



  quint16 ligatureSetCount = LigatureSets.size();
  quint16 coverage_offset = 2 + 2 + 2 + 2 * ligatureSetCount;
  quint16 coverage_size = 2 + 2 + 2 * ligatureSetCount;
  quint16 ligatureSetOffsets = coverage_offset + coverage_size;


  root << (quint16)format;
  root << coverage_offset;
  root << ligatureSetCount;


  coverage << (quint16)1;
  coverage << (quint16)ligatureSetCount;

  for (auto it = LigatureSets.constBegin(); it != LigatureSets.constEnd(); ++it) {
    quint16 coverageGlyph = it.key();
    auto seq = it.value();
    coverage << coverageGlyph;
    root << ligatureSetOffsets;

    quint16 ligatureCount = seq.size();
    quint16 ligatureOffsets = 2 + 2 * ligatureCount;

    QByteArray LigatureSetTable;
    QByteArray LigatureTables;
    LigatureSetTable << (quint16)ligatureCount;


    for (int i = 0; i < ligatureCount; i++) {

      LigatureSetTable << ligatureOffsets;

      QByteArray ligatureTable;
      ligatureTable << seq.at(i).ligatureGlyph;
      ligatureTable << (quint16)(seq.at(i).componentGlyphIDs.size() + 1);
      ligatureTable << seq.at(i).componentGlyphIDs;

      ligatureOffsets += ligatureTable.size();

      LigatureTables.append(ligatureTable);


    }
    LigatureSetTable.append(LigatureTables);

    ligatureSetOffsets += LigatureSetTable.size();

    LigatureSetTables.append(LigatureSetTable);


  }

  root.append(coverage);
  root.append(LigatureSetTables);

  return root;
};

void LigatureSubtable::readJson(const QJsonObject& json)
{
  ligatures.clear();
  for (int index = 0; index < json.size(); ++index) {
    QString ligatureName = json.keys()[index];
    quint16 uniode = getCodeFromName(ligatureName);
    if (!uniode) continue;


    QJsonArray dest = json[ligatureName].toArray();

    QVector<quint16> componentGlyphIDs;

    for (int id = 0; id < dest.size(); ++id) {
      QString name = dest[id].toString();

      uint value = getCodeFromName(name);

      if (!value) continue;

      //ligatures[uniode].append(value);
      componentGlyphIDs.append(value);
    }
    ligatures.append({ uniode ,componentGlyphIDs });

  }
}

MarkBaseSubtable::MarkBaseSubtable(Lookup* lookup) : Subtable(lookup) {}


void MarkBaseSubtable::readJson(const QJsonObject& json)
{
  if (json["base"].isString()) {
    base = { json["base"].toString() };
  }
  else {
    QJsonArray basearray = json["base"].toArray();
    base = {};
    for (int baseIndex = 0; baseIndex < basearray.size(); ++baseIndex) {
      base.append(basearray[baseIndex].toString());
    }
  }

  QJsonObject classesobject = json["classes"].toObject();
  for (int index = 0; index < classesobject.size(); ++index) {
    QString className = classesobject.keys()[index];
    QJsonObject classobject = classesobject[className].toObject();

    MarkClass newclass;

    if (classobject["mark"].isString()) {
      newclass.mark = { classobject["mark"].toString() };
    }
    else {
      QJsonArray array = classobject["mark"].toArray();
      newclass.mark = {};
      for (int ia = 0; ia < array.size(); ++ia) {
        newclass.mark.insert(array[ia].toString());
      }
    }

    newclass.basefunction = m_layout->getanchorCalcFunctions(classobject["basefunction"].toString(), this);
    newclass.markfunction = m_layout->getanchorCalcFunctions(classobject["markfunction"].toString(), this);

    if (classobject["baseparameters"].isObject()) {
      QJsonObject baseparametersObject = classobject["baseparameters"].toObject();
      for (int ia = 0; ia < baseparametersObject.size(); ++ia) {
        QString glyphName = baseparametersObject.keys()[ia];
        QJsonArray pointArray = baseparametersObject[glyphName].toArray();
        newclass.baseparameters[glyphName] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

    }

    if (classobject["markparameters"].isObject()) {
      QJsonObject markparametersObject = classobject["markparameters"].toObject();
      for (int ia = 0; ia < markparametersObject.size(); ++ia) {
        QString glyphName = markparametersObject.keys()[ia];
        QJsonArray pointArray = markparametersObject[glyphName].toArray();
        newclass.markparameters[glyphName] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

    }

    if (classobject["baseanchors"].isObject()) {
      QJsonObject baseanchorsObject = classobject["baseanchors"].toObject();
      for (int ia = 0; ia < baseanchorsObject.size(); ++ia) {
        QString glyphName = baseanchorsObject.keys()[ia];
        QJsonArray pointArray = baseanchorsObject[glyphName].toArray();
        newclass.baseanchors[glyphName] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

    }

    if (classobject["markanchors"].isObject()) {
      QJsonObject markanchorsObject = classobject["markanchors"].toObject();
      for (int ia = 0; ia < markanchorsObject.size(); ++ia) {
        QString glyphName = markanchorsObject.keys()[ia];
        QJsonArray pointArray = markanchorsObject[glyphName].toArray();
        newclass.markanchors[glyphName] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

    }

    classes[className] = newclass;
  }


}
optional<QPoint> CursiveSubtable::getExit(quint16 glyph_id, GlyphParameters parameters) {

  optional<QPoint> exit;

  auto anchorType = m_lookup->flags & Lookup::Flags::RightToLeft ? GlyphVis::AnchorType::ExitAnchorRTL : GlyphVis::AnchorType::ExitAnchor;

  if (anchors.contains(glyph_id)) {

    auto& entryexit = anchors[glyph_id];

    GlyphVis* originalglyph = m_layout->getGlyph(glyph_id);
    GlyphVis* curr = originalglyph;

    curr = originalglyph->getAlternate(parameters);

    if (!entryexit.exitName.isEmpty() && curr->conatinsAnchor(entryexit.exitName, anchorType)) {
      exit = curr->getAnchor(entryexit.exitName, anchorType);
    }
    else if (curr->conatinsAnchor(this->name, anchorType)) {
      exit = curr->getAnchor(this->name, anchorType);
    }
    else if (entryexit.exitFunction) {
      exit = entryexit.exitFunction(false, originalglyph, curr);
    }
    else {
      exit = entryexit.exit;
      //exit = calculateEntry(originalglyph, curr, entry);
    }

    if (exit && exitParameters.contains(glyph_id)) {
      exit = *exit + exitParameters[glyph_id];
    }
  }

  return exit;
}

optional<QPoint> CursiveSubtable::getEntry(quint16 glyph_id, GlyphParameters parameters) {

  optional<QPoint> entry;

  auto anchorType = m_lookup->flags & Lookup::Flags::RightToLeft ? GlyphVis::AnchorType::EntryAnchorRTL : GlyphVis::AnchorType::EntryAnchor;

  if (anchors.contains(glyph_id)) {

    auto& entryexit = anchors[glyph_id];

    GlyphVis* originalglyph = m_layout->getGlyph(glyph_id);
    GlyphVis* curr = originalglyph;

    curr = originalglyph->getAlternate(parameters);

    if (!entryexit.entryName.isEmpty() && curr->conatinsAnchor(entryexit.entryName, anchorType)) {
      entry = curr->getAnchor(entryexit.entryName, anchorType);
    }
    else if (curr->conatinsAnchor(this->name, anchorType)) {
      entry = curr->getAnchor(this->name, anchorType);
    }
    else if (entryexit.entryFunction) {
      entry = entryexit.entryFunction(true, originalglyph, curr);
    }
    else {
      entry = entryexit.entry;
      if (entry) {
        entry = calculateEntry(originalglyph, curr, *entry);
      }
    }

    if (entry && entryParameters.contains(glyph_id)) {
      entry = *entry + entryParameters[glyph_id];
    }
  }

  return entry;
}
QPoint CursiveSubtable::calculateEntry(GlyphVis* originalglyph, GlyphVis* extendedglyph, QPoint entry) {

  /*
  double xshift = extendedglyph->matrix.xpart - originalglyph->matrix.xpart;
  double yshift = extendedglyph->matrix.ypart - originalglyph->matrix.ypart;*/

  double xshift = extendedglyph->width - originalglyph->width;

  double yshift = 0;

  entry += QPoint(xshift, yshift);

  return entry;

}
void CursiveSubtable::readJson(const QJsonObject& json) {

  if (json["anchors"].isObject()) {
    auto anchorsObject = json["anchors"].toObject();
    for (int index = 0; index < anchorsObject.size(); ++index) {
      QString className = anchorsObject.keys()[index];

      EntryExit value;

      auto entryexitObject = anchorsObject[className].toObject();

      if (entryexitObject["exit"].isArray()) {
        QJsonArray pointArray = entryexitObject["exit"].toArray();
        value.exit = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

      if (entryexitObject["entry"].isArray()) {
        QJsonArray pointArray = entryexitObject["entry"].toArray();
        value.entry = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

      if (value.entry || value.exit) {
        auto glyphs = m_layout->classtoUnicode(className);

        for (auto glyph : glyphs) {
          anchors[glyph] = value;
        }
      }
    }
  }
}
void CursiveSubtable::readParameters(const QJsonObject& json) {


  if (json["exitParameters"].isObject()) {
    QJsonObject exitParametersObject = json["exitParameters"].toObject();
    for (int ia = 0; ia < exitParametersObject.size(); ++ia) {
      QString glyphName = exitParametersObject.keys()[ia];
      QJsonArray pointArray = exitParametersObject[glyphName].toArray();
      exitParameters[m_layout->glyphCodePerName[glyphName]] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
    }

  }

  if (json["entryParameters"].isObject()) {
    QJsonObject entryParametersObject = json["entryParameters"].toObject();
    for (int ia = 0; ia < entryParametersObject.size(); ++ia) {
      QString glyphName = entryParametersObject.keys()[ia];
      QJsonArray pointArray = entryParametersObject[glyphName].toArray();
      entryParameters[m_layout->glyphCodePerName[glyphName]] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
    }

  }

}
void CursiveSubtable::saveParameters(QJsonObject& json) const {

  if (exitParameters.size() != 0) {
    QJsonObject exitParametersObject;
    for (auto exitParameter = exitParameters.constBegin(); exitParameter != exitParameters.constEnd(); ++exitParameter) {
      if (!exitParameter.value().isNull()) {
        QJsonArray pointArray;
        pointArray.append(exitParameter.value().x());
        pointArray.append(exitParameter.value().y());
        exitParametersObject[m_layout->glyphNamePerCode[exitParameter.key()]] = pointArray;
      }

    }

    json["exitParameters"] = exitParametersObject;
  }

  if (entryParameters.size() != 0) {
    QJsonObject entryParametersObject;
    for (auto entryParameter = entryParameters.constBegin(); entryParameter != entryParameters.constEnd(); ++entryParameter) {
      if (!entryParameter.value().isNull()) {
        QJsonArray pointArray;
        pointArray.append(entryParameter.value().x());
        pointArray.append(entryParameter.value().y());
        entryParametersObject[m_layout->glyphNamePerCode[entryParameter.key()]] = pointArray;
      }

    }

    json["entryParameters"] = entryParametersObject;
  }



}

QByteArray CursiveSubtable::getOpenTypeTable(bool extended) {


  QByteArray anchorTables;
  QByteArray entryExitRecords;

  quint16 entryExitCount = anchors.size();

  quint16 coverageOffset = 2 + 2 + 2 + entryExitCount * 4;
  quint32 anchorOffset = coverageOffset + 2 + 2 + 2 * entryExitCount;

  QByteArray coverage;
  coverage << (quint16)1 << entryExitCount << anchors.keys();


  bool rtl = m_lookup->flags & Lookup::Flags::RightToLeft;

  for (auto anchor = anchors.constBegin(); anchor != anchors.constEnd(); ++anchor) {

    auto& entryexit = anchor.value();

    auto glyphCode = anchor.key();

    /*
    auto calcentry = entryexit.entry;

    if (!calcentry) {
      if (!entryexit.entryName.isEmpty()) {
        GlyphVis* curr = m_layout->getGlyph(anchor.key());
        auto it = curr->anchors.find({ entryexit.entryName,rtl ? GlyphVis::AnchorType::EntryAnchorRTL : GlyphVis::AnchorType::EntryAnchor });
        if (it != curr->anchors.end()) {
          calcentry = it->anchor;
        }
      }
    }*/


    std::optional<QPoint> calcentry;

    if (!extended) {
      auto glyph = m_layout->getGlyph(glyphCode);
      if (glyph->name.contains(".added_")) {

        auto& originalGlyph = m_layout->glyphs[glyph->originalglyph];

        calcentry = getEntry(originalGlyph.charcode, { .lefttatweel = glyph->charlt, .righttatweel = glyph->charrt });
      }
      else {
        calcentry = getEntry(glyphCode, {});
      }
    }
    else {
      calcentry = getEntry(glyphCode, {});
    }

    if (calcentry) {

      QPoint entry{ *(calcentry) };

      /*
      if (entryParameters.contains(anchor.key())) {
        entry += entryParameters[anchor.key()];
      }*/



      entryExitRecords << (quint16)anchorOffset;

      bool done = false;

      if (m_layout->isOTVar) {

        auto glyphName = m_layout->glyphNamePerCode[anchor.key()];

        auto regionIndexes = m_layout->toOpenType->getGlyphParameters(glyphName);
        int regionIndexesArrayIndex = regionIndexes.second;
        auto glyphParamertersArray = regionIndexes.first;

        if (glyphParamertersArray.size() != 0) {
          DefaultDelta delatX;
          DefaultDelta delatY;
          for (auto parameters : glyphParamertersArray) {
            if (parameters.scalex != 0) {
              delatX.push_back(entry.x() * (parameters.scalex / 100) - entry.x());
              delatY.push_back(0);
            }
            else {
              auto val = getEntry(anchor.key(), parameters);
              delatX.push_back(val->x() - entry.x());
              delatY.push_back(val->y() - entry.y());
            }
          }
          bool all0 = true;

          for (auto delta : delatX) {
            if (delta != 0) {
              all0 = false;
              break;
            }
          }
          if (all0) {
            for (auto delta : delatY) {
              if (delta != 0) {
                all0 = false;
                break;
              }
            }
          }

          if (!all0) {
            auto index_x = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
            auto index_y = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);

            anchorTables << (quint16)3;
            anchorTables << (quint16)entry.x();
            anchorTables << (quint16)entry.y();
            anchorTables << (quint16)10; // xDeviceOffset
            anchorTables << (quint16)16; // yDeviceOffset
            //VariationIndex x
            anchorTables << (quint16)index_x.first;
            anchorTables << (quint16)index_x.second;
            anchorTables << (quint16)0x8000;
            //VariationIndex y
            anchorTables << (quint16)index_y.first;
            anchorTables << (quint16)index_y.second;
            anchorTables << (quint16)0x8000;
            done = true;

            anchorOffset += 22;
          }
        }
      }
      if (!done) {

        anchorTables << (quint16)1;
        anchorTables << (quint16)entry.x();
        anchorTables << (quint16)entry.y();

        anchorOffset += 6;
      }



    }
    else {
      entryExitRecords << (quint16)0;
    }

    std::optional<QPoint> calcexit;


    if (!extended) {
      auto glyph = m_layout->getGlyph(glyphCode);
      if (glyph->name.contains(".added_")) {

        auto& originalGlyph = m_layout->glyphs[glyph->originalglyph];

        calcexit = getExit(originalGlyph.charcode, { .lefttatweel = glyph->charlt, .righttatweel = glyph->charrt });
      }
      else {
        calcexit = getExit(glyphCode, {});
      }
    }
    else {
      calcexit = getExit(glyphCode, {});
    }

    if (calcexit) {

      QPoint exit{ *(calcexit) };

      /*
      if (exitParameters.contains(anchor.key())) {
        exit += exitParameters[anchor.key()];
      }*/

      entryExitRecords << (quint16)anchorOffset;


      bool done = false;

      if (m_layout->isOTVar) {

        auto glyphName = m_layout->glyphNamePerCode[anchor.key()];

        auto regionIndexes = m_layout->toOpenType->getGlyphParameters(glyphName);
        int regionIndexesArrayIndex = regionIndexes.second;
        auto glyphParamertersArray = regionIndexes.first;

        if (glyphParamertersArray.size() != 0) {
          DefaultDelta delatX;
          DefaultDelta delatY;
          for (auto parameters : glyphParamertersArray) {
            if (parameters.scalex != 0) {
              delatX.push_back(exit.x() * (parameters.scalex / 100) - exit.x());
              delatY.push_back(0);
            }
            else {
              auto val = getExit(anchor.key(), parameters);
              delatX.push_back(val->x() - exit.x());
              delatY.push_back(val->y() - exit.y());
            }
          }

          bool all0 = true;

          for (auto delta : delatX) {
            if (delta != 0) {
              all0 = false;
              break;
            }
          }
          if (all0) {
            for (auto delta : delatY) {
              if (delta != 0) {
                all0 = false;
                break;
              }
            }
          }

          if (!all0) {
            auto index_x = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
            auto index_y = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);

            anchorTables << (quint16)3;
            anchorTables << (quint16)exit.x();
            anchorTables << (quint16)exit.y();
            anchorTables << (quint16)10; // xDeviceOffset
            anchorTables << (quint16)16; // yDeviceOffset
            //VariationIndex x
            anchorTables << (quint16)index_x.first;
            anchorTables << (quint16)index_x.second;
            anchorTables << (quint16)0x8000;
            //VariationIndex y
            anchorTables << (quint16)index_y.first;
            anchorTables << (quint16)index_y.second;
            anchorTables << (quint16)0x8000;
            done = true;

            anchorOffset += 22;
          }
        }
      }
      if (!done) {

        anchorTables << (quint16)1;
        anchorTables << (quint16)exit.x();
        anchorTables << (quint16)exit.y();

        anchorOffset += 6;
      }

    }
    else {
      entryExitRecords << (quint16)0;
    }
  }

  QByteArray root;
  root << (quint16)1;
  root << coverageOffset;
  root << entryExitCount;
  root.append(entryExitRecords);
  root.append(coverage);
  root.append(anchorTables);

  if (anchorOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name.toStdString() << " Subtable "
      << name.toStdString() << " Overflows : lastOffset=" << anchorOffset
      << " entryExitCount=" << entryExitCount
      << " coverage size=" << coverage.size()
      << " entryExitRecords size=" << entryExitRecords.size()
      << " anchorTables size=" << anchorTables.size()
      << std::endl;
  }



  return root;

}
void MarkBaseSubtable::saveParameters(QJsonObject& json) const {
  for (auto it = classes.constBegin(); it != classes.constEnd(); ++it) {
    QJsonObject classObject;
    QJsonObject baseparametersObject;
    auto parameters = it.value().baseparameters;
    for (auto parameter = parameters.constBegin(); parameter != parameters.constEnd(); parameter++) {
      if (!parameter.value().isNull()) {
        QString glyphName = parameter.key();
        QJsonArray pointArray;
        pointArray.append(parameter.value().x());
        pointArray.append(parameter.value().y());
        baseparametersObject[glyphName] = pointArray;
      }

    }
    parameters = it.value().markparameters;
    QJsonObject markparametersObject;
    for (auto parameter = parameters.constBegin(); parameter != parameters.constEnd(); parameter++) {
      if (!parameter.value().isNull()) {
        QString glyphName = parameter.key();
        QJsonArray pointArray;
        pointArray.append(parameter.value().x());
        pointArray.append(parameter.value().y());
        markparametersObject[glyphName] = pointArray;
      }
    }

    if (!baseparametersObject.isEmpty()) {
      classObject["baseparameters"] = baseparametersObject;
    }

    if (!markparametersObject.isEmpty()) {
      classObject["markparameters"] = markparametersObject;
    }

    if (!classObject.isEmpty()) {
      json[it.key()] = classObject;
    }

  }
}
void MarkBaseSubtable::readParameters(const QJsonObject& json) {

  for (int index = 0; index < json.size(); ++index) {
    QString className = json.keys()[index];
    QJsonObject classobject = json[className].toObject();

    MarkClass& newclass = classes[className];


    if (classobject["baseparameters"].isObject()) {
      QJsonObject baseparametersObject = classobject["baseparameters"].toObject();
      for (int ia = 0; ia < baseparametersObject.size(); ++ia) {
        QString glyphName = baseparametersObject.keys()[ia];
        QJsonArray pointArray = baseparametersObject[glyphName].toArray();
        newclass.baseparameters[glyphName] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

    }

    if (classobject["markparameters"].isObject()) {
      QJsonObject markparametersObject = classobject["markparameters"].toObject();
      for (int ia = 0; ia < markparametersObject.size(); ++ia) {
        QString glyphName = markparametersObject.keys()[ia];
        QJsonArray pointArray = markparametersObject[glyphName].toArray();
        newclass.markparameters[glyphName] = QPoint(pointArray[0].toInt(), pointArray[1].toInt());
      }

    }
  }

}
QPoint MarkBaseSubtable::getBaseAnchor(QString baseGlyphName, QString className, GlyphParameters parameters) {

  QPoint coordinate;

  auto markClass = classes[className];

  if (markClass.baseparameters.contains(baseGlyphName)) {
    coordinate = markClass.baseparameters[baseGlyphName];
  }

  /*
  if (baseGlyphName.contains("kaf.medi.ii.i") && className == "smallletters" && lefttatweel != 0) {
    std::cout << "kaf.medi.ii.i";
  }*/

  GlyphVis* curr = &m_layout->glyphs[baseGlyphName];

  curr = curr->getAlternate(parameters);

  if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
    coordinate += curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor);
  }
  else {
    QString anchorName = m_lookup->name + "_" + className;
    if (curr->conatinsAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor)) {
      coordinate += curr->getAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor);
    }
    else {
      QString anchorName = m_lookup->name;
      if (curr->conatinsAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor)) {
        coordinate += curr->getAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor);
      }
      else if (markClass.basefunction) {
        coordinate = markClass.basefunction(baseGlyphName, className, coordinate, parameters);
      }
      else if (markClass.baseanchors.contains(baseGlyphName)) {
        coordinate += markClass.baseanchors[baseGlyphName];
      }
    }
  }

  return coordinate;
}

optional<QPoint> MarkBaseSubtable::getBaseAnchor(quint16 mark_id, quint16 base_id, GlyphParameters parameters) {

  quint16 classIndex = markCodes[mark_id];

  QString className = classNamebyIndex[classIndex];

  QString baseGlyphName = m_layout->glyphNamePerCode[base_id];

  return getBaseAnchor(baseGlyphName, className, parameters);


}
QPoint MarkBaseSubtable::getMarkAnchor(QString markGlyphName, QString className, GlyphParameters parameters) {

  QPoint coordinate;

  auto markClass = classes[className];

  if (markClass.markparameters.contains(markGlyphName)) {
    coordinate = markClass.markparameters[markGlyphName];
  }

  GlyphVis* curr = &m_layout->glyphs[markGlyphName];

  curr = curr->getAlternate(parameters);

  if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
    coordinate += curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor);
  }
  else {
    QString anchorName = m_lookup->name + "_" + className;
    if (curr->conatinsAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor)) {
      coordinate += curr->getAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor);
    }
    else {
      QString anchorName = m_lookup->name;
      if (curr->conatinsAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor)) {
        coordinate += curr->getAnchor(anchorName, GlyphVis::AnchorType::MarkAnchor);
      }
      else if (markClass.markfunction != nullptr) {
        coordinate = markClass.markfunction(markGlyphName, className, coordinate, parameters);
      }
      else if (markClass.markanchors.contains(markGlyphName)) {
        coordinate += markClass.markanchors[markGlyphName];
      }
    }
  }

  return coordinate;
}
optional<QPoint> MarkBaseSubtable::getMarkAnchor(quint16 mark_id, quint16 base_id, GlyphParameters parameters) {

  quint16 classIndex = markCodes[mark_id];

  QString className = classNamebyIndex[classIndex];

  QString markGlyphName = m_layout->glyphNamePerCode[mark_id];

  return getMarkAnchor(markGlyphName, className, parameters);

}

void MarkBaseSubtable::setAnchorTable(QString className,
  quint16 glyphCode,
  QByteArray& anchorTables,
  quint32& anchorOffset,
  std::map<int,std::pair<bool,std::pair<int,int>>>& posToVar,
  bool extended,
  bool isBase
){

  QString glyphName = m_layout->glyphNamePerCode[glyphCode];

  QString originalGlyph = glyphName;
  double charlt = 0.0;
  double charrt = 0.0;

  if (!extended) {
    auto glyph = m_layout->getGlyph(glyphCode);
    if (glyph->name.contains(".added_")) {
      originalGlyph = glyph->originalglyph;
    }
    charlt = glyph->charlt;
    charrt = glyph->charrt;
  }

  QPoint coordinate = isBase ?
    getBaseAnchor(originalGlyph, className, { .lefttatweel = charlt, .righttatweel = charrt }) : 
    getMarkAnchor(originalGlyph, className, { .lefttatweel = charlt, .righttatweel = charrt });

  bool done = false;
  if (m_layout->isOTVar) {

    auto glyphName = m_layout->glyphNamePerCode[glyphCode];

    auto regionIndexes = m_layout->toOpenType->getGlyphParameters(glyphName);
    int regionIndexesArrayIndex = regionIndexes.second;
    auto glyphParamertersArray = regionIndexes.first;

    if (glyphParamertersArray.size() != 0) {
      DefaultDelta delatX;
      DefaultDelta delatY;
      for (auto& parameters : glyphParamertersArray) {
        if (parameters.scalex != 0) {
          delatX.push_back(coordinate.x() * (parameters.scalex / 100) - coordinate.x());
          delatY.push_back(0);
        }
        else {
          auto val = isBase ? getBaseAnchor(glyphName, className, parameters) : getMarkAnchor(glyphName, className, parameters);
          delatX.push_back(val.x() - coordinate.x());
          delatY.push_back(val.y() - coordinate.y());
        }

      }

      bool allx0 = true;

      for (auto delta : delatX) {
        if (delta != 0) {
          allx0 = false;
          break;
        }
      }
      bool ally0 = true;
      for (auto delta : delatY) {
        if (delta != 0) {
          ally0 = false;
          break;
        }
      }

      bool all0 = allx0 && ally0;

      if (!all0) {            
        anchorTables << (quint16)3;
        anchorTables << (quint16)coordinate.x();
        anchorTables << (quint16)coordinate.y();
        if(!allx0){
          auto index_x = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
          posToVar.insert({anchorTables.size(),{true,index_x}});
        }
        anchorTables << (quint16)0; // xDeviceOffset
        if(!ally0){
          auto index_y = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);
          posToVar.insert({anchorTables.size(),{false,index_y}});
        }
        anchorTables << (quint16)0; // yDeviceOffset
        anchorOffset += 10;       
        
        done = true;
      }
    }
  }

  if (!done) {
    anchorTables << (quint16)1;
    anchorTables << (quint16)coordinate.x();
    anchorTables << (quint16)coordinate.y();
    anchorOffset += 6;
  }

}

void MarkBaseSubtable::setVariationIndexOffset(
  QByteArray& anchorTables,
  quint32& anchorOffset,
  std::map<int,std::pair<bool,std::pair<int,int>>>& posToVar
){
  if (anchorOffset > 0xFFFF){
     std::cout << "Lookup " << m_lookup->name.toStdString() << " Subtable " << name.toStdString()
      << " Overflows. baseAnchorOffset=" << anchorOffset
      << std::endl;
  }

  int total=0;
  int found=0;
  std::map<std::pair<int,int>,int> indexes;
  
  for(auto& varIndex : posToVar){
    auto pos = varIndex.first;
    auto isX = varIndex.second.first;
    auto index = varIndex.second.second;

    quint32 offset;

    auto it = indexes.find(index);
    if(it != indexes.end()){
      found++;
      offset = it->second;
    }else{
      offset = anchorTables.size();
      indexes.insert({index,offset});
      anchorTables << (quint16)index.first;
      anchorTables << (quint16)index.second;
      anchorTables << (quint16)0x8000;
    }
    total++;

    quint32 offsetFromAnchorTable = offset - (pos -  (isX ? 6 : 8));
    if (offsetFromAnchorTable > 0xFFFF){
      std::cout << "Lookup " << m_lookup->name.toStdString() << " Subtable " << name.toStdString()
        << " Overflows. offsetFromAnchorTable=" << offsetFromAnchorTable
        << std::endl;
    }
    QByteArray offsetData;
    offsetData << (quint16)offsetFromAnchorTable;
    anchorTables.replace(pos,offsetData.size(),offsetData);

  }  

  std::cout << "Lookup " << m_lookup->name.toStdString() << " Subtable " << name.toStdString()
        << " total=" << total
        << " found=" << found
        << std::endl;
}

QByteArray MarkBaseSubtable::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray baseCoverage;
  QByteArray markCoverage;
  QByteArray markArray;
  QByteArray baseArray;
  QByteArray baseAnchorTables;  
  QByteArray markAnchorTables;

  int classIndex = 0;

  classNamebyIndex.clear();
  markCodes.clear();

  if (sortedBaseCodes.isEmpty()) {
    QSet<quint16> baseCodesSet;
    for (int i = 0; i < base.size(); ++i) {
      baseCodesSet.unite(m_layout->classtoUnicode(base.at(i)));
    }
    sortedBaseCodes = baseCodesSet.values();

    std::sort(sortedBaseCodes.begin(), sortedBaseCodes.end());
  }

  quint16 baseCount = sortedBaseCodes.size();
  quint16 markClassCount = classes.size();

  for (auto it = classes.constBegin(); it != classes.constEnd(); ++it) {
    MarkClass markClass = it.value();
    if (markClass.markCodes.isEmpty()) {
      for (auto markName : markClass.mark) {
        markClass.markCodes.unite(m_layout->classtoUnicode(markName));
      }
    }

    for (auto i = markClass.markCodes.constBegin(); i != markClass.markCodes.constEnd(); ++i) {
      markCodes[*i] = classIndex;
    }

    classNamebyIndex[classIndex] = it.key();

    classIndex++;

  }

  quint16 markCount = markCodes.size();

  // Base coverage && Base Array
  quint32 baseAnchorOffset = 2 + baseCount * (markClassCount * 2);

  std::map<int,std::pair<bool,std::pair<int,int>>> basePosToVar;

  baseCoverage << (quint16)1 << baseCount;
  baseArray << baseCount;

  for (int i = 0; i < sortedBaseCodes.size(); ++i) {
    quint16 glyphCode = sortedBaseCodes.at(i);
    QString baseglyphName = m_layout->glyphNamePerCode[glyphCode];
    baseCoverage << glyphCode;
    for (auto it = classes.constBegin(); it != classes.constEnd(); ++it) {
      baseArray << (quint16)baseAnchorOffset;      
      setAnchorTable(it.key(),glyphCode,baseAnchorTables,baseAnchorOffset,basePosToVar,extended,true);
    }
  }
  setVariationIndexOffset(baseAnchorTables,baseAnchorOffset,basePosToVar);  
  baseArray.append(baseAnchorTables);

  // Mark coverage && Mark Array


  quint32 markAnchorOffset = 2 + markCount * 4;

  std::map<int,std::pair<bool,std::pair<int,int>>> markPosToVar;

  markCoverage << (quint16)1 << markCount;
  markArray << markCount;

  for (auto it = markCodes.constBegin(); it != markCodes.constEnd(); ++it) {

    quint16 charcode = it.key();
    quint16 classIndex = it.value();
    QString className = classNamebyIndex[classIndex];
    
    markCoverage << charcode;

    markArray << classIndex;
    markArray << (quint16)markAnchorOffset;
    setAnchorTable(className,charcode,markAnchorTables,markAnchorOffset,markPosToVar,extended,false);
  }
  setVariationIndexOffset(markAnchorTables,markAnchorOffset,markPosToVar); 
  markArray.append(markAnchorTables);

  quint32 markCoverageOffset = 12;
  quint32 baseCoverageOffset = markCoverageOffset + markCoverage.size();
  quint32 markArrayOffset = baseCoverageOffset + baseCoverage.size();
  quint32 baseArrayOffset = markArrayOffset + markArray.size();

  if (baseArrayOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name.toStdString() << " Subtable " << name.toStdString()
      << " Overflows. baseCoverageOffset=" << baseCoverageOffset
      << " markArrayOffset=" << markArrayOffset
      << " baseArrayOffset=" << baseArrayOffset
      << std::endl;
  }


  root << (quint16)1 << (quint16)markCoverageOffset << (quint16)baseCoverageOffset << markClassCount << (quint16)markArrayOffset << (quint16)baseArrayOffset;
  root.append(markCoverage);
  root.append(baseCoverage);
  root.append(markArray);
  root.append(baseArray);

  isDirty = false;
  openTypeSubTable = root;

  return openTypeSubTable;



};

ChainingSubtable::ChainingSubtable(Lookup* lookup) : Subtable(lookup) {}

void ChainingSubtable::readJson(const QJsonObject& ruleObject) {

  rule = Rule();
  compiledRule = CompiledRule();



  //context
  QJsonArray inputArray = ruleObject["input"].toArray();
  for (int j = 0; j < inputArray.size(); j++) {
    QSet<QString> classSet;
    QSet<quint16> glyphSet;
    QJsonArray PosArray = inputArray[j].toArray();
    for (int k = 0; k < PosArray.size(); k++) {
      QString className = PosArray[k].toString();
      classSet.insert(className);
      auto glyphs = m_layout->classtoUnicode(className);
      glyphSet.unite(glyphs);
    }
    rule.input.append(classSet);
    compiledRule.input.append(glyphSet);

  }

  QJsonArray lookaheadArray = ruleObject["lookahead"].toArray();
  for (int j = 0; j < lookaheadArray.size(); j++) {
    QSet<QString> classSet;
    QSet<quint16> glyphSet;
    QJsonArray PosArray = lookaheadArray[j].toArray();
    for (int k = 0; k < PosArray.size(); k++) {
      QString className = PosArray[k].toString();
      classSet.insert(className);
      auto glyphs = m_layout->classtoUnicode(className);
      glyphSet.unite(glyphs);
    }
    rule.lookahead.append(classSet);
    compiledRule.lookahead.append(glyphSet);
  }
  QJsonArray backtrackArray = ruleObject["backtrack"].toArray();
  for (int j = 0; j < backtrackArray.size(); j++) {
    QSet<QString> classSet;
    QSet<quint16> glyphSet;
    QJsonArray PosArray = backtrackArray[j].toArray();
    for (int k = 0; k < PosArray.size(); k++) {
      QString className = PosArray[k].toString();
      classSet.insert(className);
      auto glyphs = m_layout->classtoUnicode(className);
      glyphSet.unite(glyphs);
    }
    rule.backtrack.append(classSet);
    compiledRule.backtrack.append(glyphSet);
  }

  QJsonArray lookuprecordsArray = ruleObject["lookuprecords"].toArray();
  for (int j = 0; j < lookuprecordsArray.size(); j++) {
    LookupRecord lookupRecord;
    QJsonObject lookupRecordObject = lookuprecordsArray[j].toObject();
    if (!lookupRecordObject.isEmpty()) {
      lookupRecord.lookupName = lookupRecordObject["lookup"].toString();
      lookupRecord.position = lookupRecordObject["position"].toInt();
    }
    else {

      auto lookupName = lookuprecordsArray[j].toString();
      if (!lookupName.isEmpty()) {
        lookupRecord.lookupName = lookupName;
        lookupRecord.position = j;
      }
      else {
        QJsonArray lookupRecordArray = lookuprecordsArray[j].toArray();
        if (lookupRecordArray.size() == 2) {
          lookupRecord.position = lookupRecordArray[0].toInt();
          lookupRecord.lookupName = lookupRecordArray[1].toString();

        }
      }
    }


    rule.lookupRecords.append(lookupRecord);
    compiledRule.lookupRecords.append(lookupRecord);
  }



}

QByteArray ChainingSubtable::getOpenTypeTable(bool extended) {

  QByteArray root;
  QByteArray coverages;

  quint16 backtrackGlyphCount = compiledRule.backtrack.size();
  quint16 inputGlyphCount = compiledRule.input.size();
  quint16 lookaheadGlyphCount = compiledRule.lookahead.size();
  quint16 substitutionCount = compiledRule.lookupRecords.size();



  quint16 beginoffsets = 2 + 2 * (3 + backtrackGlyphCount + inputGlyphCount + lookaheadGlyphCount) + 2 + 4 * substitutionCount;

  root << quint16(3);
  root << backtrackGlyphCount;

  for (int i = backtrackGlyphCount - 1; i >= 0; i--) {
    auto set = compiledRule.backtrack.at(i);
    auto coveargeVector = set.values();
    std::sort(coveargeVector.begin(), coveargeVector.end());

    quint16 coverageSize = coveargeVector.size();

    coverages << quint16(1) << coverageSize << coveargeVector;

    root << beginoffsets;
    beginoffsets += (2 + 2 + 2 * coverageSize);

  }

  root << inputGlyphCount;

  for (int i = 0; i < inputGlyphCount; i++) {
    auto set = compiledRule.input.at(i);
    auto coveargeVector = set.values();
    std::sort(coveargeVector.begin(), coveargeVector.end());

    quint16 coverageSize = coveargeVector.size();

    coverages << quint16(1) << coverageSize << coveargeVector;

    root << beginoffsets;
    beginoffsets += (2 + 2 + 2 * coverageSize);

  }

  root << lookaheadGlyphCount;

  for (int i = 0; i < lookaheadGlyphCount; i++) {
    auto set = compiledRule.lookahead.at(i);
    auto coveargeVector = set.values();
    std::sort(coveargeVector.begin(), coveargeVector.end());

    quint16 coverageSize = coveargeVector.size();

    coverages << quint16(1) << coverageSize << coveargeVector;

    root << beginoffsets;
    beginoffsets += (2 + 2 + 2 * coverageSize);

  }

  root << substitutionCount;

  for (int i = 0; i < substitutionCount; i++) {
    auto lookeprecord = compiledRule.lookupRecords.at(i);
    root << lookeprecord.position;
    quint16 lookupListIndex;

    QString fullname = m_lookup->name + "." + lookeprecord.lookupName;

    if (m_lookup->isGsubLookup()) {
      if (m_layout->gsublookupsIndexByName.contains(fullname)) {
        lookupListIndex = m_layout->gsublookupsIndexByName[fullname];
      }
      else {
        lookupListIndex = m_layout->gsublookupsIndexByName[lookeprecord.lookupName];
      }
    }
    else {
      if (m_layout->gposlookupsIndexByName.contains(fullname)) {
        lookupListIndex = m_layout->gposlookupsIndexByName[fullname];
      }
      else {
        lookupListIndex = m_layout->gposlookupsIndexByName[lookeprecord.lookupName];
      }
    }

    root << lookupListIndex;

  }

  root.append(coverages);

  return root;
}
