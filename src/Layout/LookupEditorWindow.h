#pragma once
#include <QMainWindow>

class LookupEditorWindow : public QMainWindow {
 public:
  LookupEditorWindow(const QString& title, QWidget* editor, QWidget* parent = nullptr)
      : QMainWindow(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(title + "[*]");
    setCentralWidget(editor);
    resize(1400, 880);
  }
};
