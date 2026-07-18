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

#include <QDebug>
#include "GlazeJson.h"
#include <array>
#include <algorithm>
#include <QDataStream>
#include <hb-ot-layout-common.hh>
#include <iostream>
#include <unordered_map>

#include "GlyphVis.h"
#include "Lookup.h"
#include "OtLayout.h"
#include "to_opentype.h"
#include "digitalkhatt/core/ByteBuffer.h"
#include "font.hpp"

using namespace std;

QDataStream& operator<<(QDataStream& s, const QVector<quint16>& v) {
  for (QVector<quint16>::const_iterator it = v.begin(); it != v.end(); ++it)
    s << *it;
  return s;
}

QDataStream& operator<<(QDataStream& s, const QVector<quint32>& v) {
  for (QVector<quint32>::const_iterator it = v.begin(); it != v.end(); ++it)
    s << *it;
  return s;
}

Subtable::Subtable(Lookup* lookup) {
  m_lookup = lookup;
  m_layout = lookup->layout;
  metafont = m_layout->font;
}

std::uint16_t Subtable::getCodeFromName(std::string name) {
  const QString qname = QString::fromStdString(name);
  // return m_layout->glyphCodePerName[name];
  std::uint16_t uniode;
  if (m_layout->glyphCodePerName.contains(name)) {
    uniode = m_layout->glyphCodePerName[name];
  } else {
    bool ok;
    uniode = qname.toUInt(&ok, 16);
    if (!ok) {
      printf(QString("glyph name <%1> not found\n").arg(qname).toLatin1());
      uniode = 0;
    }
  }

  return uniode;
}
std::string Subtable::getNameFromCode(std::uint16_t code) {
  return m_layout->glyphNamePerCode[code];
}

SingleSubtable::SingleSubtable(Lookup* lookup, quint16 format) : Subtable(lookup), format{format} {
}

bool SingleSubtable::isExtended() {
  if (format == 10 || format == 11) {
    return true;
  }
  return false;
}

void SingleSubtable::readJson(const ParameterJsonObject& json) {
  subst.clear();
  for (const auto& [glyphName, valueJson] : json) {
    quint16 unicode = getCodeFromName(glyphName);
    if (!unicode) continue;
    const auto name = jsonValueAs<std::string>(valueJson);
    if (!name) continue;
    quint16 value = getCodeFromName(*name);
    if (!value) continue;
    subst[unicode] = value;
  }
}
SingleSubtableWithTatweel::SingleSubtableWithTatweel(Lookup* lookup) : SingleSubtable(lookup, 11) {};

void SingleSubtableWithTatweel::generateSubstEquivGlyphs() {
  for (const auto& [glyphCode, expan] : expansion) {

    if (expan.MinLeftTatweel != 0 || expan.MinRightTatweel != 0) {
      if (expan.MinLeftTatweel > 0 && expan.MinRightTatweel > 0) {
        throw new std::runtime_error("SHOULD NOT");
      }

      GlyphParameters parameters;

      parameters.lefttatweel = expan.MinLeftTatweel;
      parameters.righttatweel = expan.MinRightTatweel;

      auto substGlyph = (uint16_t)subst[glyphCode];
      auto substEquivGlyphs = m_layout->getSubstEquivGlyphs(substGlyph);
      while (true) {
        auto oldSize = substEquivGlyphs.size();

        for (auto& glyph : substEquivGlyphs) {
          if ((expan.MinLeftTatweel > 0 && glyph.second->charrt > 0) || (expan.MinRightTatweel > 0 && glyph.second->charlt > 3)) continue;
          if ((glyph.second->charlt > 5 || glyph.second->charrt > 5)) continue;
          // if ((glyph.second->charlt < -0.1 || glyph.second->charrt < -0.1)) continue;

          // if ((expan.MinLeftTatweel > 0 && glyph.second->charlt > 5) || (expan.MinRightTatweel > 0 && glyph.second->charrt > 5)) continue;

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
digitalkhatt::ByteBuffer SingleSubtableWithTatweel::getConvertedOpenTypeTable() {
  std::map<quint16, GlyphExpansion> newexpansion;
  std::map<quint16, quint16> newsubst;

  for (const auto& [glyphCode, substGlyph] : subst) {
    GlyphExpansion expan = expansion.at(glyphCode);

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
          cout << "meem.fina.basmala=" << tt.width << ";" << name << "=" << tt2.width << std::endl;
        }*/

        newexpansion.emplace(glyphCode, expan);
        newsubst.emplace(glyphCode, found->second->charcode);
      }

      auto& addedGlyphs = m_layout->getSubstEquivGlyphs(glyphCode);

      for (auto& addedGlyph : addedGlyphs) {
        GlyphParameters parameters;

        parameters.lefttatweel = addedGlyph.second->charlt + (double)expan.MinLeftTatweel;
        parameters.righttatweel = addedGlyph.second->charrt + (double)expan.MinRightTatweel;

        auto found = addedSubstGlyphs.find(parameters);

        if (found != addedSubstGlyphs.end()) {
          newexpansion.emplace(addedGlyph.second->charcode, expan);
          newsubst.emplace(addedGlyph.second->charcode, found->second->charcode);
        }
      }
    } else {
      newexpansion.emplace(glyphCode, expan);
      newsubst.emplace(glyphCode, substGlyph);
    }
  }

  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;

  quint16 glyphCount = newexpansion.size();
  quint16 coverage_offset = 2 + 2 + 2 + 2 * glyphCount;

  root << (quint16)2;
  root << coverage_offset;
  root << glyphCount;

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (const auto& [glyphCode, substGlyph] : newsubst) {
    root << substGlyph;
    coverage << glyphCode;
  }

  root.append(coverage);

  return root;
}
digitalkhatt::ByteBuffer SingleSubtableWithTatweel::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer substituteGlyphIDs;

  quint16 glyphCount = expansion.size();
  quint16 coverage_offset = 2 + 2 + 2 + 10 * glyphCount;

  root << (quint16)format;
  root << coverage_offset;
  root << glyphCount;

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (const auto& [glyphCode, expan] : expansion) {
    root << (uint16_t)subst[glyphCode];

    if (!m_layout->useNormAxisValues) {
      OT::F16DOT16 lefttatweel;
      lefttatweel.set_float(expan.MinLeftTatweel);

      OT::F16DOT16 righttatweel;
      righttatweel.set_float(expan.MinRightTatweel);

      root << (int32_t)lefttatweel.to_int() << (int32_t)righttatweel.to_int();
    } else {
      OT::F16DOT16 value;

      ValueLimits limits;

      auto& name = m_layout->glyphNamePerCode[subst[glyphCode]];

      const auto& find = m_layout->expandableGlyphs.find(name);

      if (find != m_layout->expandableGlyphs.end()) {
        limits = find->second;
      }

      if ((expan.MinLeftTatweel < 0 && expan.MinLeftTatweel < limits.minLeft) || (expan.MinLeftTatweel > 0 && expan.MinLeftTatweel > limits.maxLeft)) {
        std::cout << "MinLeftTatweel error for glyph " << name << std::endl;
        // throw new runtime_error("MinLeftTatweel error for glyph " + name);
      } else if (expan.MinLeftTatweel < 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(-expan.MinLeftTatweel / m_layout->toOpenType->axisLimits.minLeft);
        } else {
          value.set_float(-expan.MinLeftTatweel / limits.minLeft);
        }

        root << (int32_t)value.to_int();
      } else if (expan.MinLeftTatweel > 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinLeftTatweel / m_layout->toOpenType->axisLimits.maxLeft);
        } else {
          value.set_float(expan.MinLeftTatweel / limits.maxLeft);
        }

        root << (int32_t)value.to_int();
      } else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
      }

      if ((expan.MinRightTatweel < 0 && expan.MinRightTatweel < limits.minRight) || (expan.MinRightTatweel > 0 && expan.MinRightTatweel > limits.maxRight)) {
        std::cout << "MinRightTatweel error for glyph " << name << std::endl;
        // throw new runtime_error("MinRightTatweel error for glyph " + name);
      } else if (expan.MinRightTatweel < 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(-expan.MinRightTatweel / m_layout->toOpenType->axisLimits.minRight);
        } else {
          value.set_float(-expan.MinRightTatweel / limits.minRight);
        }
        root << (int32_t)value.to_int();
        ;
      } else if (expan.MinRightTatweel > 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinRightTatweel / m_layout->toOpenType->axisLimits.maxRight);
        } else {
          value.set_float(expan.MinRightTatweel / limits.maxRight);
        }
        root << (int32_t)value.to_int();
        ;
      } else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      }
    }

    coverage << glyphCode;
  }

  root.append(coverage);

  return root;
};

