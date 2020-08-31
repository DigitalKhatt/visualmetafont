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
	metafont = m_layout->m_font;

}

Subtable::~Subtable()
{
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

void SingleSubtable::readJson(const QJsonObject &json)
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

QByteArray SingleSubtable::getOpenTypeTable() {

	QByteArray root;
	QByteArray coverage;
	QByteArray substituteGlyphIDs;



	quint16 glyphCount = subst.size();
	quint16 coverage_offset = 2 + 2 + 2 + 2 * glyphCount;


	root << (quint16)format;
	root << coverage_offset;
	root << glyphCount;


	coverage << (quint16)1;
	coverage << (quint16)glyphCount;

	QMapIterator<quint16, quint16>i(subst);

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

SingleSubtableWithTatweel::SingleSubtableWithTatweel(Lookup* lookup) : SingleSubtable(lookup, 11) {};

QByteArray SingleSubtableWithTatweel::getOpenTypeTable() {

	QByteArray root;
	QByteArray coverage;
	QByteArray substituteGlyphIDs;



	quint16 glyphCount = expansion.size();
	quint16 coverage_offset = 2 + 2 + 2 + 8 * glyphCount;


	root << (quint16)format;
	root << coverage_offset;
	root << glyphCount;


	coverage << (quint16)1;
	coverage << (quint16)glyphCount;

	QMapIterator<quint16, GlyphExpansion>i(expansion);

	while (i.hasNext()) {
		i.next();

		GlyphExpansion expan = i.value();

		OT::HBFixed lefttatweel;
		lefttatweel.set_float(expan.MinLeftTatweel);

		OT::HBFixed righttatweel;
		righttatweel.set_float(expan.MinRightTatweel);

		root << lefttatweel << righttatweel;
		coverage << (quint16)i.key();

	}

	root.append(coverage);

	return root;
};



SingleAdjustmentSubtable::SingleAdjustmentSubtable(Lookup* lookup, quint16 pformat) : Subtable(lookup), format{ pformat } {}

void SingleAdjustmentSubtable::readJson(const QJsonObject &json) {
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
QByteArray SingleAdjustmentSubtable::getOpenTypeTable() {

	QByteArray root;
	QByteArray coverage;
	QByteArray substituteGlyphIDs;



	quint16 glyphCount = singlePos.size();
	quint16 coverage_offset = 2 + 2 + 2 + 2 + (2 * 4)* glyphCount;
	quint16 valueFormat = 0xF;

	if (format == 3) {
		int h = 0;
	}

	root << format;
	root << coverage_offset;
	root << valueFormat;
	root << glyphCount;


	coverage << (quint16)1;
	coverage << (quint16)glyphCount;

	QMapIterator<quint16, ValueRecord>i(singlePos);

	while (i.hasNext()) {
		i.next();

		ValueRecord record = i.value();

		if (parameters.contains(i.key())) {
			ValueRecord par = parameters.value(i.key());
			record = { (qint16)(record.xPlacement + par.xPlacement),(qint16)(record.yPlacement + par.yPlacement) ,(qint16)(record.xAdvance + par.xAdvance) ,record.yAdvance };
		}

		root << record.xPlacement << record.yPlacement << record.xAdvance << record.yAdvance;
		coverage << (quint16)i.key();

	}

	root.append(coverage);

	return root;
}
void SingleAdjustmentSubtable::readParameters(const QJsonObject &json) {


	if (json["parameters"].isObject()) {
		QJsonObject parametersObject = json["parameters"].toObject();
		for (int ia = 0; ia < parametersObject.size(); ++ia) {
			QString glyphName = parametersObject.keys()[ia];
			QJsonArray pointArray = parametersObject[glyphName].toArray();
			parameters[m_layout->glyphCodePerName[glyphName]] = { (qint16)pointArray[0].toInt(), (qint16)pointArray[1].toInt(),(qint16)pointArray[2].toInt(),(qint16)pointArray[3].toInt() };
		}

	}

}
void SingleAdjustmentSubtable::saveParameters(QJsonObject &json) const {

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

QByteArray MultipleSubtable::getOpenTypeTable() {

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

void MultipleSubtable::readJson(const QJsonObject &json)
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

LigatureSubtable::LigatureSubtable(Lookup* lookup) : Subtable(lookup) {
}

QByteArray LigatureSubtable::getOpenTypeTable() {

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

void LigatureSubtable::readJson(const QJsonObject &json)
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
MarkBaseSubtable::~MarkBaseSubtable() {
	for (auto classe : classes)
	{
		if (classe.basefunction) {
			delete classe.basefunction;
		}
		if (classe.markfunction) {
			delete classe.markfunction;
		}
	}
}

void MarkBaseSubtable::readJson(const QJsonObject &json)
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
optional<QPoint> CursiveSubtable::getExit(quint16 glyph_id, double lefttatweel, double righttatweel) {

    optional<QPoint> ret;

    if (anchors.contains(glyph_id)) {

        auto& entryexit = anchors[glyph_id];

        if (entryexit.exit || !entryexit.exitName.isEmpty()) {

            QPoint exit;

            if(entryexit.exit){
               exit  =  *(entryexit.exit);
            }


            if (exitParameters.contains(glyph_id)) {
                exit += exitParameters[glyph_id];
            }
            if (lefttatweel != 0.0 || righttatweel != 0.0) {
                GlyphParameters parameters{};

                parameters.lefttatweel = lefttatweel;
                parameters.righttatweel = righttatweel;

                QString glyphName = m_layout->glyphNamePerCode[glyph_id];

                GlyphVis* originalglyph = &m_layout->glyphs[glyphName];

                GlyphVis* curr = originalglyph->getAlternate(parameters);

                if(!entryexit.exitName.isEmpty() && curr->conatinsAnchor(entryexit.exitName)){
                    exit = curr->getAnchor(entryexit.exitName);
                }else if(curr->conatinsAnchor(this->name)){
                    exit = curr->getAnchor(this->name);
                }else{
                   //exit = calculateEntry(originalglyph, curr, entry);
                }
            }
            ret = exit;
        }
    }

    return ret;
}

optional<QPoint> CursiveSubtable::getEntry(quint16 glyph_id, double lefttatweel, double righttatweel) {

	optional<QPoint> ret;

	if (anchors.contains(glyph_id)) {

		auto & entryexit = anchors[glyph_id];

		if (entryexit.entry) {

			QPoint entry{ *(entryexit.entry) };

			if (entryParameters.contains(glyph_id)) {
				entry += entryParameters[glyph_id];
			}

			if (lefttatweel != 0.0 || righttatweel != 0.0) {
				GlyphParameters parameters{};

				parameters.lefttatweel = lefttatweel;
				parameters.righttatweel = righttatweel;

				QString glyphName = m_layout->glyphNamePerCode[glyph_id];

				GlyphVis* originalglyph = &m_layout->glyphs[glyphName];

				GlyphVis* curr = originalglyph->getAlternate(parameters);

                if(curr->conatinsAnchor(this->name)){
                    entry = curr->getAnchor(this->name);
                }else{
                   entry = calculateEntry(originalglyph, curr, entry);
                }
            }
			ret = entry;

		}
	}

	return ret;
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
void CursiveSubtable::readJson(const QJsonObject &json) {

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
void CursiveSubtable::readParameters(const QJsonObject &json) {


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
void CursiveSubtable::saveParameters(QJsonObject &json) const {

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

QByteArray CursiveSubtable::getOpenTypeTable() {

	QByteArray root;
	QByteArray entryExitRecords;

	quint16 entryExitCount = anchors.size();

	quint16 coverageOffset = 2 + 2 + 2 + entryExitCount * 4;
	quint16 anchorOffset = coverageOffset + 2 + 2 + 2 * entryExitCount;

	root << (quint16)1 << coverageOffset << entryExitCount;


	for (auto anchor = anchors.constBegin(); anchor != anchors.constEnd(); ++anchor) {

		auto & entryexit = anchor.value();

        auto calcentry = entryexit.entry;

        if(!calcentry){
            if(!entryexit.entryName.isEmpty()){
                 GlyphVis* curr = m_layout->getGlyph(anchor.key());
                 if(curr->anchors.contains(entryexit.entryName)){
                     calcentry = curr->anchors.value(entryexit.entryName);
                 }
            }
        }


        if (calcentry) {

            QPoint entry{ *(calcentry) };

			if (entryParameters.contains(anchor.key())) {
				entry += entryParameters[anchor.key()];
			}

			root << anchorOffset;
			anchorOffset += 6;

			entryExitRecords << (quint16)1;
			entryExitRecords << (quint16)entry.x();
			entryExitRecords << (quint16)entry.y();

		}
		else {
			root << (quint16)0;
		}

        calcentry = entryexit.exit;

        if(!calcentry){
            if(!entryexit.exitName.isEmpty()){
                 GlyphVis* curr = m_layout->getGlyph(anchor.key());
                 if(curr->anchors.contains(entryexit.exitName)){
                     calcentry = curr->anchors.value(entryexit.exitName);
                 }
            }
        }

        if (calcentry) {

            QPoint exit{ *(calcentry) };

			if (exitParameters.contains(anchor.key())) {
				exit += exitParameters[anchor.key()];
			}

			root << anchorOffset;
			anchorOffset += 6;

			entryExitRecords << (quint16)1;
			entryExitRecords << (quint16)exit.x();
			entryExitRecords << (quint16)exit.y();

		}
		else {
			root << (quint16)0;
		}
	}

	//Coverage
	root << (quint16)1 << entryExitCount << anchors.keys();

	root.append(entryExitRecords);

	return root;

}
void MarkBaseSubtable::saveParameters(QJsonObject &json) const {
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
void MarkBaseSubtable::readParameters(const QJsonObject &json) {

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

optional<QPoint> MarkBaseSubtable::getBaseAnchor(quint16 mark_id, quint16 base_id, double lefttatweel, double righttatweel) {

	quint16 classIndex = markCodes[mark_id];

	QString className = classNamebyIndex[classIndex];

	QString baseGlyphName = m_layout->glyphNamePerCode[base_id];

	QPoint coordinate;

	auto markClass = classes[className];

	if (markClass.baseparameters.contains(baseGlyphName)) {
		coordinate = markClass.baseparameters[baseGlyphName];
	}

	if (markClass.baseanchors.contains(baseGlyphName)) {
		coordinate += markClass.baseanchors[baseGlyphName];
	}
	else {

		if (markClass.basefunction != nullptr) {
			coordinate = (*markClass.basefunction)(baseGlyphName, className, coordinate, lefttatweel, righttatweel);
		}
	}

	return coordinate;

	/*
	if (lefttatweel != 0 || righttatweel != 0) {

		GlyphVis* baseGlyph = &this->m_layout->glyphs[baseGlyphName];

		GlyphParameters parameters{};

		parameters.lefttatweel = lefttatweel;
		parameters.righttatweel = righttatweel;

		baseGlyph = &baseGlyph->getAlternate(parameters);
	}*/



	return coordinate;
}
optional<QPoint> MarkBaseSubtable::getMarkAnchor(quint16 mark_id, quint16 base_id, double lefttatweel, double righttatweel) {

	quint16 classIndex = markCodes[mark_id];

	QString className = classNamebyIndex[classIndex];

	QString baseGlyphName = m_layout->glyphNamePerCode[base_id];

	QString markGlyphName = m_layout->glyphNamePerCode[mark_id];

	QPoint coordinate;

	auto markClass = classes[className];

	if (markClass.markparameters.contains(markGlyphName)) {
		coordinate = markClass.markparameters[markGlyphName];
	}

	if (markClass.markanchors.contains(markGlyphName)) {
		coordinate += markClass.markanchors[markGlyphName];
	}
	else {

		if (markClass.markfunction != nullptr) {
			coordinate = (*markClass.markfunction)(markGlyphName, className, coordinate, lefttatweel, righttatweel);
		}
	}

	return coordinate;

}

QByteArray MarkBaseSubtable::getOpenTypeTable() {

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
		sortedBaseCodes = baseCodesSet.toList();

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
	quint16 baseAnchorOffset = 2 + baseCount * (markClassCount * 2);

	baseCoverage << (quint16)1 << baseCount;
	baseArray << baseCount;

	for (int i = 0; i < sortedBaseCodes.size(); ++i) {
		quint16 glyphCode = sortedBaseCodes.at(i);
		QString baseglyphName = m_layout->glyphNamePerCode[glyphCode];
		baseCoverage << glyphCode;
		for (auto it = classes.constBegin(); it != classes.constEnd(); ++it) {
			baseArray << baseAnchorOffset;
			baseAnchorOffset += 6;


			//base Anchor

			MarkClass markClass = it.value();
			QString className = it.key();


			QPoint coordinate;


			if (markClass.baseparameters.contains(baseglyphName)) {
				coordinate = markClass.baseparameters[baseglyphName];
			}



			if (markClass.baseanchors.contains(baseglyphName)) {
				coordinate += markClass.baseanchors[baseglyphName];
			}
			else {

				if (markClass.basefunction != nullptr) {
					coordinate = (*markClass.basefunction)(baseglyphName, className, coordinate);
				}


			}



			baseAnchorTables << (quint16)1;
			baseAnchorTables << (quint16)coordinate.x();
			baseAnchorTables << (quint16)coordinate.y();
		}
	}

	baseArray.append(baseAnchorTables);

	// Base coverage && Base Array


	quint16 markAnchorOffset = 2 + markCount * 4;

	markCoverage << (quint16)1 << markCount;
	markArray << markCount;



	for (auto it = markCodes.constBegin(); it != markCodes.constEnd(); ++it) {

		quint16 charcode = it.key();
		quint16 classIndex = it.value();
		QString className = classNamebyIndex[classIndex];


		QString markglyphName = m_layout->glyphNamePerCode[charcode];
		markCoverage << charcode;

		markArray << classIndex;
		markArray << markAnchorOffset;
		markAnchorOffset += 6;

		//base Anchor

		MarkClass markClass = classes[className];

		QPoint coordinate;

		if (markClass.markparameters.contains(markglyphName)) {
			coordinate = markClass.markparameters[markglyphName];
		}

		if (markClass.markanchors.contains(markglyphName)) {
			coordinate += markClass.markanchors[markglyphName];
		}
		else {

			if (markClass.markfunction != nullptr) {
				coordinate = (*markClass.markfunction)(markglyphName, className, coordinate);
			}

		}

		markAnchorTables << (quint16)1;
		markAnchorTables << (quint16)coordinate.x();
		markAnchorTables << (quint16)coordinate.y();
	}

	markArray.append(markAnchorTables);

	quint16 markCoverageOffset = 12;
	quint16 baseCoverageOffset = markCoverageOffset + markCoverage.size();
	quint16 markArrayOffset = baseCoverageOffset + baseCoverage.size();
	quint16 baseArrayOffset = markArrayOffset + markArray.size();


	root << (quint16)1 << markCoverageOffset << baseCoverageOffset << markClassCount << markArrayOffset << baseArrayOffset;
	root.append(markCoverage);
	root.append(baseCoverage);
	root.append(markArray);
	root.append(baseArray);

	isDirty = false;
	openTypeSubTable = root;

	return openTypeSubTable;



};

ChainingSubtable::ChainingSubtable(Lookup* lookup) : Subtable(lookup) {}

void ChainingSubtable::readJson(const QJsonObject &ruleObject) {

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

QByteArray ChainingSubtable::getOpenTypeTable() {

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
		auto coveargeVector = set.toList();
		std::sort(coveargeVector.begin(), coveargeVector.end());

		quint16 coverageSize = coveargeVector.size();

		coverages << quint16(1) << coverageSize << coveargeVector;

		root << beginoffsets;
		beginoffsets += (2 + 2 + 2 * coverageSize);

	}

	root << inputGlyphCount;

	for (int i = 0; i < inputGlyphCount; i++) {
		auto set = compiledRule.input.at(i);
		auto coveargeVector = set.toList();
		std::sort(coveargeVector.begin(), coveargeVector.end());

		quint16 coverageSize = coveargeVector.size();

		coverages << quint16(1) << coverageSize << coveargeVector;

		root << beginoffsets;
		beginoffsets += (2 + 2 + 2 * coverageSize);

	}

	root << lookaheadGlyphCount;

	for (int i = 0; i < lookaheadGlyphCount; i++) {
		auto set = compiledRule.lookahead.at(i);
		auto coveargeVector = set.toList();
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


