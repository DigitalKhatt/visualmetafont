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

#include "glyphwindow.h"
#include "font.hpp"
#include "glyphparametercontroller.hpp"
#include "commands.h"
#include <QtWidgets>


GlyphWindow::GlyphWindow(Glyph* glyph)
{
	setAttribute(Qt::WA_DeleteOnClose);

	undoStack = glyph->undoStack();
	//glyph->setSource(glyph->source()); 

	this->glyph = glyph;

	connect(glyph, &Glyph::valueChanged,this, &GlyphWindow::glyphChanged);

	glyphView = new GlyphView(glyph);

	scene = glyphView->getScene();

	createActions();
	createStatusBar();
	createDockWindows();

	this->setCentralWidget(glyphView);

	this->setWindowTitle(glyph->name() + "[*]");


	glyphView->setFocus();

	statusBar()->addPermanentWidget(scene->pointerPosition);
}

void GlyphWindow::executeCommands(void)
{
	//glyph->setSource(mpostEdit->toPlainText().toLatin1());

	GlyphSourceChangeCommand* command = new GlyphSourceChangeCommand(glyph, "Source Changed", glyph->source(), mpostEdit->toPlainText().toLatin1(), true);
	glyph->undoStack()->push(command);	

	glyph->getEdge();

	logOutput->appendPlainText(glyph->getLog());
	termOutput->appendPlainText(glyph->getError());

	//glyphView->setGlyph(glyph);
}

