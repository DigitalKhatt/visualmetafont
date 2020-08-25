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
#include "qmetatype.h"
#include "qpainterpath.h"

class Font;
struct mp_edge_object;
typedef struct mp_gr_knot_data*mp_gr_knot;
typedef struct MP_instance*MP;




class Glyph : public QObject {
	Q_OBJECT

public:
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
	enum ParameterType {
		None,
		point,
		direction,
		tension,
		control,
		numeric
	};
	struct Param {
		QString name;
		ParameterType type;
		ParameterPosition position;
		int applytosubpath;
		int applytopoint;
		bool isEquation;
		bool isInControllePath;
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
		QString value;
		QString controlValue;
		bool isDirConstant;		
		bool isControlConstant;
		double x;
		double y;
		mpgui_knot_type type;
		bool isEqualAfter;
		bool isEqualBefore;
		bool isAtleast;
		//QVariant firstValue;
		//QVariant secondValue;
	};
	struct Knot {
		QString value;		
		bool isConstant;
		double x;
		double y;		
		QString paramName;
		KnotEntryExit leftValue;
		KnotEntryExit rightValue;
		
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
	Glyph(QString code, MP mp,Font * parent);	
	~Glyph();

	Q_PROPERTY(QString source READ source WRITE setSource NOTIFY valueChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY valueChanged)
	Q_PROPERTY(int unicode READ unicode WRITE setUnicode NOTIFY valueChanged)
	Q_PROPERTY(double width READ width WRITE setWidth NOTIFY valueChanged)
	Q_PROPERTY(double height READ height WRITE setHeight NOTIFY valueChanged)
	Q_PROPERTY(double depth READ depth WRITE setDepth NOTIFY valueChanged)
	Q_PROPERTY(double leftTatweel READ leftTatweel WRITE setleftTatweel NOTIFY valueChanged)
	Q_PROPERTY(double rightTatweel READ rightTatweel WRITE setrightTatweel NOTIFY valueChanged) 
		
	Q_PROPERTY(QString body READ body WRITE setBody NOTIFY valueChanged)
	Q_PROPERTY(QString verbatim READ verbatim WRITE setVerbatim NOTIFY valueChanged)
	Q_PROPERTY(QString parameters READ parameters WRITE setParameters NOTIFY valueChanged)
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
	void setleftTatweel(double lefttatweel);
	double leftTatweel() const;
	void setrightTatweel(double righttatweel);
	double rightTatweel() const;	
	void setBody(QString body, bool autoParam = false);
	QString body() const;
	void setVerbatim(QString body);
	QString verbatim() const;
	void setImage(Glyph::ImageInfo image);
	Glyph::ImageInfo image() const;
	void setComponents(QHashGlyphComponentInfo components);
	QHashGlyphComponentInfo components() const;
	

	void setParameters(QString parameters);
	void parseComponents(QString componentSource);	
	void setComponent(QString name, double x, double y, double t1, double t2, double t3, double t4);
	void setParameter(QString name, Glyph::ParameterType type, double x, double y, Glyph::ParameterPosition position, int numsubpath, int numpoint, bool isEquation);
	void parsePaths(QString pathsSource);
	QString parameters() const;
	QString getError();
	QString getLog();

	mp_edge_object*  getEdge(bool resetExpParams = false);
	QUndoStack* undoStack() const;

	QHash<QString, Param> ldirections;
	QHash<QString, Param> rdirections;
	QHash<QString, Param> ltensions;
	QHash<QString, Param> rtensions;	
	QMap<QString, Param> params;
	QHashGlyphComponentInfo m_components;
	QMap<int, QMap<int, Knot*> >  controlledPaths;
	QMap<int, QString >  controlledPathNames;

	Font* font;
	
	void setPropertyWithUndo(const QString &name, const QVariant &value);

	QPainterPath getPath();

	bool isDirty;

	ComputedValues getComputedValues();


signals:	
	void valueChanged(QString name, bool structureChanged = false);

protected:
	bool event(QEvent *e) Q_DECL_OVERRIDE;

private slots:
	void componentChanged(QString name, bool structureChanged = false);

private:
	
	QPainterPath getPathFromEdge(mp_edge_object* h);
	QPainterPath mp_dump_solved_path(mp_gr_knot h);

	QString m_source;
	QString m_name;
	int	m_unicode;
	int m_charcode;
	double m_width;
	double m_height;
	double m_depth;
	double m_lefttatweel;
	double m_righttatweel;
	QString m_body;
	QString m_verbatim;
	QString m_parameters;
	Glyph::ImageInfo m_image;
	mp_edge_object* edge;
	MP mp;
	
	bool isSetSource;
	QUndoStack* m_undoStack;
	QString m_beginmacroname;
	
	
};

typedef QHash<Glyph*, Glyph::ComponentInfo> QHashGlyphComponentInfo;

Q_DECLARE_METATYPE(Glyph::ImageInfo)
Q_DECLARE_METATYPE(Glyph::ComponentInfo)
Q_DECLARE_METATYPE(QHashGlyphComponentInfo)




#endif // GLYPH_H