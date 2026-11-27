/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#include "componentitem.hpp"
#include <QBrush>
#include <QPen>

ComponentItem::ComponentItem(Glyph* glyph, Glyph* compglyp, QGraphicsItem * parent) : QGraphicsPathItem(parent) {

	m_glyph = glyph;
	m_compglyph = compglyp;

	QTransform m;
	m.scale(1, -1);

	setTransform(m);

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
