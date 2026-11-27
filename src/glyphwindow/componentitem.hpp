/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef COMPONENTITEM_HPP
#define COMPONENTITEM_HPP
#include <QGraphicsPathItem>
#include "glyph.hpp"

class ComponentItem : public QGraphicsPathItem {	

public:
	ComponentItem(Glyph* glyph, Glyph* compglyp, QGraphicsItem * parent = Q_NULLPTR);
	~ComponentItem();

protected:	
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:
	Glyph* m_glyph;
	Glyph* m_compglyph;

public:
	void calculatePath();
};

#endif // COMPONENTITEM_HPP