digitalkhatt::ByteBuffer FSMSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer coverage;

  digitalkhatt::ByteBuffer classDef;

  digitalkhatt::ByteBuffer root;

  digitalkhatt::ByteBuffer header;

  digitalkhatt::ByteBuffer chainNodes;
  std::vector<uint32_t> chainNodesSizes;

  int numNodes = dfa.states.size();

  if (numNodes == 0) {
    root << (quint16)0;
    return root;
  }

  quint16 glyphCount = dfa.glyphToClass.size();

  classDef << (quint16)2;  // Format identifier — format = 2
  classDef << glyphCount;  // Number of ClassRangeRecords

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (auto it = dfa.glyphToClass.cbegin(); it != dfa.glyphToClass.cend(); it++) {
    classDef << (quint16)it->first;
    classDef << (quint16)it->first;
    classDef << (quint16)(it->second + 1);  // Class 0 for not used glyphs

    coverage << (quint16)it->first;
  }

  if (dfa.backupStates.size() != (dfa.maxBackup - dfa.minBackup + 1)) {
    throw new std::runtime_error("invalid backupstates number");
  }

  quint16 startOffsets = 2 + 2 + 2 + 1 + 1 + 1 + 1 + dfa.backupStates.size() * 2 + 2 + numNodes * 4;

  quint16 coverageOffset = startOffsets;
  quint16 classDefOffset = coverageOffset + coverage.size();
  quint32 nextOffset = classDefOffset + classDef.size();

  header << (quint16)1;
  header << (quint16)coverageOffset;  //  Offset to Coverage;
  header << (quint16)classDefOffset;  //  Offset to ClassDef;
  header << (uint8_t)dfa.maxBackup;
  header << (uint8_t)dfa.minBackup;
  header << (uint8_t)dfa.maxLoop;
  header << (uint8_t)0;  // reserved
  for (auto it = dfa.backupStates.cbegin(); it != dfa.backupStates.cend(); it++) {
    header << (quint16)*it;
  }
  header << (quint16)numNodes;  // Number of ChainNodes

  auto addBackLink = [&m_layout = m_layout, &m_lookup = m_lookup](digitalkhatt::ByteBuffer& array, const DFABackTrackInfo& backTrackInfo) {
    array << (uint16_t)backTrackInfo.prevTransIndex;
    array << (uint16_t)backTrackInfo.actions.size();

    uint16_t lookupListIndex = 0;

    for (auto action : backTrackInfo.actions) {
      if (action.type == DFAActionType::LOOKUP) {
        const auto& fullname = action.name;
        if (m_lookup->isGsubLookup()) {
          if (m_layout->gsublookupsIndexByName.contains(fullname)) {
            lookupListIndex = m_layout->gsublookupsIndexByName[fullname];
          } else {
            throw new std::runtime_error("Invalid lookup");
          }
        } else {
          if (m_layout->gposlookupsIndexByName.contains(fullname)) {
            lookupListIndex = m_layout->gposlookupsIndexByName[fullname];
          } else {
            throw new std::runtime_error("Invalid lookup");
          }
        }
      } else if (action.type == DFAActionType::STARTNEWMATCH) {
        lookupListIndex = 0xFFFF;
      } else {
        throw new std::runtime_error("Not yet implemented");
      }

      array << (uint8_t)action.idRule;  // chainid : Action chain number of this action
      array << (uint8_t)0;              // distance : How far back to process
      array << lookupListIndex;         // lookup :	Lookup id to execute
    }
  };

  for (auto it = dfa.states.cbegin(); it != dfa.states.cend(); it++) {
    auto& state = *it;

    int numTransitions = it->transtitions.size();

    uint16_t nextChainNodeOffset = 1   /*actionid*/
                                   + 2 /* OffsetTo<BackLink> */
                                   + 2 /* numTransitions */
                                   + numTransitions * 6 /* ClassNode size*/;

    digitalkhatt::ByteBuffer chainNode;

    digitalkhatt::ByteBuffer offsets;

    chainNode << (uint8_t)state.final;  // actionid : Action identifier for a final node
    if (state.final != 0) {
      digitalkhatt::ByteBuffer backlinkArray;
      addBackLink(backlinkArray, state.backtrackfinal);
      chainNode << (uint16_t)nextChainNodeOffset;  // OffsetTo<BackLink> backLink
      nextChainNodeOffset += backlinkArray.size();
      offsets.append(backlinkArray);
    } else {
      chainNode << (uint16_t)0;  // OffsetTo<BackLink> backLink
    }
    chainNode << (uint16_t)numTransitions;  // numTransitions	: Number of transitions

    digitalkhatt::ByteBuffer classNodes;

    for (auto itTransi = it->transtitions.cbegin(); itTransi != it->transtitions.cend(); itTransi++) {
      numTransitions++;

      classNodes << (uint16_t)(itTransi->first + 1);    // classIndex	: class index value to match
      classNodes << (quint16)(itTransi->second.state);  // chainNode	: chainNode index to use on match
      // OffsetTo<BackLinkArray> backLinks
      auto& backtracks = itTransi->second.backtracks;
      if (backtracks.size() > 0) {
        uint16_t numbacklinks = backtracks.size();
        digitalkhatt::ByteBuffer backLinksArray;
        digitalkhatt::ByteBuffer offsetbackLinksArray;
        backLinksArray << numbacklinks;

        uint16_t nextbackLinksOffset = 2 + 2 * numbacklinks;

        for (auto& backtrack : itTransi->second.backtracks) {
          digitalkhatt::ByteBuffer backlinkArray;
          addBackLink(backlinkArray, backtrack);
          backLinksArray << nextbackLinksOffset;
          nextbackLinksOffset += backlinkArray.size();
          offsetbackLinksArray.append(backlinkArray);
        }

        backLinksArray.append(offsetbackLinksArray);

        classNodes << (uint16_t)nextChainNodeOffset;  // OffsetTo<BackLink> backLink
        nextChainNodeOffset += backLinksArray.size();
        offsets.append(backLinksArray);

      } else {
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

digitalkhatt::ByteBuffer SingleSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;

  std::map<quint16, quint16> newSubst;

  if (!extended) {
    for (auto i = subst.cbegin(), end = subst.cend(); i != end; ++i) {
      auto before = i->first;
      auto after = i->second;
      newSubst.emplace(before, after);
      auto& beforeGlyphs = m_layout->getSubstEquivGlyphs(before);
      auto& afterGlyphs = m_layout->getSubstEquivGlyphs(after);

      for (auto& addedGlyph : beforeGlyphs) {
        auto ret = afterGlyphs.find(addedGlyph.first);
        if (ret != afterGlyphs.end()) {
          newSubst.emplace(addedGlyph.second->charcode, ret->second->charcode);
        }
      }
    }
  } else {
    newSubst = subst;
  }

  quint16 glyphCount = newSubst.size();
  quint16 coverage_offset = 2 + 2 + 2 + 2 * glyphCount;

  root << (quint16)format;
  root << coverage_offset;
  root << glyphCount;

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (const auto& [glyphCode, substGlyph] : newSubst) {

    root << substGlyph;
    coverage << glyphCode;
  }

  root.append(coverage);

  return root;
};

SingleSubtableWithExpansion::SingleSubtableWithExpansion(Lookup* lookup) : SingleSubtable(lookup, 10) {};
digitalkhatt::ByteBuffer SingleSubtableWithExpansion::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer substituteGlyphIDs;

  quint16 glyphCount = expansion.size();
  quint16 coverage_offset = 2 + 2 + 2 + 24 * glyphCount;

  root << (quint16)format;
  root << coverage_offset;
  root << glyphCount;

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (const auto& [glyphCode, expan] : expansion) {
    root << (uint16_t)subst[glyphCode];

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
    } else {
      OT::F16DOT16 value;
      ValueLimits limits;

      auto& name = m_layout->glyphNamePerCode.at(subst[glyphCode]);

      const auto& find = m_layout->expandableGlyphs.find(name);

      if (find != m_layout->expandableGlyphs.end()) {
        limits = find->second;
      }

      if ((expan.MinLeftTatweel < 0 && expan.MinLeftTatweel < limits.minLeft) || (expan.MinLeftTatweel > 0 && expan.MinLeftTatweel > limits.maxLeft)) {
        // throw new runtime_error("MinLeftTatweel error for glyph " + name);
        value.set_float(0.0);
        root << (int32_t)value.to_int();
      } else if (expan.MinLeftTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinLeftTatweel / m_layout->toOpenType->axisLimits.maxLeft);
        } else {
          value.set_float(expan.MinLeftTatweel / limits.maxLeft);
        }
        root << (int32_t)value.to_int();
        ;
      } else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      }

      if ((expan.MaxLeftTatweel < 0 && expan.MaxLeftTatweel < limits.minLeft) || (expan.MaxLeftTatweel > 0 && expan.MaxLeftTatweel > limits.maxLeft)) {
        // throw new runtime_error("MinLeftTatweel error for glyph " + name);
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      } else if (expan.MaxLeftTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MaxLeftTatweel / m_layout->toOpenType->axisLimits.maxLeft);
        } else {
          value.set_float(expan.MaxLeftTatweel / limits.maxLeft);
        }
        root << (int32_t)value.to_int();
        ;
      } else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      }

      if ((expan.MinRightTatweel < 0 && expan.MinRightTatweel < limits.minRight) || (expan.MinRightTatweel > 0 && expan.MinRightTatweel > limits.maxRight)) {
        throw new runtime_error("MinLeftTatweel error for glyph " + name);
      } else if (expan.MinRightTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MinRightTatweel / m_layout->toOpenType->axisLimits.maxRight);
        } else {
          value.set_float(expan.MinRightTatweel / limits.maxRight);
        }
        root << (int32_t)value.to_int();
        ;
      } else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      }

      if ((expan.MaxRightTatweel < 0 && expan.MaxRightTatweel < limits.minRight) || (expan.MaxRightTatweel > 0 && expan.MaxRightTatweel > limits.maxRight)) {
        // throw new runtime_error("MinLeftTatweel error for glyph " + name);
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      } else if (expan.MaxRightTatweel != 0.0) {
        if (m_layout->toOpenType->isUniformAxis()) {
          value.set_float(expan.MaxRightTatweel / m_layout->toOpenType->axisLimits.maxRight);
        } else {
          value.set_float(expan.MaxRightTatweel / limits.maxRight);
        }
        root << (int32_t)value.to_int();
        ;
      } else {
        value.set_float(0.0);
        root << (int32_t)value.to_int();
        ;
      }
    }

    root << (uint16_t)expan.weight;
    uint32_t flags = 0;

    flags = flags | (uint32_t)(expan.startEndLig);
    flags = flags | (expan.stretchIsAbsolute << 3);
    flags = flags | (expan.shrinkIsAbsolute << 4);

    root << flags;

    coverage << glyphCode;
  }

  root.append(coverage);

  return root;
};

