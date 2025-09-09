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
#ifndef GLYPHPARAMETERCONTROLLER_H
#define GLYPHPARAMETERCONTROLLER_H
#include <QWidget>
#include "qtvariantproperty.h"
#include "qtgroupboxpropertybrowser.h"
#include "qttreepropertybrowser.h"
#include "qtpropertybrowser.h"
#include "glyph.hpp"
#include "editorfactory.hpp"

class QtSliderFactory;
class QtIntPropertyManager;
class QtDoublePropertyManager;

class GlyphParameterController : public QWidget {
	Q_OBJECT

public:
	GlyphParameterController(QWidget * parent = Q_NULLPTR);
	GlyphParameterController(Glyph* glyph, QWidget * parent = Q_NULLPTR);
	~GlyphParameterController();
	void setGlyph(Glyph* glyph);

private slots:
	void slotValueChanged(QtProperty *property, const QVariant &value);	
	void glyphChanged(QString name);

private:	
	void addProperties();

	Glyph* m_glyph;

	
	
	QMap<QtProperty *, QString>     m_propertyToName;	
	QMap<QString, QtProperty *>     m_nametoProperty;

	QtAbstractPropertyBrowser    *m_browser;
	QtVariantPropertyManager *m_manager;
	QtVariantPropertyManager *m_readOnlyManager;
	QtVariantEditorFactory * m_variantfactory;
	QtAxisFactory * m_axisFactory;
	QtDoublePropertyManager *m_axisManager;
};
#endif // GLYPHPARAMETERCONTROLLER_H
