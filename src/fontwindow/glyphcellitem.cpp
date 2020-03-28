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

#include <QtWidgets>
#include <QGraphicsView>
#include "glyphcellitem.hpp"
#include "glyphcellheaderitem.hpp"
#include "font.hpp"
#include "mdichild.h"


GlyphCellItem::GlyphCellItem(QGraphicsItem * parent) : QGraphicsObject(parent) {

	setAcceptedMouseButtons(Qt::LeftButton);
	setAcceptDrops(true);

	setFlags(QGraphicsItem::ItemClipsChildrenToShape | QGraphicsItem::ItemIsSelectable);

	double scale = 0.5;

	path = new QGraphicsPathItem(this);
	QTransform m;
	m.translate(300, 700);
	m.scale(scale, -scale);
	path->setTransform(m);
	path->setBrush(Qt::black);
	path->setZValue(-1);
	path->setFlag(QGraphicsItem::ItemStacksBehindParent);

	header = new GlyphCellHeaderItem(this);
	header->setFlag(QGraphicsItem::ItemStacksBehindParent);
	//header->setFlags(ItemIgnoresTransformations);
	//QTextOption option = header->document()->defaultTextOption();
	//option.setAlignment(Qt::AlignCenter);
	//header->document()->setDefaultTextOption(option);

	glyph = NULL;

}

GlyphCellItem::~GlyphCellItem() {

}

int GlyphCellItem::type() const
{
	// Enable the use of qgraphicsitem_cast with this item.
	return Type;
}
void GlyphCellItem::setGlyph(Glyph* glyph)
{
	if (this->glyph) {
		disconnect(this->glyph, &Glyph::valueChanged, this, &GlyphCellItem::glyphChanged);
	}

	this->glyph = glyph;

	connect(this->glyph, &Glyph::valueChanged, this, &GlyphCellItem::glyphChanged);

	header->setText(glyph->name());
	setPath();
	update();
}
void GlyphCellItem::glyphChanged(QString name) {
	setPath();
}
void GlyphCellItem::setPath() {
	QPainterPath path = glyph->getPath();
	path.setFillRule(Qt::WindingFill);
	this->path->setPath(path);
}

Glyph* GlyphCellItem::getGlyph() {
	return glyph;
}
QRectF GlyphCellItem::boundingRect() const
{
	return QRectF(0, 0, 1000, 1000);
}

void GlyphCellItem::paint(QPainter *painter,
	const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	//Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QColor selectionColor(Qt::blue);

	selectionColor.setAlphaF(0.2);

	if (this->isSelected()) {
		painter->fillRect(boundingRect(), selectionColor);
	}

	/*
	QGraphicsScene* scene = this->scene();
	if (scene) {
		QGraphicsView *view = scene->views().first();
		qreal height = view->viewportTransform().inverted().mapRect(QRectF(0, 0, 15, 15)).height();
		QRectF rect(0, 0, 1000, height);
		painter->setBrush(Qt::lightGray);
		painter->drawRect(rect);


		qreal factor = 1 / (view->transform().mapRect(QRectF(0, 0, 1, 1)).width());

		painter->scale(factor, factor);
		QRectF textRect(50 / factor, 0, 900 / factor, height / factor);

		painter->drawText(textRect, Qt::AlignCenter | Qt::TextSingleLine, glyph->getName());// , QTextOption(Qt::AlignCenter | Qt::TextSingleLine));
	}*/


}
void GlyphCellItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	//setCursor(Qt::ClosedHandCursor);

	QGraphicsObject::mousePressEvent(event);
}
void GlyphCellItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	//setCursor(Qt::OpenHandCursor);
	QGraphicsObject::mouseReleaseEvent(event);
}
void GlyphCellItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

	QColor color = Qt::blue;
	if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
		.length() < QApplication::startDragDistance()) {
		return;
	}

	QDrag *drag = new QDrag(event->widget());
	QMimeData *mime = new QMimeData;
	drag->setMimeData(mime);

	mime->setText(glyph->name());

	QStyleOptionGraphicsItem option;

	double grabwidth = 50;


	QRectF rect = path->boundingRect();
	QPixmap pixmap(grabwidth, grabwidth);
	pixmap.fill(Qt::white);

	double scalex = grabwidth / rect.width();
	double scaley = grabwidth / rect.height();

	double scale = qMin(scalex, scaley);

	QPainter painter(&pixmap);
	painter.translate(-scale * rect.x(), grabwidth + scale * rect.y());
	painter.scale(scale, -scale);
	painter.setRenderHint(QPainter::Antialiasing);



	path->paint(&painter, &option, 0);

	painter.end();

	pixmap.setMask(pixmap.createHeuristicMask());

	drag->setPixmap(pixmap);
	drag->setHotSpot(QPoint(0, grabwidth));

	Qt::DropAction dropAction = drag->exec();
	if (dropAction == Qt::MoveAction) {
		FontScene* cscene = (FontScene*)scene();
		cscene->removeItem(this);
		cscene->repositionItems();
	}

	setCursor(Qt::OpenHandCursor);
}
void GlyphCellItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
	//dragOver = false;

	if (event->mimeData()->hasText()) {
		QString glyphName = event->mimeData()->text();
		Glyph* originGlyph = glyph->font->glyphperName[glyphName];
		if (originGlyph != NULL) {
			int fromindex = glyph->font->glyphs.indexOf(originGlyph);
			int toindex = glyph->font->glyphs.indexOf(glyph);
			glyph->font->glyphs.move(fromindex, toindex);

			GlyphCellItem * cell = new GlyphCellItem();
			cell->setGlyph(originGlyph);

			FontScene* cscene = (FontScene*)scene();

			cscene->addItem(cell);
			cell->stackBefore(this);


			event->acceptProposedAction();

			//MdiChild* view=  (MdiChild*)scene()->views()[0];
			//view->refresh();


		}
	}

}
void GlyphCellItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QMenu menu;
	//QAction * scaleAct = new QAction("&Scale", this);	
	//connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
	FontScene* fontscene = (FontScene*)scene();
	auto font = glyph->font;

	menu.addAction("Duplicate");
	menu.addAction("Delete");
	QAction *a = menu.exec(event->screenPos());
	if (a != NULL) {
		if (a->text() == "Duplicate") {

			bool ok;
			QString text = QInputDialog::getText(nullptr, tr("Glyph Name"), tr("User name:"), QLineEdit::Normal, glyph->name() + ".ii", &ok);
			if (ok && !text.isEmpty()) {
				QString source = glyph->source();
				Glyph* newglyph = new Glyph(source, glyph->mp, glyph->font);
				newglyph->setName(text);
				newglyph->setUnicode(-1);
				int fromindex = glyph->font->glyphs.indexOf(glyph);
				font->glyphs.insert(fromindex + 1, newglyph);
				GlyphCellItem * cell = new GlyphCellItem();
				cell->setGlyph(newglyph);


				fontscene->addItem(cell);
				cell->stackBefore(this);
				this->stackBefore(cell);
				fontscene->repositionItems();
			}

		}
		else if (a->text() == "Delete") {
			int fromindex = glyph->font->glyphs.indexOf(glyph);

			font->glyphs.remove(fromindex);
			delete glyph;
			fontscene->removeItem(this);
			fontscene->repositionItems();
		}
	}

}




