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
#include <iostream>

GlyphParameterController::GlyphParameterController(QWidget* parent) : GlyphParameterController(NULL, parent) {
}
GlyphParameterController::GlyphParameterController(Glyph* glyph, QWidget* parent) : QWidget(parent) {
  m_browser = new QtGroupBoxPropertyBrowser(this);
  //m_browser->setRootIsDecorated(true);

  QFont textEditFont("DejaVu Sans Mono");

  //textEditFont.setPointSize(20);
  m_browser->setFont(textEditFont);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->getContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_browser);

  m_readOnlyManager = new QtVariantPropertyManager(this);
  m_manager = new QtVariantPropertyManager(this);
  m_variantfactory = new QtVariantEditorFactory(this);


  //TEST
  m_sliderManager = new QtIntPropertyManager(this);
  m_sliderFactory = new QtSliderFactory(this);

  m_browser->setFactoryForManager(m_manager, m_variantfactory);
  m_browser->setFactoryForManager(m_sliderManager, m_sliderFactory);
  //m_browser->setFactoryForManager(m_tatweelManager, m_sliderFactory);

  connect(m_manager, SIGNAL(valueChanged(QtProperty*, const QVariant&)),
    this, SLOT(slotValueChanged(QtProperty*, const QVariant&)));

  connect(m_sliderManager, &QtIntPropertyManager::valueChanged, this, &GlyphParameterController::slotValueChanged);

  setGlyph(glyph);
}
GlyphParameterController::~GlyphParameterController() {

}
void GlyphParameterController::setGlyph(Glyph* glyph) {
  if (m_glyph == glyph)
    return;

  if (m_glyph) {
    QMapIterator<QtProperty*, QString> it(m_propertyToName);
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
    QMapIterator<QtProperty*, QString> it(m_propertyToName);
    while (it.hasNext()) {
      m_browser->removeProperty(it.next().key());
    }
    m_propertyToName.clear();
    m_nametoProperty.clear();
    addProperties();
  }
  else {
    auto* prop = m_nametoProperty[name];
    if (prop) {
      QVariant val = m_glyph->property(name.toLatin1());
      int axisTypeId = qMetaTypeId<Glyph::AxisType>();
      if (val.userType() == axisTypeId) {
        Glyph::AxisType vv = val.value<Glyph::AxisType>();
        m_sliderManager->setValue(prop, vv.value * tatweelRes);
      }
      else {
        m_manager->setValue(prop, val);
      }

    }
  }

}
void GlyphParameterController::slotValueChanged(QtProperty* property, const QVariant& value)
{
  QString name = property->propertyName();

  QVariant oldVariant = m_glyph->property(name.toLatin1());


  if (oldVariant.isValid() && oldVariant != value) {
    int axisTypeId = qMetaTypeId<Glyph::AxisType>();
    if (oldVariant.userType() == axisTypeId) {    
      QVariant newVariant = QVariant::fromValue(Glyph::AxisType{ (double)value.toInt() / tatweelRes });      
      if (newVariant != oldVariant) {
        //auto oldValue = oldVariant.value< Glyph::AxisType>();
        //auto newValue = newVariant.value< Glyph::AxisType>();
        //std::cout << "oldValue=" << oldValue.value << "newValue=" << newValue.value << std::endl;
        m_glyph->setPropertyWithUndo(name.toLatin1(), newVariant);
      }
    }
    else {
      m_glyph->setPropertyWithUndo(name.toLatin1(), value);
    }

  }

}
void GlyphParameterController::addProperties()
{
  if (!m_glyph)
    return;



  const QMetaObject* metaObject = m_glyph->metaObject();
  QtProperty* subProperty = nullptr;

  for (int idx = metaObject->propertyOffset(); idx < metaObject->propertyCount(); idx++) {
    QMetaProperty metaProperty = metaObject->property(idx);
    int type = metaProperty.userType();
    QString name = metaProperty.name();
    if (name == "source" || name == "parameters" || name == "body" || name == "verbatim")
      continue;
    if (m_manager->isPropertyTypeSupported(type)) {
      subProperty = m_manager->addProperty(type, QLatin1String(metaProperty.name()));
      m_manager->setValue(subProperty, metaProperty.read(m_glyph));
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

  for (auto& [name, param] : m_glyph->params) {
    QByteArray propname = param.name.toLatin1();
    QVariant val = m_glyph->property(propname);
    if (val.isValid()) {
      QtVariantProperty* subProperty = m_manager->addProperty(val.type(), QLatin1String(propname));
      if (subProperty) {
        subProperty->setValue(val);
        m_propertyToName[subProperty] = param.name;
        m_nametoProperty[param.name] = subProperty;
        m_browser->addProperty(subProperty);
      }
      
    }
  }

  QList<QByteArray> dynamicProperties = m_glyph->dynamicPropertyNames();
  for (int i = 0; i < dynamicProperties.length(); i++) {
    QByteArray propname = dynamicProperties[i];
    if (!m_glyph->params.contains(propname)) {
      QVariant val = m_glyph->property(propname);
      auto axisTypeId = qMetaTypeId<Glyph::AxisType>();
      if (val.userType() == axisTypeId) {
        Glyph::AxisType vv = val.value< Glyph::AxisType>();
        subProperty = m_sliderManager->addProperty(propname);
        m_sliderManager->setRange(subProperty, -10 * tatweelRes, 20 * tatweelRes);
        m_sliderManager->setValue(subProperty, vv.value * tatweelRes);
        m_sliderManager->setSingleStep(subProperty, 1);
        m_propertyToName[subProperty] = propname;
        m_nametoProperty[propname] = subProperty;
        m_browser->addProperty(subProperty);
      }
    }
  }
}

