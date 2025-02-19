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
#include <QtSql>

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
#include <unordered_set>
#include <Subtable.h>
#include  <set>
#include "gllobal_strings.h"



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
  m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

  // 1 em = 50 pixel in screen
  qreal scale = (1.0 / (1000 << OtLayout::SCALEBY)) * 50;

  m_graphicsView->scale(scale, scale);

  m_graphicsScene = new GraphicsSceneAdjustment(this);

  m_graphicsView->setScene(m_graphicsScene);


  createActions();
  createDockWindows();

  setQuranText(1);

  integerSpinBox->setValue(3);

  layoutDatabase();

}

LayoutWindow::~LayoutWindow()
{
}

void LayoutWindow::layoutDatabase() {

  auto db = QSqlDatabase::database();

  if (!db.isValid()) {

    QDir appDir(QCoreApplication::applicationDirPath());
    QString dbPath = appDir.absoluteFilePath("quran-data.sqlite");
    QFile dbFile(dbPath);
    if (!dbFile.exists()) {
      QFile rsDb(":/quran-data.sqlite");
      if (!rsDb.copy(dbPath)) {
        qDebug() << rsDb.error() << rsDb.errorString();
      }
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    db.open();

    /*
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    db.open();

    QString data;
    QString dbName(":/quran-data.sqlite.sql");

    QFile file(dbName);
    if (!file.open(QIODevice::ReadOnly)) {
      qDebug() << "filenot opened" << endl;
    }
    else
    {
      QString data = file.readAll();
      auto commands = data.split(";", Qt::SkipEmptyParts);
      for (auto& command : commands) {
        auto query = db.exec(command);
        auto error = query.lastError();
        if (error.isValid()) {
          qDebug() << command << ":" << error;
        }
      }
    }
    file.close();*/
  }

  mushafLayouts = new QComboBox;

  QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table';");
  while (query.next()) {
    QString tableName = query.value(0).toString();
    if (tableName != "words") {
      mushafLayouts->addItem(tableName);
    }
  }

  connect(mushafLayouts, &QComboBox::currentTextChanged, [&](QString text) {
    QSettings settings;
    settings.setValue("LastMushafLayout", text);
    loadMushafLayout(text);
    });

  fileToolBar->addWidget(mushafLayouts);

  QSettings settings;
  QString lastLayout = settings.value("LastMushafLayout").value<QString>();
  if (!lastLayout.isEmpty()) {
    mushafLayouts->setCurrentText(lastLayout);
  }
  else {
    mushafLayouts->setCurrentText("qpc_v2_layout");
  }

  auto currentLayout = mushafLayouts->currentText();

  loadMushafLayout(currentLayout);
}

static void changeText(QString textCol, QString& word) {

  static auto re = QRegularExpression("([٠١٢٣٤٥٦٧٨٩]+)");
  static auto reRem = QRegularExpression("[\u0020\u06DF\uF64B\uf64c\uf64d\uf64e\uf650\uf652\uf651\uf662\uf663\uf64f]");
  static auto ayaCodes = QRegularExpression("[\uF500-\uF61D]");
  static auto meemIqlab = QRegularExpression("[\uf64a\uf65d\uf65b\uf66a]");
  static auto lowMeemIqlab = QRegularExpression("[\uf66b\uf66d]");
  static auto ayaWordWithWaqfs = QRegularExpression("(.+)([\uF500-\uF61D])");


  static QString waqfChars = "࣢ࣛࣝࣞۖۚۙؔۘۛؕࣕࣖࣗؗ";
  static QRegularExpression waqfSeq(QString("([٠١٢٣٤٥٦٧٨٩]*)([%1]+)").arg(waqfChars));

  if (textCol == "indopak") {
    word.remove(reRem);
    word.replace(ayaWordWithWaqfs, "\\2\\1");
    QRegularExpressionMatch match = ayaCodes.match(word);
    if (match.hasMatch()) {
      auto matched = match.captured(0).at(0);
      int ayaNumber = matched.unicode() - 0xF4FF;
      int unite = ayaNumber % 10;
      int dizaine = ayaNumber / 10 % 10;
      int centaine = ayaNumber / 100 % 10;
      QString ayaString = "\u06DD";
      ayaString += centaine ? QString(centaine + 0x0660) : "";
      ayaString += centaine || dizaine ? QString(dizaine + 0x0660) : "";
      ayaString += QString(unite + 0x0660);

      word.replace(match.capturedStart(), match.capturedLength(), ayaString);
    }
    word.replace(meemIqlab, "\u06E2");
    word.replace(lowMeemIqlab, "\u06ED");
    word.replace("\uF61E", "\u06DD");
    word.replace("\u06E0", "\u08D6");
    word.replace("\u06EA", "\u08D5");
    word.replace("\u06D7", "\u08D7");
    word.replace("\u06EB", "\u08DE"); // ARABIC EMPTY CENTRE HIGH STOP => ARABIC SMALL HIGH WORD QIF
    word.replace("\u06EA", "\u08D5"); // ARABIC EMPTY CENTRE LOW STOP => 08D5 ARABIC SMALL HIGH SAD
    word.replace("\u0653", "\u089C"); // ARABIC MADDAH ABOVE => ARABIC MADDA WAAJIB
    word.replace("\u06E4", "\u0653"); // ARABIC SMALL HIGH MADDA => ARABIC MADDAH ABOVE
    word.replace("\u06EC", "\u08E2"); // ARABIC ROUNDED HIGH STOP WITH FILLED CENTRE => ARABIC DISPUTED END OF AYAH
    word.replace("\u06E5", "\u08DF"); // 06E5 ARABIC SMALL WAW => 08DF ARABIC SMALL HIGH WORD WAQFA
    word.replace("\u06D9\u08E2\u06E6", "\u202E\u0666\u08E2\u06D9"); // ARABIC SMALL YEH => ARABIC-INDIC DIGIT SIX
    word.replace("\u065a", "\u08DD"); // 065A ARABIC VOWEL SIGN SMALL V ABOVE => 08DD ARABIC SMALL HIGH WORD SAKTA
    word.replace("\uF664", "نْثٰي");
    word.replace("\uF61F", "\u0627\u08D9");
    word.replace("\u06E1", "\u06DF"); // 06E1 ARABIC SMALL HIGH DOTLESS HEAD OF KHAH => 06DF ARABIC SMALL HIGH ROUNDED ZERO
    word.replace("\uf653", "\u06E6\u064E"); // => 06E6 ARABIC SMALL YEH + 064E ARABIC FATHA
    word.replace("\uF65E\u0646\u064F\u0640", "\u0646\u064F\u0640\u06E8\u0652"); // نُـۨجِي
    word.replace("\uF657", "\u0654\u064D");
    word.replace("\uf63c", "۝١٧١ࣖۚۛ");
    word.replace("\uf658", "وَلْيَتَؔلَطَّفْ");
    word.replace("\uf665", "نْثٰٓي");
    word.replace("\uf669", "كّٰٓي");
    word.replace("\uf666", "ثُلُثَيِ");
    word.replace("\uf667", "لّٰ࢜ـِٔيْ");
    word.replace("\uf694", "ِٔ");
    word.replace("\uf634", "۝٣١ۙۚۛ");
    word.replace("\uf690", "۝٣٦ؔ");
    word.replace("\uf691", "۝٩١ۚۖۛ");
    word.replace("\uf68e", "۝٢٠٦ࣛࣖ");
    word.replace("\uf681", "۝١٥ࣛ");
    word.replace("\uf688", "۝٥٠ࣛࣖ");
    word.replace("\uf68d", "۝١٠٩ࣛ");
    word.replace("\uf689", "۝٥٨ࣛ");
    word.replace("\uf692", "۝١٨ࣛؕ");
    word.replace("\uf693", "۝٧٧ࣛۚ");
    word.replace("\uf68f", "ࣕۙ");
    word.replace("\uf697", "ۙࣕ");
    word.replace("\uf638", "۝٥٨ۙۚۛ");
    word.replace("\uf68a", "۝٦٠ࣛࣖ");
    word.replace("\uf63d", "۝٢٠٨ࣗۖۛ");
    word.replace("\uf686", "۝٢٦ࣛ");
    word.replace("\uf633", "۝١٨ؔ");
    word.replace("\uf639", "۝٦٠ۚۖۛ");
    word.replace("\uf685", "۝٢٤ࣛ");
    word.replace("\uf631", "۝٦ۘؔ");
    word.replace("\uf63a", "۝٦٩ۚۖۛ");
    word.replace("\uf687", "۝٣٨ࣛ");
    word.replace("\uf636", "۝٤٤ۚۖۛ");
    word.replace("\uf63e", "ۛࣞۚ࣢");
    word.replace("\uf637", "۝٥٤ࣗؗ");
    word.replace("\uf668", "فّٰٓي");
    word.replace("\uf68c", "۝٦٢ࣛࣖ");
    word.replace("\uf654", "ۛۖۚ࣢");
    word.replace("\uf684", "۝٢١ࣛؕ");
    word.replace("\uf632", "۝١١ࣕۙ");
    word.replace("\uf699", "ۙࣕ࣢");
    word.replace("\uf683", "۝١٩ࣛࣖ");
    word.replace("نْۢ", "نۢ͏ْ");

    auto iter = waqfSeq.globalMatch(word);
    while (iter.hasNext()) {
      QRegularExpressionMatch match = iter.next();
      QString ayaNum = match.captured(1);
      auto isAya = !ayaNum.isEmpty();
      QString seq = match.captured(2);
      if (seq.size() < 2) continue;
      if (!isAya && seq != "ۚؗ") {
        std::reverse(seq.begin(), seq.end());
      }
      word.replace(match.capturedStart(2), match.capturedLength(2), seq);

    }
    word.replace("آٰ", "اٰ͏ٓ");
    word.replace("\u06CC", "\u064A"); // replace Yeh farsi by Arabic Yeh
    // decomposition
    word.replace("\u0623", "\u0627\u0654"); // ARABIC LETTER ALEF WITH HAMZA ABOVE
    word.replace("\u0624", "\u0648\u0654"); // ARABIC LETTER WAW WITH HAMZA ABOVE    
    word.replace("\u0626", "\u064A\u0654"); // ARABIC LETTER YEH WITH HAMZA ABOVE
  }
  else if (textCol == "nastaleeq") {
    //TODO complete conversion
    word.replace(re, "\u06DD\\1");
    word.replace("\uFDA8", "\u08DE"); // ARABIC SMALL HIGH WORD QIF
    word.replace("\uFD74", "\u08DE"); // ARABIC SMALL HIGH WORD QIF
    word.replace("\u0653", "\u089C"); // ARABIC MADDAH ABOVE => ARABIC MADDA WAAJIB
    word.replace("\u0658", "\u0653"); // ARABIC MARK NOON GHUNNA => ARABIC MADDAH ABOVE
  }
  else if (textCol == "dk_v1") {
    word.replace("\u06D6\u06D6", "\u06D6"); // duplicate 06D6
  }

  // Resolve Tajweed coloring and justification due to reording marks (otherwise for example fatha get stretched in لِئَلَّا)
  word.replace("\u0627\u0653", "\u0627\u034F\u0653");
  word.replace("\u0627\u0654", "\u0627\u034F\u0654\u034F");
  word.replace("\u0648\u0654", "\u0648\u034F\u0654\u034F");
  word.replace("\u064A\u0654", "\u064A\u034F\u0654\u034F");
  word.replace("\u0640\u0654", "\u0640\u0654\u034F");
  word.replace("\u0627\u0655", "\u0627\u0655\u034F");
}

void LayoutWindow::loadMushafLayout(QString layoutName) {
  auto textCol = layoutName.startsWith("indopak") ? "indopak" : layoutName == "qpc_v1_layout" ? "dk_v1" : "dk_v2";

  auto queryString = QString("SELECT page,line,type, %1 as text from %2 as l LEFT JOIN words w ON l.type = \"ayah\" AND l.range_start <= w.word_number_all AND l.range_end >= w.word_number_all order by page,line,word_number_all")
    .arg(textCol).arg(layoutName);

  currentQuranText.clear();
  suraNameByPage.clear();



  QSqlQuery query(queryString);
  int lastPage = 1;
  int lastLine = 1;
  int lastSurahNumber = 0;
  int wordNumberInLine = 1;
  QString currentPage;

  while (query.next()) {
    int page = query.value(0).toInt();
    int line = query.value(1).toInt();
    QString type = query.value(2).toString();
    QString word = query.value(3).toString();


    if (type == "surah_name") {
      word = QString::fromStdString(surahNames[lastSurahNumber]);
      if (textCol == "indopak") {
        word.replace("\u064E\u0670", "\u0670");
        word.replace("\u0627\u0655\u0650", "\u0627\u0650");
      }
      lastSurahNumber++;
    }
    else if (type == "basmallah") {
      if (textCol != "indopak") {
        word = "\nبِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
      }
      else {
        word = "\nبِسْمِ اللّٰهِ الرَّحْمٰنِ الرَّحِيْمِ ۝";
      }
    }
    else {
      changeText(textCol, word);
    }

    if (lastPage != page) {
      currentQuranText.append(currentPage);
      suraNameByPage.append("");
      lastPage = page;
      lastLine = 1;
      currentPage = word;
    }
    else if (lastLine != line && !(lastPage == 213 && lastLine == 4 && wordNumberInLine <= 11)) {
      currentPage += "\n" + word;
      lastLine = line;
      wordNumberInLine = 1;
    }
    else if (currentPage.isEmpty()) {
      currentPage = word;
      wordNumberInLine++;
    }
    else {
      currentPage += " " + word;
      wordNumberInLine++;
    }
  }
  currentQuranText.append(currentPage);
  suraNameByPage.append("");
  integerSpinBox->setRange(1, currentQuranText.size());
  integerSpinBox->valueChanged(integerSpinBox->value());
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

  connect(typeGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::idClicked), this, &LayoutWindow::setQuranText);

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

  otfcff2Act = new QAction(otfcff2Icon, tr("&Generate OpenType CFF2 Standard No Var"), this);
  otfcff2Act->setStatusTip(tr("Generate OpenType CFF2 Standard No Var"));
  connect(otfcff2Act, &QAction::triggered, this, &LayoutWindow::generateOpenTypeCff2StandardWithoutVar);
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
  QAction* action = new QAction(icon, tr("&Generate Mushaf..."), this);
  connect(action, &QAction::triggered, [&]() {
    generateMushaf(false);
    });
  fileMenu->addAction(action);
  fileToolBar->addAction(action);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
#endif

  QIcon genMedinaIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* genMedinaAction = new QAction(genMedinaIcon, tr("&Generate Medina Quran HTML"), this);
  connect(genMedinaAction, &QAction::triggered, this, [&]() {
    generateMushaf(true);
    });
  fileMenu->addAction(genMedinaAction);
  fileToolBar->addAction(genMedinaAction);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QIcon genLayoutIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* genLayoutAction = new QAction(genLayoutIcon, tr("&Generate Layout Info"), this);
  connect(genLayoutAction, &QAction::triggered, this, &LayoutWindow::generateLayoutInfo);
  fileMenu->addAction(genLayoutAction);
  fileToolBar->addAction(genLayoutAction);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QIcon saveCollIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* saveCollAction = new QAction(saveCollIcon, tr("&Save Collision"), this);
  connect(saveCollAction, &QAction::triggered, this, [&]() {
    saveCollision();
    });
  fileMenu->addAction(saveCollAction);
  fileToolBar->addAction(saveCollAction);
  fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);



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

  justCombo = new QComboBox;

  justCombo->addItem("No Justification", qVariantFromValue(JustType::None));
  justCombo->addItem("HarfBuzz", qVariantFromValue(JustType::HarfBuzz));
  justCombo->addItem("Madina", qVariantFromValue(JustType::Madina));
  justCombo->addItem("IndoPak", qVariantFromValue(JustType::IndoPak));
  justCombo->addItem("Experimental", qVariantFromValue(JustType::Experimental));

  jutifyToolbar->addWidget(justCombo);

  QSettings settings;
  QString lastJust = settings.value("LastJust").value<QString>();
  if (!lastJust.isEmpty()) {
    justCombo->setCurrentText(lastJust);
  }
  else {
    justCombo->setCurrentIndex(1);
  }

  this->applyJustification = justCombo->currentData().value<JustType>() != JustType::None;

  connect(justCombo, qOverload<int>(&QComboBox::currentIndexChanged), [&](int index) {
    QSettings settings;
    applyJustification = justCombo->currentData().value<JustType>() != JustType::None;
    m_otlayout->applyJustification = applyJustification;
    settings.setValue("LastJust", justCombo->currentText());
    executeRunText(true, 1);
    });

  justStyleCombo = new QComboBox;

  justStyleCombo->addItem("No Style", qVariantFromValue(JustStyle::None));
  justStyleCombo->addItem("Same Size By page", qVariantFromValue(JustStyle::SameSizeByPage));
  justStyleCombo->addItem("XScale", qVariantFromValue(JustStyle::XScale));
  justStyleCombo->addItem("FontSize", qVariantFromValue(JustStyle::FontSize));
  //justStyleCombo->addItem("SCLX Axis", qVariantFromValue(JustStyle::SCLX));



  jutifyToolbar->addWidget(justStyleCombo);

  QString lastJustStyle = settings.value("LastJustStyle").value<QString>();
  if (!lastJustStyle.isEmpty()) {
    justStyleCombo->setCurrentText(lastJustStyle);
  }
  else {
    justStyleCombo->setCurrentIndex(1);
  }

  connect(justStyleCombo, qOverload<int>(&QComboBox::currentIndexChanged), [&](int index) {
    QSettings settings;
    settings.setValue("LastJustStyle", justStyleCombo->currentText());
    executeRunText(true, 1);
    });

  QPushButton* toggleButton = new QPushButton(tr("&Collision Detection"));
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  connect(toggleButton, &QPushButton::toggled, [&](bool checked) {
    applyCollisionDetection = checked;
    executeRunText(true, 1);
    });

  fileToolBar->addWidget(toggleButton);


  toggleButton = new QPushButton(tr("&TeX Algo"));
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  connect(toggleButton, &QPushButton::toggled, [&](bool checked) {
    applyTeXAlgo = checked;
    executeRunText(true, 1);
    });



  fileToolBar->addWidget(toggleButton);

  toggleButton = new QPushButton(tr("&Force"));
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  connect(toggleButton, &QPushButton::toggled, [&](bool checked) {
    applyForce = checked;
    executeRunText(true, 1);
    });

  fileToolBar->addWidget(toggleButton);

}
bool LayoutWindow::generateOpenTypeCff2Standard() {
  return generateOpenTypeCff2(false, true);
}
bool LayoutWindow::generateOpenTypeCff2Extended() {
  return generateOpenTypeCff2(true, true);
}
bool LayoutWindow::generateOpenTypeCff2StandardWithoutVar() {
  return generateOpenTypeCff2(false, false);
}
bool LayoutWindow::generateOpenTypeCff2(bool extended, bool generateVariableOpenType) {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString otfFileName = fileInfo.path() + "/output/" + fileInfo.completeBaseName() + ".otf";

  OtLayout layout = OtLayout(m_font, extended, extended ? true : generateVariableOpenType);

  layout.toOpenType->isCff2 = true;

  auto ret = layout.toOpenType->GenerateFile(otfFileName);

  QString fileName = fileInfo.absolutePath() + "/output/" + fileInfo.completeBaseName() + "_glyphnames.lua";

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
  fileName = fileInfo.absolutePath() + "/output/" + fileInfo.completeBaseName() + "_lookupnames.lua";

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

void LayoutWindow::generateTestFile() {

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString fileName = fileInfo.path() + "/output/test" + fileInfo.completeBaseName() + "font.tex";

  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out.setCodec("UTF-8");

  auto startSection = "{\\sloppy\\huge\\myfont\\pagedir TRT\\pardir TRT\\bodydir TRT\\textdir TRT\\fontdimen2\\font=2ex\\linespread{2}\\selectfont\\vskip-30pt\n";

  out << GlobalStrings::textHeader;

  out << "\\title{" << fileInfo.completeBaseName() << ".otf Font Test }\n";
  out << "\\newfontfamily\\myfont[Renderer=OpenType,Script=Arabic]{" << fileInfo.completeBaseName() << ".otf}" << "\n";
  out << "\\begin{document}\n\\maketitle";


  out << "\\section{Joined Marks}\n";
  out << startSection;


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
        smallseenWords = smallseenWords.isEmpty() ? word : smallseenWords + " " + word;
        if (smallseenWords.length() > 60) {
          output = output.isEmpty() ? smallseenWords : output + "\n" + smallseenWords;
          smallseenWords = "";
        }
      }
    }
  }

  out << output << "\n" << smallseenWords << "\\par\n";
  out << "}\n";

  out << "\\section{Alternates}\n\n";
  out << startSection;

  QString alternates = "نفقكيصضبتثسش";
  QString fatha = "َ";

  for (auto start : { QString(""),QString("بَ") }) {
    for (auto alternate : alternates) {
      out << "\\noindent{}";
      for (auto i : { 0,1,2,3,4,5,6,7,8,9,10,11,12 }) {
        out << start << "\\setfea{" << alternate << "}{cv01=" << i << "}";
        out << "\\setfea{" << fatha << "}{cv01=" << 1 + (int)std::floor(i / 3) << "}";
        out << " ";
      }
      out << "\\par\n";
    }
  }

  out << "}\n";

  out << "\\end{document}";


}
bool LayoutWindow::generateOpenType() {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString otfFileName = fileInfo.path() + "/output/" + fileInfo.completeBaseName() + "-cff1.otf";

  OtLayout layout = OtLayout(m_font, false, true);
  layout.useNormAxisValues = true;
  layout.toOpenType->isCff2 = true;

  layout.loadLookupFile("features.fea");

  return layout.toOpenType->GenerateFile(otfFileName);

}