void GlyphWindow::glyphChanged(QString name)
{
	setWindowModified(true);
	//int pos = mpostEdit->textCursor().position();

	auto pos = mpostEdit->verticalScrollBar()->value();
	mpostEdit->setPlainText(glyph->source());

	//auto cursor = mpostEdit->textCursor();

	//cursor.setPosition(pos);

	//mpostEdit->setTextCursor(cursor);
	mpostEdit->verticalScrollBar()->setValue(pos - 1);

	pos = mpostEdit->verticalScrollBar()->value();

}
void GlyphWindow::createDockWindows()
{
	metapostCode = new QDockWidget(tr("MetaFont Code"), this);
	metapostCode->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
	
	QFont textEditFont("DejaVu Sans Mono");

  //textEditFont.setPointSize(20);

	mpostEdit = new QPlainTextEdit(metapostCode);
	mpostEdit->setFont(textEditFont);

	executeButton = new QPushButton("&Execute", metapostCode);
	connect(executeButton, SIGNAL(clicked()), this, SLOT(executeCommands()));


	QVBoxLayout* mpostLayout = new QVBoxLayout;
	mpostLayout->addWidget(mpostEdit);
	mpostLayout->addWidget(executeButton);

	QWidget* mpostPage = new QWidget(metapostCode);
	mpostPage->setLayout(mpostLayout);

# define CREATE(x) \
	x = new QPlainTextEdit ; \
	x -> setReadOnly ( true ) ; \
	x -> setFont ( textEditFont ) ;

	CREATE(logOutput)
		CREATE(termOutput)
		CREATE(psOutput)
		CREATE(svgOutput)

# undef CREATE
	
	tabWidget = new QTabWidget(metapostCode);
	tabWidget->addTab(mpostPage, "Input");
	tabWidget->addTab(logOutput, "Log");
	tabWidget->addTab(termOutput, "Term");

	mpostEdit->setPlainText(glyph->source());

	

	metapostCode->setWidget(tabWidget);
	addDockWidget(Qt::LeftDockWidgetArea, metapostCode);
	viewMenu->addAction(metapostCode->toggleViewAction());

	createUndoView();
	generatePropertiesView();

	tabifyDockWidget(dockUndo, dockProprties);
	
}
void GlyphWindow::showEvent(QShowEvent *event) {
	if (tabWidget != NULL) {
		//tabWidget->setMinimumWidth(this->width() / 3);
		//tabWidget->resize(this->width() / 3, tabWidget->height());
	}
	
}
void GlyphWindow::resizeEvent(QResizeEvent *event) {
	if (tabWidget != NULL) {
		this->resizeDocks({ metapostCode ,dockUndo }, { this->width() / 4 ,this->width() / 4 }, Qt::Horizontal);
		//tabWidget->setMinimumWidth(this->width() / 3);
		//tabWidget->resize(this->width() / 3, tabWidget->height());
	}

}
void GlyphWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void GlyphWindow::createActions()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileToolBar = addToolBar(tr("File"));

	const QIcon importIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
	QAction *importImage = new QAction(importIcon, tr("&Import Image"), this);
	importImage->setShortcuts(QKeySequence::Open);
	importImage->setStatusTip(tr("Import image as background"));
	connect(importImage, &QAction::triggered, this, &GlyphWindow::importImage);
	fileMenu->addAction(importImage);
	fileToolBar->addAction(importImage);

	const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
	QAction *saveAct = new QAction(saveIcon, tr("&Save..."), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save the current form letter"));
	connect(saveAct, &QAction::triggered, this, &GlyphWindow::save);
	fileMenu->addAction(saveAct);
	fileToolBar->addAction(saveAct);

	const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/images/print.png"));
	QAction *printAct = new QAction(printIcon, tr("&Print..."), this);
	printAct->setShortcuts(QKeySequence::Print);
	printAct->setStatusTip(tr("Print the current form letter"));
	//connect(printAct, &QAction::triggered, this, &MainWindow::print);
	fileMenu->addAction(printAct);
	fileToolBar->addAction(printAct);

	fileMenu->addSeparator();

	QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
	quitAct->setShortcuts(QKeySequence::Quit);
	quitAct->setStatusTip(tr("Quit the application"));

	QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
	editToolBar = addToolBar(tr("Edit"));

	QAction *undoAction = undoStack->createUndoAction(this);
	QAction *redoAction = undoStack->createRedoAction(this);

	
	undoAction->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png")));	
	redoAction->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/images/redo.png")));
	editMenu->addAction(undoAction);
	editToolBar->addAction(undoAction);
	editMenu->addAction(redoAction);
	editToolBar->addAction(redoAction);

	viewMenu = menuBar()->addMenu(tr("&View"));

	menuBar()->addSeparator();

	itemMenu = menuBar()->addMenu(tr("&Item"));

	QAction* enableimage = itemMenu->addAction(tr("&Enable Image"), this, &GlyphWindow::enableImge);
	enableimage->setCheckable(true);
	enableimage->setChecked(true);

	QAction* visibleimage = itemMenu->addAction(tr("&Visible Image"), this, &GlyphWindow::imageVisible);
	visibleimage->setCheckable(true);
	visibleimage->setChecked(true);

	QAction* enableFill = itemMenu->addAction(tr("&Enable Fill"), this, &GlyphWindow::enableFill);
	enableFill->setCheckable(true);
	enableFill->setChecked(false);

	


	menuBar()->addSeparator();

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

	QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &GlyphWindow::about);
	aboutAct->setStatusTip(tr("Show the application's About box"));

	QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

	QToolButton *pointerButton = new QToolButton;
	pointerButton->setCheckable(true);
	pointerButton->setChecked(true);
	pointerButton->setIcon(QIcon(":/images/pointer.png"));
	QToolButton *linePointerButton = new QToolButton;
	linePointerButton->setCheckable(true);
	linePointerButton->setIcon(QIcon(":/images/linepointer.png"));

	auto tt = QImageReader::supportedImageFormats();

	//QToolButton *addPointButton = new QToolButton;
	//addPointButton->setCheckable(true);
	//addPointButton->setIcon(QIcon(":/images/curve.svg"));

	pointerTypeGroup = new QButtonGroup(this);
	pointerTypeGroup->addButton(pointerButton, int(GlyphScene::MoveItem));
	pointerTypeGroup->addButton(linePointerButton, int(GlyphScene::Ruler));
	//pointerTypeGroup->addButton(addPointButton, int(GlyphScene::AddPoint));
	connect(pointerTypeGroup, SIGNAL(buttonClicked(int)),
		this, SLOT(pointerGroupClicked(int)));

	
	pointerToolbar = addToolBar(tr("Pointer type"));
	pointerToolbar->addWidget(pointerButton);
	pointerToolbar->addWidget(linePointerButton);	
	//pointerToolbar->addWidget(addPointButton);
}
void GlyphWindow::pointerGroupClicked(int)
{
	scene->setMode(GlyphScene::Mode(pointerTypeGroup->checkedId()));
}
void GlyphWindow::enableImge(bool checked) {
	scene->setImageEnable(checked);
}
void GlyphWindow::imageVisible(bool visible) {
	scene->setImageVisible(visible);
}
void GlyphWindow::enableFill(bool enable) {
	scene->setFillEnable(enable);
}