SingleAdjustmentSubtable::SingleAdjustmentSubtable(Lookup* lookup, quint16 pformat) : Subtable(lookup), format{pformat} {}

bool SingleAdjustmentSubtable::isExtended() {
  if (format == 3) {
    return true;
  } else {
    return false;
  }
}

void SingleAdjustmentSubtable::readJson(const ParameterJsonObject& json) {
  singlePos.clear();
  for (const auto& [className, recordJson] : json) {
    auto unicodes = m_layout->classtoUnicode(className);
    const auto record = jsonValueAs<std::array<std::int16_t, 4>>(recordJson);
    if (!record) continue;
    for (auto unicode : unicodes) {
      ValueRecord valueRecord{(*record)[0], (*record)[1], (*record)[2],
                              (*record)[3]};

      singlePos[unicode] = valueRecord;
    }
  }
}
digitalkhatt::ByteBuffer SingleAdjustmentSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer valueRecords;
  std::map<int, std::pair<int, std::pair<int, int>>> posToVar;

  quint16 glyphCount = singlePos.size();
  quint16 valueFormat;

  auto isOTVar = m_layout->isOTVar;

  if (isOTVar) {
    valueFormat = 0x77;
  } else {
    valueFormat = 0x7;
  }

  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  for (const auto& [glyphCode, initialRecord] : singlePos) {
    ValueRecord record = initialRecord;
    auto originalCode = glyphCode;

    if (!extended) {
      auto glyph = m_layout->getGlyph(originalCode);
      if (glyph->name.find(".added_") != std::string::npos) {
        originalCode = m_layout->glyphCodePerName[glyph->originalglyph];
      }
    }

    if (parameters.contains(originalCode)) {
      ValueRecord par = parameters.at(originalCode);
      record = {(qint16)(record.xPlacement + par.xPlacement), (qint16)(record.yPlacement + par.yPlacement), (qint16)(record.xAdvance + par.xAdvance), record.yAdvance};
    }

    valueRecords << record.xPlacement << record.yPlacement << record.xAdvance;  // << record.yAdvance;

    if (isOTVar) {
      auto glyphName = m_layout->glyphNamePerCode[originalCode];

      auto regionIndexes = m_layout->toOpenType->getGlyphParameters(glyphName);
      int regionIndexesArrayIndex = regionIndexes.second;
      auto glyphParamertersArray = regionIndexes.first;

      if (glyphParamertersArray.size() != 0) {
        DefaultDelta delatX;
        DefaultDelta delatY;
        DefaultDelta delatXadvance;
        // DefaultDelta delatYadvance;

        for (auto& parameters : glyphParamertersArray) {
          if (parameters.scalex != 0) {
            delatX.push_back(record.xPlacement * (parameters.scalex / 100) - record.xPlacement);
            delatY.push_back(0);
            delatXadvance.push_back(record.xAdvance * (parameters.scalex / 100) - record.xAdvance);
            // delatYadvance.push_back(0);
          }
        }

        bool allx0 = std::all_of(delatX.begin(), delatX.end(), [](int i) { return i == 0; });

        if (allx0 != 0) {
          auto indexesX = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
          posToVar.insert({valueRecords.size(), {-8, indexesX}});
        }
        valueRecords << (quint16)0;

        bool ally0 = std::all_of(delatY.begin(), delatY.end(), [](int i) { return i == 0; });

        if (ally0 != 0) {
          auto indexesY = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);
          posToVar.insert({valueRecords.size(), {-8, indexesY}});
        }
        valueRecords << (quint16)0;

        bool allxadvance0 = std::all_of(delatXadvance.begin(), delatXadvance.end(), [](int i) { return i == 0; });

        if (allxadvance0 != 0) {
          auto indexesXadvance = m_layout->getDeltaSetEntry(delatXadvance, regionIndexesArrayIndex);
          posToVar.insert({valueRecords.size(), {-8, indexesXadvance}});
        }
        valueRecords << (quint16)0;

        /*
        bool allyadvance0 = std::all_of(delatYadvance.begin(), delatYadvance.end(), [](int i) { return i==0; });

        if (allyadvance0 != 0) {
          auto indexesYadvance = m_layout->getDeltaSetEntry(delatYadvance, regionIndexesArrayIndex);
          posToVar.insert({valueRecords.size(),{-8,indexesYadvance}});

        }
        valueRecords << (quint16)0;     */

      } else {
        valueRecords << (quint16)0;
        valueRecords << (quint16)0;
        valueRecords << (quint16)0;
        // valueRecords << (quint16)0;
      }
    }

    coverage << glyphCode;
  }

  setVariationIndexOffset(valueRecords, 0, posToVar);

  /*
for(auto& varIndex : posToVar){
  auto pos = varIndex.first;
  auto isX = varIndex.second.first;
  auto index = varIndex.second.second;

  digitalkhatt::ByteBuffer offsetData;
  offsetData << (quint16)(8+valueRecords.size());
  valueRecords.replace(pos,offsetData.size(),offsetData);

  valueRecords << (quint16)index.first;
  valueRecords << (quint16)index.second;
  valueRecords << (quint16)0x8000;
}*/

  quint32 coverageOffset = 8 + valueRecords.size();

  if (coverageOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name << " Subtable " << name << " Overflows : " << coverageOffset << std::endl;
  }

  root << format;
  root << (quint16)coverageOffset;
  root << valueFormat;
  root << glyphCount;
  root.append(valueRecords);
  root.append(coverage);

  return root;
}
void SingleAdjustmentSubtable::readParameters(const ParameterJsonObject& json) {
  const auto found = json.find("parameters");
  if (found == json.end()) return;
  std::map<std::string, std::array<std::int16_t, 4>> values;
  if (glz::read_json(values, found->second)) return;
  for (const auto& [glyphName, value] : values)
    parameters[m_layout->glyphCodePerName[glyphName]] = {
        value[0], value[1], value[2], value[3]};
}
void SingleAdjustmentSubtable::saveParameters(ParameterJsonObject& json) const {
  if (parameters.size() != 0) {
    std::map<std::string, std::array<std::int16_t, 4>> parametersObject;
    for (const auto& [glyphCode, parameter] : parameters) {
      if (!parameter.isEmpty()) {
        parametersObject[m_layout->glyphNamePerCode[glyphCode]] = {
            parameter.xPlacement, parameter.yPlacement, parameter.xAdvance,
            parameter.yAdvance};
      }
    }

    if (!parametersObject.empty()) json["parameters"] = parametersObject;
  }
}

PairAdjustmentSubtable::PairAdjustmentSubtable(Lookup* lookup, quint16 pformat) : Subtable(lookup), format{pformat} {}

