/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#include "glyphwindow.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>

#include "font.hpp"
#include "OtLayout.h"


int main(int argc, char* argv[])
{

#ifndef __EMSCRIPTEN__
#if defined(_WIN32) && not defined (QT_SHARED_BUILD)
  Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif
#else
  Q_IMPORT_PLUGIN(QWasmIntegrationPlugin)
#endif

#ifndef QT_SHARED_BUILD
  Q_IMPORT_PLUGIN(QSvgIconPlugin)
  Q_IMPORT_PLUGIN(QSvgPlugin)
  Q_IMPORT_PLUGIN(QJpegPlugin)
#endif

  QApplication app(argc, argv);

  QTextEdit console;

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

