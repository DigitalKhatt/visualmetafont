/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "editorfactory.hpp"
#include <QtWidgets/qslider.h>
#include <QtCore/QMap>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qspinbox.h>
#include <QtWidgets/qframe.h>
// ---------- EditorFactoryPrivate :
// Base class for editor factory private classes. Manages mapping of properties to editors and vice versa.

template <class Editor>
class EditorFactoryPrivate
{
public:

    typedef QList<Editor *> EditorList;
    typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
    typedef QMap<Editor *, QtProperty *> EditorToPropertyMap;

    Editor *createEditor(QtProperty *property, QWidget *parent);
    void initializeEditor(QtProperty *property, Editor *e);
    void slotEditorDestroyed(QObject *object);

    PropertyToEditorListMap  m_createdEditors;
    EditorToPropertyMap m_editorToProperty;
};

template <class Editor>
Editor *EditorFactoryPrivate<Editor>::createEditor(QtProperty *property, QWidget *parent)
{
    Editor *editor = new Editor(parent);
    initializeEditor(property, editor);
    return editor;
}

template <class Editor>
void EditorFactoryPrivate<Editor>::initializeEditor(QtProperty *property, Editor *editor)
{
    typename PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
    if (it == m_createdEditors.end())
        it = m_createdEditors.insert(property, EditorList());
    it.value().append(editor);
    m_editorToProperty.insert(editor, property);
}

template <class Editor>
void EditorFactoryPrivate<Editor>::slotEditorDestroyed(QObject *object)
{
    const typename EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
    for (typename EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor !=  ecend; ++itEditor) {
        if (itEditor.key() == object) {
            Editor *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            const typename PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
            if (pit != m_createdEditors.end()) {
                pit.value().removeAll(editor);
                if (pit.value().empty())
                    m_createdEditors.erase(pit);
            }
            m_editorToProperty.erase(itEditor);
            return;
        }
    }
}
// QtAxisFactory

class QtAxisFactoryPrivate : public EditorFactoryPrivate<QWidget>
{
    QtAxisFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtAxisFactory)
public:
    void slotPropertyChanged(QtProperty *property, double value);
    void slotRangeChanged(QtProperty *property, double min, double max);
    void slotSingleStepChanged(QtProperty *property, double step);
    void slotSetValue(int value);
private:
  const int tatweelRes = 10;
};

void QtAxisFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
    if (!m_createdEditors.contains(property))
        return;
    QListIterator<QWidget *> itEditor(m_createdEditors[property]);
    while (itEditor.hasNext()) {
        QWidget *editor = itEditor.next();
        QLayout* layout = editor->layout();
        QSlider* slider = (QSlider*) layout->itemAt(1)->widget();        
        slider->blockSignals(true);
        slider->setValue(value * tatweelRes);
        slider->blockSignals(false);
        QSpinBox* spinBox = (QSpinBox*) layout->itemAt(2)->widget();        
        spinBox->blockSignals(true);
        spinBox->setValue(value * tatweelRes);
        spinBox->blockSignals(false);
    }
}

void QtAxisFactoryPrivate::slotRangeChanged(QtProperty *property, double min, double max)
{
    if (!m_createdEditors.contains(property))
        return;

    QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
    if (!manager)
        return;

    QListIterator<QWidget *> itEditor(m_createdEditors[property]);
    while (itEditor.hasNext()) {
        QWidget *editor = itEditor.next();
        QLayout* layout = editor->layout();
        QSlider* slider = (QSlider*) layout->itemAt(1)->widget();
        //QSlider* slider = (QSlider*)editor;
        slider->blockSignals(true);
        slider->setRange(min*tatweelRes, max*tatweelRes);
        slider->setValue(manager->value(property)*tatweelRes);
        slider->blockSignals(false);
    }
}

void QtAxisFactoryPrivate::slotSingleStepChanged(QtProperty *property, double step)
{
    if (!m_createdEditors.contains(property))
        return;
    QListIterator<QWidget *> itEditor(m_createdEditors[property]);
    while (itEditor.hasNext()) {
        QWidget *editor = itEditor.next();
        QLayout* layout = editor->layout();
        QSlider* slider = (QSlider*) layout->itemAt(1)->widget();        
        slider->blockSignals(true);
        slider->setSingleStep(step);
        slider->blockSignals(false);
    }
}