void PairAdjustmentSubtable::getPairValue(hb_cursive_anchor_context_t* context) {
  auto pairValue = pairPos.at(context->glyph_id).at(context->base_glyph_id);

  auto isValueRecord1 = std::holds_alternative<ValueRecord>(pairValue.valueRecord1);
  auto isValueRecord2 = std::holds_alternative<ValueRecord>(pairValue.valueRecord2);

  if (isValueRecord1 && isValueRecord2) return;

  float lefttatweel = m_layout->normalToParameter(context->glyph_id, context->lefttatweel, true);
  float righttatweel = m_layout->normalToParameter(context->glyph_id, context->righttatweel, false);
  float lefttatweel2 = m_layout->normalToParameter(context->base_glyph_id, context->lefttatweel2, true);
  float righttatweel2 = m_layout->normalToParameter(context->base_glyph_id, context->righttatweel2, false);

  auto firstParams = GlyphParameters{.lefttatweel = lefttatweel, .righttatweel = righttatweel};
  auto secondParams = GlyphParameters{.lefttatweel = lefttatweel2, .righttatweel = righttatweel2};

  auto glypVis1 = m_layout->getAlternate(context->glyph_id, firstParams);
  auto glypVis2 = m_layout->getAlternate(context->base_glyph_id, secondParams);

  uint8_t* byte_ptr = (uint8_t*)context->data;

  ValueRecord valueRecord1;
  ValueRecord valueRecord2;
  if (isValueRecord1) {
    valueRecord1 = std::get<ValueRecord>(pairValue.valueRecord1);
  } else {
    valueRecord1 = std::get<PairAdjustFunc>(pairValue.valueRecord1)(glypVis1, glypVis2);
  }
  if (isValueRecord2) {
    valueRecord2 = std::get<ValueRecord>(pairValue.valueRecord2);
  } else {
    auto& func = std::get<PairAdjustFunc>(pairValue.valueRecord2);
    if (func) {
      valueRecord2 = func(glypVis1, glypVis2);
    }
  }

  if (valueFormat1 & 0x01) {
    *byte_ptr++ = (uint8_t)(valueRecord1.xPlacement >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord1.xPlacement & 0xFF);
  }
  if (valueFormat1 & 0x02) {
    *byte_ptr++ = (uint8_t)(valueRecord1.yPlacement >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord1.yPlacement & 0xFF);
  }
  if (valueFormat1 & 0x04) {
    *byte_ptr++ = (uint8_t)(valueRecord1.xAdvance >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord1.xAdvance & 0xFF);
  }
  if (valueFormat1 & 0x08) {
    *byte_ptr++ = (uint8_t)(valueRecord1.yAdvance >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord1.yAdvance & 0xFF);
  }
  if (valueFormat2 & 0x01) {
    *byte_ptr++ = (uint8_t)(valueRecord2.xPlacement >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord2.xPlacement & 0xFF);
  }
  if (valueFormat2 & 0x02) {
    *byte_ptr++ = (uint8_t)(valueRecord2.yPlacement >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord2.yPlacement & 0xFF);
  }
  if (valueFormat2 & 0x04) {
    *byte_ptr++ = (uint8_t)(valueRecord2.xAdvance >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord2.xAdvance & 0xFF);
  }
  if (valueFormat2 & 0x08) {
    *byte_ptr++ = (uint8_t)(valueRecord2.yAdvance >> 8);
    *byte_ptr++ = (uint8_t)(valueRecord2.yAdvance & 0xFF);
  }
}

digitalkhatt::ByteBuffer PairAdjustmentSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer pairSetOffsets;
  digitalkhatt::ByteBuffer pairSetTables;
  std::map<int, std::pair<int, std::pair<int, int>>> posToVar;

  quint16 glyphCount = pairPos.size();
  valueFormat1 = 0;
  valueFormat2 = 0;
  quint16 pairSetCount = glyphCount;
  u_int32_t headerSize = 10;

  u_int32_t currentPairSetOffset = headerSize + pairSetCount * 2;
  coverage << (quint16)1;
  coverage << (quint16)glyphCount;

  std::map<quint16, std::map<quint16, PairValueFinal>> pairPosFinal;

  for (auto i = pairPos.cbegin(), end = pairPos.cend(); i != end; ++i) {
    const auto& pairValues = i->second;
    auto& pairValuesFinal = pairPosFinal[i->first];
    auto glypVis1 = m_layout->getGlyph(i->first);
    for (auto j = pairValues.cbegin(), end = pairValues.cend(); j != end; ++j) {
      auto glypVis2 = m_layout->getGlyph(j->first);
      const auto& pairValue = j->second;
      ValueRecord valueRecord1;
      ValueRecord valueRecord2;
      if (std::holds_alternative<ValueRecord>(pairValue.valueRecord1)) {
        valueRecord1 = std::get<ValueRecord>(pairValue.valueRecord1);
      } else {
        valueRecord1 = std::get<PairAdjustFunc>(pairValue.valueRecord1)(glypVis1, glypVis2);
      }
      if (std::holds_alternative<ValueRecord>(pairValue.valueRecord2)) {
        valueRecord2 = std::get<ValueRecord>(pairValue.valueRecord2);
      } else {
        auto& func = std::get<PairAdjustFunc>(pairValue.valueRecord2);
        if (func) {
          valueRecord2 = func(glypVis1, glypVis2);
        }
      }

      valueFormat1 |= valueRecord1.format();
      valueFormat2 |= valueRecord2.format();
      pairValuesFinal.emplace(j->first, PairValueFinal{valueRecord1, valueRecord2});
    }
  }

  for (auto i = pairPosFinal.cbegin(), end = pairPosFinal.cend(); i != end; ++i) {
    const auto& pairValues = i->second;

    coverage << (quint16)i->first;

    digitalkhatt::ByteBuffer currentPairSetTable;

    currentPairSetTable << (u_int16_t)pairValues.size();
    for (auto j = pairValues.cbegin(), end = pairValues.cend(); j != end; ++j) {
      currentPairSetTable << (u_int16_t)j->first;
      auto& pairValue = j->second;
      auto& valueRecord1 = pairValue.valueRecord1;
      auto& valueRecord2 = pairValue.valueRecord2;
      if (valueFormat1 & 0x01) {
        currentPairSetTable << valueRecord1.xPlacement;
      }
      if (valueFormat1 & 0x02) {
        currentPairSetTable << valueRecord1.yPlacement;
      }
      if (valueFormat1 & 0x04) {
        currentPairSetTable << valueRecord1.xAdvance;
      }
      if (valueFormat1 & 0x08) {
        currentPairSetTable << valueRecord1.yAdvance;
      }
      if (valueFormat2 & 0x01) {
        currentPairSetTable << valueRecord2.xPlacement;
      }
      if (valueFormat2 & 0x02) {
        currentPairSetTable << valueRecord2.yPlacement;
      }
      if (valueFormat2 & 0x04) {
        currentPairSetTable << valueRecord2.xAdvance;
      }
      if (valueFormat2 & 0x08) {
        currentPairSetTable << valueRecord2.yAdvance;
      }
    }

    pairSetOffsets << (u_int16_t)currentPairSetOffset;
    pairSetTables.append(currentPairSetTable);
    currentPairSetOffset += currentPairSetTable.size();
  }
  quint32 coverageOffset = headerSize + pairSetOffsets.size() + pairSetTables.size();

  if (coverageOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name << " Subtable " << name << " Overflows : " << coverageOffset << std::endl;
  }

  root << format;
  root << (u_int16_t)coverageOffset;
  root << (u_int16_t)valueFormat1;
  root << (u_int16_t)valueFormat2;
  root << (u_int16_t)pairSetCount;
  root.append(pairSetOffsets);
  root.append(pairSetTables);
  root.append(coverage);

  openTypeSubTable = root;

  isDirty = false;

  return openTypeSubTable;
}

MultipleSubtable::MultipleSubtable(Lookup* lookup) : Subtable(lookup) {
}

digitalkhatt::ByteBuffer MultipleSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer sequencetables;

  quint16 total = subst.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root << (quint16)1;
  root << coverage_offset;
  root << total;

  coverage << (quint16)1;
  coverage << (quint16)total;

  for (const auto& [glyphCode, seqtable] : subst) {

    root << debutsequence;
    coverage << glyphCode;
    sequencetables << (quint16)seqtable.size();
    for (quint16 glyph : seqtable) sequencetables << glyph;

    debutsequence += 2 + 2 * seqtable.size();
  }

  root.append(coverage);
  root.append(sequencetables);

  return root;
};

void MultipleSubtable::readJson(const ParameterJsonObject& json) {
  subst.clear();
  for (const auto& [glyphName, destinationJson] : json) {
    uint uniode = getCodeFromName(glyphName);
    if (!uniode) continue;
    const auto destination = jsonValueAs<std::vector<std::string>>(destinationJson);
    if (!destination) continue;
    for (const auto& name : *destination) {
      uint value = getCodeFromName(name);

      if (!value) continue;

      subst[uniode].push_back(value);
    }
  }
}

AlternateSubtable::AlternateSubtable(Lookup* lookup, quint16 format) : Subtable(lookup), format{format} {}

