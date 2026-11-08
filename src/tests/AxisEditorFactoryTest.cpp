#include "editorfactory.hpp"
#include "qtpropertymanager.h"
#include <QApplication>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <iostream>
#include <stdexcept>

static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
int main(int argc, char** argv) {
  QApplication app(argc, argv);
  try {
    QtDoublePropertyManager manager;
    QtAxisFactory factory;
    factory.addPropertyManager(&manager);
    QWidget parent;
    auto* weight = manager.addProperty("Weight");
    manager.setRange(weight, 100, 900);
    manager.setValue(weight, 400);
    factory.setDefaultValue(weight, 400);
    auto* editor = static_cast<QtAbstractEditorFactoryBase*>(&factory)->createEditor(weight, &parent);
    auto* slider = editor->findChild<QSlider*>("axisSlider");
    auto* value = editor->findChild<QDoubleSpinBox*>("axisValue");
    auto* reset = editor->findChild<QPushButton*>("axisReset");
    require(slider && value && reset, "custom slider/value/reset controls");
    require(value->value() == 400 && !reset->isEnabled(), "actual coordinate and default reset state");
    slider->setValue(6500);
    require(manager.value(weight) == 650 && value->value() == 650 && reset->isEnabled(), "slider updates model and numeric input");
#ifdef Q_OS_MAC
    require(slider->style()->objectName() == "fusion", "axis sliders avoid asynchronous native painting");
#endif
    editor->resize(480, 32);
    editor->show();
    app.processEvents();
    auto handleRect = [&] {
      QStyleOptionSlider option;
      option.initFrom(slider);
      option.orientation = Qt::Horizontal;
      option.minimum = slider->minimum(); option.maximum = slider->maximum();
      option.sliderPosition = slider->sliderPosition(); option.sliderValue = slider->value();
      option.singleStep = slider->singleStep(); option.pageStep = slider->pageStep();
      option.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
      return slider->style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, slider);
    };
    const QPoint press = handleRect().center();
    QMouseEvent down(QEvent::MouseButtonPress, press, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(slider, &down);
    require(slider->isSliderDown(), "mouse grabs the visible handle");
    // Alternate extremes without waits, like a fast drag, and verify that every
    // event has one position for the handle, track, numeric field and model.
    for (int i = 0; i < 100; ++i) {
      const QPoint target(i % 2 ? slider->width() - 1 : 0, press.y());
      QMouseEvent move(QEvent::MouseMove, target, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(slider, &move);
      require(slider->value() == slider->sliderPosition() &&
          manager.value(weight) == slider->value() / 10.0 && value->value() == manager.value(weight),
          "fast drag keeps handle, fill, model and number synchronized");
      const auto bounds = handleRect();
      require(i % 2 ? bounds.right() >= slider->width() - 2 : bounds.left() <= 1,
          "handle geometry reaches both drag endpoints");
      slider->grab(); // Exercise painting between successive drag events too.
    }
    QMouseEvent up(QEvent::MouseButtonRelease, QPoint(slider->width() - 1, press.y()), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(slider, &up);
    require(!slider->isSliderDown() && manager.value(weight) == 900, "release preserves the final drag value");
    value->setValue(675.25);
    require(manager.value(weight) == 675.25 && slider->value() == 6753, "numeric precision survives slider synchronization");
    reset->click();
    require(manager.value(weight) == 400 && value->value() == 400 && slider->value() == 4000 && !reset->isEnabled(), "weight resets to 400");
    manager.setRange(weight, 200, 600);
    manager.setSingleStep(weight, 2.5);
    require(value->minimum() == 200 && value->maximum() == 600 && slider->maximum() == 6000, "updated range reaches both controls");
    require(value->singleStep() == 2.5 && slider->singleStep() == 25, "consistent step sizes");
    auto* second = static_cast<QtAbstractEditorFactoryBase*>(&factory)->createEditor(weight, &parent);
    manager.setValue(weight, 500);
    require(second->findChild<QDoubleSpinBox*>("axisValue")->value() == 500 && value->value() == 500, "multiple editors stay synchronized");
    auto* scale = manager.addProperty("Horizontal scale");
    manager.setRange(scale, 50, 200); manager.setValue(scale, 100); factory.setDefaultValue(scale, 100);
    auto* scaleEditor = static_cast<QtAbstractEditorFactoryBase*>(&factory)->createEditor(scale, &parent);
    scaleEditor->findChild<QSlider*>("axisSlider")->setValue(1500);
    scaleEditor->findChild<QPushButton*>("axisReset")->click();
    require(manager.value(scale) == 100, "scale resets to 100");
    auto* legacy = manager.addProperty("Left extension");
    manager.setRange(legacy, -10, 20); manager.setSingleStep(legacy, .1);
    auto* legacyEditor = static_cast<QtAbstractEditorFactoryBase*>(&factory)->createEditor(legacy, &parent);
    legacyEditor->findChild<QSlider*>("axisSlider")->setValue(-3);
    require(manager.value(legacy) == -.3, "fractional legacy extension");
    legacyEditor->findChild<QPushButton*>("axisReset")->click();
    require(manager.value(legacy) == 0, "legacy reset stays zero");
    std::cout << "PASS custom axis editor: slider, numeric precision, defaults, range/step and synchronization\n";
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
