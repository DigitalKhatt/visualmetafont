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

#include "imageitem.hpp"
#include <QtWidgets>

ImageItem::ImageItem(Glyph * glyph, QGraphicsItem * parent) : QGraphicsPixmapItem(parent) {

	//QMatrix m;
	//m.scale(1, -1);

	//setMatrix(m);

	this->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	this->setZValue(-2);
	this->glyph = glyph;
}
ImageItem::ImageItem(Glyph * glyph, const QPixmap &pixmap, QGraphicsItem *parent)
	: QGraphicsPixmapItem(pixmap, parent) {

	this->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	this->setZValue(-2);
	this->glyph = glyph;

}

ImageItem::~ImageItem() {

}

void ImageItem::setPath(QString path) {
	m_path = path;
}

QString ImageItem::path() {
	return m_path;
}

void ImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QMenu menu;
	//QAction * scaleAct = new QAction("&Scale", this);	
	//connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

	menu.addAction("Scale");	
	menu.addAction("UnScale");
	menu.addAction("Delete");
	QAction *a = menu.exec(event->screenPos());
	if (a != NULL) {
		if (a->text() == "Scale") {
			Glyph::ImageInfo imageInfo = glyph->image();
			imageInfo.transform.scale(1.18, 1.18);
			//glyph->setImage(imageInfo);
			QVariant var;
			var.setValue(imageInfo);
			glyph->setPropertyWithUndo("image", var);
		}
		else if (a->text() == "UnScale") {
			Glyph::ImageInfo imageInfo = glyph->image();
			imageInfo.transform.scale(1 / 1.18, 1 / 1.18);
			//glyph->setImage(imageInfo);
			QVariant var;
			var.setValue(imageInfo);
			glyph->setPropertyWithUndo("image", var);
		}
		else if (a->text() == "Delete") {
			Glyph::ImageInfo imageInfo;
			QVariant var;
			var.setValue(imageInfo);
			m_path = "";
			setPixmap(QPixmap());
			glyph->setPropertyWithUndo("image", var);
		}
	}
	
}
void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

	Glyph::ImageInfo imageInfo = glyph->image();
	imageInfo.pos = imageInfo.pos + (event->scenePos() - event->lastScenePos());
	glyph->setImage(imageInfo);
	
}
