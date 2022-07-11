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

#include "hb-ot-layout-gsub-table.hh"
#include "LayoutWindow.h"


#include <QtWidgets>

#include <QTreeView>
#include <QFile>


#include "JustificationContext.h"

//#include "hb-font.hh"
//#include "hb-buffer.hh"
//#include "hb-ft.hh"

#include "font.hpp"

#include "glyph.hpp"




#include "GraphicsViewAdjustment.h"
#include "GraphicsSceneAdjustment.h"

#include "qurantext/quran.h"
//#include <QGLWidget>

#include "GlyphItem.h"

#include "Lookup.h"
#include "GlyphVis.h"
#include "qpoint.h"
#include "automedina/automedina.h"

#include <vector>

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"

#include <iostream>
#include <QPrinter>

#include <math.h> 

#include "to_opentype.h"


//#include "hb.hh"
//#include "hb-ot-shape.hh"
//#include "hb-ot-layout-gsub-table.hh"








//#include "hb-buffer-private.hh"



static hb_buffer_t* copyandreverse_buffer(hb_buffer_t* src)
{
  hb_buffer_t* dst = hb_buffer_create();
  hb_segment_properties_t props;
  hb_buffer_get_segment_properties(src, &props);
  hb_buffer_set_segment_properties(dst, &props);
  hb_buffer_set_flags(dst, hb_buffer_get_flags(src));
  hb_buffer_set_cluster_level(dst, hb_buffer_get_cluster_level(src));
  hb_buffer_append(dst, src, 0, -1);
  dst->reverse();

  return dst;
}

