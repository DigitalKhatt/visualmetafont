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

#pragma once

#include <QString>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QHash>
#include "OtLayout.h"
#include <optional>
#include "JustificationContext.h"


struct Lookup;
class QJsonObject;
class AnchorCalc;
class Font;

struct Subtable {
	friend class OtLayout;

	Subtable(Lookup* lookup);
	virtual ~Subtable() {};

	virtual  void readJson(const QJsonObject& json) {};
	virtual QByteArray getOptOpenTypeTable(bool extended) {
		if (isDirty) {
			return getOpenTypeTable(extended);
		}
		else {
			return openTypeSubTable;
		}
	};
	virtual QByteArray getConvertedOpenTypeTable() {
		return getOpenTypeTable(false);
	}

	virtual QByteArray getOpenTypeTable(bool extended) {
		return QByteArray();
	};
	virtual quint16 getCodeFromName(QString name);
	virtual QString getNameFromCode(quint16 code);

	virtual void saveParameters(QJsonObject& json) const {}
	virtual void readParameters(const QJsonObject& json) {}

	virtual bool isExtended() { return false; }

	virtual bool isConvertible() { return false; }

	Lookup* getLookup() {
		return m_lookup;
	}

	QString name;

protected:
	Lookup* m_lookup;
	Font* metafont;
	OtLayout* m_layout;
	bool isDirty = true;
	QByteArray openTypeSubTable;
};

struct SingleSubtable : Subtable {

	SingleSubtable(Lookup* lookup, quint16 format = 2);
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;

	QMap<quint16, quint16 > subst;

	bool isExtended() override;

	quint16 format;



};

struct SingleSubtableWithExpansion : SingleSubtable {

	SingleSubtableWithExpansion(Lookup* lookup);
	QByteArray getOpenTypeTable(bool extended) override;
	//void readJson(const QJsonObject &json) override;

	QMap<quint16, GlyphExpansion > expansion;

	bool isConvertible() override { return false; }

	QByteArray getConvertedOpenTypeTable() override;

};

struct SingleSubtableWithTatweel : SingleSubtable {

	SingleSubtableWithTatweel(Lookup* lookup);
	//QByteArray getOpenTypeTable() override;
	//void readJson(const QJsonObject &json) override;

	QMap<quint16, GlyphExpansion > expansion;

	QByteArray getOpenTypeTable(bool extended) override;

	bool isConvertible() override { return true; }

	QByteArray getConvertedOpenTypeTable() override;

};

struct MultipleSubtable : Subtable {

	MultipleSubtable(Lookup* lookup);
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;

	QMap<quint16, QVector<quint16> > subst;

};

struct AlternateSubtable : Subtable {

  AlternateSubtable(Lookup* lookup);
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;

  QMap<quint16, QVector<quint16> > alternates;

};

struct LigatureSubtable : Subtable {

	LigatureSubtable(Lookup* lookup);
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;

	struct Ligature {
		quint16 ligatureGlyph;
		QVector<quint16> componentGlyphIDs;
	};

	QVector<Ligature> ligatures;

	quint16 format = 1;
};

struct ValueRecord {
	qint16 xPlacement;
	qint16 yPlacement;
	qint16 xAdvance;
	qint16 yAdvance;

	bool operator==(const ValueRecord& rhs) const
	{
		return (this->xAdvance == rhs.xAdvance) && (this->yPlacement == rhs.yPlacement) && (this->xAdvance == rhs.xAdvance) && (this->yAdvance == rhs.yAdvance);
	}

	bool isEmpty() const {
		ValueRecord empty{};
		return *this == empty;
	}
};

struct SingleAdjustmentSubtable : Subtable {



	SingleAdjustmentSubtable(Lookup* lookup, quint16 format = 2);
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;
	void saveParameters(QJsonObject& json) const override;
	void readParameters(const QJsonObject& json) override;

	QMap<quint16, ValueRecord> singlePos;
	QMap<quint16, ValueRecord> parameters;

	bool isExtended() override;

	quint16 format;

};

struct CursiveSubtable : Subtable {