void AlternateSubtable::generateSubstEquivGlyphs() {
  for (const auto& [glyphCode, seqtable] : alternates) {
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

digitalkhatt::ByteBuffer AlternateSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer sequencetables;

  quint16 total = alternates.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root << (quint16)1;
  root << coverage_offset;
  root << total;

  coverage << (quint16)1;
  coverage << (quint16)total;

  for (const auto& [glyphCode, seqtable] : alternates) {
    root << debutsequence;
    coverage << glyphCode;
    sequencetables << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        auto newGlyph = m_layout->getAlternate(alternateGlyph.code, parameters, true, false);

        sequencetables << (quint16)newGlyph->charcode;
      } else {
        sequencetables << (quint16)alternateGlyph.code;
      }
    }
    // sequencetables << seqtable;

    debutsequence += 2 + 2 * seqtable.size();
  }

  root.append(coverage);
  root.append(sequencetables);

  return root;
};

AlternateSubtableWithTatweel::AlternateSubtableWithTatweel(Lookup* lookup) : AlternateSubtable(lookup, 10) {};

void AlternateSubtableWithTatweel::generateSubstEquivGlyphs() {
  for (const auto& [glyphCode, seqtable] : alternates) {
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

digitalkhatt::ByteBuffer AlternateSubtableWithTatweel::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer sequencetables;

  quint16 total = alternates.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root << (quint16)format;
  root << coverage_offset;
  root << total;

  coverage << (quint16)1;
  coverage << (quint16)total;

  for (const auto& [glyphCode, seqtable] : alternates) {
    root << debutsequence;
    coverage << glyphCode;

    digitalkhatt::ByteBuffer alternatesArray;
    digitalkhatt::ByteBuffer tatweelsArray;


    alternatesArray << (quint16)seqtable.size();
    tatweelsArray << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      alternatesArray << (quint16)alternateGlyph.code;
      if (!m_layout->useNormAxisValues) {
        OT::F16DOT16 lefttatweel;
        lefttatweel.set_float(alternateGlyph.lefttatweel);

        OT::F16DOT16 righttatweel;
        righttatweel.set_float(alternateGlyph.righttatweel);

        tatweelsArray << (int32_t)lefttatweel.to_int() << (int32_t)righttatweel.to_int();
      } else {
        throw std::runtime_error("Not implemented");
      }
    }

    // sequencetables << alternatesArray;
    // sequencetables << tatweelsArray;

    sequencetables.append(alternatesArray);
    sequencetables.append(tatweelsArray);

    debutsequence += alternatesArray.size() + tatweelsArray.size();
  }

  root.append(coverage);
  root.append(sequencetables);

  return root;
};

digitalkhatt::ByteBuffer AlternateSubtableWithTatweel::getConvertedOpenTypeTable() {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer sequencetables;

  quint16 total = alternates.size();
  uint coverage_size = 2 + 2 + 2 * total;
  quint16 coverage_offset = 2 + 2 + 2 + 2 * total;
  quint16 debutsequence = coverage_offset + coverage_size;

  root << (quint16)1;
  root << coverage_offset;
  root << total;

  coverage << (quint16)1;
  coverage << (quint16)total;

  for (const auto& [glyphCode, seqtable] : alternates) {
    root << debutsequence;
    coverage << glyphCode;
    sequencetables << (quint16)seqtable.size();

    for (auto& alternateGlyph : seqtable) {
      if (alternateGlyph.lefttatweel != 0.0 || alternateGlyph.righttatweel != 0.0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = alternateGlyph.lefttatweel;
        parameters.righttatweel = alternateGlyph.righttatweel;

        auto newGlyph = m_layout->getAlternate(alternateGlyph.code, parameters, true, false);

        sequencetables << (quint16)newGlyph->charcode;
      } else {
        sequencetables << (quint16)alternateGlyph.code;
      }
    }
    // sequencetables << seqtable;

    debutsequence += 2 + 2 * seqtable.size();
  }

  root.append(coverage);
  root.append(sequencetables);

  return root;
};

LigatureSubtable::LigatureSubtable(Lookup* lookup) : Subtable(lookup) {
}