void GlyphWindow::fillContour(bool fill) {

}
void GlyphWindow::about()
{
	QMessageBox::about(this, tr("About Dock Widgets"),
		tr("The <b>Dock Widgets</b> example demonstrates how to "
			"use Qt's dock widgets. You can enter your own text, "
			"click a customer to add a customer name and "
			"address, and click standard paragraphs to add them."));
}
void GlyphWindow::importImage()
{
	QFileDialog dialog(this, tr("Import Image"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}
void GlyphWindow::initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
	static bool firstDialog = true;

	if (firstDialog) {
		firstDialog = false;
		QSettings settings;
		QString lastPictureLocation = settings.value("LastPictureLocation").value<QString>();
		if (lastPictureLocation == "") {
			const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
			dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
		}
		else {
			dialog.setDirectory(lastPictureLocation);
		}
	}

	QStringList mimeTypeFilters;
	const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
		? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
	foreach(const QByteArray &mimeTypeName, supportedMimeTypes)
		mimeTypeFilters.append(mimeTypeName);
	mimeTypeFilters.sort();
	dialog.setMimeTypeFilters(mimeTypeFilters);
	dialog.selectMimeTypeFilter("image/jpeg");
	if (acceptMode == QFileDialog::AcceptSave)
		dialog.setDefaultSuffix("jpg");
}
bool GlyphWindow::loadFile(const QString &fileName)
{
	QImageReader reader(fileName);
	reader.setAutoTransform(true);
	const QImage newImage = reader.read();
	if (newImage.isNull()) {
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Cannot load %1: %2")
			.arg(QDir::toNativeSeparators(fileName), reader.errorString()));
		return false;
	}

	

	QFileInfo imageFileInfo{ fileName };  //QFileInfo(fileName).absolutePath()
	Glyph::ImageInfo imageInfo;
	if (!imageFileInfo.isRelative()) {
		QDir d = QFileInfo(glyph->font->path()).absoluteDir();
		QString relPath = d.relativeFilePath(imageFileInfo.absoluteFilePath());
		if (!relPath.startsWith("..")) {
			imageInfo.path = relPath;
		}
	}
	else {
		imageInfo.path = fileName;
	}

	QSettings settings;

	settings.setValue("LastPictureLocation", imageFileInfo.absolutePath());	

	QVariant var;
	var.setValue(imageInfo);
	glyph->setPropertyWithUndo("image", var);
	

	//const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
	//	.arg(QDir::toNativeSeparators(fileName)).arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
	//statusBar()->showMessage(message);

	return true;
}
bool GlyphWindow::save() {
	QApplication::setOverrideCursor(Qt::WaitCursor);
	bool ret =  glyph->font->saveFile();
	QApplication::restoreOverrideCursor();
	setWindowModified(false);
	return ret;
}
void GlyphWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) {
		writeSettings();
		event->accept();
	}
	else {
		event->ignore();
	}
}
bool GlyphWindow::maybeSave()
{
	if (!isWindowModified())
		return true;
	const QMessageBox::StandardButton ret
		= QMessageBox::warning(this, tr("Application"),
			tr("The document has been modified.\n"
				"Do you want to save your changes?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
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
void GlyphWindow::writeSettings()
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settings.setValue("geometry", saveGeometry());
}
void GlyphWindow::createUndoView()
{	

	dockUndo = new QDockWidget(this);
	dockUndo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
	dockUndo->setObjectName(QStringLiteral("dockWidget"));
	QWidget* dockWidgetContents = new QWidget();
	dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
	QVBoxLayout* vboxLayout1 = new QVBoxLayout(dockWidgetContents);
	vboxLayout1->setSpacing(4);
	vboxLayout1->setContentsMargins(0, 0, 0, 0);
	vboxLayout1->setObjectName(QStringLiteral("vboxLayout1"));
	QHBoxLayout* hboxLayout = new QHBoxLayout();

	hboxLayout->setSpacing(6);

	hboxLayout->setContentsMargins(0, 0, 0, 0);

	hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
	QSpacerItem* spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	hboxLayout->addItem(spacerItem);

	QLabel * label = new QLabel(dockWidgetContents);
	label->setObjectName(QStringLiteral("label"));
	label->setText("Undo limit");

	hboxLayout->addWidget(label);

	QSpinBox* undoLimit = new QSpinBox(dockWidgetContents);
	undoLimit->setObjectName(QStringLiteral("undoLimit"));

	hboxLayout->addWidget(undoLimit);


	vboxLayout1->addLayout(hboxLayout);

	undoView = new QUndoView(dockWidgetContents);
	undoView->setObjectName(QStringLiteral("undoView"));	
	undoView->setCleanIcon(QIcon(":/images/ok.png"));
	undoView->setStack(undoStack);

	vboxLayout1->addWidget(undoView);

	dockUndo->setWidget(dockWidgetContents);
	addDockWidget(Qt::RightDockWidgetArea, dockUndo);
	dockUndo->setWindowTitle("Undo Stack");

	viewMenu->addAction(dockUndo->toggleViewAction());
}
void GlyphWindow::generatePropertiesView() {
	
	dockProprties = new QDockWidget(tr("Proprties"), this);
	dockProprties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	
	addDockWidget(Qt::RightDockWidgetArea, dockProprties);
	dockProprties->setWindowTitle("Proprties");

	viewMenu->addAction(dockProprties->toggleViewAction());

	GlyphParameterController * controller = new GlyphParameterController(glyph, this);

	
	QScrollArea *scroll3 = new QScrollArea();
	scroll3->setWidgetResizable(true);
	scroll3->setWidget(controller);	

	dockProprties->setWidget(scroll3);

}
