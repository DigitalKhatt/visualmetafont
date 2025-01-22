/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>


#include "mdichild.h"
#include "glyphview.hpp"
#include "glyphcellitem.hpp"
#include "pairitem.hpp"





MdiChild::MdiChild() {
	fontscene = nullptr;
	curFile = "";
	setAttribute(Qt::WA_DeleteOnClose);
	isUntitled = true;
	//setViewport(new QGLWidget);
	setRenderHint(QPainter::Antialiasing);
}

MdiChild::~MdiChild() {
}

void MdiChild::newFile()
{
	static int sequenceNumber = 1;

	isUntitled = true;
	curFile = tr("document%1.txt").arg(sequenceNumber++);
	setWindowTitle(curFile + "[*]");

	//connect(document(), &QTextDocument::contentsChanged,
   //         this, &MdiChild::documentWasModified);
}

bool MdiChild::loadFile(const QString &fileName)
{


	font = new Font(this);

	if (!font->loadFile(fileName)) {
		QMessageBox::warning(this, tr("MDI"),
			tr("Cannot read file %1")
			.arg(fileName));
		//.arg(file.errorString()));
		return false;
	}


	fontscene = new FontScene(this);
	setScene(fontscene);
	for (int i = 0; i < font->glyphs.length(); i++) {
		Glyph* glyph = font->glyphs[i];
		GlyphCellItem * cell = new GlyphCellItem();
		cell->setGlyph(glyph);
		fontscene->addItem(cell);
	}




	QApplication::restoreOverrideCursor();

	setCurrentFile(fileName);

	//connect(document(), &QTextDocument::contentsChanged,
	//        this, &MdiChild::documentWasModified);



	QVariant factor = QSettings().value("FontViewScaleFactor");

	if (factor.isNull()) {
		scaleView(0.18);
	}
	else {
		scaleView(factor.toDouble());
	}




	return true;
}

bool MdiChild::save()
{
	if (isUntitled) {
		return saveAs();
	}
	else {
		return saveFile(curFile);
	}
}

bool MdiChild::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
		curFile);
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	bool ret = font->saveFile();
	QApplication::restoreOverrideCursor();

	//setCurrentFile(fileName);
	return ret;
}

QString MdiChild::userFriendlyCurrentFile()
{
	return strippedName(curFile);
}

void MdiChild::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) {
		event->accept();
	}
	else {
		event->ignore();
	}
}
void MdiChild::resizeEvent(QResizeEvent *event) {
	if (fontscene != nullptr) {
		fontscene->repositionItems();
	}
	
}


void MdiChild::documentWasModified()
{
	//setWindowModified(document()->isModified());
}

bool MdiChild::maybeSave()
{
	//if (!document()->isModified())
	return true;
	const QMessageBox::StandardButton ret
		= QMessageBox::warning(this, tr("MDI"),
			tr("'%1' has been modified.\n"
				"Do you want to save your changes?")
			.arg(userFriendlyCurrentFile()),
			QMessageBox::Save | QMessageBox::Discard
			| QMessageBox::Cancel);
	switch (ret) {
	case QMessageBox::Save:
		return save();
	case QMessageBox::Cancel:
		return false;
	default:
		break;
	}
	return true;
}

void MdiChild::setCurrentFile(const QString &fileName)
{
	curFile = QFileInfo(fileName).canonicalFilePath();
	isUntitled = false;
	//document()->setModified(false);
	//setWindowModified(false);
	setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString MdiChild::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).absoluteFilePath();
}
void MdiChild::scaleView(qreal scaleFactor)
{
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.01 || factor > 4)
		return;

	QSettings().setValue("FontViewScaleFactor", factor);

	scale(scaleFactor, scaleFactor);

	fontscene->repositionItems();

	auto list = fontscene->selectedItems();

	if (list.size() > 0) {
		auto item = fontscene->selectedItems().at(0);

		if (item) {
			centerOn(item);
		}
	}
}



void MdiChild::zoomIn()
{
	scaleView(qreal(1.2));
}

void MdiChild::zoomOut()
{
	scaleView(1 / qreal(1.5));
}
void MdiChild::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Plus:
		zoomIn();
		break;
	case Qt::Key_Minus:
		zoomOut();
		break;
	case Qt::Key_Space:
	default:
		QGraphicsView::keyPressEvent(event);
	}
}
void MdiChild::refresh() {
	fontscene = new FontScene(this);
	setScene(fontscene);
	for (int i = 0; i < font->glyphs.length(); i++) {
		Glyph* glyph = font->glyphs[i];
		GlyphCellItem * cell = new GlyphCellItem();
		cell->setGlyph(glyph);
		fontscene->addItem(cell);
	}
	invalidateScene();

}

