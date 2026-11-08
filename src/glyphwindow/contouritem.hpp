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

#pragma once

# include <QtCore>
# include <QtGui>



# include "mplibps.h"
#include "glyph.hpp"
#include "knotcontrolleditem.hpp"



class ContourItem : public QGraphicsObject {
	Q_OBJECT

	friend class GlyphScene;
public:
	ContourItem(QGraphicsItem * parent = Q_NULLPTR);
	~ContourItem();

	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
	
	
	void setFillEnabled(bool enable);
	void setFillGradient(const QColor &color1, const QColor &color2);
	void setPenWidth(int width);
	void setPenColor(const QColor &color);
	void setRotationAngle(int degrees);
	void setGlyph(Glyph* glyph, const QString& pictureName = {});
  void setOutlineVisible(bool visible);
  void setFinalPositions(bool enabled);
	void generateedge(mp_edge_object* h, bool newelement = true);
  void layoutAnchorLabels();
  void setAnchorDisplayState(const QSet<QString>& hidden, int labelMode, const QString& selected);
  void setPictureName(const QString& name);
  QString pictureName() const { return pictureName_; }
  QStringList pictureNames() const { return pictureNames_; }
  bool hasEditablePaths() const;

private:

	
	QPainterPath mp_dump_solved_path(mp_fill_object* fill,int numsubpath, bool newelement = true,
      int sourcePath = -2, const mp_fill_object* sourceGeometry = nullptr,
      const QTransform& sourceTransform = {});
  QString pictureName_;
  QStringList pictureNames_;
  QMap<int,int> sourceBindings_;
  bool outlineVisible_ = true;
  bool finalPositions_ = true;

	Glyph* glyph = nullptr;
	QGraphicsPathItem* path;
	QGraphicsItem* parampoints;
	QGraphicsItemGroup* labels;
  QGraphicsPathItem* anchorLabels = nullptr;
  struct AnchorLabel {
    QPointF point;
    QGraphicsSimpleTextItem* text;
    QGraphicsLineItem* leader;
    QString key;
    QGraphicsPathItem* marker;
    int type;
  };
  QVector<AnchorLabel> anchorLabelItems;
  QSet<QString> hiddenAnchorKeys_;
  QString selectedAnchorKey_;
  int anchorLabelMode_ = 0;
  void setAnchorsVisible(bool visible);
	QColor fillColor1;
	QColor fillColor2;
	int penWidth;
	QColor penColor;
	int rotationAngle;
	QMap<int, QMap<int, KnotControlledItem*> >  knotControlledItems;
	bool enableFill;
	
};
