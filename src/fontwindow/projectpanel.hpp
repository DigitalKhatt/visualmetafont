#pragma once
#include <QWidget>
class MdiChild;

class ProjectPanel : public QWidget {
 public:
  explicit ProjectPanel(MdiChild* child, QWidget* parent = nullptr);
};
