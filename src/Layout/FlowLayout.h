#pragma once

#include <QLayout>
#include <QList>
#include <QRect>
#include <QStyle>

// Standard reflow layout (Qt's well-known "Flow Layout" example): lays child
// widgets left-to-right, wrapping to a new line when the current line runs
// out of horizontal space. Used to pack many small label+spinbox rows
// without wasting vertical space when the container is wide enough to fit
// several side by side.
class FlowLayout : public QLayout {
 public:
  explicit FlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
  explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
  ~FlowLayout() override;

  void addItem(QLayoutItem* item) override;
  int horizontalSpacing() const;
  int verticalSpacing() const;
  Qt::Orientations expandingDirections() const override;
  bool hasHeightForWidth() const override;
  int heightForWidth(int) const override;
  int count() const override;
  QLayoutItem* itemAt(int index) const override;
  QSize minimumSize() const override;
  void setGeometry(const QRect& rect) override;
  QSize sizeHint() const override;
  QLayoutItem* takeAt(int index) override;

 private:
  int doLayout(const QRect& rect, bool testOnly) const;
  int smartSpacing(QStyle::PixelMetric pm) const;

  QList<QLayoutItem*> itemList;
  int m_hSpace;
  int m_vSpace;
};