struct BaseMarkPair {
  QString baseName;
  double baseLeftTatweel = 0;
  double baseRightTatweel = 0;
  QString markName;
  double markLeftTatweel = 0;
  double markRighttTatweel = 0;
  bool operator==(const BaseMarkPair& other) const {
    return (
      baseName == other.baseName
      && baseLeftTatweel == other.baseLeftTatweel
      && baseRightTatweel == other.baseRightTatweel
      && markName == other.markName
      && markLeftTatweel == other.markLeftTatweel
      && markRighttTatweel == other.markRighttTatweel
      );
  }
};

namespace std {
  template<>
  struct hash<BaseMarkPair> {
    const size_t operator()(const BaseMarkPair& x) const
    {
      return std::hash<QString>()(x.baseName) ^ std::hash<double>()(x.baseLeftTatweel) ^ std::hash<double>()(x.baseRightTatweel)
        ^ std::hash<QString>()(x.markName) ^ std::hash<double>()(x.markLeftTatweel) ^ std::hash<double>()(x.markRighttTatweel);
    }
  };
}



void LayoutWindow::checkOffMarks() {
  double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;

  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMushaf(scale, lineWidth, m_otlayout, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

  struct CheckResult {
    int pageIndex;
    int lineIndex;
    int baseGlyphIndex;
    int markGlyphIndex;
    QString  baseGlyphName;
    double baseLeftTatweel = 0;
    double baseRightTatweel = 0;
    QString  markGlyphName;
    double markLeftTatweel = 0;
    double markRighttTatweel = 0;
    QString word;
  };






  QVector<CheckResult> results;

  for (int p = 0; p < result.pages.size(); p++) {
    auto& page = result.pages[p];

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

        linePositions.append(QPoint{ pos.x(),pos.y() });

      }

      pagePositions.append(linePositions);

    }

    for (int l = 0; l < page.size(); l++) {

      auto& line = page[l];

      double scale = line.fontSize;

      QList<QPoint>& linePositions = pagePositions[l];
      LineLayoutInfo suraName;

      QVector<QPainterPath> paths;

      GlyphVis* baseGlyph = nullptr;
      QPoint basePos;
      int baseIndex;

      for (int g = 0; g < line.glyphs.size(); g++) {

        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        bool isMark = m_otlayout->automedina->classes["marks"].contains(glyphName);

        if (!isMark) {
          baseGlyph = m_otlayout->getGlyph(glyphName, { .lefttatweel = glyphLayout.lefttatweel, .righttatweel = glyphLayout.righttatweel });
          baseIndex = g;
          basePos = linePositions[baseIndex];
          continue;
        }

        if (!baseGlyph) {
          throw std::runtime_error("Error");
        }

        GlyphVis* markGlyph = m_otlayout->getGlyph(glyphName, { .lefttatweel = glyphLayout.lefttatweel, .righttatweel = glyphLayout.righttatweel });
        QPointF markPos = linePositions[g];

        double markXStart = markPos.x() + markGlyph->bbox.llx * scale;
        double markWidth = (markGlyph->bbox.urx - markGlyph->bbox.llx) * scale;
        double markXEnd = markXStart + markWidth;


        double baseXStart = basePos.x() + baseGlyph->bbox.llx * scale;
        double baseWidth = (baseGlyph->bbox.urx - baseGlyph->bbox.llx) * scale;
        double baseXEnd = baseXStart + baseWidth;
        double leftOff = baseXStart - markXStart;
        double rightOff = markXEnd - baseXEnd;

        auto maxAcceptOff = markWidth * 0.25;

        if ((leftOff > maxAcceptOff || rightOff > maxAcceptOff) && baseWidth > 1.5 * markWidth) {
          QString text = result.originalPages[p][l];
          int startCluster = 0;
          int endCluster = text.size();

          for (int i = baseIndex; i >= 0; i--) {
            auto& glyphLayout = line.glyphs[i];
            QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
            if (glyphName.contains("space")) {
              startCluster = glyphLayout.cluster + 1;
              break;
            }
          }
          for (int i = g; i < line.glyphs.size(); i++) {
            auto& glyphLayout = line.glyphs[i];
            QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
            if (glyphName.contains("space")) {
              endCluster = glyphLayout.cluster;
              break;
            }
          }

          QString  word = text.mid(startCluster, endCluster - startCluster);

          auto baseGlyphName = baseGlyph->name == "alternatechar" ? baseGlyph->originalglyph : baseGlyph->name;
          auto markGlyphName = markGlyph->name == "alternatechar" ? markGlyph->originalglyph : markGlyph->name;

          results.append({ p,l,baseIndex,g,
            baseGlyphName,baseGlyph->charlt, baseGlyph->charrt,
            markGlyphName,markGlyph->charlt,markGlyph->charrt,
            word });
        }
      }
    }
  }

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/offmarks.txt";

  QFile file(outputFileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");

  std::unordered_set<BaseMarkPair> duplicates;

  QVector<QString> words;

  for (auto result : results) {

    BaseMarkPair pair{ result.baseGlyphName,result.baseLeftTatweel,result.baseRightTatweel,
      result.markGlyphName,result.markLeftTatweel,result.markRighttTatweel };
    if (!duplicates.contains(pair)) {
      out << "Page=" << result.pageIndex + 1 << ",Line=" << result.lineIndex + 1
        << ",baseGlyph=" << result.baseGlyphName
        << ",markGlyph=" << result.markGlyphName
        << ",Word=" << result.word << "\n";
      duplicates.insert(pair);
      words.append(result.word);
    }
  }

  out << "**************************************words**************\n";
  int nbWordbyline = 0;
  for (auto& word : words) {
    if (nbWordbyline == 8) {
      out << '\n';
      nbWordbyline = 0;
    }
    else if (nbWordbyline != 0) {
      out << ' ';
    }
    out << word;
    nbWordbyline++;

  }

}

