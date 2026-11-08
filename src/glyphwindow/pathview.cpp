
#include "pathview.h"
#include "glyph.hpp"

#include <QPainter>


PathView::PathView(Glyph* glyph, QWidget* parent)
  : QWidget(parent)
{

  this->glyph = glyph;

  connect(this->glyph, &Glyph::valueChanged, this, &PathView::glyphChanged);

  setBackgroundRole(QPalette::Base);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  //QTransform transform{ 3,0,0,3,0,0 };

  //this->path = glyph->getPath() * transform;
  //auto box = this->path.boundingRect();
  //this->path = this->path.translated(-box.left(), box.top());

  //this->path = glyph->getPath();
  //path.setFillRule(Qt::WindingFill);

  this->picture = glyph->getPicture();

}
void PathView::glyphChanged(QString name) {
  /*path = glyph->getPath();
  path.setFillRule(Qt::WindingFill);*/
  this->picture = glyph->getPicture();
  update();
}
QSize PathView::minimumSizeHint() const
{
  return QSize(160,160);
}

QSize PathView::sizeHint() const
{
  return QSize(300,320);
}


void PathView::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  const auto box=QRectF(picture.boundingRect());
  const auto target=QRectF(contentsRect()).adjusted(8,8,-8,-8);
  if (box.isEmpty() || target.isEmpty()) return;
  const auto scale=qMin(target.width()/box.width(),target.height()/box.height());
  QTransform transform{scale,0,0,-scale,
      target.center().x()-scale*box.center().x(),target.center().y()+scale*box.center().y()};

  painter.setPen(Qt::NoPen);  
  painter.setBrush(Qt::black);
  //painter.drawPath(path * transform);
  painter.setTransform(transform);
  painter.drawPicture(0,0,picture);
}
