/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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

	Glyph* m_glyph = nullptr;

	
	
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