digitalkhatt::ByteBuffer LigatureSubtable::getOpenTypeTable(bool extended) {
  struct Ligaturetable {
    quint16 ligatureGlyph;
    QVector<quint16> componentGlyphIDs;
  };

  QMap<quint16, QVector<Ligaturetable>> LigatureSets;

  for (auto ligature : ligatures) {
    quint16 ligatureGlyph = ligature.ligatureGlyph;
    auto seq = ligature.componentGlyphIDs;
    LigatureSets[seq.at(0)].append({ligatureGlyph, QVector<quint16>(seq.begin() + 1, seq.end())});
  }

  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverage;
  digitalkhatt::ByteBuffer LigatureSetTables;

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

    digitalkhatt::ByteBuffer LigatureSetTable;
    digitalkhatt::ByteBuffer LigatureTables;
    LigatureSetTable << (quint16)ligatureCount;

    for (int i = 0; i < ligatureCount; i++) {
      LigatureSetTable << ligatureOffsets;

      digitalkhatt::ByteBuffer ligatureTable;
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

void LigatureSubtable::readJson(const ParameterJsonObject& json) {
  ligatures.clear();
  for (const auto& [ligatureName, destinationJson] : json) {
    quint16 uniode = getCodeFromName(ligatureName);
    if (!uniode) continue;
    const auto destination = jsonValueAs<std::vector<std::string>>(destinationJson);
    if (!destination) continue;
    QVector<quint16> componentGlyphIDs;
    for (const auto& name : *destination) {
      uint value = getCodeFromName(name);

      if (!value) continue;

      // ligatures[uniode].append(value);
      componentGlyphIDs.append(value);
    }
    ligatures.push_back({static_cast<quint16>(uniode), std::vector<quint16>(componentGlyphIDs.begin(), componentGlyphIDs.end())});
  }
}

MarkBaseSubtable::MarkBaseSubtable(Lookup* lookup) : Subtable(lookup) {}

void MarkBaseSubtable::readJson(const ParameterJsonObject& json) {
  base.clear();
  if (const auto* baseValue = findJsonValue(json, "base")) {
    if (const auto singleBase = jsonValueAs<std::string>(*baseValue)) {
      base = {*singleBase};
    } else if (const auto baseArray =
                   jsonValueAs<std::vector<std::string>>(*baseValue)) {
      base.assign(baseArray->begin(), baseArray->end());
    }
  }

  const auto* classesValue = findJsonValue(json, "classes");
  if (!classesValue || !classesValue->is_object()) return;
  for (const auto& [className, classJson] : classesValue->get_object()) {
    if (!classJson.is_object()) continue;
    const auto& classObject = classJson.get_object();

    MarkClass newclass;

    if (const auto* markValue = findJsonValue(classObject, "mark")) {
      if (const auto singleMark = jsonValueAs<std::string>(*markValue)) {
        newclass.mark = {*singleMark};
      } else if (const auto markArray =
                     jsonValueAs<std::vector<std::string>>(*markValue)) {
        newclass.mark.insert(markArray->begin(), markArray->end());
      }
    }

    newclass.basefunction = m_layout->getanchorCalcFunctions(
        jsonValueAs<std::string>(classObject, "basefunction").value_or(""), this);
    newclass.markfunction = m_layout->getanchorCalcFunctions(
        jsonValueAs<std::string>(classObject, "markfunction").value_or(""), this);

    auto readPoints = [&](std::string_view key, auto& destination) {
      const auto points =
          jsonValueAs<std::map<std::string, std::array<int, 2>>>(classObject, key);
      if (!points) return;
      for (const auto& [glyphName, point] : *points)
        destination[glyphName] = Point{point[0], point[1]};
    };
    readPoints("baseparameters", newclass.baseparameters);
    readPoints("markparameters", newclass.markparameters);
    readPoints("baseanchors", newclass.baseanchors);
    readPoints("markanchors", newclass.markanchors);

    classes[className] = std::move(newclass);
  }
}
optional<Point> CursiveSubtable::getExit(quint16 glyph_id, GlyphParameters parameters) {
  optional<Point> exit;

  auto anchorType = m_lookup->flags & Lookup::Flags::RightToLeft ? GlyphVis::AnchorType::ExitAnchorRTL : GlyphVis::AnchorType::ExitAnchor;

  auto otherType = anchorType == GlyphVis::AnchorType::ExitAnchorRTL ? GlyphVis::AnchorType::ExitAnchor : GlyphVis::AnchorType::ExitAnchorRTL;

  if (anchors.contains(glyph_id)) {
    auto& entryexit = anchors[glyph_id];

    GlyphVis* originalglyph = m_layout->getGlyph(glyph_id);
    GlyphVis* curr = originalglyph;

    curr = originalglyph->getAlternate(parameters);

    if (!entryexit.exitName.empty() && curr->conatinsAnchor(entryexit.exitName, anchorType)) {
      exit = curr->getAnchor(entryexit.exitName, anchorType);
    } else if (curr->conatinsAnchor(this->name, anchorType)) {
      exit = curr->getAnchor(this->name, anchorType);
    } else if (curr->conatinsAnchor(this->name, otherType)) {
      exit = curr->getAnchor(this->name, otherType);
    } else if (entryexit.exitFunction) {
      exit = entryexit.exitFunction(false, originalglyph, curr);
    } else {
      exit = entryexit.exit;
      // exit = calculateEntry(originalglyph, curr, entry);
    }

    if (exit && exitParameters.contains(glyph_id)) {
      exit = *exit + exitParameters[glyph_id];
    }
  }

  return exit;
}

optional<Point> CursiveSubtable::getEntry(quint16 glyph_id, GlyphParameters parameters) {
  optional<Point> entry;

  auto anchorType = m_lookup->flags & Lookup::Flags::RightToLeft ? GlyphVis::AnchorType::EntryAnchorRTL : GlyphVis::AnchorType::EntryAnchor;

  auto otherType = anchorType == GlyphVis::AnchorType::EntryAnchorRTL ? GlyphVis::AnchorType::EntryAnchor : GlyphVis::AnchorType::EntryAnchorRTL;

  if (anchors.contains(glyph_id)) {
    auto& entryexit = anchors[glyph_id];

    GlyphVis* originalglyph = m_layout->getGlyph(glyph_id);
    GlyphVis* curr = originalglyph;

    curr = originalglyph->getAlternate(parameters);

    if (!entryexit.entryName.empty() && curr->conatinsAnchor(entryexit.entryName, anchorType)) {
      entry = curr->getAnchor(entryexit.entryName, anchorType);
    } else if (curr->conatinsAnchor(this->name, anchorType)) {
      entry = curr->getAnchor(this->name, anchorType);
    } else if (curr->conatinsAnchor(this->name, otherType)) {
      entry = curr->getAnchor(this->name, otherType);
    } else if (entryexit.entryFunction) {
      entry = entryexit.entryFunction(true, originalglyph, curr);
    } else {
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
Point CursiveSubtable::calculateEntry(GlyphVis* originalglyph, GlyphVis* extendedglyph, Point entry) {
  /*
  double xshift = extendedglyph->matrix.xpart - originalglyph->matrix.xpart;
  double yshift = extendedglyph->matrix.ypart - originalglyph->matrix.ypart;*/

  double xshift = extendedglyph->width - originalglyph->width;

  double yshift = 0;

  entry += Point(xshift, yshift);

  return entry;
}
void CursiveSubtable::readJson(const ParameterJsonObject& json) {
  const auto* anchorsValue = findJsonValue(json, "anchors");
  if (anchorsValue && anchorsValue->is_object()) {
    for (const auto& [className, entryExitJson] : anchorsValue->get_object()) {
      if (!entryExitJson.is_object()) continue;

      EntryExit value;
      const auto& entryExitObject = entryExitJson.get_object();
      if (const auto point =
              jsonValueAs<std::array<int, 2>>(entryExitObject, "exit"))
        value.exit = Point{(*point)[0], (*point)[1]};
      if (const auto point =
              jsonValueAs<std::array<int, 2>>(entryExitObject, "entry"))
        value.entry = Point{(*point)[0], (*point)[1]};

      if (value.entry || value.exit) {
        auto glyphs = m_layout->classtoUnicode(className);

        for (auto glyph : glyphs) {
          anchors[glyph] = value;
        }
      }
    }
  }
}
void CursiveSubtable::readParameters(const ParameterJsonObject& json) {
  using PointValues = std::map<std::string, std::array<int, 2>>;
  auto readPoints = [&](std::string_view key, auto& destination) {
    const auto found = json.find(std::string{key});
    if (found == json.end()) return;
    PointValues values;
    if (glz::read_json(values, found->second)) return;
    for (const auto& [glyphName, point] : values)
      destination[m_layout->glyphCodePerName[glyphName]] = Point{point[0], point[1]};
  };
  readPoints("exitParameters", exitParameters);
  readPoints("entryParameters", entryParameters);
}
void CursiveSubtable::saveParameters(ParameterJsonObject& json) const {
  if (exitParameters.size() != 0) {
    std::map<std::string, std::array<int, 2>> exitParametersObject;
    for (const auto& [glyphCode, exitParameter] : exitParameters) {
      if (!exitParameter.isNull()) {
        exitParametersObject[m_layout->glyphNamePerCode[glyphCode]] = {
            exitParameter.x(), exitParameter.y()};
      }
    }

    if (!exitParametersObject.empty()) json["exitParameters"] = exitParametersObject;
  }

  if (entryParameters.size() != 0) {
    std::map<std::string, std::array<int, 2>> entryParametersObject;
    for (const auto& [glyphCode, entryParameter] : entryParameters) {
      if (!entryParameter.isNull()) {
        entryParametersObject[m_layout->glyphNamePerCode[glyphCode]] = {
            entryParameter.x(), entryParameter.y()};
      }
    }

    if (!entryParametersObject.empty()) json["entryParameters"] = entryParametersObject;
  }
}

void CursiveSubtable::setAnchorTable(std::uint16_t glyphCode,
                                     digitalkhatt::ByteBuffer& entryExitRecords,
                                     digitalkhatt::ByteBuffer& anchorTables,
                                     std::uint32_t& anchorOffset,
                                     std::map<int, std::pair<int, std::pair<int, int>>>& posToVar,
                                     bool extended,
                                     bool isEntry) {
  const auto& glyphName = m_layout->glyphNamePerCode[glyphCode];

  std::string originalGlyphName = glyphName;
  double charlt = 0.0;
  double charrt = 0.0;

  if (!extended) {
    auto glyph = m_layout->getGlyph(glyphCode);
    if (glyph->name.find(".added_") != std::string::npos) {
      originalGlyphName = glyph->originalglyph;
    }
    charlt = glyph->charlt;
    charrt = glyph->charrt;
  }

  auto& originalGlyph = m_layout->glyphs[originalGlyphName];

  std::optional<Point> calcanchor = isEntry ? getEntry(originalGlyph.charcode, {.lefttatweel = charlt, .righttatweel = charrt}) : getExit(originalGlyph.charcode, {.lefttatweel = charlt, .righttatweel = charrt});

  if (!calcanchor) {
    entryExitRecords << (quint16)0;
    return;
  }

  Point anchor{*(calcanchor)};

  entryExitRecords << (quint16)anchorOffset;

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
          delatX.push_back(anchor.x() * (parameters.scalex / 100) - anchor.x());
          delatY.push_back(0);
        } else {
          auto val = isEntry ? getEntry(glyphCode, parameters) : getExit(glyphCode, parameters);
          delatX.push_back(val->x() - anchor.x());
          delatY.push_back(val->y() - anchor.y());
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
        anchorTables << (quint16)anchor.x();
        anchorTables << (quint16)anchor.y();
        if (!allx0) {
          auto index_x = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
          posToVar.insert({anchorTables.size(), {anchorTables.size() - 6, index_x}});  //(pos -  (isX ? 6 : 8)
        }
        anchorTables << (quint16)0;  // xDeviceOffset
        if (!ally0) {
          auto index_y = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);
          posToVar.insert({anchorTables.size(), {anchorTables.size() - 8, index_y}});
        }
        anchorTables << (quint16)0;  // yDeviceOffset
        anchorOffset += 10;

        done = true;
      }
    }
  }

  if (!done) {
    anchorTables << (quint16)1;
    anchorTables << (quint16)anchor.x();
    anchorTables << (quint16)anchor.y();
    anchorOffset += 6;
  }
}

digitalkhatt::ByteBuffer CursiveSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer anchorTables;
  digitalkhatt::ByteBuffer entryExitRecords;

  quint16 entryExitCount = anchors.size();

  // quint16 coverageOffset = 2 + 2 + 2 + entryExitCount * 4;
  // quint32 anchorOffset = coverageOffset + 2 + 2 + 2 * entryExitCount;

  quint32 anchorOffset = 2 + 2 + 2 + entryExitCount * 4;

  digitalkhatt::ByteBuffer coverage;
  coverage << (quint16)1 << entryExitCount;
  for (const auto& [glyphCode, anchor] : anchors) coverage << glyphCode;

  bool rtl = m_lookup->flags & Lookup::Flags::RightToLeft;

  std::map<int, std::pair<int, std::pair<int, int>>> posToVar;

  for (const auto& [glyphCode, anchor] : anchors) {
    setAnchorTable(glyphCode, entryExitRecords, anchorTables, anchorOffset, posToVar, extended, true);
    setAnchorTable(glyphCode, entryExitRecords, anchorTables, anchorOffset, posToVar, extended, false);
  }

  setVariationIndexOffset(anchorTables, anchorOffset, posToVar);

  quint32 coverageOffset = 6 + entryExitRecords.size() + anchorTables.size();

  if (coverageOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name << " Subtable " << name
              << " Overflows. coverageOffset=" << coverageOffset
              << std::endl;
  }

  digitalkhatt::ByteBuffer root;
  root << (quint16)1;
  root << (quint16)coverageOffset;
  root << entryExitCount;
  root.append(entryExitRecords);
  root.append(anchorTables);
  root.append(coverage);

  return root;
}
void MarkBaseSubtable::saveParameters(ParameterJsonObject& json) const {
  for (auto it = classes.cbegin(); it != classes.cend(); ++it) {
    ParameterJsonObject classObject;
    std::map<std::string, std::array<int, 2>> baseparametersObject;
    auto parameters = it->second.baseparameters;
    for (const auto& [glyphName, point] : parameters) {
      if (!point.isNull()) {
        baseparametersObject[glyphName] = {point.x(), point.y()};
      }
    }
    parameters = it->second.markparameters;
    std::map<std::string, std::array<int, 2>> markparametersObject;
    for (const auto& [glyphName, point] : parameters) {
      if (!point.isNull()) {
        markparametersObject[glyphName] = {point.x(), point.y()};
      }
    }

    if (!baseparametersObject.empty()) {
      classObject["baseparameters"] = baseparametersObject;
    }

    if (!markparametersObject.empty()) {
      classObject["markparameters"] = markparametersObject;
    }

    if (!classObject.empty()) {
      json[it->first] = std::move(classObject);
    }
  }
}
void MarkBaseSubtable::readParameters(const ParameterJsonObject& json) {
  using PointValues = std::map<std::string, std::array<int, 2>>;
  for (const auto& [className, classValue] : json) {
    if (!classValue.is_object()) continue;
    const auto& classObject = classValue.get_object();
    MarkClass& newclass = classes[className];
    auto readPoints = [&](std::string_view key, auto& destination) {
      const auto found = classObject.find(std::string{key});
      if (found == classObject.end()) return;
      PointValues values;
      if (glz::read_json(values, found->second)) return;
      for (const auto& [glyphName, point] : values)
        destination[glyphName] = Point{point[0], point[1]};
    };
    readPoints("baseparameters", newclass.baseparameters);
    readPoints("markparameters", newclass.markparameters);
  }
}
Point MarkBaseSubtable::getBaseAnchor(std::string baseGlyphName, std::string className, GlyphParameters parameters) {
  Point coordinate;

  auto markClass = classes[className];

  if (markClass.baseparameters.contains(baseGlyphName)) {
    coordinate = markClass.baseparameters[baseGlyphName];
  }

  GlyphVis* curr = &m_layout->glyphs[baseGlyphName];
  const QString qBaseGlyphName = QString::fromStdString(baseGlyphName);

  curr = curr->getAlternate(parameters);

  if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
    coordinate += curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor);
  } else {
    QString anchorName = QString::fromStdString(m_lookup->name + "_" + className);
    if (curr->conatinsAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor)) {
      coordinate += curr->getAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor);
    } else {
      QString anchorName = QString::fromStdString(m_lookup->name);
      if (curr->conatinsAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor)) {
        coordinate += curr->getAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor);
      } else if (markClass.basefunction) {
        coordinate = markClass.basefunction(baseGlyphName, className, coordinate, parameters);
      } else if (markClass.baseanchors.contains(baseGlyphName)) {
        coordinate += markClass.baseanchors[baseGlyphName];
      }
    }
  }

  return coordinate;
}