LayoutWindow::LayoutWindow(Font* font, QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{

  setAttribute(Qt::WA_DeleteOnClose);
  this->m_font = font;

  m_graphicsView = new GraphicsViewAdjustment(this);

  // 1 em = 50 pixel in screen
  qreal scale = (1.0 / (1000 << OtLayout::SCALEBY)) * 50;

  m_graphicsView->scale(scale, scale);

  m_graphicsScene = new GraphicsSceneAdjustment(this);

  m_graphicsView->setScene(m_graphicsScene);


  createActions();
  createDockWindows();

  setQutranText(1);

  integerSpinBox->setValue(3);

}

LayoutWindow::~LayoutWindow()
{
}

void LayoutWindow::createActions()
{
  QPushButton* medinabutton = new QPushButton(tr("&Medina Pages"));
  medinabutton->setCheckable(true);
  medinabutton->setChecked(true);

  QPushButton* texbutton = new QPushButton(tr("&TeX Pages"));
  texbutton->setCheckable(true);
  texbutton->setChecked(false);

  auto typeGroup = new QButtonGroup(this);
  typeGroup->addButton(medinabutton, 1);
  typeGroup->addButton(texbutton, 2);

  connect(typeGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::idClicked), this, &LayoutWindow::setQutranText);

  auto pagesToolbar = addToolBar(tr("Pages type"));
  pagesToolbar->addWidget(medinabutton);
  pagesToolbar->addWidget(texbutton);

  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileToolBar = addToolBar(tr("File"));

  const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* saveAct = new QAction(saveIcon, tr("&Save Positioning Adjustment..."), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the current form letter"));
  connect(saveAct, &QAction::triggered, this, &LayoutWindow::save);
  fileMenu->addAction(saveAct);
  fileToolBar->addAction(saveAct);

  const QIcon otfIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* otfAct = new QAction(otfIcon, tr("&Generate OpenTpe font"), this);
  //otfAct->setShortcuts(QKeySequence::Save);
  otfAct->setStatusTip(tr("Generate OpenTpe font"));
  connect(otfAct, &QAction::triggered, this, &LayoutWindow::generateOpenType);
  fileMenu->addAction(otfAct);
  //fileToolBar->addAction(otfAct);

  const QIcon otfcff2Icon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* otfcff2Act = new QAction(otfcff2Icon, tr("&Generate OpenType CFF2 Standard"), this);
  //otfcff2Act->setShortcuts(QKeySequence::Save);
  otfcff2Act->setStatusTip(tr("Generate OpenTpe CFF2 Standard"));
  connect(otfcff2Act, &QAction::triggered, this, &LayoutWindow::generateOpenTypeCff2Standard);
  fileMenu->addAction(otfcff2Act);

  otfcff2Act = new QAction(otfcff2Icon, tr("&Generate OpenType CFF2 Extended"), this);
  //otfcff2Act->setShortcuts(QKeySequence::Save);
  otfcff2Act->setStatusTip(tr("Generate OpenTpe CFF2 Extended"));
  connect(otfcff2Act, &QAction::triggered, this, &LayoutWindow::generateOpenTypeCff2Extended);
  fileMenu->addAction(otfcff2Act);






  auto tt = QImageReader::supportedImageFormats();

  //QToolButton *addPointButton = new QToolButton;
  //addPointButton->setCheckable(true);
  //addPointButton->setIcon(QIcon(":/images/curve.svg"));



#if defined(ENABLE_PDF_GENERATION)
  const QIcon exportPdfIcon = QIcon::fromTheme("document-save", QIcon(":/images/downloadpdf.png"));
  QAction* exportPDFAct = new QAction(exportPdfIcon, tr("&Export page to PDF..."), this);
  //saveAct->setShortcuts(QKeySequence::e);
  //saveAct->setStatusTip(tr("Save the current form letter"));
  connect(exportPDFAct, &QAction::triggered, this, &LayoutWindow::exportpdf);
  fileMenu->addAction(exportPDFAct);
  fileToolBar->addAction(exportPDFAct);

  const QIcon generateAllPDFIcon = QIcon::fromTheme("document-save", QIcon(":/images/downloadpdf.png"));
  QAction* generateAllPDF = new QAction(generateAllPDFIcon, tr("&Generate Quran using TeX Algo..."), this);
  //saveAct->setShortcuts(QKeySequence::e);
  //saveAct->setStatusTip(tr("Save the current form letter"));
  connect(generateAllPDF, &QAction::triggered, this, &LayoutWindow::generateAllQuranTexBreaking);
  fileMenu->addAction(generateAllPDF);
  fileToolBar->addAction(generateAllPDF);

  QIcon icon = QIcon::fromTheme("document-save", QIcon(":/images/downloadpdf.png"));
  QAction* action = new QAction(icon, tr("&Generate Medina Quran..."), this);
  connect(action, &QAction::triggered, this, &LayoutWindow::generateAllQuranTexMedina);
  fileMenu->addAction(action);
  fileToolBar->addAction(action);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  icon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  action = new QAction(icon, tr("&Generate Medina Quran VAR HTML"), this);
  connect(action, &QAction::triggered, this, &LayoutWindow::generateMadinaVARHTML);
  fileMenu->addAction(action);
  fileToolBar->addAction(action);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  icon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  action = new QAction(icon, tr("&Generate Layout Info"), this);
  connect(action, &QAction::triggered, this, &LayoutWindow::generateLayoutInfo);
  fileMenu->addAction(action);
  fileToolBar->addAction(action);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);



#endif

  fontSizeSpinBox = new QSpinBox;
  fontSizeSpinBox->setRange(1, 500);
  fontSizeSpinBox->setSingleStep(1);
  fontSizeSpinBox->setValue(100);
  fontSizeSpinBox->setKeyboardTracking(false);

  connect(fontSizeSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
    [=](int i) {

    OtLayout::EMSCALE = (double)i / 100;

    executeRunText(true, 1);
  });



  viewMenu = menuBar()->addMenu(tr("&View"));

  auto jutifyToolbar = addToolBar(tr("Justify"));
  jutifyToolbar->addWidget(fontSizeSpinBox);

  otherMenu = menuBar()->addMenu(tr("&Other"));

  QPushButton* toggleButton = new QPushButton(tr("&Justification"));
  toggleButton->setCheckable(true);
  toggleButton->setChecked(true);

  connect(toggleButton, &QPushButton::toggled, [&](bool checked) {
    applyJustification = checked;
    m_otlayout->applyJustification = checked;
    executeRunText(true, 1);
  });

  jutifyToolbar->addWidget(toggleButton);

  toggleButton = new QPushButton(tr("&Collision Detection"));
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  connect(toggleButton, &QPushButton::toggled, [&](bool checked) {
    applyCollisionDetection = checked;
    executeRunText(true, 1);
  });

  fileToolBar->addWidget(toggleButton);

}
bool LayoutWindow::generateOpenTypeCff2Standard() {
  return generateOpenTypeCff2(false);
}
bool LayoutWindow::generateOpenTypeCff2Extended() {
  return generateOpenTypeCff2(true);
}
bool LayoutWindow::generateOpenTypeCff2(bool extended) {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString otfFileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-cff2.otf";

  OtLayout layout = OtLayout(m_otlayout->mp, extended);

  //layout.isOTVar = false;

  layout.loadLookupFile("lookups.json");

  layout.toOpenType->isCff2 = true;

  auto ret = layout.toOpenType->GenerateFile(otfFileName);

  QString fileName = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + "_glyphnames.lua";

  QFile file(fileName);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  QTextStream out(&file);

  out << fileInfo.baseName() << ".glyphnames = {\n";

  for (auto& key : layout.glyphNamePerCode.keys()) {
    auto name = layout.glyphNamePerCode.value(key);
    out << "  [" << key << "] = \"" << name << "\", \n";
  }

  out << "}";

  file.close();

  //Lookup names
  fileName = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + "_lookupnames.lua";

  QFile file2(fileName);

  if (!file2.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  QTextStream out2(&file2);

  out2 << fileInfo.baseName() << ".gsubLookups = {\n";

  for (int i = 0; i < layout.gsublookups.size(); i++) {
    auto name = layout.gsublookups.at(i)->name;
    out2 << "  [" << i << "] = \"" << name << "\", \n";
  }

  out2 << "}";

  file2.close();


  return ret;



}
bool LayoutWindow::generateOpenType() {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString otfFileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".otf";

  OtLayout layout = OtLayout(m_otlayout->mp, false);

  layout.loadLookupFile("lookups.json");

  return layout.toOpenType->GenerateFile(otfFileName);

}

bool LayoutWindow::save() {
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QFile file("parameters.json");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  QJsonObject parametersObject;
  m_otlayout->saveParameters(parametersObject);
  QJsonDocument saveDoc(parametersObject);

  file.write(saveDoc.toJson());


  QApplication::restoreOverrideCursor();
  setWindowModified(false);
  return true;
}
bool LayoutWindow::exportpdf() {

  double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;

  loadLookupFile("lookups.json");

  QString textt = textEdit->toPlainText();

  auto lines = textt.split(char(10), Qt::SkipEmptyParts);

  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto page = m_otlayout->justifyPage(scale, lineWidth, lineWidth, lines, LineJustification::Distribute, true, true);

  QList<QList<LineLayoutInfo>> pages{ page };
  QList<QStringList> originalPages{ lines };

  //lineWidth = (17000 - (2 * 400));

  int margin = 0;

  GlyphVis::BBox cropBox{ std::numeric_limits<double>::max(),  std::numeric_limits<double>::max(),std::numeric_limits<double>::min(),std::numeric_limits<double>::min() };

  for (auto& page : pages) {
    double gap = 0;
    for (auto& line : page) {
      hb_position_t currentxPos = line.xstartposition;
      hb_position_t currentyPos = line.ystartposition;
      line.type = LineType::Line;
      for (int i = 0; i < line.glyphs.size(); i++) {
        auto glyph = line.glyphs[i];

        QString glyphName = m_otlayout->glyphNamePerCode[glyph.codepoint];
        GlyphVis* glyphVis = &m_otlayout->glyphs[glyphName];

        if (glyph.lefttatweel != 0 || glyph.righttatweel != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = glyph.lefttatweel;
          parameters.righttatweel = glyph.righttatweel;

          glyphVis = glyphVis->getAlternate(parameters);
        }

        currentxPos -= line.glyphs[i].x_advance;
        int x = currentxPos + line.glyphs[i].x_offset;
        int y = currentyPos - line.glyphs[i].y_offset;

        auto llx = x + ((int)(glyphVis->bbox.llx) << OtLayout::SCALEBY);
        if (llx < cropBox.llx)
          cropBox.llx = llx;

        auto lly = y - ((int)(glyphVis->bbox.ury) << OtLayout::SCALEBY);

        if (lly < cropBox.lly)
          cropBox.lly = lly;

        auto urx = x + ((int)(glyphVis->bbox.urx) << OtLayout::SCALEBY);
        if (urx > cropBox.urx)
          cropBox.urx = urx;

        auto ury = y - ((int)(glyphVis->bbox.lly) << OtLayout::SCALEBY);
        if (ury > cropBox.ury)
          cropBox.ury = ury;
      }
      if (gap == 0) {
        cropBox.lly -= (margin / 2) << OtLayout::SCALEBY;
        gap = cropBox.lly;
      }

      line.ystartposition -= gap;
    }
  }

  /*
  QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };

  QuranPdfWriter quranWriter("quran.pdf", m_otlayout);
  quranWriter.setPageLayout(pageLayout);
  quranWriter.setResolution(4800 << OtLayout::SCALEBY);*/

  qreal width = ((cropBox.urx - cropBox.llx)) / std::pow(2, OtLayout::SCALEBY) + margin;
  qreal height = ((cropBox.ury - cropBox.lly)) / std::pow(2, OtLayout::SCALEBY) + margin;

  QPageSize pageSize{ {(width * 72. / 4800) ,(height * 72. / 4800)},QPageSize::Point, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };

  //auto rec = pageLayout.fullRectPixels(4800 << OtLayout::SCALEBY);
#if defined(ENABLE_PDF_GENERATION)
  QuranPdfWriter quranWriter("quran.pdf", m_otlayout);
  quranWriter.setPageLayout(pageLayout);
  quranWriter.setResolution(4800 << OtLayout::SCALEBY);

  quranWriter.generateQuranPages(pages, -cropBox.llx + ((int)(margin / 2) << OtLayout::SCALEBY), originalPages, scale, 0);

#endif

  return true;
}

LayoutPages LayoutWindow::shapeMedina(int scale, int lineWidth, OtLayout* layout, hb_buffer_cluster_level_t  cluster_level) {

  loadLookupFile("lookups.json");

  QList<QList<LineLayoutInfo>> pages;
  QStringList originalPage;
  QList<QStringList> originalPages;

  QString suraWord = "سُورَةُ";
  QString bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

  QString surapattern = "^("
    + suraWord + " .*|"
    + bism
    + "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
    + ")$";

  QRegularExpression surabism(surapattern, QRegularExpression::MultilineOption);

  bool newface = true;


  QString sajdapatterns = "(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)"; // sajdapatterns.replace("\u0657", "\u08F0").replace("\u065E", "\u08F1").replace("\u0656", "\u08F2");
  QRegularExpression sajdaRe = QRegularExpression(sajdapatterns, QRegularExpression::MultilineOption);

  int beginsajda = 0;
  int endsajda = 0;
  int sajdamatched = 0;

  QRegularExpression reaya("(\\d+)");
  reaya.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);




  for (int pagenum = 0; pagenum < 604; pagenum++) {

    //if (pagenum == 155) {
    //	auto test = 5;
    //}

    QString textt = QString::fromUtf8(qurantext[pagenum] + 1);

    textt = textt.replace(QRegularExpression(" *" + QString("۞") + " *"), QString("۞") + " ");

    textt == textt.replace(reaya, QString("۝") + "\\1");

    auto lines = textt.split(char(10), Qt::SkipEmptyParts);

    auto justification = LineJustification::Distribute;
    int beginsura = OtLayout::TopSpace << OtLayout::SCALEBY;

    if (pagenum == 0 || pagenum == 1) {
      justification = LineJustification::Center;
      beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3)) << OtLayout::SCALEBY;
    }

    auto page = layout->justifyPage(scale, lineWidth, lineWidth, lines, justification, newface, true, cluster_level);

    newface = false;

    //int currentyPos = TopSpace;

    for (int i = 0; i < page.size(); ++i) {

      auto& currentpage = page[i];

      // check if suran name or bism
      auto match = surabism.match(lines[i]);
      if (match.hasMatch()) {

        auto temp = layout->justifyPage(scale, lineWidth, lineWidth, { lines[i] }, LineJustification::Center, false, true, cluster_level);

        if (match.captured(0).startsWith("سُ")) {
          temp[0].type = LineType::Sura;
        }
        else {
          temp[0].type = LineType::Bism;
        }

        page[i] = temp[0];
      }
      else {
        // check if sajda
        match = sajdaRe.match(lines[i]);
        if (match.hasMatch()) {

          sajdamatched++;

          int startOffset = match.capturedStart(match.lastCapturedIndex()); // startOffset == 6
          int endOffset = match.capturedEnd(match.lastCapturedIndex()) - 1; // endOffset == 9


          while (layout->glyphGlobalClasses[lines[i][endOffset].unicode()] == OtLayout::MarkGlyph)
            endOffset--;

          bool beginDone = false;

          auto& glyphs = currentpage.glyphs;

          for (auto& glyphLayout : glyphs) {

            if (glyphLayout.cluster == startOffset && !beginDone) {
              glyphLayout.beginsajda = true;
              beginDone = true;;
              beginsajda++;

            }
            else if (glyphLayout.cluster == endOffset) {
              glyphLayout.endsajda = true;
              endsajda++;
              break;
            }

          }

        }
      }


      if (i == 0 && (pagenum == 0 || pagenum == 1)) {
        page[i].type = LineType::Sura;
        page[i].ystartposition = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1)) << OtLayout::SCALEBY;
      }
      else {
        page[i].ystartposition = beginsura;
        beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;
      }
    }



    pages.append(page);
    originalPages.append(lines);
  }

  if (beginsajda != 15 || endsajda != 15 || sajdamatched != 15) {
    qDebug() << "sajdas problems?";
  }

  if (this->applyCollisionDetection) {
    adjustOverlapping(pages, lineWidth, originalPages, scale);
  }

  LayoutPages result;

  result.originalPages = originalPages;
  result.pages = pages;

  return result;

}
bool LayoutWindow::generateLayoutInfo() {

  OtLayout layout = OtLayout(m_otlayout->mp, true);

  layout.loadLookupFile("lookups.json");

  int calScale = 1 << OtLayout::SCALEBY;

  int scale = calScale * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMedina(scale, lineWidth, &layout);

  GenerateLayout generateLayout{ &layout,result };

  generateLayout.generateLayout(lineWidth, scale);


  return true;
}


