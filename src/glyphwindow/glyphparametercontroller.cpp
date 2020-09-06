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

#include "glyphparametercontroller.hpp"
#include "qtpropertymanager.h"
#include "qteditorfactory.h"
# include <QtWidgets>

GlyphParameterController::GlyphParameterController(QWidget * parent) : GlyphParameterController(NULL, parent) {
}
GlyphParameterController::GlyphParameterController(Glyph* glyph, QWidget * parent) : QWidget(parent) {
	m_browser = new QtGroupBoxPropertyBrowser(this);
	//m_browser->setRootIsDecorated(true);


	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(m_browser);

	m_readOnlyManager = new QtVariantPropertyManager(this);
	m_manager = new QtVariantPropertyManager(this);
	m_variantfactory = new QtVariantEditorFactory(this);

	//TEST
	m_sliderManager = new QtIntPropertyManager(this);
	m_sliderFactory = new QtSliderFactory(this);

	m_browser->setFactoryForManager(m_manager, m_variantfactory);
	m_browser->setFactoryForManager(m_sliderManager, m_sliderFactory);

	connect(m_manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
		this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));

	connect(m_sliderManager, &QtIntPropertyManager::valueChanged, this, &GlyphParameterController::slotValueChanged);

	setGlyph(glyph);
}
GlyphParameterController::~GlyphParameterController() {

}
void GlyphParameterController::setGlyph(Glyph* glyph) {
	if (m_glyph == glyph)
		return;

	if (m_glyph) {
		QMapIterator<QtProperty *, QString> it(m_propertyToName);
		while (it.hasNext()) {
			m_browser->removeProperty(it.next().key());
		}
		m_propertyToName.clear();
		m_nametoProperty.clear();
	}

	m_glyph = glyph;

	if (!m_glyph)
		return;

	connect(glyph, &Glyph::valueChanged, this, &GlyphParameterController::glyphChanged);

	addProperties();

}
void GlyphParameterController::glyphChanged(QString name) {

	if (name == "source") {
		QMapIterator<QtProperty *, QString> it(m_propertyToName);
		while (it.hasNext()) {
			m_browser->removeProperty(it.next().key());
		}
		m_propertyToName.clear();
		m_nametoProperty.clear();
		addProperties();
	}
	else {
		auto * prop = m_nametoProperty[name];
		if (prop) {
			QVariant val = m_glyph->property(name.toLatin1());
			if (name == "leftTatweel" || name == "rightTatweel") {
				m_sliderManager->setValue(prop, val.toInt());
			}
			else {
				m_manager->setValue(prop, val);
				//prop->setValue(val);
			}

		}
		/*
		else {
			for (auto prop : m_sliderManager->properties()) {
				if (prop->propertyName() == name) {
					QVariant val = m_glyph->property(name.toLatin1());
					m_sliderManager->setValue(prop, val.toInt());
					break;
				}
			}

		}*/
	}

}
void GlyphParameterController::slotValueChanged(QtProperty *property, const QVariant &value)
{
	/*
	if (!m_propertyToName.contains(property))
		return;

	QString name = m_propertyToName.value(property);*/

	QString name = property->propertyName();

	QVariant oldvalue = m_glyph->property(name.toLatin1());

	if (oldvalue.isValid() && oldvalue != value) {
		m_glyph->setPropertyWithUndo(name.toLatin1(), value);
	}

}
void GlyphParameterController::addProperties()
{
	if (!m_glyph)
		return;



	const QMetaObject *metaObject = m_glyph->metaObject();
	QtProperty *subProperty = nullptr;

	for (int idx = metaObject->propertyOffset(); idx < metaObject->propertyCount(); idx++) {
		QMetaProperty metaProperty = metaObject->property(idx);
		int type = metaProperty.userType();
		QString name = metaProperty.name();
		if (name == "source" || name == "parameters" || name == "body" || name == "verbatim")
			continue;
		if (m_manager->isPropertyTypeSupported(type)) {
			if (name == "leftTatweel" || name == "rightTatweel") {
				subProperty = m_sliderManager->addProperty(name.toLatin1());
				m_sliderManager->setRange(subProperty, -10, 20);
				m_sliderManager->setValue(subProperty, metaProperty.read(m_glyph).toInt());				
			}
			else {
				subProperty = m_manager->addProperty(type, QLatin1String(metaProperty.name()));
				m_manager->setValue(subProperty, metaProperty.read(m_glyph));
				//subProperty->setValue(metaProperty.read(m_glyph));
			}
		}
		else {
			subProperty = m_readOnlyManager->addProperty(QVariant::String, name.toLatin1());
			m_manager->setValue(subProperty, QLatin1String("< Unknown Type >"));
			// subProperty->setValue(QLatin1String("< Unknown Type >"));
			subProperty->setEnabled(false);
		}
		m_propertyToName[subProperty] = name;
		m_nametoProperty[name] = subProperty;
		m_browser->addProperty(subProperty);
	}

	QMapIterator<QString, Glyph::Param> i(m_glyph->params);
	while (i.hasNext()) {
		i.next();
		Glyph::Param param = i.value();
		//if (param.type == Glyph::point) {
		QByteArray propname = param.name.toLatin1();
		//if(param.type !=  Glyph::ParameterType::expression){
		  QVariant val = m_glyph->property(propname);
		  //if (QMetaType::QPointF == val.type()) {
		  QtVariantProperty *subProperty = m_manager->addProperty(val.type(), QLatin1String(propname));
		  subProperty->setValue(val);
		  m_propertyToName[subProperty] = param.name;
		  m_nametoProperty[param.name] = subProperty;
		  m_browser->addProperty(subProperty);
		  //}
		//}


	//}
	}
}

