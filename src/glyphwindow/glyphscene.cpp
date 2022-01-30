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

#include "glyphscene.hpp"
#include "pairitem.hpp"
#include "commands.h"
#include <qmetaobject.h>
#include <QGraphicsSceneMouseEvent>
#include <qgraphicsview.h>
#include <knotcontrolleditem.hpp>
#include "qmessagebox.h"
#include "font.hpp"

GlyphScene::GlyphScene(QObject * parent) : QGraphicsScene(parent) {// QGraphicsScene(-1000, -1700, 3000, 2500, parent) {

	pointerPosition = new QLabel("Hello");
	ruler = NULL;
	myMode = MoveItem;
	ismoving = false;

	contour = NULL;
	image = NULL;
	guides = NULL;
	m_glyph = NULL;

	//connect(this, &QGraphicsScene::selectionChanged, this, &GlyphScene::selectionChangedSlot);
}
GlyphScene::~GlyphScene() {

}
void GlyphScene::selectionChangedSlot() {
	int i;
	i = 0;
}
void GlyphScene::setGlyph(Glyph * glyph)
{

	if (this->m_glyph) {
		disconnect(this->m_glyph, &Glyph::valueChanged, this, &GlyphScene::glyphValueChanged);
	}

	this->m_glyph = glyph;

	if (contour) {
		delete contour;
	}

	if (!components.isEmpty()) {
		for (int i = 0; i < components.length(); i++) {
			delete components[i];
		}
		components.clear();
	}

	contour = new ContourItem();
	contour->setPos(0, 0);
	addItem(contour);
	contour->setGlyph(glyph);

	connect(glyph, &Glyph::valueChanged, this, &GlyphScene::glyphValueChanged, Qt::UniqueConnection);

	bool imageEnabled = true;

	if (image != NULL) {
		imageEnabled = image->isEnabled();
		delete image;
	}

	image = new ImageItem(glyph);
	image->setEnabled(imageEnabled);
	addItem(image);


	Glyph::ImageInfo imageInfo = glyph->image();
	if (!imageInfo.path.isEmpty()) {
		loadImage(imageInfo);
	}

	Glyph::QHashGlyphComponentInfo localcomponents = glyph->components();
	Glyph::QHashGlyphComponentInfo::iterator comp;
	for (comp = localcomponents.begin(); comp != localcomponents.end(); ++comp) {
		Glyph* compglyph = comp.key();
		//QPainterPath compath = compglyph->getPath();	
		//Glyph::ComponentInfo info = comp.value();
		//compath = comp.value().transform.map(compath);

		ComponentItem* compitem = new ComponentItem(glyph, compglyph);
		//compitem->setPath(compath);
		//compitem->setPos(QPointF(info.pos.x(),-info.pos.y()));

		components.append(compitem);

		addItem(compitem);
	}

	if (guides)
		delete guides;

	guides = new GuidesItem(glyph);

	addItem(guides);

	views().first()->centerOn(contour);


}
void GlyphScene::setImageVisible(bool  visible) {

	if (visible)
		itemFlags = itemFlags | ItemFlags::ImageVisible;
	else
		itemFlags = itemFlags & ~ItemFlags::ImageVisible;

	if (image != NULL) {
		image->setVisible(visible);
	}

}
void GlyphScene::setImageEnable(bool  enable) {

	if (enable)
		itemFlags = itemFlags | ItemFlags::ImageEnabled;
	else
		itemFlags = itemFlags & ~ItemFlags::ImageEnabled;

	if (image != NULL) {
		image->setEnabled(enable);
	}

}