bool LayoutWindow::generateMadinaVARHTML() {

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);

  OtLayout layout = OtLayout(m_otlayout->mp, true);

  layout.automedina->cvxxfeatures.clear();

  layout.automedina->cvxxfeatures.append({});

  auto& cv01feature = layout.automedina->cvxxfeatures[0];

  QMap<quint16, quint16> unicodeMappings;

  /*
  unicodeMappings.insert(layout.glyphCodePerName["behshape.onedotdown.isol"], layout.glyphCodePerName["behshape.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["behshape.twodotsup.isol"], layout.glyphCodePerName["behshape.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["behshape.three_dots.isol"], layout.glyphCodePerName["behshape.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["heh.twodotsup.isol"], layout.glyphCodePerName["heh.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["hah.onedotdown.isol"], layout.glyphCodePerName["hah.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["hah.onedotup.isol"], layout.glyphCodePerName["hah.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["dal.onedotup.isol"], layout.glyphCodePerName["dal.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["reh.onedotup.isol"], layout.glyphCodePerName["reh.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["seen.three_dots.isol"], layout.glyphCodePerName["seen.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["sad.onedotup.isol"], layout.glyphCodePerName["sad.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["tah.onedotup.isol"], layout.glyphCodePerName["tah.isol"]);
  unicodeMappings.insert(layout.glyphCodePerName["ain.onedotup.isol"], layout.glyphCodePerName["ain.isol"]);*/


  //layout.isOTVar = false;

  layout.loadLookupFile("lookups.json");

  int calScale = 1 << OtLayout::SCALEBY;

  int scale = calScale * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMedina(scale, lineWidth, &layout, HB_BUFFER_CLUSTER_LEVEL_CHARACTERS);

  auto htmlFileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-quran.html";

  QFile file(htmlFileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");

  auto& originalText = result.originalPages;
  auto& pages = result.pages;

  hb_font_t* hbfont = layout.createFont(scale, false);;

  out << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
  out << "<head>\n";
  out << "<meta charset=\"utf-8\"/>\n";
  out << "<meta name=\"generator\" content=\"DigitalKhatt\"/>\n";

  out << "<style>\n";
  out << "@font-face {\n";
  out << "font-family: '" << fileInfo.completeBaseName() << "';\n";
  out << "font-style: normal;\n";
  out << "src: url('digitalkhatt-quran.otf') format('opentype');\n";
  out << "font-display: block;}\n";
  out << "body {\n";
  out << "font-family: '" << fileInfo.completeBaseName() << "';  \n";
  out << "line-height: normal;\n";
  out << "font-size: 100px;\n";
  out << "direction: rtl;\n";
  out << "white-space: nowrap;\n";
  //out << "text-align: justify;\n";
  //out << "word-spacing:-0.3em;\n";
  out << "}\n";
  //out << "span {\n";
  //out << "display: inline-block;\n";
  //out << "direction: ltr;\n";
  //out << "}\n";
  out << ".char {\n";
  out << "position: relative;\n";
  out << "display: inline-block;\n";
  //out << "direction: ltr;\n";
  //out << "right: 0px;\n";
  //out << "text-align: 0px;\n";
  out << "}\n";
  //out << ".char::after { content:\"\\00a0\"}\n";
  out << ".line {\n";
  out << "position: relative;\n";
  out << "}\n";
  out << ".linesura {\n";
  out << "position: relative;\n";
  out << "}\n";
  out << "</style>\n";
  out << "</head>\n";
  out << "<body>\n";

  for (int p = 0; p < originalText.length(); p++) {
    auto& pageText = originalText[p];
    auto& page = pages[p];
    out << "<div #page class='page";
    if (p == 0 || p == 1) {
      out << " center";
    }
    else {
      out << " justify";
    }
    out << "' data-page-number='" << p + 1 << "'>" << '\n';
    out << "<div class='innerpage'>" << '\n';
    for (int l = 0; l < pageText.length(); l++) {
      auto& lineText = pageText[l];
      auto& line = page[l];
      //out << "<div style='right:" << line.xstartposition << "px;top:" << line.ystartposition << "px;' data-line-number='" << l + 1 << "'";
      out << "<div data-line-number='" << l + 1 << "'";
      if (line.type == LineType::Bism) {
        out << " class = 'linebism";
      }
      else if (line.type == LineType::Sura) {
        out << " class = 'linesura";
      }
      else {
        out << " class = 'line";
      }
      out << "'>" << '\n';

      struct GlyphsUnicodes {
        QVector<GlyphLayoutInfo*> glyphs;
        QString unicodes;
      };

      QMap<int, GlyphsUnicodes> unicodes;

      int lastValue = -1;

      for (auto& glyph : line.glyphs) {


        if (glyph.cluster < lastValue) {
          std::cout << "page=" << p << ",line=" << l << ",lastValue=" << lastValue << ",cluster=" << glyph.cluster << std::endl;
          throw new std::runtime_error("Problem");
        }

        lastValue = glyph.cluster;


        unicodes[glyph.cluster].glyphs.append(&glyph);
      }

      auto keys = unicodes.keys();

      for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
        auto start = *iter;
        auto next = iter + 1;
        auto end = next == keys.end() ? lineText.size() : *next; // exclusive
        QString univalues = lineText.mid(start, end - start);
        auto& value = unicodes[start];

        //delete tatweel
        univalues = univalues.replace(QChar(0x0640), "");
        value.unicodes = univalues;

        if (value.unicodes.size() == 2 && value.glyphs.size() == 2 && value.unicodes[1].unicode() == value.glyphs[1]->codepoint) {
          unicodes[start + 1] = { {value.glyphs[1]},QString(value.unicodes[1]) };
          value = { {value.glyphs[0]},QString(value.unicodes[0]) };
        }
      }


      for (auto iter = unicodes.begin(); iter != unicodes.end(); ++iter) {
        auto& value = iter.value();
        if (value.unicodes.size() > 1) {
          if (value.glyphs.size() != 1) {
            std::cout << "page=" << p << ",line=" << l << ",start=" << iter.key() << ",unicodes=" << value.unicodes.toStdString() << std::endl;
            throw new std::runtime_error("Problem");
          }
          for (auto charv : value.unicodes) {
            if (charv.unicode() != 0x06DD && (charv.unicode() < 0x0660 || charv.unicode() > 0x0669)) {
              std::cout << "page=" << p << ",line=" << l << ",start=" << iter.key() << ",unicodes=" << value.unicodes.toStdString() << ",unicode=" << charv.unicode() << std::endl;
              //throw new std::runtime_error("Problem");
            }

          }

        }
      }

      double currentxPos = 0;
      double cumulatedRight = 0.0;
      for (auto iter = unicodes.begin(); iter != unicodes.end(); ++iter) {
        auto& value = iter.value();
        if (value.unicodes.size() == 1) {
          if (value.glyphs.size() != 1 && value.glyphs.size() != 2) {
            std::cout << "page=" << p << ",line=" << l << ",start=" << iter.key() << ",unicodes=" << value.unicodes.toStdString() << std::endl;
            throw new std::runtime_error("Problem");
          }
          /*
          if (value.glyphs.size() == 2 && !layout.glyphNamePerCode[value.glyphs[1]->codepoint].contains("dot")) {
            std::cout << "page=" << p << ",line=" << l << ",start=" << iter.key() << ",unicodes=" << value.unicodes.toStdString() << std::endl;
            //throw new std::runtime_error("Problem");
          }*/

          auto unicode = value.unicodes[0].unicode();
          if (unicodeMappings.contains(unicode)) {
            unicode = unicodeMappings.value(unicode);
          }

          auto& alternates = cv01feature[unicode];

          int alternateIndex = -1;
          for (int aIndex = 0; aIndex < alternates.size(); aIndex++) {
            if (alternates[aIndex] == value.glyphs[0]->codepoint) {
              alternateIndex = aIndex + 1;
              break;
            }
          }

          if (alternateIndex == -1) {
            //if (value.glyphs[0]->codepoint != unicode) {
            alternates.append(value.glyphs[0]->codepoint);
            alternateIndex = alternates.size();
            //}

          }

          auto glyph = value.glyphs[0];

          QString axes;
          if (glyph->lefttatweel != 0.0 || glyph->righttatweel != 0) {
            axes = "font-variation-settings:";
            if (glyph->lefttatweel != 0.0) {

              float leftTatweel = 0.0;

              if (glyph->lefttatweel > 0) {
                auto maxLeft = layout.expandableGlyphs[layout.glyphNamePerCode[glyph->codepoint]].maxLeft;
                auto maxAxisLeft = layout.toOpenType->axisLimits.maxLeft;

                float f214limit = maxAxisLeft * roundf(maxLeft / maxAxisLeft * 16384.f) / 16384.f;

                leftTatweel = glyph->lefttatweel > f214limit ? f214limit : glyph->lefttatweel;

              }
              else {
                auto minLeft = layout.expandableGlyphs[layout.glyphNamePerCode[glyph->codepoint]].minLeft;
                auto minAxisLeft = layout.toOpenType->axisLimits.minLeft;

                float f214limit = minAxisLeft * roundf(minLeft / minAxisLeft * 16384.f) / 16384.f;

                leftTatweel = glyph->lefttatweel < f214limit ? f214limit : glyph->lefttatweel;
              }

              axes = axes + QString("\"LTAT\" %1").arg(leftTatweel);
              if (glyph->righttatweel != 0.0) {
                axes = axes + ",";
              }
            }
            if (glyph->righttatweel != 0.0) {

              float rightTatweel = 0.0;

              if (glyph->righttatweel > 0) {
                auto maxRight = layout.expandableGlyphs[layout.glyphNamePerCode[glyph->codepoint]].maxRight;
                auto maxAxisRight = layout.toOpenType->axisLimits.maxRight;

                float f214limit = maxAxisRight * roundf(maxRight / maxAxisRight * 16384.f) / 16384.f;

                rightTatweel = glyph->righttatweel > f214limit ? f214limit : glyph->righttatweel;

              }
              else {
                auto minRight = layout.expandableGlyphs[layout.glyphNamePerCode[glyph->codepoint]].minRight;
                auto minAxisLRight = layout.toOpenType->axisLimits.minRight;

                float f214limit = minAxisLRight * roundf(minRight / minAxisLRight * 16384.f) / 16384.f;

                rightTatweel = glyph->righttatweel < f214limit ? f214limit : glyph->righttatweel
                  ;
              }

              axes = axes + QString("\"RTAT\" %1").arg(rightTatweel);
            }
            axes = axes + ";";
          }
          QString cv01;

          if (alternateIndex != -1) {
            cv01 = QString("font-feature-settings: \"cv01\" %1;").arg(alternateIndex);
          }

          QString position;
          if (glyph->x_advance != 0) {
            position = QString("width:%1em;").arg(glyph->x_advance / (scale * 1000.0));
          }
          if (glyph->x_offset != 0 || glyph->y_offset != 0) {
            position = position + QString("transform: translate(%1em, %2em);").arg(glyph->x_offset / (1000.0 * scale)).arg(-glyph->y_offset / (1000.0 * scale));
            //position = position + QString("position:relative;left:%1em;top:%2em;").arg((double)glyph->x_offset / (1000 * scale)).arg(-(double)glyph->y_offset / (1000 * scale));
          }
          /*
          position = "";

          if (glyph->x_advance != 0) {
            position = QString("width:%1em;").arg(glyph->x_advance / (scale * 1000.0));
          }

          auto right = currentxPos - glyph->x_offset / (1000.0 * scale);
          auto top = -glyph->y_offset / (1000.0 * scale);
          position = position + QString("top:%1em;right:%2em;").arg(top).arg(right);*/

          position = "";



          auto defaultAdvance = layout.gethHorizontalAdvance(hbfont, glyph->codepoint, glyph->lefttatweel, glyph->righttatweel, nullptr);

          if (defaultAdvance != 0) {
            position = QString("width:%1em;").arg(defaultAdvance / (scale * 1000.0));
          }

          cumulatedRight += (glyph->x_advance - defaultAdvance) / (1000.0 * scale);
          auto right = cumulatedRight - glyph->x_offset / (1000.0 * scale);
          auto top = -glyph->y_offset / (1000.0 * scale);
          position = position + QString("top:%1em;right:%2em;").arg(top).arg(right);

          QString other;

          if (value.unicodes[0].unicode() == 0x20) { // space
            other = "display:inline;";
          }

          out << QString("<span class='char' style='%1%2%3%4'>").arg(cv01).arg(axes).arg(position).arg(other);
          out << value.unicodes;
          out << "</span>";

          currentxPos += glyph->x_advance / (1000.0 * scale);
        }
        else if (value.glyphs.size() >= 1) {
          auto glyph = value.glyphs[0];

          QString position = "";

          auto defaultAdvance = layout.gethHorizontalAdvance(hbfont, glyph->codepoint, glyph->lefttatweel, glyph->righttatweel, nullptr);

          if (defaultAdvance != 0) {
            position = QString("width:%1em;").arg(defaultAdvance / (scale * 1000.0));
          }

          cumulatedRight += (glyph->x_advance - defaultAdvance) / (1000.0 * scale);
          auto right = cumulatedRight - glyph->x_offset / (1000.0 * scale);
          auto top = -glyph->y_offset / (1000.0 * scale);
          position = position + QString("top:%1em;right:%2em;").arg(top).arg(right);


          out << QString("<span class='char' style='%1'>").arg(position);
          out << value.unicodes;
          out << "</span>";

          currentxPos += glyph->x_advance / (1000.0 * scale);
        }
      }


      out << "\n</div>" << '\n';
    }
    out << "</div>" << '\n';
    out << "</div>" << '\n';
    out << "</body>\n";
    out << "</html>\n";
  }


  QString otfFileName = fileInfo.path() + "/" + fileInfo.completeBaseName() + "-quran.otf";

  //OtLayout layout = OtLayout(m_otlayout->mp, extended);

  //layout.isOTVar = false;

  layout.loadLookupFile("lookups-html.json");

  layout.toOpenType->isCff2 = true;

  auto ret = layout.toOpenType->GenerateFile(otfFileName, "lookups-html.json");

  delete hbfont;

  return true;

}