bool LayoutWindow::save() {
  QApplication::setOverrideCursor(Qt::WaitCursor);

  auto parametersFileName = QDir(m_font->currentDir()).filePath("parameters.json");

  QFile file(parametersFileName);
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

  loadLookupFile("features.fea");

  QString textt = textEdit->toPlainText();

  auto lines = textt.split(char(10), Qt::SkipEmptyParts);

  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto page = m_otlayout->justifyPage(scale, lineWidth, lineWidth, lines, LineJustification::Distribute, true, true,
    justStyleCombo->currentData().value<JustStyle>(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES,
    justCombo->currentData().value<JustType>());

  QList<QList<LineLayoutInfo>> pages{ page };
  QList<QStringList> originalPages{ lines };

  //lineWidth = (17000 - (2 * 400));

  int margin = 0;

  GlyphVis::BBox cropBox{ std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::min(), std::numeric_limits<double>::min() };

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
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/quran.pdf";
  QuranPdfWriter quranWriter(outputFileName, m_otlayout);
  quranWriter.setPageLayout(pageLayout);
  quranWriter.setResolution(4800 << OtLayout::SCALEBY);

  quranWriter.generateQuranPages(pages, -cropBox.llx + ((int)(margin / 2) << OtLayout::SCALEBY), originalPages, scale, 0);

#endif

  return true;
}

static QMap<int, double> madinaLineWidths =
{
  { 586 * 15 + 1, 0.81},
  { 593 * 15 + 2, 0.81},
  { 594 * 15 + 5, 0.63},
  { 600 * 15 + 10,0.63 },
  { 601 * 15 + 3, 1 },
  { 601 * 15 + 4, 1 },
  { 601 * 15 + 7, 1 },
  { 601 * 15 + 8, 1 },
  { 601 * 15 + 9, 1 },
  { 601 * 15 + 10, 1 },
  { 601 * 15 + 13, 1 },
  { 601 * 15 + 14, 1 },
  { 601 * 15 + 15, 1 },
  { 602 * 15 + 5, 0.63 },
  { 602 * 15 + 11, 0.9 },
  { 602 * 15 + 15, 0.53 },
  { 603 * 15 + 10, 0.66 },
  { 603 * 15 + 13, 1 },
  { 603 * 15 + 15, 0.60 },
  { 604 * 15 + 3, 1 },
  { 604 * 15 + 4, 0.55 },
  { 604 * 15 + 7, 1 },
  { 604 * 15 + 8, 1 },
  { 604 * 15 + 9, 0.55 },
  { 604 * 15 + 12, 1 },
  { 604 * 15 + 13, 1 },
  { 604 * 15 + 14, 0.675 },
  { 604 * 15 + 15, 0.5 },
};

LayoutPages LayoutWindow::shapeMushaf(double scale, int pageWidth, OtLayout* layout, hb_buffer_cluster_level_t  cluster_level) {
  loadLookupFile("features.fea");

  LayoutPages result;
  QStringList originalPage;

  bool newface = true;


  auto justification = LineJustification::Distribute;

  QString suraWord = "سُورَةُ";
  QString bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

  QString surapattern = "^("
    + suraWord + " .*|"
    + bism
    + "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
    + ")$";

  QRegularExpression surabism(surapattern, QRegularExpression::MultilineOption);

  for (int pagenum = 0; pagenum < currentQuranText.size(); pagenum++) {

    auto& pageText = currentQuranText[pagenum];

    auto lines = pageText.split(char(10), Qt::SkipEmptyParts);
    QVector<LineToJustify> newLines;

    for (int lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
      auto newJustification = justification;
      auto line = QStringList{ lines[lineIndex] };
      int key = (pagenum + 1) * 15 + (lineIndex + 1);
      int lineWidth = pageWidth;
      auto match = surabism.match(lines[lineIndex]);

      LineType lineType = LineType::Line;

      if (match.hasMatch()) {
        if (match.captured(0).startsWith("سُ")) {
          lineType = LineType::Sura;
        }
        else {
          lineType = LineType::Bism;
        }

        if (!((pagenum == 0 || pagenum == 1) && lineIndex == 1)) {
          lineWidth = 0;
          newJustification = LineJustification::Center;
        }

      }

      if (madinaLineWidths.contains(key))
      {
        double ratio = madinaLineWidths.value(key);

        if (ratio < 1) {
          lineWidth = pageWidth * ratio;
          newJustification = LineJustification::Center;
        }
      }

      newLines.append({ lines[lineIndex] ,lineWidth ,newJustification,lineType });
    }



    auto shapedPage = layout->justifyPage(scale, pageWidth, newLines, newface, true, justStyleCombo->currentData().value<JustStyle>(), cluster_level, justCombo->currentData().value<JustType>());
    newface = false;
    result.pages.append(shapedPage);
    result.originalPages.append(lines);

  }

  return result;

}

LayoutPages LayoutWindow::shapeMedina(double scale, int pageWidth, OtLayout* layout, hb_buffer_cluster_level_t  cluster_level) {

  loadLookupFile("features.fea");

  LayoutPages result;
  QStringList originalPage;


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

  sajdapatterns = sajdapatterns.replace("\u0626", "\u0626\u034F");

  QRegularExpression sajdaRe = QRegularExpression(sajdapatterns, QRegularExpression::MultilineOption);

  int beginsajda = 0;
  int endsajda = 0;
  int sajdamatched = 0;

  QString page584 = QString::fromUtf8(qurantext[583] + 1);
  bool oldMadinah = page584.contains("سُورَةُ عَبَسَ");



  for (int pagenum = 0; pagenum < 604; pagenum++) {
    //for (int pagenum = 48; pagenum < 49; pagenum++) {

    QString textt = QString::fromUtf8(qurantext[pagenum] + 1);

    /*
    textt = textt.replace("\u0623", "\u0627\u0654");
    textt = textt.replace("\u0624", "\u0648\u0654");
    textt = textt.replace("\u0625", "\u0627\u0655");
    textt = textt.replace("\u0626", "\u064A\u0654");*/

    textt = textt.replace("\u0626", "\u0626\u034F");


    QStringList lines;

    if (!oldMadinah) {
      if (pagenum == 583) {
        textt.append(QString("سُورَةُ عَبَسَ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          //return {};
          throw new std::runtime_error("ERROR");
        }

      }
      else if (pagenum == 584) {
        textt = textt.replace(QString("سُورَةُ عَبَسَ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 75) {
        textt.append(QString("سُورَةُ النِّسَاءِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 76) {
        textt = textt.replace(QString("سُورَةُ النِّسَاءِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 348) {
        textt.append(QString("سُورَةُ النُّورِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 349) {
        textt = textt.replace(QString("سُورَةُ النُّورِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 365) {
        textt.append(QString("سُورَةُ الشُّعَرَاءِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 366) {
        textt = textt.replace(QString("سُورَةُ الشُّعَرَاءِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 375) {
        textt.append(QString("سُورَةُ النَّمْلِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 376) {
        textt = textt.replace(QString("سُورَةُ النَّمْلِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 444) {
        textt.append(QString("سُورَةُ الصَّافَّاتِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 445) {
        textt = textt.replace(QString("سُورَةُ الصَّافَّاتِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 451) {
        textt.append(QString("سُورَةُ صٓ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 452) {
        textt = textt.replace(QString("سُورَةُ صٓ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 497) {
        textt.append(QString("سُورَةُ الجَاثِيَةِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 498) {
        textt = textt.replace(QString("سُورَةُ الجَاثِيَةِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 505) {
        textt.append(QString("سُورَةُ مُحَمَّدٍ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 506) {
        textt = textt.replace(QString("سُورَةُ مُحَمَّدٍ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 524) {
        textt.append(QString("سُورَةُ النَّجْمِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 525) {
        textt = textt.replace(QString("سُورَةُ النَّجْمِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 527) {
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 547) {
        textt.append(QString("سُورَةُ المُمْتَحنَةِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 548) {
        textt = textt.replace(QString("سُورَةُ المُمْتَحنَةِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 554) {
        textt.append(QString("سُورَةُ التَّغَابُنِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 555) {
        textt = textt.replace(QString("سُورَةُ التَّغَابُنِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else if (pagenum == 556) {
        textt.append(QString("سُورَةُ الطَّلَاقِ") + "\n");
        auto page = layout->pageBreak(scale, pageWidth, false, textt, 1);
        if (page.size() == 1) {
          lines = page[0];
        }
        else {
          throw new std::runtime_error("ERROR");
        }
      }
      else if (pagenum == 557) {
        textt = textt.replace(QString("سُورَةُ الطَّلَاقِ") + "\n", "");
        lines = layout->pageBreak(scale, pageWidth, false, textt, 1)[0];
      }
      else {
        lines = textt.split(char(10), Qt::SkipEmptyParts);
      }
    }
    else {
      lines = textt.split(char(10), Qt::SkipEmptyParts);
    }



    if (applyTeXAlgo && pagenum <= 599 && pagenum > 1 && pagenum != 378) {
      auto result = layout->pageBreak(scale, pageWidth, false, textt, 1);
      if (result.size() == 1) {
        lines = result[0];
      }
      else {
        throw new std::runtime_error("ERROR");
      }
    }

    auto justification = LineJustification::Distribute;
    int beginsura = OtLayout::TopSpace << OtLayout::SCALEBY;

    if (pagenum == 0 || pagenum == 1) {
      double diameter = pageWidth * 0.9;
      auto ratio = 0.9;
      madinaLineWidths[(pagenum + 1) * 15 + 2] = ratio * 0.5;
      madinaLineWidths[(pagenum + 1) * 15 + 3] = ratio * 0.7;
      madinaLineWidths[(pagenum + 1) * 15 + 4] = ratio * 0.9;
      madinaLineWidths[(pagenum + 1) * 15 + 5] = ratio;
      madinaLineWidths[(pagenum + 1) * 15 + 6] = ratio * 0.9;
      madinaLineWidths[(pagenum + 1) * 15 + 7] = ratio * 0.7;
      madinaLineWidths[(pagenum + 1) * 15 + 8] = ratio * 0.4;

      beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3)) << OtLayout::SCALEBY;
    }

    QVector<LineToJustify> newLines;

    for (int lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
      auto newJustification = justification;
      auto line = QStringList{ lines[lineIndex] };
      int key = (pagenum + 1) * 15 + (lineIndex + 1);
      int lineWidth = pageWidth;
      auto match = surabism.match(lines[lineIndex]);

      LineType lineType = LineType::Line;

      if (match.hasMatch()) {
        if (match.captured(0).startsWith("سُ")) {
          lineType = LineType::Sura;
        }
        else {
          lineType = LineType::Bism;
        }

        if (!((pagenum == 0 || pagenum == 1) && lineIndex == 1)) {
          lineWidth = 0;
          newJustification = LineJustification::Center;
        }

      }

      if (madinaLineWidths.contains(key))
      {
        double ratio = madinaLineWidths.value(key);

        if (ratio < 1) {
          lineWidth = pageWidth * ratio;
          newJustification = LineJustification::Center;
        }
      }

      newLines.append({ lines[lineIndex] ,lineWidth ,newJustification,lineType });
    }

    auto shapedPage = layout->justifyPage(scale, pageWidth, newLines, newface, true, justStyleCombo->currentData().value<JustStyle>(), cluster_level, justCombo->currentData().value<JustType>());

    for (int lineIndex = 0; lineIndex < shapedPage.size(); lineIndex++) {
      auto& lineLayoutInfo = shapedPage[lineIndex];

      auto match = surabism.match(lines[lineIndex]);

      newface = false;

      if (lineLayoutInfo.type == LineType::Line) {
        // check if sajda
        match = sajdaRe.match(lines[lineIndex]);
        if (match.hasMatch()) {

          sajdamatched++;

          int startOffset = match.capturedStart(match.lastCapturedIndex()); // startOffset == 6
          int endOffset = match.capturedEnd(match.lastCapturedIndex()) - 1; // endOffset == 9


          while (lines[lineIndex][endOffset].isMark())
            endOffset--;

          bool beginDone = false;

          auto& glyphs = lineLayoutInfo.glyphs;

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

      if (lineIndex == 0 && (pagenum == 0 || pagenum == 1)) {
        lineLayoutInfo.ystartposition = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1)) << OtLayout::SCALEBY;
      }
      else {
        lineLayoutInfo.ystartposition = beginsura;
        beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;
      }
    }


    result.pages.append(shapedPage);
    result.originalPages.append(lines);
  }

  if (beginsajda != 15 || endsajda != 15 || sajdamatched != 15) {
    qDebug() << "sajdas problems?";
  }

  return result;

}
bool LayoutWindow::generateLayoutInfo() {

  OtLayout layout = OtLayout(m_font, true, true);
  layout.useNormAxisValues = false;

  layout.loadLookupFile("features.fea");

  int calScale = 1 << OtLayout::SCALEBY;

  int scale = calScale * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMedina(scale, lineWidth, &layout);

  GenerateLayout generateLayout{ &layout,result };

  generateLayout.generateLayout(lineWidth, scale);

  generateLayout.generateLayoutProtoBuf(lineWidth, scale);


  return true;
}


bool LayoutWindow::generateMadinaVARHTML() {

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);

  OtLayout layout = OtLayout(m_font, true, true);

  layout.automedina->cvxxfeatures.clear();

  layout.automedina->cvxxfeatures.append(QMap<quint16, QVector<ExtendedGlyph> >());

  auto& cv01feature = layout.automedina->cvxxfeatures[0];

  QMap<quint16, quint16> unicodeMappings;

  layout.loadLookupFile("features.fea");

  int calScale = 1 << OtLayout::SCALEBY;

  int scale = calScale * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMedina(scale, lineWidth, &layout, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

  auto htmlFileName = fileInfo.path() + "/output/" + fileInfo.completeBaseName() + "-quran.html";

  QFile file(htmlFileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");

  auto& originalText = result.originalPages;
  auto& pages = result.pages;

  hb_font_t* hbfont = layout.createFont(scale, false);

  bool position_relative = true;

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
  if (position_relative) {
    out << "position: relative;\n";
    out << "display: inline-block;\n";
  }
  else {
    out << "position: absolute;\n";
  }
  //out << "direction: ltr;\n";
  //out << "right: 0px;\n";
  //out << "text-align: 0px;\n";
  out << "}\n";
  //out << ".char::after { content:\"\\00a0\"}\n";
  out << ".page {\n";
  out << "width:1200px; margin:0 auto;\n";
  out << "}\n";
  out << ".line {\n";
  out << "position: relative;\n";
  out << "height: 200px;\n";
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
        out << " class = 'line linesura";
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


        // deal with reordering of hamza
        /*if (value.unicodes.size() == 2 && value.glyphs.size() == 2 && value.unicodes[1].unicode() == value.glyphs[1]->codepoint) {
          unicodes[start + 1] = { {value.glyphs[1]},QString(value.unicodes[1]) };
          value = { {value.glyphs[0]},QString(value.unicodes[0]) };
        }*/

        if (value.unicodes.size() > 1) {
          if (value.unicodes[0] == 0x06DD) {
            // Aya
          }
          else if (value.unicodes.size() == 3 && value.glyphs.size() == 3
            //&& value.unicodes[0].unicode() == value.glyphs[0]->codepoint
            && value.unicodes[1].unicode() == value.glyphs[1]->codepoint
            && value.unicodes[2].unicode() == value.glyphs[2]->codepoint
            ) {
            unicodes[start + 1] = { {value.glyphs[1]},QString(value.unicodes[1]) };
            unicodes[start + 2] = { {value.glyphs[2]},QString(value.unicodes[2]) };
            value = { {value.glyphs[0]},QString(value.unicodes[0]) };
          }
          else {
            std::cout << "page=" << p << ",line=" << l << ",start=" << start << ",unicodes=" << value.unicodes.toStdString() << ",codes=";
            for (auto charv : value.unicodes) {
              std::cout << std::hex << charv.unicode() << ";";
            }
            std::cout << ",codepoints=";
            for (auto glyph : value.glyphs) {
              std::cout << std::hex << glyph->codepoint << ";";
            }

            std::cout << std::endl;
          }
        }
      }


      for (auto iter = unicodes.begin(); iter != unicodes.end(); ++iter) {
        auto& value = iter.value();
        if (value.unicodes.size() > 1) {
          if (value.glyphs.size() != 1) {
            //std::cout << "page=" << p << ",line=" << l << ",start=" << iter.key() << ",unicodes=" << value.unicodes.toStdString() << std::endl;
             //throw new std::runtime_error("Problem");
          }
          for (auto charv : value.unicodes) {
            if (charv.unicode() != 0x06DD && (charv.unicode() < 0x0660 || charv.unicode() > 0x0669)) {
              //std::cout << "page=" << p << ",line=" << l << ",start=" << iter.key() << ",unicodes=" << value.unicodes.toStdString() << ",unicode=" << charv.unicode() << std::endl;
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
            if (alternates[aIndex].code == value.glyphs[0]->codepoint) {
              alternateIndex = aIndex + 1;
              break;
            }
          }

          if (alternateIndex == -1) {
            //if (value.glyphs[0]->codepoint != unicode) {
            alternates.append({ value.glyphs[0]->codepoint,0.0,0.0 });
            alternateIndex = alternates.size();
            //}

          }

          auto glyph = value.glyphs[0];

          //currentxPos += glyph->x_advance / (1000.0 * scale);

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

          if (position_relative) {
            auto defaultAdvance = layout.gethHorizontalAdvance(hbfont, glyph->codepoint, { .lefttatweel = glyph->lefttatweel,.righttatweel = glyph->righttatweel }, nullptr);

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
          }
          else {

            auto defaultAdvance = layout.gethHorizontalAdvance(hbfont, glyph->codepoint, { .lefttatweel = glyph->lefttatweel,.righttatweel = glyph->righttatweel }, nullptr);

            if (defaultAdvance != 0) {
              position = QString("width:%1em;").arg(defaultAdvance / (scale * 1000.0));
            }

            cumulatedRight = currentxPos + (glyph->x_advance - defaultAdvance) / (1000.0 * scale);
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
          }

          currentxPos += glyph->x_advance / (1000.0 * scale);

        }
        else if (value.glyphs.size() >= 1) {
          auto glyph = value.glyphs[0];

          currentxPos += glyph->x_advance / (1000.0 * scale);

          QString position = "";

          if (position_relative) {
            auto defaultAdvance = layout.gethHorizontalAdvance(hbfont, glyph->codepoint, { .lefttatweel = glyph->lefttatweel,.righttatweel = glyph->righttatweel }, nullptr);

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
          }
          else {


            auto defaultAdvance = layout.gethHorizontalAdvance(hbfont, glyph->codepoint, { .lefttatweel = glyph->lefttatweel,.righttatweel = glyph->righttatweel }, nullptr);

            if (defaultAdvance != 0) {
              position = QString("width:%1em;").arg(defaultAdvance / (scale * 1000.0));
            }

            cumulatedRight = currentxPos + (glyph->x_advance - defaultAdvance) / (1000.0 * scale);
            auto right = cumulatedRight - glyph->x_offset / (1000.0 * scale);
            auto top = -glyph->y_offset / (1000.0 * scale);
            position = position + QString("top:%1em;right:%2em;").arg(top).arg(right);


            out << QString("<span class='char' style='%1'>").arg(position);
            out << value.unicodes;
            out << "</span>";
          }

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

  layout.loadLookupFile("automedina-html.fea");

  layout.toOpenType->isCff2 = true;

  auto ret = layout.toOpenType->GenerateFile(otfFileName, "automedina-html.fea");

  delete hbfont;

  return true;

}

void LayoutWindow::saveCollision() {
  double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;

  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = shapeMushaf(scale, lineWidth, m_otlayout, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

  adjustOverlapping(result.pages, lineWidth, result.originalPages, scale, true);

}

bool LayoutWindow::generateMushaf(bool isHTML) {

  double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;

  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_buffer_cluster_level_t  cluster_level = HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES;

  if (isHTML) {
    cluster_level = HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS;
  }

  auto result = shapeMushaf(scale, lineWidth, m_otlayout, cluster_level);

  if (this->applyCollisionDetection) {
    adjustOverlapping(result.pages, lineWidth, result.originalPages, scale, false);
  }

  if (this->applyForce) {
    applyDirectedForceLayout(result.pages, result.originalPages, lineWidth, 0, 1, scale);
  }

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);

  if (!isHTML) {
    auto res = 4800 << OtLayout::SCALEBY;
    QPageSize pageSize{ { 90.2 ,144.5  },QPageSize::Millimeter, "MedianQuranBook" };
    QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };
#if defined(ENABLE_PDF_GENERATION)

    QString outputFileName = fileInfo.path() + "/output/mushaf.pdf";
    QuranPdfWriter quranWriter(outputFileName, m_otlayout);
    quranWriter.setPageLayout(pageLayout);
    quranWriter.setResolution(4800 << OtLayout::SCALEBY);
    //quranWriter.setResolution(72);

    quranWriter.generateQuranPages(result.pages, lineWidth, result.originalPages, scale);
#endif
  }
  else {
    ExportToHTML extohtml{ m_otlayout };

    extohtml.generateQuranPages(result.pages, lineWidth, result.originalPages, scale);
  }


  QFile file2(fileInfo.path() + "/output/medinashaping.dat");
  file2.open(QIODevice::WriteOnly);
  QDataStream out2(&file2);   // we will serialize the data into the file
  out2 << OtLayout::EMSCALE;
  //out2 << result.pages;
  for (auto& page : result.pages) {
    for (auto& line : page) {
      for (auto& glyph : line.glyphs) {
        out2 << glyph.codepoint;
        out2 << glyph.x_advance;
        out2 << glyph.x_offset;
        out2 << glyph.y_offset;
        out2 << glyph.lefttatweel;
        out2 << glyph.righttatweel;
      }
    }
  }
  out2 << result.originalPages;
  //out << result.suraNamebyPage;
  //out << locations;
  file2.close();

  return true;

}

bool LayoutWindow::generateAllQuranTexBreaking() {

  loadLookupFile("features.fea");

  int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  QString quran;

  for (int i = 581; i < 600; i++) {
    //const char * text = qurantext[i];
    const char* tt;


    tt = qurantext[i] + 1;


    //unsigned int text_len = strlen(tt);

    quran.append(quran.fromUtf8(tt));

    //hb_buffer_add_utf8(buffer, tt, text_len, 0, text_len);

  }

  LayoutPages pages;

  pages.originalPages = m_otlayout->pageBreak(scale, lineWidth, true, quran, 19);

  if (pages.originalPages.count() == 0) {
    QMessageBox msgBox;
    msgBox.setText("No feasable solution. Try to change the scale.");
    msgBox.exec();
    return false;
  }



  if (this->applyJustification) {
    for (int pagenum = 0; pagenum < pages.originalPages.length(); pagenum++) {


      auto justification = LineJustification::Distribute;


      auto page = m_otlayout->justifyPage(scale, lineWidth, lineWidth, pages.originalPages[pagenum], justification, false, true);

      pages.pages.append(page);

      /*
      for (int lineIndex = 0; lineIndex < page.length(); lineIndex++) {
        if (result.pages[pagenum][lineIndex].type == LineType::Line) {
          result.pages[pagenum][lineIndex] = page[lineIndex];
        }
        else {
          auto temp = m_otlayout->justifyPage(scale, 0, lineWidth, QStringList{ result[pagenum][lineIndex] }, LineJustification::Center, false, true);
          //temp[0].type = result.pages[pagenum][lineIndex].type;
          //temp[0].ystartposition = result.pages[pagenum][lineIndex].ystartposition;
          result.pages[pagenum][lineIndex].glyphs = temp[0].glyphs;
        }
      }*/

    }
  }

  if (this->applyCollisionDetection) {
    adjustOverlapping(pages.pages, lineWidth, pages.originalPages, scale, false);
  }

  QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };

#if defined(ENABLE_PDF_GENERATION)
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/allquran.pdf";
  QuranPdfWriter quranWriter(outputFileName, m_otlayout);
  quranWriter.setPageLayout(pageLayout);
  quranWriter.setResolution(4800 << OtLayout::SCALEBY);

  quranWriter.generateQuranPages(pages.pages, lineWidth, pages.originalPages, scale);

#endif
  return true;

}


void LayoutWindow::loadLookupFile(QString fileName) {

  m_otlayout->loadLookupFile(fileName.toStdString());

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
      auto textt = currentQuranText[i - 1];
      textEdit->setPlainText(textt);
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

  action = new QAction(tr("Find overfulls"), this);
  action->setStatusTip(tr("Find overfulls"));
  connect(action, &QAction::triggered, [this]() {this->findOverflows(true); });

  otherMenu->addAction(action);

  action = new QAction(tr("Find underfulls"), this);
  action->setStatusTip(tr("Find underfulls"));
  connect(action, &QAction::triggered, [this]() {this->findOverflows(false); });

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

  action = new QAction(tr("Compare Indopak Fonts"), this);
  action->setStatusTip(tr("Compare Indopak Fonts"));
  connect(action, &QAction::triggered, this, &LayoutWindow::compareIndopakFonts);

  otherMenu->addAction(action);

  action = new QAction(tr("Generate Test File"), this);
  action->setStatusTip(tr("Generate Test File"));
  connect(action, &QAction::triggered, this, &LayoutWindow::generateTestFile);

  otherMenu->addAction(action);

  action = new QAction(tr("Check Off Marks"), this);
  action->setStatusTip(tr("Check Off Marks"));
  connect(action, &QAction::triggered, this, &LayoutWindow::checkOffMarks);

  otherMenu->addAction(action);


  m_otlayout = new OtLayout(m_font, true, true, this);
  m_otlayout->useNormAxisValues = false;
  m_otlayout->extended = true;
  m_otlayout->applyJustification = applyJustification;

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

  connect(lokkupTreeWidget, &QTreeWidget::itemDoubleClicked, [&, this](QTreeWidgetItem* item, int column) {
    auto lookupName = item->text(0);
    if (item->childCount() == 0 && this->m_otlayout->lookupsIndexByName.contains(item->text(0))) {

      editLookup(item->text(0));

    }

    });

  lookupTree->setWidget(lokkupTreeWidget);

  this->setCentralWidget(m_graphicsView);
}


void LayoutWindow::serializeTexPages() {

  int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;
  int topSpace = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  auto result = m_otlayout->pageBreak(scale, lineWidth, false, 600);

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

  QFile file("output/texpages.dat");
  file.open(QIODevice::WriteOnly);
  QDataStream out(&file);   // we will serialize the data into the file
  out << OtLayout::EMSCALE;
  out << result.originalPages;
  out << result.suraNamebyPage;
  out << locations;

}
void LayoutWindow::createDataBase() {

  QDir appDir(QCoreApplication::applicationDirPath());
  QString dbPath = appDir.absoluteFilePath("quran-data.sqlite");
  QString dbNewPath = appDir.absoluteFilePath("quran-data-new.sqlite");

  QFile newFile{ dbNewPath };

  if (newFile.exists()) {
    if (!newFile.remove()) {
      qDebug() << newFile.errorString();
    }
  }

  QFile::copy(dbPath, dbNewPath);

  newFile.setPermissions(QFile::ReadOther | QFile::WriteOther);


  auto newDb = QSqlDatabase::addDatabase("QSQLITE", "NewDataBase");
  newDb.setDatabaseName(dbNewPath);
  newDb.open();
  //newDb.exec("PRAGMA synchronous = OFF");
  //newDb.exec("PRAGMA journal_mode = MEMORY");

  QSqlQuery query{ newDb };

  query.exec(QString("ALTER TABLE words ADD COLUMN dk_indopak STRING"));
  auto error = query.lastError();
  if (error.isValid()) {
    qDebug() << error;
  }

  query.exec("SELECT word_number_all,indopak FROM words");

  QSqlQuery update{ newDb };
  update.prepare("UPDATE words set dk_indopak = :word where word_number_all = :id");

  QVariantList ids;
  QVariantList words;


  while (query.next()) {
    int word_number = query.value(0).toInt();
    QString indopak = query.value(1).toString();

    changeText("indopak", indopak);

    ids << word_number;
    words << indopak;
  }

  update.addBindValue(words);
  update.addBindValue(ids);

  newDb.transaction();

  if (!update.execBatch()) {
    qDebug() << update.lastError();
    newDb.rollback();
  }

  newDb.commit();

  newDb.close();
}
void LayoutWindow::compareFonts(QString layoutName, QString textCol) {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString fileName = fileInfo.path() + "/output/comparefonts/compare_" + layoutName + "_" + textCol + ".html";

  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out.setCodec("UTF-8");

  out << "<html>" << '\n';
  out << "<head>" << '\n';
  out << "<meta charset='utf-8'>" << '\n';
  out << "<title>Compare fonts</title>" << '\n';
  out << "<link rel='stylesheet' href='comparefonts.css'>" << '\n';
  out << "</head>" << '\n';
  out << "<body>" << '\n';
  out << "<table style='font-size:50px;'>" << '\n';

  auto queryString = QString("SELECT page,line,indopak,nastaleeq,surah_number,ayah_number from %2 as l LEFT JOIN words w ON l.type = \"ayah\" AND l.range_start <= w.word_number_all AND l.range_end >= w.word_number_all order by page,line,word_number_all")
    .arg(layoutName);



  hb_font_t* font = m_otlayout->createFont(1000);

  QSqlQuery query(queryString);

  hb_buffer_t* buffer = hb_buffer_create();

  std::set<int> chars;
  while (query.next()) {
    int page = query.value(0).toInt();
    int line = query.value(1).toInt();
    QString word = query.value(textCol).toString();


    int surah_number = query.value("surah_number").toInt();
    int ayah_number = query.value("ayah_number").toInt();

    if (word.isEmpty()) continue;

    hb_buffer_clear_contents(buffer);


    hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
    hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
    hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));
    hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

    hb_buffer_add_utf16(buffer, word.utf16(), word.length(), 0, word.length());

    hb_shape(font, buffer, NULL, 0);

    uint glyph_count;
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

    for (int i = glyph_count - 1; i >= 0; i--) {
      auto glyph = glyph_info[i];
      if (glyph.codepoint == 0) {
        auto unicode = word[glyph.cluster].unicode();
        if (!chars.contains(unicode)) {
          chars.insert(unicode);
          auto hexv = std::format("{:04x}", unicode);
          QString newWord = word;
          changeText(textCol, newWord);

          QString otherColumn = textCol == "indopak" ? "nastaleeq" : "indopak";
          QString otherWord = query.value(otherColumn).toString();
          /*QString newOtherWord = otherWord;
          changeText(otherColumn, newOtherWord);*/

          out << "<tr>\n";
          out << "<td>" << surah_number << "</td>\n";
          out << "<td>" << ayah_number << "</td>\n";
          out << "<td>" << page << "</td>\n";
          out << "<td>" << line << "</td>\n";
          out << "<td>" << QString(hexv.c_str()) << "</td>\n";
          if (textCol == "indopak") {
            out << "<td  class='indopakalquran'><a target='_blank' href='https://quranwbw.com/" << surah_number << "#" << ayah_number << "'>" << word << "</a></td>\n";
            out << "<td  class='hafsnastaleeq'>" << otherWord << "</td>\n";
          }
          else {
            out << "<td  class='indopakalquran'><a target='_blank' href='https://quranwbw.com/" << surah_number << "#" << ayah_number << "'>" << otherWord << "</a></td>\n";
            out << "<td  class='hafsnastaleeq'>" << word << "</td>\n";
          }

          out << "<td  class='indopak'>" << newWord << "</td>\n";
          out << "</tr>\n";

        }
      }
    }


  }

  out << "</html>" << '\n';
  out << "</body>" << '\n';
  out << "</table>" << '\n';

  hb_buffer_destroy(buffer);
  hb_font_destroy(font);
}

void LayoutWindow::compareWaqfs(QString layoutName, QString textCol) {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString fileName = fileInfo.path() + "/output/comparefonts/comparewaqfs_" + layoutName + "_" + textCol + ".html";
  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  QString ayaString;
  QTextStream ayaMarksStream(&ayaString);
  QString finaString;
  QTextStream finaMarkStream(&finaString);
  QString meemiqlabString;
  QTextStream meemiqlabStream(&meemiqlabString);
  QString smalllowmeemString;
  QTextStream smalllowmeemStream(&smalllowmeemString); \
    QString yehFarsiString;
  QTextStream yehFarsiStream(&yehFarsiString);


  out.setCodec("UTF-8");

  out << "<html>" << '\n';
  out << "<head>" << '\n';
  out << "<meta charset='utf-8'>" << '\n';
  out << "<title>Compare Marks</title>" << '\n';
  out << "<link rel='stylesheet' href='comparefonts.css'>" << '\n';
  out << "</head>" << '\n';
  out << "<body>" << '\n';


  QString waqfChars = "࣢ࣛࣝࣞۖۚۙؔۘۛؕࣕࣖࣗؗ";

  QString meemIqlab = "\u06E2";

  QString smalllowmeem = "\u06ED";

  auto queryString = QString("SELECT page,line,indopak,nastaleeq,surah_number,ayah_number from %2 as l LEFT JOIN words w ON l.type = \"ayah\" AND l.range_start <= w.word_number_all AND l.range_end >= w.word_number_all order by page,line,word_number_all")
    .arg(layoutName);


  QSqlQuery query(queryString);

  QRegularExpression waqfSeq(QString("([٠١٢٣٤٥٦٧٨٩]*)([%1]+)").arg(waqfChars + meemIqlab));

  std::set<QString> ayaSeqs;
  std::set<QString> finaSeqs;
  int lineNum = 0;
  while (query.next()) {
    int page = query.value(0).toInt();
    int line = query.value(1).toInt();
    QString word = query.value(textCol).toString();


    int surah_number = query.value("surah_number").toInt();
    int ayah_number = query.value("ayah_number").toInt();

    if (word.isEmpty()) continue;

    QString newWord = word;
    changeText(textCol, newWord);

    QString indopakWord, nastaleeqWord;

    if (textCol == "indopak") {
      indopakWord = word;
      nastaleeqWord = query.value("nastaleeq").toString();
    }
    else {
      nastaleeqWord = word;
      indopakWord = query.value("indopak").toString();
    }

    auto iter = waqfSeq.globalMatch(newWord);
    while (iter.hasNext()) {
      QRegularExpressionMatch match = iter.next();
      QString ayaNum = match.captured(1);
      auto isAya = !ayaNum.isEmpty();
      QString seq = match.captured(2);
      if (seq.size() < 2) continue;

      if ((isAya && !ayaSeqs.contains(seq)) || (!isAya && !finaSeqs.contains(seq))) {
        if (isAya) {
          ayaSeqs.insert(seq);
        }
        else {
          finaSeqs.insert(seq);
        }

        QTextStream& outStream = isAya ? ayaMarksStream : finaMarkStream;

        outStream << "<tr>\n";
        outStream << "<td>" << surah_number << "</td>\n";
        outStream << "<td>" << ayah_number << "</td>\n";
        outStream << "<td>" << page << "</td>\n";
        outStream << "<td>" << line << "</td>\n";
        outStream << "<td  class='indopak'>" << newWord << "</td>\n";
        outStream << "<td  class='indopakalquran'><a target='_blank' href='https://quranwbw.com/" << surah_number << "#" << ayah_number << "'>" << indopakWord << "</a></td>\n";
        outStream << "<td  class='hafsnastaleeq'>" << nastaleeqWord << "</td>\n";
        outStream << "</tr>\n";
      }
    }

    if (newWord.contains(meemIqlab)) {
      meemiqlabStream << "<tr>\n";
      meemiqlabStream << "<td>" << surah_number << "</td>\n";
      meemiqlabStream << "<td>" << ayah_number << "</td>\n";
      meemiqlabStream << "<td>" << page << "</td>\n";
      meemiqlabStream << "<td>" << line << "</td>\n";
      meemiqlabStream << "<td  class='indopak'>" << newWord << "</td>\n";
      meemiqlabStream << "<td  class='indopakalquran'><a target='_blank' href='https://quranwbw.com/" << surah_number << "#" << ayah_number << "'>" << indopakWord << "</a></td>\n";
      meemiqlabStream << "<td  class='hafsnastaleeq'>" << nastaleeqWord << "</td>\n";
      meemiqlabStream << "</tr>\n";
    }
    if (newWord.contains(smalllowmeem)) {
      smalllowmeemStream << "<tr>\n";
      smalllowmeemStream << "<td>" << surah_number << "</td>\n";
      smalllowmeemStream << "<td>" << ayah_number << "</td>\n";
      smalllowmeemStream << "<td>" << page << "</td>\n";
      smalllowmeemStream << "<td>" << line << "</td>\n";
      smalllowmeemStream << "<td  class='indopak'>" << newWord << "</td>\n";
      smalllowmeemStream << "<td  class='indopakalquran'><a target='_blank' href='https://quranwbw.com/" << surah_number << "#" << ayah_number << "'>" << indopakWord << "</a></td>\n";
      smalllowmeemStream << "<td  class='hafsnastaleeq'>" << nastaleeqWord << "</td>\n";
      smalllowmeemStream << "</tr>\n";
    }
    /*
    if (word.contains("\u06CC")) {
      yehFarsiStream << "<tr>\n";
      yehFarsiStream << "<td>" << surah_number << "</td>\n";
      yehFarsiStream << "<td>" << ayah_number << "</td>\n";
      yehFarsiStream << "<td>" << page << "</td>\n";
      yehFarsiStream << "<td>" << line << "</td>\n";
      yehFarsiStream << "<td  class='indopak'>" << newWord << "</td>\n";
      yehFarsiStream << "<td  class='indopakalquran'><a target='_blank' href='https://quranwbw.com/" << surah_number << "#" << ayah_number << "'>" << indopakWord << "</a></td>\n";
      yehFarsiStream << "<td  class='hafsnastaleeq'>" << nastaleeqWord << "</td>\n";
      yehFarsiStream << "</tr>\n";
    }*/
  }
  out << "<div style='font-size:30px;'>Final Waqf Marks</div>" << '\n';
  out << "<table style='font-size:50px;'>" << '\n';
  out << finaString;
  out << "</table>" << '\n';

  out << "<div style='font-size:30px;'>Aya Waqf Marks</div>" << '\n';
  out << "<table style='font-size:50px;'>" << '\n';
  out << ayaString;
  out << "</table>" << '\n';

  out << "<div style='font-size:30px;'>High Meem Iqlab</div>" << '\n';
  out << "<table style='font-size:50px;'>" << '\n';
  out << meemiqlabString;
  out << "</table>" << '\n';

  out << "<div style='font-size:30px;'>Low Meem Iqlab</div>" << '\n';
  out << "<table style='font-size:50px;'>" << '\n';
  out << smalllowmeemString;
  out << "</table>" << '\n';

  /*
  out << "<div style='font-size:30px;'>Yeh Farsi</div>" << '\n';
  out << "<table style='font-size:50px;'>" << '\n';
  out << yehFarsiString;
  out << "</table>" << '\n';*/

  out << "</html>" << '\n';
  out << "</body>" << '\n';

}


void LayoutWindow::compareIndopakFonts() {

  auto layoutName = mushafLayouts->currentText();

  compareFonts(layoutName, "indopak");
  compareWaqfs(layoutName, "indopak");
  createDataBase();
  //compareFonts(layoutName, "nastaleeq");


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

  QFile file("output/medinapages.dat");
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

void LayoutWindow::findOverflows(bool overfull) {
  loadLookupFile("features.fea");


  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  //bool conti = true;

  struct Line {
    int pageNumber;
    int lineNumber;
    double overflow;
    double percentage;
  };

  struct PageWidths {
    int pageNumber;
    float minWidth;
    float maxWidth;
    float diff;
    int minLine;
    int maxLine;
  };

  QMap<double, QMap<double, Line>> alloverflows;

  double scale = OtLayout::EMSCALE;

  double emScale = (1 << OtLayout::SCALEBY) * scale;

  //const int minSpace = OtLayout::MINSPACEWIDTH * emScale;
  //const int  defaultSpace = OtLayout::SPACEWIDTH * emScale;

  QMap<double, Line> measures;

  QString surapattern = QString("^(")
    + "سُورَةُ" + " .*"
    + "|" + "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
    + "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
    + "|" + "بِسْمِ اللّٰهِ الرَّحْمٰنِ الرَّحِيْمِ ۝"
    + ")$";

  QRegularExpression surabism(surapattern, QRegularExpression::MultilineOption);

  std::vector<PageWidths> widths;

  for (int pagenum = 0; pagenum < currentQuranText.size(); pagenum++) {

    QString textt = currentQuranText[pagenum];

    auto lines = textt.split(char(10), Qt::SkipEmptyParts);

    auto page = m_otlayout->justifyPage(emScale, lineWidth, lineWidth, lines, LineJustification::Distribute, false, true);


    PageWidths minmax{ 0,std::numeric_limits<float>::max() ,std::numeric_limits<float>::min() ,0,0,0 };


    for (int linenum = 0; linenum < lines.length(); linenum++) {

      auto& line = page[linenum];

      auto match = surabism.match(lines[linenum]);

      if (pagenum + 1 == 573) {
        std::cout << "Amine" << std::endl;
      }

      if (!match.hasMatch()) {
        auto textWidth = lineWidth - line.overfull;
        if (textWidth < minmax.minWidth) {
          minmax.minWidth = textWidth;
          minmax.minLine = linenum + 1;
        }
        if (textWidth > minmax.maxWidth) {
          minmax.maxWidth = textWidth;
          minmax.maxLine = linenum + 1;
        }
      }

      if (overfull) {
        if (line.overfull > 0) {
          auto overflow = line.overfull / emScale;
          //if (overflow > 0.01) {
          measures.insert(-line.overfull, { pagenum + 1,linenum + 1, overflow,(line.overfull / lineWidth) * 100 });
          // }

        }
      }
      else {

        LineType lineType = LineType::Line;

        if (match.hasMatch()) {
          if (match.captured(0).startsWith("سُ")) {
            lineType = LineType::Sura;
          }
          else {
            lineType = LineType::Bism;
          }
        }
        if (lineType == LineType::Line && line.overfull < 0) {
          auto underfull = -line.overfull / emScale;
          measures.insert(line.overfull, { pagenum + 1,linenum + 1, underfull,(-line.overfull / lineWidth) * 100 });
        }
      }


    }
    widths.push_back({ pagenum + 1, minmax.minWidth ,minmax.maxWidth ,minmax.maxWidth - minmax.minWidth,minmax.minLine,minmax.maxLine });
  }

  if (measures.count() > 0) {
    alloverflows[scale] = measures;
  }
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);


  QString name = overfull ? "output/overfulls" : "output/underfulls";

  if (applyJustification) {
    name = name + QString("_with_just");
  }

  QString fileName = fileInfo.path() + "/" + name + QString("_%1.csv").arg(scale);
  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out.setCodec("ISO 8859-1");

  for (auto key : alloverflows.keys()) {
    auto overflow = alloverflows.value(key);
    for (auto& line : overflow) {
      out << key << "," << line.pageNumber << "," << line.lineNumber << "," << (line.overflow) << "," << (line.percentage) << "%" << "\n";
    }
  }
  file.close();

  std::sort(widths.begin(), widths.end(), [](const PageWidths& a, const PageWidths& b) { return a.diff > b.diff; });
  QString diffFileName = fileInfo.path() + "/output/minmaxwidths.csv";
  QFile fileDiff(diffFileName);
  fileDiff.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream outDiff(&fileDiff);
  outDiff.setCodec("ISO 8859-1");
  for (auto width : widths) {
    outDiff << width.pageNumber << "," << width.diff << "," << width.minWidth << "," << width.maxWidth << "," << width.minLine << "," << width.maxLine << "\n";
  }
  fileDiff.close();

}

void LayoutWindow::calculateMinimumSize() {
  loadLookupFile("features.fea");


  int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_buffer_t* buffer = buffer = hb_buffer_create();

  //bool conti = true;

  struct Line {
    int pageNumber;
    int lineNumber;
    double overflow;
  };

  QMap<double, QVector<Line>> alloverflows;

  double scale = 0.75;

  while (scale <= 0.94) {

    double emScale = (1 << OtLayout::SCALEBY) * scale;

    QVector<Line> overflows;


    for (int pagenum = 0; pagenum < currentQuranText.size(); pagenum++) {

      QString textt = currentQuranText[pagenum];

      auto lines = textt.split(char(10), Qt::SkipEmptyParts);

      auto page = m_otlayout->justifyPage(emScale, lineWidth, lineWidth, lines, LineJustification::Distribute, false, true);


      for (int linenum = 0; linenum < lines.length(); linenum++) {

        auto line = page[linenum];

        if (line.overfull > 0) {
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

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString fileName = fileInfo.path() + "/output/overflows.csv";

  if (applyJustification) {
    fileName = fileInfo.path() + "/output/overflows_with_just.csv";
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

void LayoutWindow::setQuranText(int type) {

  currentQuranText.clear();
  suraNameByPage.clear();

  if (type == 1) {
    for (int i = 0; i < 604; i++) {
      currentQuranText.append(qurantext[i] + 1);
      suraNameByPage.append("");
    }
  }
  else {
    loadLookupFile("features.fea");

    int scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
    int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;
    auto result = m_otlayout->pageBreak(scale, lineWidth, false, 604);



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

  double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;

  if (m_graphicsScene->items().isEmpty()) {
    refresh = 2;
  }

  if (refresh == 2) {
    newFace = true;
    loadLookupFile("features.fea");
    if (!m_otlayout->extended) {
      m_otlayout->generateSubstEquivGlyphs();
      loadLookupFile("features.fea");
    }
  }

  QString textt = textEdit->toPlainText();

  const int lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  QStringList lines;
  if (applyTeXAlgo) {
    auto result = m_otlayout->pageBreak(scale, lineWidth, false, textt, 1);
    if (result.size() == 1) {
      lines = result[0];
    }
  }
  else {
    lines = textt.split(char(10), Qt::SkipEmptyParts);
  }

  auto page = m_otlayout->justifyPage(scale, lineWidth, lineWidth, lines, LineJustification::Distribute, newFace, true, justStyleCombo->currentData().value<JustStyle>(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS, justCombo->currentData().value<JustType>());

  QVector<int> set;

  QList<QList<LineLayoutInfo>> pages = { page };

  QList<QStringList> originalPages = { lines };

  if (this->applyForce) {
    applyDirectedForceLayout(pages, originalPages, lineWidth, 0, 1, scale);
  }

  if (this->applyCollisionDetection) {
    QVector<OverlapResult> result;
    adjustOverlapping(pages, lineWidth, 0, 1, set, scale, result, false);
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
    double currentxPos = line.xstartposition;
    double currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
    double xScale = line.fontSize * line.xscale;
    double yScale = line.fontSize * -1;    
    for (auto& glyphLayout : line.glyphs) {
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

      if (m_otlayout->glyphs.contains(glyphName)) {
        GlyphVis& glyph = m_otlayout->glyphs[glyphName];


        GlyphItem* glyphItem = nullptr;
        if (refresh) {
          glyphItem = new GlyphItem(xScale, yScale, &glyph, m_otlayout, { .lefttatweel = glyphLayout.lefttatweel, .righttatweel = glyphLayout.righttatweel, .scalex = 0 }, glyphLayout.lookup_index, glyphLayout.subtable_index, glyphLayout.base_codepoint);
          glyphItem->setFlag(QGraphicsItem::ItemIsMovable);
          glyphItem->setFlag(QGraphicsItem::ItemIsSelectable);
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


          currentxPos -= glyphLayout.x_advance * line.xscale;
          QPoint pos(currentxPos + (glyphLayout.x_offset * line.xscale), currentyPos - (glyphLayout.y_offset));
          glyphItem->setPos(pos);
        }
      }
    }

  }

  //m_otlayout->clearAlternates();

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

void LayoutWindow::adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, QList<QStringList> originalPages, double emScale, bool onlySameLine) {

  //QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageSize pageSize{ { 90.2,147.5 },QPageSize::Millimeter, "MedianQuranBook" };
  QPageLayout pageLayout{ pageSize , QPageLayout::Portrait,QMarginsF(0, 0, 0, 0) };

  int totalpageNb = pages.size();

  int nbthreads = 12;
  int pageperthread = totalpageNb / nbthreads;
  int remainingPages = totalpageNb;

  std::vector<QThread*> threads;
  std::vector<QVector<int>*> overlappages;

  std::vector<QVector<OverlapResult>*> overlapResults;

  // fetch all gryph initially otherwise mpost is not thread safe when executing getAlternate
  for (auto& page : pages) {
    for (auto& line : page) {
      for (auto& glyph : line.glyphs) {
        //GlyphVis* currentGlyph = 
        m_otlayout->getGlyph(glyph.codepoint, { .lefttatweel = glyph.lefttatweel, .righttatweel = glyph.righttatweel });
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

    QVector<OverlapResult>* result1 = new QVector<OverlapResult>();

    overlapResults.push_back(result1);


    QThread* thread = QThread::create([this, &pages, &result1, begin, pageperthread, set, lineWidth, emScale, onlySameLine] { adjustOverlapping(pages, lineWidth, begin, pageperthread, *set, emScale, *result1, onlySameLine); });

    threads.push_back(thread);

    thread->start();
  }


  for (auto t : threads) {
    t->wait();
    delete t;
  }

  QVector<OverlapResult> overlapResult;

  QMap<QVector<int>, OverlapResult> sequences;

  for (auto overlaps : overlapResults) {

    for (auto& overlap : *overlaps) {
      auto& page = pages[overlap.pageIndex];
      auto& line = page[overlap.lineIndex];
      QVector<int> sequence;
      for (int i = overlap.prevGlyph; i <= overlap.nextGlyph; i++) {
        auto& glyphLayout = line.glyphs[i];
        sequence.append(glyphLayout.codepoint);
      }


      overlapResult.append(overlap);
      /*if (!sequences.contains(sequence)) {
        sequences.insert(sequence, overlap);
        overlapResult.append(overlap);
      }*/


    }
    delete overlaps;
  }

  generateOverlapLookups(pages, originalPages, overlapResult);

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
      lineInfo.fontSize = emScale;
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
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/overlapping.pdf";
  QuranPdfWriter allquran_overlapping(outputFileName, m_otlayout);
  allquran_overlapping.setPageLayout(pageLayout);
  allquran_overlapping.setResolution(4800 << OtLayout::SCALEBY);

  allquran_overlapping.generateQuranPages(newpages, lineWidth, neworiginalPages, emScale);
#endif
}
// taken from Qt qt/src/gui/graphicsview/qgraphicsitem.cpp
static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath& path, const QPen& pen)
{
  // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
  // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
  const qreal penWidthZero = qreal(0.00000001);
  if (path == QPainterPath() || pen == Qt::NoPen)
    return path;
  QPainterPathStroker ps;
  ps.setCapStyle(pen.capStyle());
  if (pen.widthF() <= 0.0)
    ps.setWidth(penWidthZero);
  else
    ps.setWidth(pen.widthF());
  ps.setJoinStyle(pen.joinStyle());
  ps.setMiterLimit(pen.miterLimit());
  QPainterPath p = ps.createStroke(path);
  p.addPath(path);
  return p;
}


void LayoutWindow::adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, int beginPage, int nbPages, QVector<int>& set, double emScale, QVector<OverlapResult>& result, bool onlySameLine) {

  double minDistance = 10;
  
  QPen pen = QPen();
  pen.setWidth(std::ceil(minDistance * emScale));


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

        linePositions.append(QPoint{ pos.x(),pos.y() });

      }

      pagePositions.append(linePositions);

    }

    for (int l = 0; l < page.size(); l++) {

      auto& line = page[l];

      double scale = line.fontSize; //(1 << OtLayout::SCALEBY) * 1; // 0.85;		  

      QTransform pathtransform;
      pathtransform = pathtransform.scale(scale * 1.00, -scale * 1.00);

      QList<QPoint>& linePositions = pagePositions[l];
      LineLayoutInfo suraName;

      QVector<QPainterPath> paths;

      for (int g = 0; g < line.glyphs.size(); g++) {

        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        GlyphVis& currentGlyph = *m_otlayout->getGlyph(glyphName, { .lefttatweel = glyphLayout.lefttatweel, .righttatweel = glyphLayout.righttatweel, .scalex = line.xscaleparameter });
        QPoint pos = linePositions[g];
        QPainterPath path;
        if (!glyphName.contains("space") && !glyphName.contains("cgj")) {
          auto gg = qt_graphicsItem_shapeFromPath(currentGlyph.path, pen);
          path = pathtransform.map(gg);
          path.translate(pos);

          paths.append(path);
        }
        else {
          paths.append(path);
        }



        if (glyphName.contains("space") || glyphName.contains("linefeed") || !m_otlayout->glyphs.contains(glyphName)) continue;

        //bool isIsol = glyphName.contains("isol");

        //bool isFina = glyphName.contains(".fina");

        bool isMark = m_otlayout->automedina->classes["marks"].contains(glyphName);

        //bool isWaqfMark = m_otlayout->automedina->classes["waqfmarks"].contains(glyphName);



        // verify with the line above
        //if (l > 0 && false) {
        if (l > 0 && !onlySameLine) {
          int prev_index = l - 1;
          QList<QPoint>& prev_linePositions = pagePositions[prev_index];
          auto& prev_line = page[prev_index];

          for (int prev_g = 0; prev_g < prev_line.glyphs.size(); prev_g++) {
            auto& prev_glyphLayout = prev_line.glyphs[prev_g];
            QString prev_glyphName = m_otlayout->glyphNamePerCode[prev_glyphLayout.codepoint];

            bool isPrevMark = m_otlayout->automedina->classes["marks"].contains(prev_glyphName);
            bool isPrevrSpace = prev_glyphName.contains("space") || prev_glyphName.contains("linefeed");
            //bool isPrevIsol = prev_glyphName.contains("isol");


            if ((isMark || isPrevMark) && !isPrevrSpace) { //|| isIsol || isPrevIsol

              GlyphVis& otherGlyph = *m_otlayout->getGlyph(prev_glyphName, {
                .lefttatweel = prev_glyphLayout.lefttatweel,
                .righttatweel = prev_glyphLayout.righttatweel,
                .scalex = line.xscaleparameter }); //m_otlayout->glyphs[prev_glyphName];

              QPoint otherpos = prev_linePositions[prev_g];

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

        bool isSameWord = true;

        for (int gg = g - 1; gg >= 0; gg--) {

          auto& otherglyphLayout = line.glyphs[gg];
          QString otherglyphName = m_otlayout->glyphNamePerCode[otherglyphLayout.codepoint];

          bool isOtherSpace = otherglyphName.contains("space") || otherglyphName.contains("linefeed");

          if (isOtherSpace) {
            isSameWord = false;
            continue;
          }

          if (isSameWord) {
            //TODO include lam.init kaf.medi for example

            bool isPrevInit = otherglyphName.contains(".init");
            bool isPrevMedi = otherglyphName.contains(".medi");

            if (glyphName.contains(".fina") && (isPrevMedi || isPrevInit)) continue;
            if (glyphName.contains(".medi") && (isPrevMedi || isPrevInit)) continue;
          }



          GlyphVis& otherGlyph = *m_otlayout->getGlyph(otherglyphName, { .lefttatweel = otherglyphLayout.lefttatweel, .righttatweel = otherglyphLayout.righttatweel }); //glyphs[otherglyphName];
          QPointF otherpos = linePositions[gg];

          QPainterPath& otherpath = paths[gg];

          if (path.intersects(otherpath)) {

            glyphLayout.color = 0xFF000000;

            otherglyphLayout.color = 0xFF000000;
            intersection = true;

            OverlapResult overlap;

            overlap.pageIndex = p;
            overlap.lineIndex = l;
            overlap.nextGlyph = g;
            overlap.prevGlyph = gg;

            result.append(overlap);

          }

        }


      }
    }

    if (intersection) {
      set.append(p);
    }


  }
}
void LayoutWindow::generateOverlapLookups(const QList<QList<LineLayoutInfo>>& pages, const QList<QStringList>& originalPages, const QVector<OverlapResult>& result) {

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/overlaps.txt";

  QFile file(outputFileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");



  std::set<QString> words;
  std::set<QString> otherWords;

  auto& basesClass = m_otlayout->automedina->classes["bases"];
  auto& marksClass = m_otlayout->automedina->classes["marks"];

  QString innerLookupsString;

  QTextStream innerlookups{ &innerLookupsString };

  QSet<QVector<int>> sequences;

  struct GlyphPos {
    QSet<quint16> set;
    QString lookupName;
  };

  QVector<QVector<GlyphPos>> posSubtables;
  std::map<QString, std::unordered_set<int>> subLookupKerns;



  int lastsubLookupNumber = 0;

  if (m_otlayout->lookupsIndexByName.contains("adjustoverlap")) {
    auto adjustoverlapLookup = m_otlayout->lookups[m_otlayout->lookupsIndexByName["adjustoverlap"]];
    for (auto subtable : adjustoverlapLookup->subtables) {
      //SingleAdjustmentSubtable* kernTable = dynamic_cast<SingleAdjustmentSubtable*>(subtable);
      ChainingSubtable* chainingSubtable = dynamic_cast<ChainingSubtable*>(subtable);

      QVector<GlyphPos> positions;

      for (int pos = 0; pos < chainingSubtable->compiledRule.input.size(); pos++) {
        GlyphPos glyphPos{ chainingSubtable->compiledRule.input[pos] ,{} };
        for (auto& lookupRecord : chainingSubtable->compiledRule.lookupRecords) {
          if (lookupRecord.position == pos) {
            glyphPos.lookupName = lookupRecord.lookupName;
            break;
          }
        }
        positions.append(glyphPos);
      }

      posSubtables.append(positions);
      for (auto& lookupRecord : chainingSubtable->compiledRule.lookupRecords) {
        if (!lookupRecord.lookupName.isEmpty()) {
          SingleAdjustmentSubtable* kernTable = dynamic_cast<SingleAdjustmentSubtable*>(m_otlayout->lookups[m_otlayout->lookupsIndexByName[lookupRecord.lookupName]]->subtables[0]);
          int number = std::stoi(lookupRecord.lookupName.mid(15).toStdString());
          if (subLookupKerns.find(lookupRecord.lookupName) == subLookupKerns.end()) {

            auto pair = subLookupKerns.insert({ lookupRecord.lookupName,{} });
            auto& res = *pair.first;
            for (auto codepoint : kernTable->singlePos.keys()) {
              res.second.insert(codepoint);
            }
            if (number > lastsubLookupNumber) {
              lastsubLookupNumber = number;
            }
          }
        }

      }

    }
  }

  int subLookupNumber = lastsubLookupNumber + 1;



  QString subLookups;


  for (auto overlap : result) {

    auto& page = pages[overlap.pageIndex];
    auto& line = page[overlap.lineIndex];
    auto& text = originalPages[overlap.pageIndex][overlap.lineIndex];



    auto& prevGlyphLayout = line.glyphs[overlap.prevGlyph];
    QString prevGlyphName = m_otlayout->glyphNamePerCode[prevGlyphLayout.codepoint];

    auto& nextGlyphLayout = line.glyphs[overlap.nextGlyph];
    QString nextGlyphName = m_otlayout->glyphNamePerCode[nextGlyphLayout.codepoint];

    bool betweenBases;

    if (basesClass.contains(prevGlyphName) && basesClass.contains(nextGlyphName)) {
      betweenBases = true;
    }
    else {
      betweenBases = false;
    }

    QVector<int> basesIndexes;

    int prevBaseIndex;
    for (prevBaseIndex = overlap.prevGlyph; prevBaseIndex >= 0; prevBaseIndex--) {
      auto& glyphLayout = line.glyphs[prevBaseIndex];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (basesClass.contains(glyphName)) {
        break;
      }
    }

    basesIndexes.append(prevBaseIndex);

    for (int nextBaseIndex = prevBaseIndex + 1; nextBaseIndex <= overlap.nextGlyph; nextBaseIndex++) {
      auto& glyphLayout = line.glyphs[nextBaseIndex];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (basesClass.contains(glyphName)) {
        basesIndexes.append(nextBaseIndex);
      }
    }



    int lastIndex = basesIndexes.last() > overlap.nextGlyph ? basesIndexes.last() : overlap.nextGlyph;

    QVector<int> sequence;

    bool containsSpace = false;

    for (int i = basesIndexes.first(); i <= lastIndex; i++) {
      auto& glyphLayout = line.glyphs[i];
      sequence.append(glyphLayout.codepoint);
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (glyphName.contains("space")) {
        containsSpace = true;
      }
    }

    if (sequences.contains(sequence)) continue;

    sequences.insert(sequence);

    // generate word

    int startCluster = 0;
    int endCluster = text.size();

    for (int i = overlap.prevGlyph; i >= 0; i--) {
      auto& glyphLayout = line.glyphs[i];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (glyphName.contains("space")) {
        startCluster = glyphLayout.cluster + 1;
        break;
      }
    }
    for (int i = overlap.nextGlyph; i < line.glyphs.size(); i++) {
      auto& glyphLayout = line.glyphs[i];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (glyphName.contains("space")) {
        endCluster = glyphLayout.cluster;
        break;
      }
    }

    QString word = text.mid(startCluster, endCluster - startCluster);

    if (containsSpace || betweenBases) {

      if (otherWords.find(word) == otherWords.end()) {
        otherWords.insert(word);
      }
      std::cout << "pos";
      for (int glyphIndex = basesIndexes.first(); glyphIndex <= lastIndex; glyphIndex++) {
        auto& glyphLayout = line.glyphs[glyphIndex];
        sequence.append(glyphLayout.codepoint);
        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        std::cout << " " << glyphName.toStdString() << "'";
      }
      std::cout << "; # page " << overlap.pageIndex + 1 << " line " << overlap.lineIndex + 1 << " " << word.toStdString() << std::endl;
      continue;
    }

    if (words.find(word) == words.end()) {
      words.insert(word);
    }

    int seqLength = lastIndex - basesIndexes.first() + 1;

    bool alreadyExist = false;
    QVector<GlyphPos>* posToAdd = nullptr;
    for (auto& posSubtable : posSubtables) {
      if (posSubtable.size() == seqLength) {
        bool found = true;
        alreadyExist = true;
        for (int i = 0; i < seqLength; i++) {
          auto& glyphLayout = line.glyphs[basesIndexes.first() + i];
          if (posSubtable[i].set.contains(glyphLayout.codepoint)) {
            found = false;
          }
          else {
            alreadyExist = false;
          }
        }
        if (alreadyExist) break;
        else if (found && posToAdd == nullptr) {
          posToAdd = &posSubtable;
        }
      }
    }

    if (alreadyExist) continue;


    QVector<GlyphPos> posSubtable;
    for (int i = 0; i < seqLength; i++) {
      auto& glyphLayout = line.glyphs[basesIndexes.first() + i];
      GlyphPos glyphPos;

      glyphPos.set.insert(glyphLayout.codepoint);
      /*
      glyphPos.lookupName = QString("adjustoverlap.l%1").arg(subLookupNumber++);
      subLookupKerns.insert({ glyphPos.lookupName  ,{glyphLayout.codepoint} });
      posSubtable.append(glyphPos);*/

      auto find = false;
      for (auto& sublookup : subLookupKerns) {
        auto res = sublookup.second.find(glyphLayout.codepoint);
        if (res == sublookup.second.end()) {
          glyphPos.lookupName = sublookup.first;
          find = true;
          sublookup.second.insert(glyphLayout.codepoint);
          break;
        }
      }
      if (!find) {
        glyphPos.lookupName = QString("adjustoverlap.l%1").arg(subLookupNumber++);
        subLookupKerns.insert({ glyphPos.lookupName ,{glyphLayout.codepoint} });
      }
      posSubtable.append(glyphPos);
    }

    posSubtables.append(posSubtable);


  }

  // Output

  out << "**************************************inner mark collisions**************\n";
  int nbWordbyline = 0;
  for (auto& word : words) {
    if (nbWordbyline == 8) {
      out << '\n';
      nbWordbyline = 0;
    }
    else if (nbWordbyline != 0) {
      out << ' ';
    }
    out << word;
    nbWordbyline++;

  }

  out << "\n**************************************otherWords**************\n";

  nbWordbyline = 0;
  for (auto& word : otherWords) {
    if (nbWordbyline == 6) {
      out << '\n';
      nbWordbyline = 0;
    }
    else if (nbWordbyline != 0) {
      out << ' ';
    }
    out << word;
    nbWordbyline++;

  }

  out << "\n****************************************************\n";

  for (auto& subLookupKern : subLookupKerns) {
    auto lookupName = subLookupKern.first; // QString("adjustoverlap.l%1").arg(subLookupKern.first);
    QString sublookup = "  lookup " + lookupName + " {\n";
    for (auto& codepoint : subLookupKern.second) {
      QString glyphName = m_otlayout->glyphNamePerCode[codepoint];
      sublookup += "    pos /^" + glyphName + "([.]added_.*)$/ <0 0 0 0>;\n";
    }
    sublookup += "  } " + lookupName + ";\n";
    subLookups += sublookup;
  }


  std::sort(posSubtables.begin(), posSubtables.end(), [](const QVector<GlyphPos>& a, const QVector<GlyphPos>& b) {
    return a.size() > b.size();
    });

  QString mainLookup;
  for (auto& posSubtable : posSubtables) {
    QString posLine = "  pos";
    QString debugLine = "pos";
    for (auto glyphPos : posSubtable) {
      if (glyphPos.set.size() > 1) {
        posLine += " [";
        debugLine += " [";
      }

      for (auto glyph : glyphPos.set) {
        QString glyphName = m_otlayout->glyphNamePerCode[glyph];
        posLine += " /^" + glyphName + "([.]added_.*)?$/";
        debugLine += " /^" + glyphName + "/";
      }
      if (glyphPos.set.size() > 1) {
        posLine += "]";
        debugLine += "]";
      }
      posLine += "'";
      debugLine += "'";
      if (!glyphPos.lookupName.isEmpty()) {
        posLine += "lookup " + glyphPos.lookupName;
      }
    }
    posLine += ";\n";
    debugLine += ";\n";
    std::cout << debugLine.toStdString();
    mainLookup += posLine;
  }


  out << "lookup adjustoverlap {" << '\n';
  out << "  feature mkmk;\n";
  out << subLookups;
  out << mainLookup;
  out << "} adjustoverlap;" << '\n';




  file.close();
}