optional<Point> MarkBaseSubtable::getBaseAnchor(quint16 mark_id, quint16 base_id, GlyphParameters parameters) {
  quint16 classIndex = markCodes[mark_id];

  const std::string& className = classNamebyIndex[classIndex];

  const auto& baseGlyphName = m_layout->glyphNamePerCode[base_id];

  return getBaseAnchor(baseGlyphName, className, parameters);
}
Point MarkBaseSubtable::getMarkAnchor(std::string markGlyphName, std::string className, GlyphParameters parameters) {
  Point coordinate;

  auto markClass = classes[className];

  if (markClass.markparameters.contains(markGlyphName)) {
    coordinate = markClass.markparameters[markGlyphName];
  }

  GlyphVis* curr = &m_layout->glyphs[markGlyphName];
  const QString qMarkGlyphName = QString::fromStdString(markGlyphName);

  curr = curr->getAlternate(parameters);

  if (curr->conatinsAnchor(className, GlyphVis::AnchorType::MarkAnchor)) {
    coordinate += curr->getAnchor(className, GlyphVis::AnchorType::MarkAnchor);
  } else {
    QString anchorName = QString::fromStdString(m_lookup->name + "_" + className);
    if (curr->conatinsAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor)) {
      coordinate += curr->getAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor);
    } else {
      QString anchorName = QString::fromStdString(m_lookup->name);
      if (curr->conatinsAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor)) {
        coordinate += curr->getAnchor(anchorName.toStdString(), GlyphVis::AnchorType::MarkAnchor);
      } else if (markClass.markfunction != nullptr) {
        coordinate = markClass.markfunction(markGlyphName, className, coordinate, parameters);
      } else if (markClass.markanchors.contains(markGlyphName)) {
        coordinate += markClass.markanchors[markGlyphName];
      }
    }
  }

  return coordinate;
}
optional<Point> MarkBaseSubtable::getMarkAnchor(quint16 mark_id, quint16 base_id, GlyphParameters parameters) {
  quint16 classIndex = markCodes[mark_id];

  const std::string& className = classNamebyIndex[classIndex];

  const auto& markGlyphName = m_layout->glyphNamePerCode[mark_id];

  return getMarkAnchor(markGlyphName, className, parameters);
}