bool LayoutWindow::generateAllQuranTexMedina() {

  int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMedina(scale, lineWidth, m_otlayout);


  QFile file("quran_medina_inputtext.txt");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");
  //out.setEncoding(QStringConverter::Utf8);

  for (auto page : result.originalPages) {
    for (auto line : page) {
      //out << line.replace("\u06E5","").replace("\u06E6", "") << "\n";   // serialize a string

      out << line.replace("\u0640", "") << "\n";   // serialize a string
    }
  }

  file.close();

  QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };
#if defined(ENABLE_PDF_GENERATION)
  QuranPdfWriter quranWriter("quranMedina.pdf", m_otlayout);
  quranWriter.setPageLayout(pageLayout);
  quranWriter.setResolution(4800 << OtLayout::SCALEBY);

  quranWriter.generateQuranPages(result.pages, lineWidth, result.originalPages, scale);


  ExportToHTML extohtml{ m_otlayout };

  extohtml.generateQuranPages(result.pages, lineWidth, result.originalPages, scale);

#endif
  return true;

}

bool LayoutWindow::generateAllQuranTexBreaking() {

  loadLookupFile("lookups.json");

  int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;
  auto result = m_otlayout->pageBreak(scale, lineWidth, false);

  if (result.pages.count() == 0) {
    QMessageBox msgBox;
    msgBox.setText("No feasable solution. Try to change the scale.");
    msgBox.exec();
    return false;
  }

  if (this->applyJustification) {
    for (int pagenum = 0; pagenum < result.originalPages.length(); pagenum++) {

      if (pagenum != 0 && pagenum != 1) {
        auto justification = LineJustification::Distribute;


        auto page = m_otlayout->justifyPage(scale, lineWidth, lineWidth, result.originalPages[pagenum], justification, false, true);

        for (int lineIndex = 0; lineIndex < page.length(); lineIndex++) {
          if (result.pages[pagenum][lineIndex].type == LineType::Line) {
            result.pages[pagenum][lineIndex] = page[lineIndex];
          }
          else {
            auto temp = m_otlayout->justifyPage(scale, 0, lineWidth, QStringList{ result.originalPages[pagenum][lineIndex] }, LineJustification::Center, false, true);
            //temp[0].type = result.pages[pagenum][lineIndex].type;
            //temp[0].ystartposition = result.pages[pagenum][lineIndex].ystartposition;
            result.pages[pagenum][lineIndex].glyphs = temp[0].glyphs;
          }
        }
      }
    }
  }

  if (this->applyCollisionDetection) {
    adjustOverlapping(result.pages, lineWidth, result.originalPages, scale);
  }

  QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };

#if defined(ENABLE_PDF_GENERATION)
  QuranPdfWriter quranWriter("allquran.pdf", m_otlayout);
  quranWriter.setPageLayout(pageLayout);
  quranWriter.setResolution(4800 << OtLayout::SCALEBY);

  quranWriter.generateQuranPages(result.pages, lineWidth, result.originalPages, scale);

