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
#ifndef GLYPH_H
#define GLYPH_H

#include "qtransform.h"
#include <QUndoStack>
#include <QMap>
#include <QHash>
#include <QVector>
#include <optional>

#include "qpainterpath.h"
#include "exp.hpp"
#include "pathpointexp.hpp"
#include <unordered_set>
#include "qpicture.h"


class Font;
struct mp_edge_object;
typedef struct mp_gr_knot_data* mp_gr_knot;
typedef struct MP_instance* MP;

class Glyph : public QObject {
	Q_OBJECT

public:
	struct AxisType {
		double value;
	};
	enum ParameterPosition {
		left,
		right,
	};
	enum mpgui_knot_type {
		mpgui_endpoint = 0,
		mpgui_explicit,
		mpgui_given,
		mpgui_curl,
		mpgui_open,
		mpgui_end_cycle,
		mpgui_formula
	};
	enum path_join_type {
		path_join_control = 0,
		path_join_tension,
		path_join_macro
	};
	struct Param {
		QString name;
		ParameterPosition position;
		int applytosubpath;
		int applytopoint;
		bool isEquation;
		bool isInControllePath;
		QString affects;
		QVariant value;
		std::unique_ptr<MFExpr> expr;
		Param() {};
        Param(const Param& a);
		Param(Param&& a);
        Param& operator=(Param const& other);
		Param& operator=(Param&& other);
	};
	struct ImageInfo
	{
		QString path;
		QPointF pos;
		QTransform transform;
	};
	struct ComponentInfo
	{
		QPointF pos;
		QTransform transform;
	};

	struct KnotEntryExit {
		//QString value;
		QString macrovalue;
		//QString controlValue;		
		//bool isControlConstant = false;
		//double x = 0;
		//double y = 0;
		mpgui_knot_type type = mpgui_endpoint;
		path_join_type jointtype = path_join_control;
		bool isEqualAfter = false;
		bool isEqualBefore = false;
		bool isAtleast = false;
		std::unique_ptr<MFExpr> dirExpr = nullptr;
		std::unique_ptr<MFExpr> tensionExpr = nullptr;

		KnotEntryExit() {}

		~KnotEntryExit() {}

		bool isControllable() {
			return (dirExpr && (dirExpr->containsConstant() || dirExpr->containsParam())) || (tensionExpr && (tensionExpr->containsConstant() || tensionExpr->containsParam()));
		}

		KnotEntryExit& operator=(const KnotEntryExit& other)
		{
			if (this != &other)
			{
				//this->value = other.value;
				this->macrovalue = other.macrovalue;
				//this->controlValue = other.controlValue;				
				//this->isControlConstant = other.isControlConstant;
				//this->x = other.x;
				//this->y = other.y;
				this->type = other.type;
				this->jointtype = other.jointtype;
				this->isEqualAfter = other.isEqualAfter;
				this->isEqualBefore = other.isEqualBefore;
				this->isAtleast = other.isAtleast;

				if (other.dirExpr) {
					this->dirExpr = other.dirExpr->clone();
				}
				else {
					this->dirExpr = nullptr;
				}

				if (other.tensionExpr) {
					this->tensionExpr = other.tensionExpr->clone();
				}
				else {
					this->tensionExpr = nullptr;
				}
			}


			return *this;
		}

		KnotEntryExit(const KnotEntryExit& other)
		{
			*this = other;
		}
	};
	struct Knot {
		KnotEntryExit leftValue;
		KnotEntryExit rightValue;
		std::unique_ptr<MFExpr> expr = nullptr;

		Knot() = default;

		//~Knot() {}

		Knot(const Knot& other) {
			this->leftValue = other.leftValue;
			this->rightValue = other.rightValue;
			if (other.expr) {
				this->expr = other.expr->clone();
			}

		}

		Knot& operator=(const Knot& other)
		{
			this->leftValue = other.leftValue;
			this->rightValue = other.rightValue;
			if (other.expr) {
				this->expr = other.expr->clone();
			}

			return *this;
		}
	};

	struct BBox {
		double llx;
		double lly;
		double urx;
		double ury;
	};

	struct ComputedValues {
		int charcode;
		double width;
		double height;
		double depth;
		BBox bbox;
		std::optional<QPoint> leftAnchor;
		std::optional<QPoint> rightAnchor;
	};

	typedef QHash<Glyph*, Glyph::ComponentInfo> QHashGlyphComponentInfo;
	typedef QVector<Knot> GlyphPath;


public:
	friend class GlyphCellItem;
	Glyph(QString code, Font* parent);
	~Glyph();

