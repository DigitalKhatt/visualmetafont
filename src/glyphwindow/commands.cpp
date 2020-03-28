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

#include "commands.h"
#include "glyph.hpp"

static const int changeGlyphPropertyCommandId = 1;



ChangeGlyphPropertyCommand::ChangeGlyphPropertyCommand(Glyph *glyph, const QString &propertyName, QVariant value, QUndoCommand *parent)
	: QUndoCommand(parent)
{

	setText(QObject::tr("Set %1's value").arg(propertyName));

	m_glyph = glyph;
	m_propertyName = propertyName;
	m_oldValue = glyph->property(propertyName.toLatin1());
	m_newValue = value;
}

void ChangeGlyphPropertyCommand::undo()
{

	QByteArray name = m_propertyName.toLatin1();
	m_glyph->setProperty(name, m_oldValue);

}

void ChangeGlyphPropertyCommand::redo()
{
	m_glyph->setProperty(m_propertyName.toLatin1(), m_newValue);
}

bool ChangeGlyphPropertyCommand::mergeWith(const QUndoCommand *command)
{
	if (command->id() != changeGlyphPropertyCommandId)
		return false;

	const ChangeGlyphPropertyCommand *other = static_cast<const ChangeGlyphPropertyCommand*>(command);
	if (m_propertyName != other->m_propertyName)
		return false;

	m_newValue = other->m_newValue;
	return true;
}

int ChangeGlyphPropertyCommand::id() const
{
	return changeGlyphPropertyCommandId;
}

GlyphSourceChangeCommand::GlyphSourceChangeCommand(Glyph *glyph, const QString &name, QString oldvalue, QString newvalue,
	bool structureChanged, QUndoCommand * parent) : QUndoCommand(parent)
{
	setText(QObject::tr(name.toLatin1()));

	m_glyph = glyph;
	m_oldValue = oldvalue;
	m_newValue = newvalue;
	m_structureChanged = structureChanged;
}
void GlyphSourceChangeCommand::undo()
{
	m_glyph->setSource(m_oldValue, m_structureChanged);
}

void GlyphSourceChangeCommand::redo()
{
	m_glyph->setSource(m_newValue, m_structureChanged);
}
GlyphParamsChangeCommand::GlyphParamsChangeCommand(Glyph *glyph, const QString &name, QMap<QString, QVariant> oldvalue, QMap<QString, QVariant> newvalue,
	QMap<int, QMap<int, Glyph::Knot> >old_controlledPaths, QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths, QUndoCommand *parent) : QUndoCommand(parent)
{
	setText(QObject::tr(name.toLatin1()));

	m_glyph = glyph;
	m_oldValue = oldvalue;
	m_newValue = newvalue;
	this->old_controlledPaths = old_controlledPaths;
	this->new_controlledPaths = new_controlledPaths;
	bypassRedo = true;
}
void GlyphParamsChangeCommand::undo()
{
	QMap<QString, QVariant>::iterator i;
	for (i = m_newValue.begin(); i != m_newValue.end(); ++i) {
		m_glyph->setProperty(i.key().toLatin1(), m_oldValue[i.key()]);
	}

	QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
	while (j.hasNext()) {
		j.next();
		QMapIterator<int, Glyph::Knot*> h(j.value());
		while (h.hasNext()) {
			h.next();
			*m_glyph->controlledPaths[j.key()][h.key()] = old_controlledPaths[j.key()][h.key()];
		}
	}

	m_glyph->setWidth(m_glyph->width());

}

void GlyphParamsChangeCommand::redo()
{
	if (bypassRedo) {
		bypassRedo = false;
		return;
	}

	QMap<QString, QVariant>::iterator i;
	for (i = m_newValue.begin(); i != m_newValue.end(); ++i) {
		m_glyph->setProperty(i.key().toLatin1(), i.value());
	}

	QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
	while (j.hasNext()) {
		j.next();
		QMapIterator<int, Glyph::Knot*> h(j.value());
		while (h.hasNext()) {
			h.next();
			*m_glyph->controlledPaths[j.key()][h.key()] = new_controlledPaths[j.key()][h.key()];
		}
	}

	m_glyph->setWidth(m_glyph->width());
}
GlyphPathsChangeCommand::GlyphPathsChangeCommand(Glyph *glyph, const QString &name,
	QMap<int, QMap<int, Glyph::Knot> >old_controlledPaths, QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths, QUndoCommand *parent) : QUndoCommand(parent)
{
	setText(QObject::tr(name.toLatin1()));

	m_glyph = glyph;

	this->old_controlledPaths = old_controlledPaths;
	this->new_controlledPaths = new_controlledPaths;
}
void GlyphPathsChangeCommand::undo()
{
	auto iter = m_glyph->controlledPaths.begin();
	while (iter != m_glyph->controlledPaths.end()) {
		auto iter2 = iter.value().begin();
		auto end = iter.value().end();
		end--;
		while (iter2 != end) {
			delete iter2.value();
			++iter2;
		}
		++iter;
	}

	m_glyph->controlledPaths.clear();

	QMapIterator<int, QMap<int, Glyph::Knot> > j(old_controlledPaths);
	while (j.hasNext()) {
		j.next();
		QMapIterator<int, Glyph::Knot> h(j.value());
		while (h.hasNext()) {
			h.next();
			m_glyph->controlledPaths[j.key()][h.key()] = new Glyph::Knot();
			*m_glyph->controlledPaths[j.key()][h.key()] = old_controlledPaths[j.key()][h.key()];
		}
	}

	m_glyph->isDirty = true;
	emit m_glyph->valueChanged("controlledPaths", true);

}

void GlyphPathsChangeCommand::redo()
{
	
	auto iter = m_glyph->controlledPaths.begin();
	while (iter != m_glyph->controlledPaths.end()) {
		auto iter2 = iter.value().begin();
		auto end = iter.value().end();
		end--;
		while (iter2 != end) {
			delete iter2.value();
			++iter2;
		}
		++iter;
	}

	m_glyph->controlledPaths.clear();

	QMapIterator<int, QMap<int, Glyph::Knot> > j(new_controlledPaths);
	while (j.hasNext()) {
		j.next();
		QMapIterator<int, Glyph::Knot> h(j.value());
		while (h.hasNext()) {
			h.next();
			m_glyph->controlledPaths[j.key()][h.key()] = new Glyph::Knot();
			*m_glyph->controlledPaths[j.key()][h.key()] = new_controlledPaths[j.key()][h.key()];
		}
	}

	m_glyph->isDirty = true;
	emit m_glyph->valueChanged("controlledPaths", true);
}
