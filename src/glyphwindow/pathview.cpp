
#include "pathview.h"
#include "glyph.hpp"

#include <QPainter>


PathView::PathView(Glyph* glyph, QWidget* parent)
  : QWidget(parent)
{

  this->glyph = glyph;

  connect(this->glyph, &Glyph::valueChanged, this, &PathView::glyphChanged);

  setBackgroundRole(QPalette::Base);

  //QTransform transform{ 3,0,0,3,0,0 };

  //this->path = glyph->getPath() * transform;
  //auto box = this->path.boundingRect();
  //this->path = this->path.translated(-box.left(), box.top());

  this->path = glyph->getPath();
  path.setFillRule(Qt::WindingFill);

}
void PathView::glyphChanged(QString name) {
  path = glyph->getPath();
  path.setFillRule(Qt::WindingFill);
  update();
}
/*
QSize PathView::minimumSizeHint() const
{
  //return QSize(50, 50);
  auto box = this->path.boundingRect().size();
  return box.toSize();
}

QSize PathView::sizeHint() const
{
  auto box = this->path.boundingRect().size();
  return box.toSize();
}*/


void PathView::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  auto widthh = width() ;
  auto heighth = height();

  auto box = this->path.boundingRect();

  auto scalew = widthh / box.width();
  auto scaleh = heighth / box.height();

  

  auto scale = scalew < scaleh ? scalew : scaleh;

  QTransform transform{ scale ,0,0,-scale,- scale * box.left(),scale * box.bottom() };

  painter.setPen(Qt::NoPen);  
  painter.setBrush(Qt::black);
  painter.drawPath(path * transform);
}
