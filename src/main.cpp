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
#include "mainwindow.h"
#include <QtWidgets/QApplication>

#include "font.hpp"
#include "OtLayout.h"
/*
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtTemp4 w;
	w.show();
	return a.exec();
}*/

int mainold(int argc, char* argv[]) {
#ifdef _WIN32
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

	QApplication app(argc, argv);

	Font* font = new Font();
	if (!font->loadFile("medinafont.mp")) {
		return 1;
	}

	for (int i = 0; i < font->glyphs.length(); i++) {
		Glyph* glyph = font->glyphs[i];
		glyph->getEdge();
	}

	OtLayout* layout = new OtLayout(font->mp);

	return app.exec();
}

int main(int argc, char* argv[])
{

	//::SetProcessDPIAware();
	//QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	//QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);



#ifndef __EMSCRIPTEN__
#ifdef _WIN32
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif
#else
	Q_IMPORT_PLUGIN(QWasmIntegrationPlugin)
#endif

#ifdef _WIN32
		Q_IMPORT_PLUGIN(QSvgIconPlugin)
		Q_IMPORT_PLUGIN(QSvgPlugin)
#else
    Q_IMPORT_PLUGIN(QSvgIconPlugin)
    Q_IMPORT_PLUGIN(QSvgPlugin)
#endif
		Q_IMPORT_PLUGIN(QJpegPlugin)
		//	Q_IMPORT_PLUGIN(QGl)

		QApplication app(argc, argv);

	QTextEdit console;

	//new Q_DebugStream(std::cout, &console); //Redirect Console output to QTextEdit
	//new Q_DebugStream(std::cerr, &console); //Redirect Console output to QTextEdit

	//console.setWindowTitle("Console");
	//console.showMaximized();





	QScreen* srn = QApplication::screens().at(0);

	double ff = srn->physicalDotsPerInch();
	double gg = srn->physicalDotsPerInchX();
	double hh = srn->physicalDotsPerInchY();
	QSizeF ss = srn->physicalSize();

	QApplication::setOrganizationName("digitalkhatt");
	QApplication::setOrganizationDomain("digitalkhatt.org");
	QApplication::setApplicationName("visualmetafont");

	MainWindow mainWin;
	mainWin.showMaximized();

	mainWin.openLatestRecentFile();

	return app.exec();



}