#endif
  return true;

}


void LayoutWindow::loadLookupFile(QString fileName) {

  //QJsonModel * model = new QJsonModel;	

  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QByteArray myarray = file.readAll();
    QJsonParseError error;
    QJsonDocument mDocument = QJsonDocument::fromJson(myarray, &error);
    if (error.error != error.NoError) {
      auto errorString = error.errorString();
      std::cout << errorString.toStdString();

    }
    //model->loadJson(mDocument);
    m_otlayout->readJson(mDocument.object());
  }

  QFile file2("parameters.json");
  if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QByteArray myarray = file2.readAll();
    QJsonDocument mDocument = QJsonDocument::fromJson(myarray);
    m_otlayout->readParameters(mDocument.object());
  }

  QSettings settings;

  lokkupTreeWidget->clear();
  lokkupTreeWidget->setColumnCount(1);
  lokkupTreeWidget->blockSignals(true);
  QList<QTreeWidgetItem*> topItems;
  for (auto& feature : m_otlayout->allFeatures.keys()) {
    auto featureItem = new QTreeWidgetItem(lokkupTreeWidget, QStringList(feature));

    featureItem->setData(0, Qt::UserRole, feature);
    topItems.append(featureItem);
    auto lookups = m_otlayout->allFeatures[feature];
    bool alldisabled = true;
    bool onedisabled = false;
    for (auto lookup : lookups) {
      auto lookupItem = new QTreeWidgetItem(QStringList(lookup->name));
      lookupItem->setData(0, Qt::UserRole, lookup->name);
      bool disabled = settings.value("DisabledLookups/" + lookup->name).toBool();
      lookupItem->setCheckState(0, disabled ? Qt::Checked : Qt::Unchecked);
      featureItem->addChild(lookupItem);
      if (disabled) {
        m_otlayout->disabledLookups.insert(lookup);
      }
      alldisabled = alldisabled && disabled;
      onedisabled = onedisabled || disabled;
    }
    if (alldisabled) {
      featureItem->setCheckState(0, Qt::Checked);
    }
    else {
      featureItem->setCheckState(0, Qt::Unchecked);
    }

    if (onedisabled) {
      featureItem->setExpanded(true);
    }
  }

  lokkupTreeWidget->setHeaderLabels(QStringList() << "Features");

  lokkupTreeWidget->insertTopLevelItems(0, topItems);

  lokkupTreeWidget->blockSignals(false);

}
void LayoutWindow::layoutParameterChanged() {
  executeRunText(true, 0);
}
void LayoutWindow::resizeEvent(QResizeEvent* event) {

  this->resizeDocks({ lookupTree ,textRun }, { this->width() / 4 ,this->width() / 3 }, Qt::Horizontal);
  //tabWidget->setMinimumWidth(this->width() / 3);
  //tabWidget->resize(this->width() / 3, tabWidget->height());


}
void LayoutWindow::createDockWindows()
{
  textRun = new QDockWidget(tr("Text Example"), this);
  textRun->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

  QFont textEditFont("DejaVu Sans Mono");

  textEdit = new QPlainTextEdit(textRun);
  textEdit->setFont(textEditFont);


  executeRunButton = new QPushButton("&Reload Opentype lookups", textRun);
  connect(executeRunButton, &QPushButton::clicked, [=](int i) {
    executeRunText(true, 2);
  });

  auto refreshButton = new QPushButton("&Refresh", textRun);
  connect(refreshButton, &QPushButton::clicked, [=](int i) {
    executeRunText(true, 1);
  });

  integerSpinBox = new QSpinBox;
  integerSpinBox->setRange(1, 604);
  integerSpinBox->setSingleStep(1);
  integerSpinBox->setValue(1);
  integerSpinBox->setKeyboardTracking(false);

  connect(integerSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
    [=](int i) {
    //const char* tt = qurantext[i - 1] + 1;
    textEdit->setPlainText(currentQuranText[i - 1]);
    suraName->setText(suraNameByPage[i - 1]);
    executeRunText(false, 1);
  });

  suraName = new QLabel;
  auto lfont = suraName->font();
  lfont.setPointSize(18);
  lfont.setBold(true);
  suraName->setFont(lfont);
  lfont.setBold(false);
  textEdit->setFont(lfont);


  QVBoxLayout* textRunLayout = new QVBoxLayout;
  textRunLayout->addWidget(integerSpinBox);
  textRunLayout->addWidget(suraName);
  textRunLayout->addWidget(textEdit);
  textRunLayout->addWidget(executeRunButton);
  textRunLayout->addWidget(refreshButton);

  QWidget* textRunWidget = new QWidget(textRun);
  textRunWidget->setLayout(textRunLayout);

  textRun->setWidget(textRunWidget);

  addDockWidget(Qt::RightDockWidgetArea, textRun);
  viewMenu->addAction(textRun->toggleViewAction());

  lookupTree = new QDockWidget(tr("Lookup Tree"), this);
  lookupTree->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

  addDockWidget(Qt::LeftDockWidgetArea, lookupTree);
  viewMenu->addAction(lookupTree->toggleViewAction());

  auto action = new QAction(tr("Calculate minimum size"), this);
  action->setStatusTip(tr("Calculate minimum size"));
  connect(action, &QAction::triggered, this, &LayoutWindow::calculateMinimumSize);

  otherMenu->addAction(action);

  action = new QAction(tr("Find overflows"), this);
  action->setStatusTip(tr("Find overflows"));
  connect(action, &QAction::triggered, this, &LayoutWindow::findOverflows);

  otherMenu->addAction(action);



  action = new QAction(tr("Test kasheda"), this);
  action->setStatusTip(tr("Test kasheda"));
  connect(action, &QAction::triggered, this, &LayoutWindow::testKasheda);

  otherMenu->addAction(action);

  action = new QAction(tr("Serialize Tex Pages"), this);
  action->setStatusTip(tr("Serialize Tex Pages"));
  connect(action, &QAction::triggered, this, &LayoutWindow::serializeTexPages);

  otherMenu->addAction(action);

  action = new QAction(tr("Serialize Medina Pages"), this);
  action->setStatusTip(tr("Serialize Medina Pages"));
  connect(action, &QAction::triggered, this, &LayoutWindow::serializeMedinaPages);

  otherMenu->addAction(action);


  m_otlayout = new OtLayout(m_font->mp, true, this);
  m_otlayout->isOTVar = true;
  //m_otlayout->useNormAxisValues = false;

  connect(m_otlayout, &OtLayout::parameterChanged, this, &LayoutWindow::layoutParameterChanged);

  lokkupTreeWidget = new QTreeWidget(this);

  connect(lokkupTreeWidget, &QTreeWidget::itemChanged, [&, this](QTreeWidgetItem* item, int column) {
    this->lokkupTreeWidget->blockSignals(true);
    auto value = item->data(0, Qt::UserRole).toString();
    QSettings settings;
    if (item->childCount() != 0 && this->m_otlayout->allFeatures.contains(item->text(0))) {
      for (int i = 0; i < item->childCount(); i++) {
        auto child = item->child(i);
        settings.setValue("DisabledLookups/" + child->text(0), item->checkState(0) == Qt::Checked);
        child->setCheckState(0, item->checkState(0));
        auto lookupIndex = this->m_otlayout->lookupsIndexByName[child->text(0)];
        Lookup* lookup = this->m_otlayout->lookups[lookupIndex];
        if (item->checkState(0) == Qt::Checked) {
          this->m_otlayout->disabledLookups.insert(lookup);
        }
        else {
          this->m_otlayout->disabledLookups.remove(lookup);
        }
      }
    }
    else if (this->m_otlayout->lookupsIndexByName.contains(item->text(0))) {
      settings.setValue("DisabledLookups/" + item->text(0), item->checkState(0) == Qt::Checked);
      auto lookupIndex = this->m_otlayout->lookupsIndexByName[item->text(0)];
      Lookup* lookup = this->m_otlayout->lookups[lookupIndex];
      if (item->checkState(0) == Qt::Checked) {
        this->m_otlayout->disabledLookups.insert(lookup);
      }
      else {
        this->m_otlayout->disabledLookups.remove(lookup);
      }
    }
    this->m_otlayout->dirty = true;
    this->executeRunText(true, 1);
    this->lokkupTreeWidget->blockSignals(false);

  });

  lookupTree->setWidget(lokkupTreeWidget);

  this->setCentralWidget(m_graphicsView);
}