	struct EntryExit {
		std::optional<QPoint> entry;
		CalcAnchor entryFunction;
		QString entryName;
		std::optional<QPoint> exit;
		CalcAnchor exitFunction;
		QString exitName;
	};
	CursiveSubtable(Lookup* lookup) : Subtable{ lookup } {}
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;
	void readParameters(const QJsonObject& json) override;
	void saveParameters(QJsonObject& json) const override;

	QMap<quint16, EntryExit> anchors;

	QMap<quint16, QPoint> exitParameters;
	QMap<quint16, QPoint> entryParameters;

	virtual std::optional<QPoint> getEntry(quint16 glyph_id, double lefttatweel, double righttatweel);


	virtual QPoint calculateEntry(GlyphVis* originalglyph, GlyphVis* extendedglyph, QPoint entry);

	virtual std::optional<QPoint> getExit(quint16 glyph_id, double lefttatweel, double righttatweel);



};

struct MarkBaseSubtable : Subtable {

	struct MarkClass {
		QSet<QString> mark;
		QSet<quint16> markCodes;
		CalcAnchor  basefunction;
		CalcAnchor  markfunction;
		QMap<QString, QPoint> baseparameters;
		QMap<QString, QPoint> markparameters;
		QMap<QString, QPoint> baseanchors;
		QMap<QString, QPoint> markanchors;
	};
	MarkBaseSubtable(Lookup* lookup);
	
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;
	void saveParameters(QJsonObject& json) const override;
	void readParameters(const QJsonObject& json) override;



	QVector<QString> base;
	QMap<QString, MarkClass> classes;

	QList<quint16> sortedBaseCodes;

	QVector<quint16> baseCoverage;
	QMap<quint16, quint16> markCoverage;

	//  Computed during getOpenTypeTable
	QMap<quint16, quint16> markCodes;
	QMap<quint16, QString> classNamebyIndex;

	virtual std::optional<QPoint> getBaseAnchor(quint16 mark_id, quint16 base_id, double lefttatweel, double righttatweel);
	virtual std::optional<QPoint> getMarkAnchor(quint16 mark_id, quint16 base_id, double lefttatweel, double righttatweel);
};

struct ChainingSubtable : Subtable {

	struct LookupRecord {
		quint16 position;
		QString lookupName;
	};
	struct Rule {
		QVector<QSet<QString>> backtrack;
		QVector<QSet<QString>> lookahead;
		QVector<QSet<QString>> input;
		QVector<LookupRecord> lookupRecords;
	};

	struct CompiledRule {
		QVector<QSet<quint16>> backtrack;
		QVector<QSet<quint16>> lookahead;
		QVector<QSet<quint16>> input;
		QVector<LookupRecord> lookupRecords;
	};

	ChainingSubtable(Lookup* lookup);
	QByteArray getOpenTypeTable(bool extended) override;
	void readJson(const QJsonObject& json) override;


	Rule rule;

	//Compiled	
	CompiledRule compiledRule;
};

enum class DFAActionType {
  LOOKUP,
  ACTION,
  STARTNEWMATCH
};

struct DFAAction {
  DFAActionType type;
	std::string name;
  int idRule;
};

struct DFABackTrackInfo {
  int prevTransIndex;
  std::vector<DFAAction> actions;
};

struct DFATransOut {
  int state;
  std::vector<DFABackTrackInfo> backtracks;
};

struct DFASTate {
	std::map<int, DFATransOut> transtitions;
	int final = 0;	
	std::map <int, std::vector<DFAAction>> actions;
  DFABackTrackInfo backtrackfinal;
};

class DFA {
public:
	int minBackup = 0;
	int maxBackup = 0;
	int maxLoop = 10;
	std::vector<int> backupStates;
	std::vector<DFASTate> states;
  QVector<QSet<quint16>> eqClasses;
  QMap<quint16, quint16> glyphToClass;
};

struct FSMSubtable : Subtable {
public:
	FSMSubtable(Lookup* lookup) : Subtable{ lookup } {}

  QByteArray getOpenTypeTable(bool extended) override;

	DFA dfa;
};