void MarkBaseSubtable::setAnchorTable(std::string className,
                                      std::uint16_t glyphCode,
                                      digitalkhatt::ByteBuffer& anchorTables,
                                      std::uint32_t& anchorOffset,
                                      std::map<int, std::pair<int, std::pair<int, int>>>& posToVar,
                                      bool extended,
                                      bool isBase) {
  const auto& glyphName = m_layout->glyphNamePerCode[glyphCode];

  std::string originalGlyph = glyphName;
  double charlt = 0.0;
  double charrt = 0.0;

  if (!extended) {
    auto glyph = m_layout->getGlyph(glyphCode);
    if (glyph->name.find(".added_") != std::string::npos) {
      originalGlyph = glyph->originalglyph;
    }
    charlt = glyph->charlt;
    charrt = glyph->charrt;
  }

  Point coordinate = isBase ? getBaseAnchor(originalGlyph, className, {.lefttatweel = charlt, .righttatweel = charrt}) : getMarkAnchor(originalGlyph, className, {.lefttatweel = charlt, .righttatweel = charrt});

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
        } else {
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
        if (!allx0) {
          auto index_x = m_layout->getDeltaSetEntry(delatX, regionIndexesArrayIndex);
          posToVar.insert({anchorTables.size(), {anchorTables.size() - 6, index_x}});
        }
        anchorTables << (quint16)0;  // xDeviceOffset
        if (!ally0) {
          auto index_y = m_layout->getDeltaSetEntry(delatY, regionIndexesArrayIndex);
          posToVar.insert({anchorTables.size(), {anchorTables.size() - 8, index_y}});
        }
        anchorTables << (quint16)0;  // yDeviceOffset
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

void Subtable::setVariationIndexOffset(
    digitalkhatt::ByteBuffer& anchorTables,
    std::uint32_t anchorOffset,
    std::map<int, std::pair<int, std::pair<int, int>>>& posToVar) {
  if (anchorOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name << " Subtable " << name
              << " Overflows. anchorOffset=" << anchorOffset
              << std::endl;
  }

  int total = 0;
  int found = 0;
  std::map<std::pair<int, int>, int> indexes;

  for (auto& varIndex : posToVar) {
    auto pos = varIndex.first;
    auto start = varIndex.second.first;
    auto index = varIndex.second.second;

    quint32 offset;

    auto it = indexes.find(index);
    if (it != indexes.end()) {
      found++;
      offset = it->second;
    } else {
      offset = anchorTables.size();
      indexes.insert({index, offset});
      anchorTables << (quint16)index.first;
      anchorTables << (quint16)index.second;
      anchorTables << (quint16)0x8000;
    }
    total++;

    quint32 offsetFromAnchorTable = offset - start;
    if (offsetFromAnchorTable > 0xFFFF) {
      std::cout << "Lookup " << m_lookup->name << " Subtable " << name
                << " Overflows. offsetFromAnchorTable=" << offsetFromAnchorTable
                << std::endl;
    }
    digitalkhatt::ByteBuffer offsetData;
    offsetData << (quint16)offsetFromAnchorTable;
    anchorTables.replace(pos, offsetData.size(), offsetData);
  }
  /*
  std::cout << "Lookup " << m_lookup->name << " Subtable " << name
        << " total=" << total
        << " found=" << found
        << std::endl;*/
}

digitalkhatt::ByteBuffer MarkBaseSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer baseCoverage;
  digitalkhatt::ByteBuffer markCoverage;
  digitalkhatt::ByteBuffer markArray;
  digitalkhatt::ByteBuffer baseArray;
  digitalkhatt::ByteBuffer baseAnchorTables;
  digitalkhatt::ByteBuffer markAnchorTables;

  int classIndex = 0;

  classNamebyIndex.clear();
  markCodes.clear();

  if (sortedBaseCodes.empty()) {
    QSet<quint16> baseCodesSet;
    for (int i = 0; i < base.size(); ++i) {
      const auto codes = m_layout->classtoUnicode(base.at(i));
      for (const auto code : codes) baseCodesSet.insert(code);
    }
    const auto values = baseCodesSet.values();
    sortedBaseCodes.assign(values.begin(), values.end());

    std::sort(sortedBaseCodes.begin(), sortedBaseCodes.end());
  }

  quint16 baseCount = sortedBaseCodes.size();
  quint16 markClassCount = classes.size();

  for (auto it = classes.cbegin(); it != classes.cend(); ++it) {
    MarkClass markClass = it->second;
    if (markClass.markCodes.empty()) {
      for (auto markName : markClass.mark) {
        const auto codes = m_layout->classtoUnicode(markName);
        markClass.markCodes.insert(codes.begin(), codes.end());
      }
    }

    for (auto i = markClass.markCodes.cbegin(); i != markClass.markCodes.cend(); ++i) {
      markCodes[*i] = classIndex;
    }

    classNamebyIndex[classIndex] = it->first;

    classIndex++;
  }

  quint16 markCount = markCodes.size();

  // Base coverage && Base Array
  quint32 baseAnchorOffset = 2 + baseCount * (markClassCount * 2);

  std::map<int, std::pair<int, std::pair<int, int>>> basePosToVar;

  baseCoverage << (quint16)1 << baseCount;
  baseArray << baseCount;

  for (int i = 0; i < sortedBaseCodes.size(); ++i) {
    quint16 glyphCode = sortedBaseCodes.at(i);
    const auto& baseglyphName = m_layout->glyphNamePerCode[glyphCode];
    baseCoverage << glyphCode;
    for (auto it = classes.cbegin(); it != classes.cend(); ++it) {
      baseArray << (quint16)baseAnchorOffset;
      setAnchorTable(it->first, glyphCode, baseAnchorTables, baseAnchorOffset, basePosToVar, extended, true);
    }
  }
  setVariationIndexOffset(baseAnchorTables, baseAnchorOffset, basePosToVar);
  baseArray.append(baseAnchorTables);

  // Mark coverage && Mark Array

  quint32 markAnchorOffset = 2 + markCount * 4;

  std::map<int, std::pair<int, std::pair<int, int>>> markPosToVar;

  markCoverage << (quint16)1 << markCount;
  markArray << markCount;

  for (auto it = markCodes.cbegin(); it != markCodes.cend(); ++it) {
    quint16 charcode = it->first;
    quint16 classIndex = it->second;
    const std::string& className = classNamebyIndex[classIndex];

    markCoverage << charcode;

    markArray << classIndex;
    markArray << (quint16)markAnchorOffset;
    setAnchorTable(className, charcode, markAnchorTables, markAnchorOffset, markPosToVar, extended, false);
  }
  setVariationIndexOffset(markAnchorTables, markAnchorOffset, markPosToVar);
  markArray.append(markAnchorTables);

  quint32 markCoverageOffset = 12;
  quint32 baseCoverageOffset = markCoverageOffset + markCoverage.size();
  quint32 markArrayOffset = baseCoverageOffset + baseCoverage.size();
  quint32 baseArrayOffset = markArrayOffset + markArray.size();

  if (baseArrayOffset > 0xFFFF) {
    std::cout << "Lookup " << m_lookup->name << " Subtable " << name
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

void ChainingSubtable::readJson(const ParameterJsonObject& ruleObject) {
  rule = Rule();
  compiledRule = CompiledRule();

  auto readContext = [&](std::string_view key, auto& ruleDestination,
                         auto& compiledDestination) {
    const auto positions =
        jsonValueAs<std::vector<std::vector<std::string>>>(ruleObject, key);
    if (!positions) return;
    for (const auto& position : *positions) {
      std::unordered_set<std::string> classSet;
      std::unordered_set<std::uint16_t> glyphSet;
      for (const auto& className : position) {
        classSet.insert(className);
        auto glyphs = m_layout->classtoUnicode(className);
        glyphSet.insert(glyphs.begin(), glyphs.end());
      }
      ruleDestination.emplace_back(classSet.begin(), classSet.end());
      compiledDestination.emplace_back(glyphSet.begin(), glyphSet.end());
    }
  };
  readContext("input", rule.input, compiledRule.input);
  readContext("lookahead", rule.lookahead, compiledRule.lookahead);
  readContext("backtrack", rule.backtrack, compiledRule.backtrack);

  const auto* lookupRecordsValue = findJsonValue(ruleObject, "lookuprecords");
  if (!lookupRecordsValue || !lookupRecordsValue->is_array()) return;
  const auto& lookupRecords = lookupRecordsValue->get_array();
  for (std::size_t j = 0; j < lookupRecords.size(); ++j) {
    LookupRecord lookupRecord;
    const auto& record = lookupRecords[j];
    if (record.is_object()) {
      const auto& object = record.get_object();
      lookupRecord.lookupName =
          jsonValueAs<std::string>(object, "lookup").value_or("");
      lookupRecord.position = jsonValueAs<int>(object, "position").value_or(0);
    } else if (const auto lookupName = jsonValueAs<std::string>(record)) {
      lookupRecord.lookupName = *lookupName;
      lookupRecord.position = static_cast<int>(j);
    } else if (record.is_array()) {
      const auto& pair = record.get_array();
      if (pair.size() == 2) {
        lookupRecord.position = jsonValueAs<int>(pair[0]).value_or(0);
        lookupRecord.lookupName =
            jsonValueAs<std::string>(pair[1]).value_or("");
      }
    }

    rule.lookupRecords.push_back(lookupRecord);
    compiledRule.lookupRecords.push_back(lookupRecord);
  }
}

digitalkhatt::ByteBuffer ChainingSubtable::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer coverages;

  quint16 backtrackGlyphCount = compiledRule.backtrack.size();
  quint16 inputGlyphCount = compiledRule.input.size();
  quint16 lookaheadGlyphCount = compiledRule.lookahead.size();
  quint16 substitutionCount = compiledRule.lookupRecords.size();

  quint16 beginoffsets = 2 + 2 * (3 + backtrackGlyphCount + inputGlyphCount + lookaheadGlyphCount) + 2 + 4 * substitutionCount;

  root << quint16(3);
  root << backtrackGlyphCount;

  for (int i = backtrackGlyphCount - 1; i >= 0; i--) {
    auto set = compiledRule.backtrack.at(i);
    std::vector<quint16> coveargeVector(set.begin(), set.end());
    std::sort(coveargeVector.begin(), coveargeVector.end());

    quint16 coverageSize = coveargeVector.size();

    coverages << quint16(1) << coverageSize;
    for (quint16 glyph : coveargeVector) coverages << glyph;

    root << beginoffsets;
    beginoffsets += (2 + 2 + 2 * coverageSize);
  }

  root << inputGlyphCount;

  for (int i = 0; i < inputGlyphCount; i++) {
    auto set = compiledRule.input.at(i);
    std::vector<quint16> coveargeVector(set.begin(), set.end());
    std::sort(coveargeVector.begin(), coveargeVector.end());

    quint16 coverageSize = coveargeVector.size();

    coverages << quint16(1) << coverageSize;
    for (quint16 glyph : coveargeVector) coverages << glyph;

    root << beginoffsets;
    beginoffsets += (2 + 2 + 2 * coverageSize);
  }

  root << lookaheadGlyphCount;

  for (int i = 0; i < lookaheadGlyphCount; i++) {
    auto set = compiledRule.lookahead.at(i);
    std::vector<quint16> coveargeVector(set.begin(), set.end());
    std::sort(coveargeVector.begin(), coveargeVector.end());

    quint16 coverageSize = coveargeVector.size();

    coverages << quint16(1) << coverageSize;
    for (quint16 glyph : coveargeVector) coverages << glyph;

    root << beginoffsets;
    beginoffsets += (2 + 2 + 2 * coverageSize);
  }

  root << substitutionCount;

  for (int i = 0; i < substitutionCount; i++) {
    auto lookeprecord = compiledRule.lookupRecords.at(i);
    root << lookeprecord.position;
    quint16 lookupListIndex;

    std::string fullname = m_lookup->name + "." + lookeprecord.lookupName;

    if (m_lookup->isGsubLookup()) {
      if (m_layout->gsublookupsIndexByName.contains(fullname)) {
        lookupListIndex = m_layout->gsublookupsIndexByName[fullname];
      } else {
        lookupListIndex = m_layout->gsublookupsIndexByName[lookeprecord.lookupName];
      }
    } else {
      if (m_layout->gposlookupsIndexByName.contains(fullname)) {
        lookupListIndex = m_layout->gposlookupsIndexByName[fullname];
      } else {
        lookupListIndex = m_layout->gposlookupsIndexByName[lookeprecord.lookupName];
      }
    }

    root << lookupListIndex;
  }

  root.append(coverages);

  return root;
}