void LayoutWindow::serializeTexPages() {

  int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;
  int topSpace = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = m_otlayout->pageBreak(scale, lineWidth, false);

  if (result.pages.count() == 0) {
    QMessageBox msgBox;
    msgBox.setText("No feasable solution. Try to change the scale.");
    msgBox.exec();
    return;
  }

  QList<SuraLocation> locations;

  int height = OtLayout::TopSpace << OtLayout::SCALEBY;

  int suraNumber = 1;

  for (int pageIndex = 0; pageIndex < result.pages.size(); pageIndex++) {
    auto& page = result.pages.at(pageIndex);
    for (int lineIndex = 0; lineIndex < page.size(); lineIndex++) {
      auto& line = page.at(lineIndex);
      if (line.type == LineType::Sura) {
        int y = (line.ystartposition - 3 * height / 5) * 72. / (4800 << OtLayout::SCALEBY);
        SuraLocation location{
          QString("%1 ( %2 )").arg(result.originalPages.at(pageIndex).at(lineIndex)).arg(suraNumber++),
          pageIndex,0, y
        };
        locations.append(location);
      }
    }
  }

  QFile file("texpages.dat");
  file.open(QIODevice::WriteOnly);
  QDataStream out(&file);   // we will serialize the data into the file
  out << OtLayout::EMSCALE;
  out << result.originalPages;
  out << result.suraNamebyPage;
  out << locations;

}

void LayoutWindow::serializeMedinaPages() {

  int scale = (1 << OtLayout::SCALEBY) * 0.85;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMedina(scale, lineWidth, m_otlayout);

  QList<SuraLocation> locations;

  int height = OtLayout::TopSpace << OtLayout::SCALEBY;

  int suraNumber = 1;

  for (int pageIndex = 0; pageIndex < result.pages.size(); pageIndex++) {
    auto& page = result.pages.at(pageIndex);
    for (int lineIndex = 0; lineIndex < page.size(); lineIndex++) {
      auto& line = page.at(lineIndex);
      if (line.type == LineType::Sura) {
        int y = (line.ystartposition - 3 * height / 5) * 72. / (4800 << OtLayout::SCALEBY);
        SuraLocation location{ QString("%1 ( %2 )").arg(result.originalPages.at(pageIndex).at(lineIndex)).arg(suraNumber++)
          ,pageIndex,0, y };
        locations.append(location);
      }
    }
  }

  QFile file("medinapages.dat");
  file.open(QIODevice::WriteOnly);
  QDataStream out(&file);   // we will serialize the data into the file
  out << OtLayout::EMSCALE;
  out << result.originalPages;
  out << result.suraNamebyPage;
  out << locations;

}

void LayoutWindow::testKasheda() {

  //subscript alef
  //ٰ 
  QRegularExpression smallseen("(\\S*[ٜۣۧۜۨࣳـ۬]\\S*)");

  QString smallseenWords;

  QString output;

  QSet<QString> words;

  for (auto& text : currentQuranText) {
    QRegularExpressionMatchIterator i = smallseen.globalMatch(text);
    while (i.hasNext()) {
      QRegularExpressionMatch match = i.next();
      int startOffset = match.capturedStart(match.lastCapturedIndex()); // startOffset == 6
      int endOffset = match.capturedEnd(match.lastCapturedIndex()) - 1; // endOffset == 9
      QString c0 = match.captured(0);
      QString captured = match.captured(match.lastCapturedIndex());

      QString word = match.captured(0);

      if (!words.contains(word)) {
        words.insert(word);

        if (smallseenWords.isEmpty()) {
          smallseenWords = word;
        }
        else {
          smallseenWords = smallseenWords + " " + word;

          if (smallseenWords.length() > 60) {
            if (output.isEmpty()) {
              output = smallseenWords;
            }
            else {
              output = output + "\n" + smallseenWords;
            }
            smallseenWords = "";
          }
        }

      }



    }
  }

  output = output + "\n" + smallseenWords;


  textEdit->setPlainText(output);
  suraName->setText("Test Kasheda");
  executeRunText(false, 1);
}

void LayoutWindow::findOverflows() {
  loadLookupFile("lookups.json");


  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_buffer_t* buffer = buffer = hb_buffer_create();

  //bool conti = true;

  struct Line {
    int pageNumber;
    int lineNumber;
    double overflow;
    double percentage;
  };

  QMap<double, QMap<double, Line>> alloverflows;

  double scale = OtLayout::EMSCALE;

  int emScale = (1 << OtLayout::SCALEBY) * scale;

  //const int minSpace = OtLayout::MINSPACEWIDTH * emScale;
  //const int  defaultSpace = OtLayout::SPACEWIDTH * emScale;

  QMap<double, Line> overflows;

  QRegularExpression reaya("(\\d+)");
  reaya.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);

  for (int pagenum = 0; pagenum <= 603; pagenum++) {

    QString textt = QString::fromUtf8(qurantext[pagenum] + 1);

    textt == textt.replace(reaya, QString("۝") + "\\1");

    auto lines = textt.split(char(10), Qt::SkipEmptyParts);

    auto page = m_otlayout->justifyPage(emScale, lineWidth, lineWidth, lines, LineJustification::Distribute, false, true);


    for (int linenum = 0; linenum < lines.length(); linenum++) {     

      auto line = page[linenum];

      if (line.overfull > 0) {
        auto overflow = line.overfull / emScale;
        //if (overflow > 0.01) {
          overflows.insert(-line.overfull, { pagenum + 1,linenum + 1, overflow,(line.overfull / lineWidth) * 100 });
       // }
        
      }
    }
  }

  if (overflows.count() > 0) {
    alloverflows[scale] = overflows;
  }


  hb_buffer_destroy(buffer);

  QString fileName = QString("overflows.csv");

  if (applyJustification) {
    fileName = QString("overflows_with_just.csv");
  }

  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("ISO 8859-1");
  //out.setEncoding(QStringConverter::Latin1);

  for (auto key : alloverflows.keys()) {
    auto overflow = alloverflows.value(key);
    for (auto line : overflow) {
      out << key << "," << line.pageNumber << "," << line.lineNumber << "," << (line.overflow) << "," << (line.percentage) << "%" << "\n";
    }
  }

  file.close();



}

void LayoutWindow::calculateMinimumSize() {
  loadLookupFile("lookups.json");


  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_buffer_t* buffer = buffer = hb_buffer_create();

  //bool conti = true;

  struct Line {
    int pageNumber;
    int lineNumber;
    double overflow;
  };

  QMap<double, QVector<Line>> alloverflows;

  double scale = 0.8;

  while (scale <= 0.94) {

    int emScale = (1 << OtLayout::SCALEBY) * scale;

    //const int minSpace = OtLayout::MINSPACEWIDTH * emScale;
    //const int  defaultSpace = OtLayout::SPACEWIDTH * emScale;

    QVector<Line> overflows;

    for (int pagenum = 0; pagenum <= 603; pagenum++) {

      QString textt = QString::fromUtf8(qurantext[pagenum] + 1);

      auto lines = textt.split(char(10), Qt::SkipEmptyParts);

      auto page = m_otlayout->justifyPage(emScale, lineWidth, lineWidth, lines, LineJustification::Distribute, false, true);


      for (int linenum = 0; linenum < lines.length(); linenum++) {

        auto line = page[linenum];

        if (line.overfull != 0) {
          overflows.append({ pagenum + 1,linenum + 1, line.overfull / emScale });
        }
      }
    }

    if (overflows.count() > 0) {
      alloverflows[scale] = overflows;
    }

    scale = scale + 0.05;
  }

  hb_buffer_destroy(buffer);

  QString fileName = "overflows.csv";

  if (applyJustification) {
    fileName = "overflows_with_just.csv";
  }

  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("ISO 8859-1");
  //out.setEncoding(QStringConverter::Latin1);

  for (auto key : alloverflows.keys()) {
    auto overflow = alloverflows.value(key);
    for (auto line : overflow) {
      out << key << "," << line.pageNumber << "," << line.lineNumber << "," << (std::round)(line.overflow) << "\n";
    }
  }

  file.close();



}

void LayoutWindow::setQutranText(int type) {

  currentQuranText.clear();

  if (type == 1) {
    for (int i = 0; i < 604; i++) {
      currentQuranText.append(qurantext[i] + 1);
      suraNameByPage.append("");
    }
  }
  else {
    loadLookupFile("lookups.json");

    int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
    int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;
    auto result = m_otlayout->pageBreak(scale, lineWidth, false);



    for (auto& page : result.originalPages) {
      QString newPage;
      for (auto& line : page) {
        newPage.append(line + "\n");
      }
      currentQuranText.append(newPage);
    }

    suraNameByPage = result.suraNamebyPage;


  }

  integerSpinBox->setRange(1, currentQuranText.size());
}