void GlyphScene::setFillEnable(bool  enable) {

	if (enable)
		itemFlags = itemFlags | ItemFlags::Fill;
	else
		itemFlags = itemFlags & ~ItemFlags::Fill;

	if (contour != NULL) {
		contour->setFillEnabled(enable);
	}

}
void GlyphScene::glyphValueChanged(QString name, bool structureChanged) {

	/*
	if (name == "image" || name == "imagetransform" || name == "source") {
	Glyph::ImageInfo imageInfo = glyph->property("image").value<Glyph::ImageInfo>();
	if (name == "image" || (name == "source" && structureChanged)) {
	if (!imageInfo.path.isEmpty())
	loadImage(imageInfo);
	}
	else {
	image->setPos(imageInfo.pos);
	image->setTransform(imageInfo.transform);
	}
	}*/

	if (ismoving)
		return;

	Glyph::ImageInfo imageInfo = m_glyph->image();

	if (image->path() == imageInfo.path) {
	    auto pos = imageInfo.pos;
	    if(m_glyph->getEdge() != nullptr){
		auto edge = m_glyph->getEdge();
		pos = pos + QPointF(edge->xpart,-edge->ypart);
	    }

		image->setPos(pos);
		image->setTransform(imageInfo.transform);
	}
	else if (imageInfo.path.isEmpty()) {
		image->setPath("");
		image->setPixmap(QPixmap());
	}
	else {
		loadImage(imageInfo);
	}

	contour->generateedge(m_glyph->getEdge(), structureChanged);

	guides->update();

	for (int i = 0; i < components.length(); i++) {
		components[i]->calculatePath();
	}

	update();
}
void GlyphScene::loadImage(Glyph::ImageInfo imageInfo) {

	QString imagePath = imageInfo.path;
	QFileInfo imageFileInfo{ imagePath };
	
	if (imageFileInfo.isRelative()) {
		QDir d = QFileInfo(this->m_glyph->font->path()).absoluteDir();
		QString absolute = d.absolutePath();
		imagePath = absolute + "/" + imageInfo.path;
	}
	
	QImageReader reader(imagePath);
	reader.setAutoTransform(true);
	const QImage newImage = reader.read();
	if (newImage.isNull()) {

		QMessageBox::information(this->views()[0], QGuiApplication::applicationDisplayName(),
			tr("Cannot load %1: %2")
			.arg(QDir::toNativeSeparators(imagePath), reader.errorString()));
	}

	image->setPath(imageInfo.path);
	image->setPixmap(QPixmap::fromImage(newImage));

	auto pos = imageInfo.pos;
	if(m_glyph->getEdge() != nullptr){
	    auto edge = m_glyph->getEdge();
	    pos = pos + QPointF(edge->xpart,-edge->ypart);
	}

	    image->setPos(pos);
	image->setTransform(imageInfo.transform);

}
void GlyphScene::setMode(Mode mode)
{
	myMode = mode;

	switch (myMode) {
	case Ruler:
	{
		views().first()->setDragMode(QGraphicsView::NoDrag);
		break;
	}
	case MoveItem: {
		delete ruler;
		ruler = NULL;
		views().first()->setDragMode(QGraphicsView::RubberBandDrag);
		break;
	}
	case AddPoint: {
		views().first()->setDragMode(QGraphicsView::NoDrag);
		QList<QGraphicsItem*> list = items();
		foreach(QGraphicsItem* item, list) {
			item->setSelected(false);
		}
		break;
	}
	}
}
void GlyphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	QString string = QString("%1, %2")
		.arg(mouseEvent->scenePos().x())
		.arg(mouseEvent->scenePos().y()); // Update the cursor position text

	pointerPosition->setText(string);

	if (!(mouseEvent->buttons() & Qt::LeftButton))
		return;

	if (myMode == Ruler && ruler != NULL) {
		QLineF newLine(ruler->line().p1(), mouseEvent->scenePos());
		ruler->setLine(newLine);
	}
	else if (myMode == MoveItem) {
		//QGraphicsScene::mouseMoveEvent(mouseEvent);



		QGraphicsItem *item = mouseGrabberItem();
		if (!item || item->isBlockedByModalPanel())
			return;


		/*for (int i = 0x1; i <= 0x10; i <<= 1) {
			Qt::MouseButton button = Qt::MouseButton(i);
			mouseEvent->setButtonDownPos(button, mouseGrabberButtonDownPos.value(button, item->d_ptr->genericMapFromScene(mouseEvent->scenePos(), mouseEvent->widget())));
			mouseEvent->setButtonDownScenePos(button, mouseGrabberButtonDownScenePos.value(button, mouseEvent->scenePos()));
			mouseEvent->setButtonDownScreenPos(button, mouseGrabberButtonDownScreenPos.value(button, mouseEvent->screenPos()));
		}*/
		mouseEvent->setPos(item->mapFromScene(mouseEvent->scenePos()));
		mouseEvent->setLastPos(item->mapFromScene(mouseEvent->lastScenePos()));

		QList<QGraphicsItem *> selectedItems;
		selectedItems = this->selectedItems();
		ismoving = true;
		wasMouseMoveEvent = true;
		foreach(QGraphicsItem *item, selectedItems) {

			sendEvent(item, mouseEvent);
		}
		ismoving = false;
		m_glyph->setWidth(m_glyph->width());
	}



}
void GlyphScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	wasMouseMoveEvent = false;

	//if (mouseEvent->button() != Qt::LeftButton)
	//	return;

	Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();

	bool shift = false;
	bool ctrl = false;
	bool alt = false;

	if (modifiers & Qt::ShiftModifier) {
		shift = true;
	}
	if (modifiers & Qt::ControlModifier) {
		ctrl = true;
	}
	if (modifiers & Qt::AltModifier) {
		alt = true;
	}



	switch (myMode) {
	case Ruler:
	{
		QLineF line = QLineF(mouseEvent->scenePos(), mouseEvent->scenePos());
		if (ruler == NULL) {
			ruler = new RulerItem(line);
			addItem(ruler);
		}
		else {
			ruler->setLine(line);
		}
		break;
	}
	case MoveItem:
	{

		if (shift) {
			QList<QGraphicsItem *> selectedItems;
			selectedItems = this->selectedItems();
			if (selectedItems.count() == 1) {
				KnotItem *knotPoint = qgraphicsitem_cast<KnotItem *>(selectedItems[0]);
				if (knotPoint != NULL) {
					KnotControlledItem *selectdpoint = qgraphicsitem_cast<KnotControlledItem *>(knotPoint->parentItem());
					if (selectdpoint != NULL) {
						//addPointAfterPoint(selectdpoint, mouseEvent->scenePos(),"left");
						mouseEvent->setAccepted(true);
						break;
					}

				}
			}
			else {
				//addnewPath(mouseEvent->scenePos());
				mouseEvent->setAccepted(true);
				break;
			}
		}

		m_oldValues.clear();
		m_newValues.clear();
		old_controlledPaths.clear();
		new_controlledPaths.clear();

		auto mo = m_glyph->metaObject();
		for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
			m_oldValues[mo->property(i).name()] = mo->property(i).read(m_glyph);
		}

		QList<QByteArray> dynamicProperties = m_glyph->dynamicPropertyNames();
		for (int i = 0; i < dynamicProperties.length(); i++) {
			QByteArray propname = dynamicProperties[i];
			QVariant val = m_glyph->property(propname);
			m_oldValues[propname] = val;

		}

		QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
		while (j.hasNext()) {
			j.next();
			QMapIterator<int, Glyph::Knot*> h(j.value());
			while (h.hasNext()) {
				h.next();
				old_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
			}
		}





		connect(m_glyph, &Glyph::valueChanged, this, &GlyphScene::recordGlyphChange);

		QGraphicsScene::mousePressEvent(mouseEvent);
		break;
	}
	case AddPoint: {
		QList<QGraphicsItem *> selectedItems;
		selectedItems = this->selectedItems();
		if (selectedItems.count() == 1) {
			KnotItem *knotPoint = qgraphicsitem_cast<KnotItem *>(selectedItems[0]);
			if (knotPoint != NULL) {
				KnotControlledItem *selectdpoint = qgraphicsitem_cast<KnotControlledItem *>(knotPoint->parentItem());
				if (selectdpoint != NULL) {
					addPointAfterPoint(selectdpoint, mouseEvent->scenePos(), "");
					break;
				}

			}
		}

		QGraphicsScene::mousePressEvent(mouseEvent);

		break;


	}
	}



}
void GlyphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

	Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();

	bool shift = false;
	bool ctrl = false;
	bool alt = false;

	if (modifiers & Qt::ShiftModifier) {
		shift = true;
	}
	if (modifiers & Qt::ControlModifier) {
		ctrl = true;
	}
	if (modifiers & Qt::AltModifier) {
		alt = true;
	}

	if (ruler != 0 && myMode == Ruler) {
	}
	else {

		if (shift) {
			QList<QGraphicsItem *> selectedItems;
			selectedItems = this->selectedItems();
			QPointF begin = mouseEvent->buttonDownScenePos(Qt::LeftButton);
			QPointF end = mouseEvent->scenePos();
			QLineF line(begin, end);
			QString dir;
			if (line.length() > 20) {
				if (line.angle() > -45 && line.angle() <= 45) {
					dir = "right";
				}
				else if (line.angle() > 45 && line.angle() <= 135) {
					dir = "up";
				}
				else if (line.angle() > 135 && line.angle() <= 225) {
					dir = "left";
				}
				else {
					dir = "down";
				}
			}
			if (selectedItems.count() == 1) {
				KnotItem *knotPoint = qgraphicsitem_cast<KnotItem *>(selectedItems[0]);
				if (knotPoint != NULL) {
					KnotControlledItem *selectdpoint = qgraphicsitem_cast<KnotControlledItem *>(knotPoint->parentItem());
					if (selectdpoint != NULL) {


						addPointAfterPoint(selectdpoint, begin, dir);
						mouseEvent->setAccepted(true);
						return;
					}

				}
			}
			else {
				addnewPath(begin, dir);
				mouseEvent->setAccepted(true);
				return;
			}
		}

		disconnect(m_glyph, &Glyph::valueChanged, this, &GlyphScene::recordGlyphChange);

		if (wasMouseMoveEvent) {

			QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
			while (j.hasNext()) {
				j.next();
				QMapIterator<int, Glyph::Knot*> h(j.value());
				while (h.hasNext()) {
					h.next();
					new_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
				}
			}

			if (!m_newValues.isEmpty() || !new_controlledPaths.isEmpty()) {
				GlyphParamsChangeCommand* command = new GlyphParamsChangeCommand(m_glyph, "Item(s) Moved", m_oldValues, m_newValues, old_controlledPaths, new_controlledPaths);
				m_glyph->undoStack()->push(command);
			}
		}
	}

	wasMouseMoveEvent = false;

	QGraphicsScene::mouseReleaseEvent(mouseEvent);

}
void GlyphScene::recordGlyphChange(QString name) {
	if (name == "imagetransform") {
		name = "image";
	}
	m_newValues[name] = m_glyph->property(name.toLatin1());
}
void GlyphScene::addPointAfterPoint(KnotControlledItem* point, QPointF newpoint, QString dir) {
	int numpath = point->m_numsubpath;
	int numpoint = point->m_numpoint;

	if (m_glyph->controlledPaths.contains(numpath) && m_glyph->controlledPaths[numpath].contains(numpoint)) {

		QMap<int, Glyph::Knot*>  newcontrolledPaths;
		auto controlledPath = m_glyph->controlledPaths[numpath];
		auto iterator = controlledPath.begin();
		while (iterator != controlledPath.end()) {
			if (iterator.key() < numpoint) {
				newcontrolledPaths[iterator.key()] = controlledPath[iterator.key()];
			}
			else if (iterator.key() == numpoint) {

				newcontrolledPaths[iterator.key()] = controlledPath[iterator.key()];
				Glyph::Knot* newknot = new Glyph::Knot();
				newknot->isConstant = true;
				Glyph::KnotEntryExit left = {};
				Glyph::KnotEntryExit right = {};
				left.isControlConstant = true;
        left.jointtype = Glyph::path_join_tension;
				left.x = 0;
				left.y = 1;
				if (!dir.isNull()) {
					left.type = Glyph::mpgui_given;
					left.value = dir;
				}
				else {
					left.type = Glyph::mpgui_open;
				}

				right.isControlConstant = true;
        right.jointtype = Glyph::path_join_tension;
				right.x = 0;
				right.y = 1;
				right.type = Glyph::mpgui_open;
				newknot->leftValue = left;
				newknot->rightValue = right;
				newknot->x = newpoint.x();
				newknot->y = -newpoint.y();

				newcontrolledPaths[iterator.key() + 1] = newknot;
			}
			else {
				newcontrolledPaths[iterator.key() + 1] = controlledPath[iterator.key()];
			}
			++iterator;
		}

		QMap<int, QMap<int, Glyph::Knot> >  old_controlledPaths;
		QMap<int, QMap<int, Glyph::Knot> >  new_controlledPaths;

		QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
		while (j.hasNext()) {
			j.next();
			QMapIterator<int, Glyph::Knot*> h(j.value());
			while (h.hasNext()) {
				h.next();
				old_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
			}
		}

		m_glyph->controlledPaths[numpath] = newcontrolledPaths;

		QMapIterator<int, QMap<int, Glyph::Knot*> > jj(m_glyph->controlledPaths);
		while (jj.hasNext()) {
			jj.next();
			QMapIterator<int, Glyph::Knot*> h(jj.value());
			while (h.hasNext()) {
				h.next();
				new_controlledPaths[jj.key()][h.key()] = *m_glyph->controlledPaths[jj.key()][h.key()];
			}
		}


		GlyphPathsChangeCommand* command = new GlyphPathsChangeCommand(m_glyph, "Paths(s) changed", old_controlledPaths, new_controlledPaths);
		m_glyph->undoStack()->push(command);



		contour->knotControlledItems[numpath][numpoint + 1]->incurve->setSelected(true);

	}
}
void GlyphScene::addnewPath(QPointF newpoint, QString dir) {

	QMap<int, Glyph::Knot*> newpath;

	Glyph::Knot* newknot = new Glyph::Knot();
	newknot->isConstant = true;
	Glyph::KnotEntryExit left = {};
	Glyph::KnotEntryExit right = {};
	left.isControlConstant = true;
	left.x = 0;
	left.y = 1;
	if (!dir.isNull()) {
		left.type = Glyph::mpgui_given;
		left.value = dir;
	}
	else {
		left.type = Glyph::mpgui_open;
	}
	right.isControlConstant = true;
	right.x = 0;
	right.y = 1;
	right.type = Glyph::mpgui_open;
	newknot->leftValue = left;
	newknot->rightValue = right;
	newknot->x = newpoint.x();
	newknot->y = -newpoint.y();

	int numpath = m_glyph->controlledPaths.size();
	newpath[0] = newknot;
	newpath[1] = newknot;

	m_glyph->controlledPathNames[numpath] = "fill";


	QMap<int, QMap<int, Glyph::Knot> >  old_controlledPaths;
	QMap<int, QMap<int, Glyph::Knot> >  new_controlledPaths;

	QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
	while (j.hasNext()) {
		j.next();
		QMapIterator<int, Glyph::Knot*> h(j.value());
		while (h.hasNext()) {
			h.next();
			old_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
		}
	}



	m_glyph->controlledPaths[numpath] = newpath;

	QMapIterator<int, QMap<int, Glyph::Knot*> > jj(m_glyph->controlledPaths);
	while (jj.hasNext()) {
		jj.next();
		QMapIterator<int, Glyph::Knot*> h(jj.value());
		while (h.hasNext()) {
			h.next();
			new_controlledPaths[jj.key()][h.key()] = *m_glyph->controlledPaths[jj.key()][h.key()];
		}
	}


	GlyphPathsChangeCommand* command = new GlyphPathsChangeCommand(m_glyph, "Paths(s) changed", old_controlledPaths, new_controlledPaths);
	m_glyph->undoStack()->push(command);



	contour->knotControlledItems[numpath][0]->incurve->setSelected(true);
}