	Q_PROPERTY(QString source READ source WRITE setSource NOTIFY valueChanged)
		Q_PROPERTY(QString name READ name WRITE setName NOTIFY valueChanged)
		Q_PROPERTY(int unicode READ unicode WRITE setUnicode NOTIFY valueChanged)
		Q_PROPERTY(double width READ width WRITE setWidth NOTIFY valueChanged)
		Q_PROPERTY(double height READ height WRITE setHeight NOTIFY valueChanged)
		Q_PROPERTY(double depth READ depth WRITE setDepth NOTIFY valueChanged)

		Q_PROPERTY(QString body READ body WRITE setBody NOTIFY valueChanged)
		Q_PROPERTY(QString verbatim READ verbatim WRITE setVerbatim NOTIFY valueChanged)
		Q_PROPERTY(Glyph::ImageInfo image READ image WRITE setImage NOTIFY valueChanged)
		Q_PROPERTY(QHashGlyphComponentInfo components READ components WRITE setComponents NOTIFY valueChanged)

		void setSource(QString source, bool structureChanged = true);
	QString source();
	void setBeginMacroName(QString macro);
	QString beginMacroName();

	void setName(QString source);
	QString name() const;
	void setUnicode(int unicode);
	int unicode() const;
	int charcode();
	void setWidth(double width);
	double width() const;
	void setHeight(double height);
	double height() const;
	void setDepth(double depth);
	double depth() const;
	void setBody(QString body, bool autoParam = false);
	QString body() const;
	void setVerbatim(QString body);
	QString verbatim() const;
	void setImage(Glyph::ImageInfo image);
	Glyph::ImageInfo image() const;
	void setComponents(QHashGlyphComponentInfo components);
	QHashGlyphComponentInfo components() const;



	void parseComponents(QString componentSource);
	void setComponent(QString name, double x, double y, double t1, double t2, double t3, double t4);
	void setParameter(QString name, MFExpr* exp, bool isEquation, bool isInControllePath, QString affects);
	void setParameter(QString name, MFExpr* exp, bool isEquation);
	void setParameter(QString name, MFExpr* exp, bool isEquation, bool isInControllePath);
	void parsePaths(QString pathsSource);

	mp_edge_object* getEdge();
	QUndoStack* undoStack() const;


	std::map<QString, Param> params;
	QMap<QString, Param*> dependents;
	QHashGlyphComponentInfo m_components;
	QMap<int, QMap<int, Knot*> >  controlledPaths;
	QMap<int, QString >  controlledPathNames;
	QString getLog();
	QString getInfo();

	Font* font;

	void setPropertyWithUndo(const QString& name, const QVariant& value);

	bool isDirty;

	ComputedValues getComputedValues();

	bool setProperty(const char* name, const QVariant& value, bool updateParam = false);

	double axis(QString name);
#ifndef DIGITALKHATT_WEBLIB
	QPainterPath getPath();
	QPicture getPicture();

	static QPicture getPicture(mp_edge_object* h);

	static QPainterPath mp_dump_solved_path(mp_gr_knot h);

	static QPainterPath getPath(mp_edge_object* h);
#endif
signals:
	void valueChanged(QString name, bool structureChanged = false);

protected:
	bool event(QEvent* e) Q_DECL_OVERRIDE;

private slots:
	void componentChanged(QString name, bool structureChanged = false);

private:

	QPainterPath getPathFromEdge(mp_edge_object* h);
	//QPainterPath mp_dump_solved_path(mp_gr_knot h);

	QString m_source;
	QString m_name;
	int	m_unicode;
	int m_charcode;
	double m_width;
	double m_height;
	double m_depth;
	QString m_body;
	QString m_verbatim;
	Glyph::ImageInfo m_image;
	mp_edge_object* edge;

	bool isSetSource;
	QUndoStack* m_undoStack;
	QString m_beginmacroname;

	//TODO make it configurable
	std::vector<QString> axisNames = { "lefttatweel","righttatweel","third","fourth","fifth" };

};

typedef QHash<Glyph*, Glyph::ComponentInfo> QHashGlyphComponentInfo;

Q_DECLARE_METATYPE(Glyph::ImageInfo)
Q_DECLARE_METATYPE(Glyph::ComponentInfo)
Q_DECLARE_METATYPE(QHashGlyphComponentInfo)
Q_DECLARE_METATYPE(Glyph::AxisType)




#endif // GLYPH_H
