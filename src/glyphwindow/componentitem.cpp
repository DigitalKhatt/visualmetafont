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

#include "componentitem.hpp"
#include <QBrush>
#include <QPen>

ComponentItem::ComponentItem(Glyph* glyph, Glyph* compglyp, QGraphicsItem * parent) : QGraphicsPathItem(parent) {

	m_glyph = glyph;
	m_compglyph = compglyp;

	QMatrix m;
	m.scale(1, -1);

	setMatrix(m);

	calculatePath();

	

	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
	setBrush(Qt::gray);
	setPen(Qt::NoPen);

	//setZValue(-5);

	
	
}
void ComponentItem::calculatePath() {

	QPainterPath compath = m_compglyph->getPath();
	Glyph::ComponentInfo info = m_glyph->components()[m_compglyph];

	QPointF newpos = QPointF(info.pos.x(), -info.pos.y());
	compath = info.transform.map(compath);
	setPath(compath);

	if (newpos != pos()) {
		setPos(newpos);
	}

}

ComponentItem::~ComponentItem() {
	
}

void ComponentItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

	QGraphicsPathItem::mouseMoveEvent(event);
}
QVariant ComponentItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	QPointF pair = QPointF(this->pos().x(),-this->pos().y());
	auto comps = m_glyph->components();
	switch (change) {
	case ItemPositionChange:
		//ensureVisible();
		break;
	case ItemPositionHasChanged:		
		comps[m_compglyph].pos = pair;
		m_glyph->setComponents(comps);
	default:
		break;
	};

	return QGraphicsItem::itemChange(change, value);
}
