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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include "glyph.hpp"



class ChangeGlyphPropertyCommand : public QUndoCommand
{
public:
	ChangeGlyphPropertyCommand(Glyph *glyph, const QString &propertyName, QVariant value, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
	bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	Glyph *m_glyph;    
    QString m_propertyName;
	QVariant m_oldValue;
	QVariant m_newValue;
};

class GlyphSourceChangeCommand : public QUndoCommand
{
public:
	GlyphSourceChangeCommand(Glyph *glyph, const QString &name, QString oldvalue, QString newvalue,bool structureChanged, QUndoCommand *parent = 0);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	//bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
	//int id() const Q_DECL_OVERRIDE;

private:
	Glyph *m_glyph;	
	QString m_oldValue;
	QString m_newValue;
	bool m_structureChanged;
	
};

class GlyphParamsChangeCommand : public QUndoCommand
{
public:
	GlyphParamsChangeCommand(Glyph *glyph, const QString &name, QMap<QString, QVariant> oldvalue, QMap<QString, QVariant> newvalue, 
		QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths, QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths,  QUndoCommand *parent = 0);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	//bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
	//int id() const Q_DECL_OVERRIDE;

private:
	Glyph *m_glyph;
	QMap<QString, QVariant>  m_oldValue;
	QMap<QString, QVariant>  m_newValue;
	QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths;
	QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths;
	bool bypassRedo;
};

class GlyphPathsChangeCommand : public QUndoCommand
{
public:
	GlyphPathsChangeCommand(Glyph *glyph, const QString &name,
		QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths, QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths, QUndoCommand *parent = 0);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	//bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
	//int id() const Q_DECL_OVERRIDE;

private:
	Glyph *m_glyph;	
	QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths;
	QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths;
	
};

#endif // COMMANDS_H