static hb_bool_t setMessage(hb_buffer_t* buffer,
  hb_font_t* font,
  const char* message,
  void* user_data) {


  OtLayout* layout = reinterpret_cast<OtLayout*>(user_data);

  QString messa = message;

  int lookup = 0;
  if (messa.mid(0, 5) == "start") {
    lookup = messa.mid(13).toInt();
  }
  else {
    lookup = messa.mid(11).toInt();
  }

  if (lookup < layout->gsublookups.size()) {
    qDebug() << message << " " << layout->gsublookups.at(lookup)->name;
    // printf("%s %s\n", message, layout->gsublookups.at(lookup)->name.toLatin1().data());
  }

  if (lookup < layout->gposlookups.size()) {
    qDebug() << message << " " << layout->gposlookups.at(lookup)->name;
    //printf("%s %s\n", message, layout->gposlookups.at(lookup)->name.toLatin1().data());
  }


  return true;

}
void LayoutWindow::testQuarn() {

  hb_buffer_t* buffer = buffer = hb_buffer_create();
  hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));

  hb_font_t* font = m_otlayout->createFont(1000);

  for (int i = 0; i < 604; i++) {
    //hb_buffer_clear_contents(buffer);
    //hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
    //hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
    //hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));
    const char* text = qurantext[i];
    unsigned int text_len = strlen(text);

    hb_buffer_add_utf8(buffer, text, text_len, 0, text_len);

    //hb_shape(font, buffer, NULL, 0);


  }

  hb_shape(font, buffer, NULL, 0);

  uint glyph_count;
  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  //hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  //auto automedina = m_otlayout->automedina;
  auto waqgmark = m_otlayout->automedina->classtoUnicode("waqfmarks");
  auto marks = m_otlayout->automedina->classtoUnicode("marks");
  int totlaWaqfMark = 0;

  QMultiMap<QString, QString> beforewagf;

  for (uint i = glyph_count - 1; i >= 0; i--) {
    if (waqgmark.contains(glyph_info[i].codepoint)) {
      auto waqfName = m_otlayout->glyphNamePerCode[glyph_info[i].codepoint];
      for (uint j = i + 1; j < glyph_count; j++) {
        if (marks.contains(glyph_info[j].codepoint))
          continue;
        auto baseName = m_otlayout->glyphNamePerCode[glyph_info[j].codepoint];
        beforewagf.insertMulti(baseName, waqfName);
        break;
      }

      totlaWaqfMark++;
    }
  }

  auto keys = beforewagf.uniqueKeys();

  qDebug() << "Total waqf count : " << totlaWaqfMark << '\n' << "Total bases : " << beforewagf.uniqueKeys().size();

  delete font;

}
void LayoutWindow::executeRunText(bool newFace, int refresh)
{

  int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;

  if (m_graphicsScene->items().isEmpty()) {
    refresh = 2;
  }

  if (refresh == 2) {
    newFace = true;
    loadLookupFile("lookups.json");
  }

  QString textt = textEdit->toPlainText();

  QRegularExpression reaya("(\\d+)");
  reaya.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);

  textt == textt.replace(reaya, QString("۝") + "\\1");

  auto lines = textt.split(char(10), Qt::SkipEmptyParts);

  const int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;


  auto page = m_otlayout->justifyPage(scale, lineWidth, lineWidth, lines, LineJustification::Distribute, newFace, true);

  QVector<int> set;

  QList<QList<LineLayoutInfo>> pages = { page };

  if (this->applyCollisionDetection) {
    adjustOverlapping(pages, lineWidth, 0, 1, set, scale);
  }

  page = pages[0];

  int itemCount = 0;

  int pos_x = m_graphicsView->horizontalScrollBar()->value();
  int pos_y = m_graphicsView->verticalScrollBar()->value();

  if (refresh) {


    m_graphicsView->setScene(NULL);

    m_graphicsScene->clear();
    delete m_graphicsScene;

    m_graphicsScene = new GraphicsSceneAdjustment(this);
  }
  else {
    itemCount = m_graphicsScene->items().size() - 1;
  }

  auto listitems = m_graphicsScene->items();

  //QString glyphName = m_otlayout->glyphNamePerCode[57357];

  for (auto line : page) {
    int currentxPos = line.xstartposition;
    int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
    for (auto glyphLayout : line.glyphs) {
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

      if (m_otlayout->glyphs.contains(glyphName)) {
        GlyphVis& glyph = m_otlayout->glyphs[glyphName];

        //auto tt = m_otlayout->automedina->classes["bases"].contains(glyphName);

        GlyphItem* glyphItem = nullptr;
        if (refresh) {
          glyphItem = new GlyphItem(scale, &glyph, m_otlayout, glyphLayout.lookup_index, glyphLayout.subtable_index, glyphLayout.base_codepoint, glyphLayout.lefttatweel, glyphLayout.righttatweel);
          glyphItem->setFlag(QGraphicsItem::ItemIsMovable);
          m_graphicsScene->addItem(glyphItem);


        }
        else {
          if (itemCount >= 0) {
            glyphItem = (GlyphItem*)listitems.at(itemCount--);
          }
          else {
            std::cout << "Problem glyphs > elements" << '\n';
          }

        }

        if (glyphItem) {
          //coloring
          if (glyphLayout.color) {
            int color = (int)glyphLayout.color;
            glyphItem->setBrush(QColor{ (color >> 24) & 0xff ,(color >> 16) & 0xff ,(color >> 8) & 0xff });
          }


          currentxPos -= glyphLayout.x_advance;
          QPoint pos(currentxPos + (glyphLayout.x_offset), currentyPos - (glyphLayout.y_offset));
          glyphItem->setPos(pos);
        }
      }
    }

  }

  m_otlayout->clearAlternates();

  if (refresh) {
    m_graphicsView->setScene(m_graphicsScene);
  }

  m_graphicsView->horizontalScrollBar()->setValue(pos_x);
  m_graphicsView->verticalScrollBar()->setValue(pos_y);

}
void LayoutWindow::simpleAdjustPage(hb_buffer_t* buffer) {
  uint glyph_count;

  const int minSpace = OtLayout::MINSPACEWIDTH << OtLayout::SCALEBY;
  const int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  QVector<quint32> spaces;
  int currentlineWidth = 0;
  quint32 linenum = 0;

  for (int i = glyph_count - 1; i >= 0; i--) {

    if (glyph_info[i].codepoint == 10) {
      linenum++;
      int spaceaverage = (lineWidth - currentlineWidth) / spaces.size();
      if (spaceaverage > minSpace) {
        for (auto index : spaces) {
          glyph_pos[index].x_advance = spaceaverage;
        }
      }
      currentlineWidth = 0;
      spaces.clear();
    }
    else if (glyph_info[i].codepoint == 32) {
      glyph_pos[i].x_advance = minSpace;
      spaces.append(i);
    }
    else {
      currentlineWidth += glyph_pos[i].x_advance;
    }

  }

}

void LayoutWindow::adjustPage(QString text, hb_font_t* shapeFont, hb_buffer_t* buffer) {
  uint glyph_count;

  const int minSpace = OtLayout::MINSPACEWIDTH << OtLayout::SCALEBY;
  const int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  QVector<quint32> spaces;
  int currentlineWidth = 0;
  quint32 linenum = 0;
  int beginLine = glyph_count - 1;

  for (int i = glyph_count - 1; i >= 0; i--) {

    if (glyph_info[i].codepoint == 10) {
      linenum++;
      int spaceaverage = (lineWidth - currentlineWidth) / spaces.size();
      if (spaceaverage > minSpace) {
        for (auto index : spaces) {
          glyph_pos[index].x_advance = spaceaverage;
        }
      }
      else {
        // shrink
        //hb_buffer_t *shrinkbuffer = hb_buffer_create();

        hb_feature_t f;
        f.tag = HB_TAG('s', 'h', 'r', '6');
        f.value = 1;
        f.start = glyph_info[beginLine].cluster;
        f.end = glyph_info[i].cluster;

        hb_buffer_clear_contents(buffer);

        hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
        hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
        hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));

        //int item_length = glyph_info[i].cluster - glyph_info[beginLine].cluster + 1;

        hb_buffer_add_utf16(buffer, text.utf16(), -1, 0, -1);





        hb_shape(shapeFont, buffer, &f, 1);

        glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
        glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

        //hb_buffer_destroy(shrinkbuffer);

      }
      currentlineWidth = 0;
      spaces.clear();
      beginLine = i - 1;
    }
    else if (glyph_info[i].codepoint == 32) {
      glyph_pos[i].x_advance = minSpace;
      spaces.append(i);
    }
    else {
      currentlineWidth += glyph_pos[i].x_advance;
    }

  }



}

