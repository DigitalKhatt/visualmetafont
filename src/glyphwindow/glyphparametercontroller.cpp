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
#include "font.hpp"
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
  auto* reset = new QPushButton(tr("Reset internal parameters"), this);
  reset->setObjectName("resetDesignAxes");
  layout->addWidget(reset);
  connect(reset, &QPushButton::clicked, this, [this] {
    if (!m_glyph) return;
    m_glyph->undoStack()->beginMacro(tr("Reset internal parameters"));
    for (const auto& axis : m_glyph->font->project.internalParameters())
      m_glyph->setPropertyWithUndo(axis.key.toUtf8(), QVariant::fromValue(Glyph::AxisType{axis.defaultValue}));
    m_glyph->undoStack()->endMacro();
  });

  m_readOnlyManager = new QtVariantPropertyManager(this);
  m_manager = new QtVariantPropertyManager(this);
  m_variantfactory = new QtVariantEditorFactory(this);
  
  m_axisManager = new QtDoublePropertyManager(this);
  m_axisFactory = new QtAxisFactory(this);

  m_browser->setFactoryForManager(m_manager, m_variantfactory);
  m_browser->setFactoryForManager(m_axisManager, m_axisFactory);

  connect(m_manager, &QtVariantPropertyManager::valueChanged, this, &GlyphParameterController::slotValueChanged);
  connect(m_axisManager, &QtDoublePropertyManager::valueChanged, this, &GlyphParameterController::slotValueChanged);

  setGlyph(glyph);
}
GlyphParameterController::~GlyphParameterController() {

}
void GlyphParameterController::setGlyph(Glyph* glyph) {
  if (m_glyph == glyph)
    return;

  if (m_glyph) {
    disconnect(m_glyph, nullptr, this, nullptr);
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
        m_axisManager->setValue(prop, vv.value);
      }
      else if (name == "sourceFile") {
        m_readOnlyManager->setValue(prop, val);
      }
      else {
        m_manager->setValue(prop, val);
      }

    }
  }

}
void GlyphParameterController::slotValueChanged(QtProperty* property, const QVariant& value)
{
  if (!m_glyph || !m_propertyToName.contains(property)) return;
  QString name = m_propertyToName.value(property);

  QVariant oldVariant = m_glyph->property(name.toLatin1());


  if (oldVariant.isValid() && oldVariant != value) {
    int axisTypeId = qMetaTypeId<Glyph::AxisType>();
    if (oldVariant.userType() == axisTypeId) {    
      QVariant newVariant = QVariant::fromValue(Glyph::AxisType{ (double)value.toDouble()  });      
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
    if (!metaProperty.isWritable()) {
      subProperty = m_readOnlyManager->addProperty(type, name == "sourceFile" ? tr("Source file") : name);
      m_readOnlyManager->setValue(subProperty, metaProperty.read(m_glyph));
    }
    else if (m_manager->isPropertyTypeSupported(type)) {
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
        
        ProjectParameter descriptor;
        for (const auto& axis : m_glyph->font->project.internalParameters())
          if (axis.key == QString::fromUtf8(propname)) descriptor = axis;
        if (!descriptor.visible) continue;
        QtProperty* subProperty = m_axisManager->addProperty(descriptor.label);
        subProperty->setToolTip(QString("Internal parameter • params%1 • default %2\n%3")
            .arg(descriptor.metaPostIndex).arg(descriptor.defaultValue).arg(descriptor.description));
        m_axisManager->setRange(subProperty, descriptor.minimum, descriptor.maximum);
        m_axisManager->setDecimals(subProperty, 2);
        m_axisManager->setValue(subProperty, vv.value);
        m_axisManager->setSingleStep(subProperty, descriptor.step);
        m_axisFactory->setDefaultValue(subProperty, descriptor.defaultValue);
        m_propertyToName[subProperty] = propname;
        m_nametoProperty[propname] = subProperty;
        m_browser->addProperty(subProperty);
        /*QtVariantProperty* subProperty = m_manager->addProperty(QVariant::Double, QLatin1String(propname));
        if (subProperty) {
          subProperty->setValue(vv.value * tatweelRes);
          m_propertyToName[subProperty] = propname;
          m_nametoProperty[propname] = subProperty;
          m_browser->addProperty(subProperty);
        }*/
      }
    }
  }
}