void QtAxisFactoryPrivate::slotSetValue(int value)
{
    QObject *object = q_ptr->sender()->parent();
    const QMap<QWidget *, QtProperty *>::ConstIterator ecend = m_editorToProperty.constEnd();
    for (QMap<QWidget *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor ) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtDoublePropertyManager *manager = q_ptr->propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, (double)value/tatweelRes);
            return;
        }
    }
}

QtAxisFactory::QtAxisFactory(QObject *parent)
    : QtAbstractEditorFactory<QtDoublePropertyManager>(parent), d_ptr(new QtAxisFactoryPrivate())
{
    d_ptr->q_ptr = this;

}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtAxisFactory::~QtAxisFactory()
{
    qDeleteAll(d_ptr->m_editorToProperty.keys());
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtAxisFactory::connectPropertyManager(QtDoublePropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty*,double)),
                this, SLOT(slotPropertyChanged(QtProperty*,double)));
    connect(manager, SIGNAL(rangeChanged(QtProperty*,double,double)),
                this, SLOT(slotRangeChanged(QtProperty*,double,double)));
    connect(manager, SIGNAL(singleStepChanged(QtProperty*,double)),
                this, SLOT(slotSingleStepChanged(QtProperty*,double)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtAxisFactory::createEditor(QtDoublePropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    
    slider->setSingleStep(manager->singleStep(property));
    slider->setRange(manager->minimum(property)*tatweelRes, manager->maximum(property)*tatweelRes);
    slider->setValue(manager->value(property)*tatweelRes);
    slider->setFocusPolicy(Qt::ClickFocus);    

    /*QWidget *editor = slider;
    d_ptr->initializeEditor(property, editor);*/

    
    QWidget *editor = new QWidget(parent);    
    d_ptr->initializeEditor(property, editor);
    editor->setContentsMargins(0,0,0,0);

    QHBoxLayout *layout = new QHBoxLayout(editor);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QPushButton *clear = new QPushButton("X");    
    
    clear->setStyleSheet(R"(
        QPushButton {             
            background-color:white;           
            border-width: 2px;
            border-radius: 6px;
            font: bold 12px;
            min-width: 10px;
            padding: 0px 8px;
            border-color: gray;
    })");
    clear->setContentsMargins(0, 0, 0, 0);   
   
    

    QSpinBox *spinBox = new QSpinBox;
    spinBox->setSingleStep(manager->singleStep(property));
    spinBox->setRange(manager->minimum(property)*tatweelRes, manager->maximum(property)*tatweelRes);
    spinBox->setValue(manager->value(property)*tatweelRes);
    spinBox->setKeyboardTracking(false);  
    spinBox->setContentsMargins(0, 0, 0, 0);
    //spinBox->setStyleSheet("QSpinBox { margin: 0px; padding: 0px; }");

    layout->addWidget(clear,0,Qt::AlignVCenter);
    layout->addWidget(slider,1,Qt::AlignVCenter);
    layout->addWidget(spinBox,0,Qt::AlignVCenter);    
    editor->setFocusProxy(slider);
    
    

    connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=] (int value) {
        slider->setValue(value);
    });

    connect(clear, &QPushButton::pressed, this, [=] () {
        slider->setValue(0);
    });

    connect(slider, &QSlider::valueChanged,this, [this] (int value) {
        d_ptr->slotSetValue(value);
    });

    //connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slotSetValue(int)));
    connect(editor, SIGNAL(destroyed(QObject*)),
                this, SLOT(slotEditorDestroyed(QObject*)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtAxisFactory::disconnectPropertyManager(QtDoublePropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty*,double)),
                this, SLOT(slotPropertyChanged(QtProperty*,double)));
    disconnect(manager, SIGNAL(rangeChanged(QtProperty*,double,double)),
                this, SLOT(slotRangeChanged(QtProperty*,double,double)));
    disconnect(manager, SIGNAL(singleStepChanged(QtProperty*,double)),
                this, SLOT(slotSingleStepChanged(QtProperty*,double)));
}

#include "moc_editorfactory.cpp"
#include "editorfactory.moc"