void LayoutWindow::adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, QList<QStringList> originalPages, int emScale) {

  //QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageSize pageSize{ { 90.2,147.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };

  int totalpageNb = pages.size();

  int nbthreads = 12;
  int pageperthread = totalpageNb / nbthreads;
  int remainingPages = totalpageNb;

  std::vector<QThread*> threads;
  std::vector<QVector<int>*> overlappages;

  // fetch all gryph initially otherwise mpost is not thread safe when executing getAlternate
  for (auto& page : pages) {
    for (auto& line : page) {
      for (auto& glyph : line.glyphs) {
        //GlyphVis* currentGlyph = 
        m_otlayout->getGlyph(glyph.codepoint, glyph.lefttatweel, glyph.righttatweel);
      }
    }
  }

  while (remainingPages != 0) {

    int begin = totalpageNb - remainingPages;
    if (pageperthread == 0 || remainingPages < pageperthread) {
      pageperthread = remainingPages;
    }

    remainingPages -= pageperthread;

    QVector<int>* set = new QVector<int>();

    overlappages.push_back(set);

    QThread* thread = QThread::create([this, &pages, begin, pageperthread, set, lineWidth, emScale] { adjustOverlapping(pages, lineWidth, begin, pageperthread, *set, emScale); });

    threads.push_back(thread);

    thread->start();
  }


  for (auto t : threads) {
    t->wait();
    delete t;
  }

  QList<QList<LineLayoutInfo>> newpages;
  QList<QStringList> neworiginalPages;

  for (auto t : overlappages) {
    for (auto pIndex : *t) {
      newpages.append(pages[pIndex]);
      neworiginalPages.append(originalPages[pIndex]);

      // ADD page number

      int pageNumber = pIndex + 1;
      int digits[] = { -1, -1, -1 };


      if (pageNumber < 10) {
        digits[0] = pageNumber;
      }
      else if (pageNumber < 100) {
        digits[0] = pageNumber % 10;
        digits[1] = pageNumber / 10;
      }
      else {
        digits[0] = pageNumber % 10;
        digits[1] = (pageNumber / 10) % 10;
        digits[2] = pageNumber / 100;
      }

      int totalwidth = 0;
      LineLayoutInfo lineInfo;

      for (int i = 0; i < 3; i++) {
        int digit = digits[i];
        if (digit == -1) break;

        auto& digitglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + digit]];
        GlyphLayoutInfo glyphInfo;

        glyphInfo.codepoint = 1632 + digit;
        glyphInfo.cluster = 0;
        glyphInfo.x_advance = (int)digitglyph.width + 40 << OtLayout::SCALEBY;
        glyphInfo.x_offset = 0;
        glyphInfo.y_offset = 0;
        glyphInfo.lookup_index = 0;
        glyphInfo.subtable_index = 0;
        glyphInfo.base_codepoint = 0;
        glyphInfo.lefttatweel = 0;
        glyphInfo.righttatweel = 0;

        lineInfo.glyphs.push_back(glyphInfo);

        totalwidth += glyphInfo.x_advance;

      }

      lineInfo.ystartposition = 27400 + 200 << OtLayout::SCALEBY;
      lineInfo.xstartposition = (lineWidth - totalwidth) / 2;

      auto& curpage = newpages.last();

      curpage.append(lineInfo);

      auto& gg = neworiginalPages.last();
      gg.append(QString::number(pageNumber));
    }

    delete t;
  }
#if defined(ENABLE_PDF_GENERATION)
  QuranPdfWriter allquran_overlapping("allquran_overlapping.pdf", m_otlayout);
  allquran_overlapping.setPageLayout(pageLayout);
  allquran_overlapping.setResolution(4800 << OtLayout::SCALEBY);

  allquran_overlapping.generateQuranPages(newpages, lineWidth, neworiginalPages, emScale);
#endif
}
void LayoutWindow::adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, int beginPage, int nbPages, QVector<int>& set, int emScale) {

  double scale = (double)emScale / (1 << OtLayout::SCALEBY); //(1 << OtLayout::SCALEBY) * 1; // 0.85;		
  //const int lineWidth = lineWidth; // (17000 - (2 * 400)) << OtLayout::SCALEBY;

  QHash<QPainterPath, GlyphLayoutInfo*> dict;

  //QTransform transform;
  QTransform pathtransform;

  //transform = transform.scale(scale, scale);
  pathtransform = pathtransform.scale(scale, -scale);

  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];

    QList<QList<QPoint>> pagePositions;
    bool intersection = false;

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);

      QList<QPoint> linePositions;

      for (int g = 0; g < line.glyphs.size(); g++) {

        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        currentxPos -= glyphLayout.x_advance;
        QPoint pos(currentxPos + (glyphLayout.x_offset), currentyPos - (glyphLayout.y_offset));

        linePositions.append(QPoint{ pos.x() >> OtLayout::SCALEBY,pos.y() >> OtLayout::SCALEBY });

      }

      pagePositions.append(linePositions);

    }

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      QList<QPoint>& linePositions = pagePositions[l];
      LineLayoutInfo suraName;


      for (int g = 0; g < line.glyphs.size(); g++) {

        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        if (glyphName.contains("space") || !m_otlayout->glyphs.contains(glyphName)) continue;

        //bool isIsol = glyphName.contains("isol");

        //bool isFina = glyphName.contains(".fina");

        bool isMark = m_otlayout->automedina->classes["marks"].contains(glyphName);

        //bool isWaqfMark = m_otlayout->automedina->classes["waqfmarks"].contains(glyphName);

        GlyphVis& currentGlyph = *m_otlayout->getGlyph(glyphName, glyphLayout.lefttatweel, glyphLayout.righttatweel);
        QPoint pos = linePositions[g];

        QRect rect{ QPoint(currentGlyph.bbox.llx,-currentGlyph.bbox.ury),QPoint(currentGlyph.bbox.urx,-currentGlyph.bbox.lly) };
        rect.translate(pos);

        QPainterPath path = pathtransform.map(currentGlyph.path);
        path.translate(pos);

        // verify with the line above
        //if (l > 0 && false) {
        if (l > 0) {
          int prev_index = l - 1;
          QList<QPoint>& prev_linePositions = pagePositions[prev_index];
          auto& prev_line = page[prev_index];

          for (int prev_g = 0; prev_g < prev_line.glyphs.size(); prev_g++) {
            auto& prev_glyphLayout = prev_line.glyphs[prev_g];
            QString prev_glyphName = m_otlayout->glyphNamePerCode[prev_glyphLayout.codepoint];

            bool isPrevMark = m_otlayout->automedina->classes["marks"].contains(prev_glyphName);
            bool isPrevrSpace = prev_glyphName.contains("space");
            //bool isPrevIsol = prev_glyphName.contains("isol");


            if ((isMark || isPrevMark) && !isPrevrSpace) { //|| isIsol || isPrevIsol

              GlyphVis& otherGlyph = *m_otlayout->getGlyph(prev_glyphName, prev_glyphLayout.lefttatweel, prev_glyphLayout.righttatweel); //m_otlayout->glyphs[prev_glyphName];
              QPoint otherpos = prev_linePositions[prev_g];

              QRect rectother{ QPoint(otherGlyph.bbox.llx,-otherGlyph.bbox.ury),QPoint(otherGlyph.bbox.urx,-otherGlyph.bbox.lly) };
              rectother.translate(otherpos);

              if (rect.intersects(rectother)) {

                QPainterPath otherpath = pathtransform.map(otherGlyph.path);
                otherpath.translate(otherpos);

                if (path.intersects(otherpath)) {

                  glyphLayout.color = 0xFF000000;
                  prev_glyphLayout.color = 0xFF000000;
                  intersection = true;


                }


              }

            }

          }

        }


        // verify previous glyphs in the same line
        if (g == 0) continue;


        for (int gg = g - 1; gg >= 0; gg--) {

          auto& otherglyphLayout = line.glyphs[gg];
          QString otherglyphName = m_otlayout->glyphNamePerCode[otherglyphLayout.codepoint];

          bool isOtherMark = m_otlayout->automedina->classes["marks"].contains(otherglyphName);
          //bool isOtherWaqfMark = m_otlayout->automedina->classes["waqfmarks"].contains(otherglyphName);
          bool isOtherSpace = otherglyphName.contains("space");
          //bool isPrevIsol = otherglyphName.contains("isol");
          //bool isPrevFina = otherglyphName.contains(".fina");

          if ((isMark || isOtherMark) && !isOtherSpace) {// || isIsol || isPrevIsol || (isFina && isPrevFina)

            GlyphVis& otherGlyph = *m_otlayout->getGlyph(otherglyphName, otherglyphLayout.lefttatweel, otherglyphLayout.righttatweel); //glyphs[otherglyphName];
            QPoint otherpos = linePositions[gg];

            QRect rectother{ QPoint(otherGlyph.bbox.llx,-otherGlyph.bbox.ury),QPoint(otherGlyph.bbox.urx,-otherGlyph.bbox.lly) };
            rectother.translate(otherpos);

            if (rect.intersects(rectother)) {

              QPainterPath otherpath = pathtransform.map(otherGlyph.path);
              otherpath.translate(otherpos);

              if (path.intersects(otherpath)) {

                int adjust = 0;

                if ((glyphName == "kasra" || glyphName.contains("dot")) && (otherglyphName.contains("reh") || otherglyphName.contains("waw") || otherglyphName == "meem.fina" || otherglyphName == "meem.fina.afterkaf" || otherglyphName == "meem.fina.afterheh")) {

                  adjust = rectother.bottom() - rect.top();
                  continue;
                }

                //if (!isWaqfMark) {

                glyphLayout.color = 0xFF000000;

                if (adjust > 0) {
                  glyphLayout.y_offset -= adjust << OtLayout::SCALEBY;
                }


                otherglyphLayout.color = 0xFF000000;
                intersection = true;

                //}



              }


            }

          }
        }


      }
    }

    if (intersection) {
      set.append(p);
    }


  }
}





