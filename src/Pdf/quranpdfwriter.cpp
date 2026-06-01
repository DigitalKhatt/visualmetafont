/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file originated from the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "quranpdfwriter.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qfile.h>

#include <QRegularExpression>

#include "GlyphVis.h"
#include "OtLayout.h"
#include "QDateTime"
#include "QtGui/private/qpagedpaintdevice_p.h"
#include "hb.hh"
#include "private/qpdf_p.h"
#include "qdebug.h"
#include "qmessagebox.h"
#include "qpagedpaintdevice.h"
#include "qpdf.h"
#include "qsvgrenderer.h"

class QuranPdfWriterPrivate : public QObjectPrivate {
 public:
  QuranPdfWriterPrivate(OtLayout* otlayout)
      : QObjectPrivate() {
    engine = new MyQPdfEngine(otlayout);
    output = 0;
  }
  ~QuranPdfWriterPrivate() {
    delete engine;
    delete output;
  }

  MyQPdfEngine* engine;
  QFile* output;
};

class QuranPdfPagedPaintDevicePrivate : public QPagedPaintDevicePrivate {
 public:
  QuranPdfPagedPaintDevicePrivate(QuranPdfWriterPrivate* d)
      : QPagedPaintDevicePrivate(), pd(d) {}

  virtual ~QuranPdfPagedPaintDevicePrivate() {}

  bool setPageLayout(const QPageLayout& newPageLayout) Q_DECL_OVERRIDE {
    // Try to set the paint engine page layout
    pd->engine->setPageLayout(newPageLayout);
    // Set QPagedPaintDevice layout to match the current paint engine layout
    m_pageLayout = pd->engine->pageLayout();
    return m_pageLayout.isEquivalentTo(newPageLayout);
  }

  bool setPageSize(const QPageSize& pageSize) Q_DECL_OVERRIDE {
    // Try to set the paint engine page size
    pd->engine->setPageSize(pageSize);
    // Set QPagedPaintDevice layout to match the current paint engine layout
    m_pageLayout = pd->engine->pageLayout();
    return m_pageLayout.pageSize().isEquivalentTo(pageSize);
  }

  bool setPageOrientation(QPageLayout::Orientation orientation) Q_DECL_OVERRIDE {
    // Set the print engine value
    pd->engine->setPageOrientation(orientation);
    // Set QPagedPaintDevice layout to match the current paint engine layout
    m_pageLayout = pd->engine->pageLayout();
    return m_pageLayout.orientation() == orientation;
  }

  bool setPageMargins(const QMarginsF& margins) {
    return setPageMargins(margins, pageLayout().units());
  }

  bool setPageMargins(const QMarginsF& margins, QPageLayout::Unit units) Q_DECL_OVERRIDE {
    // Try to set engine margins
    pd->engine->setPageMargins(margins, units);
    // Set QPagedPaintDevice layout to match the current paint engine layout
    m_pageLayout = pd->engine->pageLayout();
    return m_pageLayout.margins() == margins && m_pageLayout.units() == units;
  }

  QPageLayout pageLayout() const Q_DECL_OVERRIDE {
    return pd->engine->pageLayout();
  }

  QuranPdfWriterPrivate* pd;

  QPageLayout m_pageLayout;
};

QuranPdfWriter::QuranPdfWriter(const QString& filename, OtLayout* otlayout)
    : QObject(*new QuranPdfWriterPrivate(otlayout)),
      QPagedPaintDevice(new QuranPdfPagedPaintDevicePrivate(d_func())) {
  Q_D(QuranPdfWriter);

  d->engine->setOutputFilename(filename);

  // Set QPagedPaintDevice layout to match the current paint engine layout
  devicePageLayout() = d->engine->pageLayout();
}

/*!
Constructs a PDF writer that will write the pdf to \a device.
*/
QuranPdfWriter::QuranPdfWriter(QIODevice* device, OtLayout* otlayout)
    : QObject(*new QuranPdfWriterPrivate(otlayout)),
      QPagedPaintDevice(new QuranPdfPagedPaintDevicePrivate(d_func())) {
  Q_D(QuranPdfWriter);

  d->engine->d_func()->outDevice = device;

  // Set QPagedPaintDevice layout to match the current paint engine layout
  devicePageLayout() = d->engine->pageLayout();
}

/*!
Destroys the pdf writer.
*/
QuranPdfWriter::~QuranPdfWriter() {
}

/*!
Returns the title of the document.
*/
QString QuranPdfWriter::title() const {
  // Q_D(const QuranPdfWriter);
  // return d->engine->d_func()->title;
  return "The Noble Quran - القرآن الكريم";
}

/*!
Sets the title of the document being created to \a title.
*/
void QuranPdfWriter::setTitle(const QString& title) {
  // d->engine->d_func()->title = title;
}

/*!
Returns the creator of the document.
*/
QString QuranPdfWriter::creator() const {
  return "Amine Anane";
  // return d->engine->d_func()->creator;
}

/*!
Sets the creator of the document to \a creator.
*/
void QuranPdfWriter::setCreator(const QString& creator) {
  // engine->creator = creator;
}

QPaintEngine* QuranPdfWriter::paintEngine() const {
  Q_D(const QuranPdfWriter);

  return d->engine;
}

void QuranPdfWriter::setResolution(int resolution) {
  Q_D(const QuranPdfWriter);

  d->engine->setResolution(resolution);
}

int QuranPdfWriter::resolution() const {
  Q_D(const QuranPdfWriter);

  return d->engine->resolution();
}

void QuranPdfWriter::setPageSize(PageSize size) {
  QPagedPaintDevice::setPageSize(QPageSize(QPageSize::PageSizeId(size)));
}

/*!
\reimp

\obsolete Use setPageSize(QPageSize(size, QPageSize::Millimeter)) instead

\sa setPageSize()
*/

void QuranPdfWriter::setPageSizeMM(const QSizeF& size) {
  QPagedPaintDevice::setPageSize(QPageSize(size, QPageSize::Millimeter));
}

/*!
\internal

Returns the metric for the given \a id.
*/
int QuranPdfWriter::metric(PaintDeviceMetric id) const {
  Q_D(const QuranPdfWriter);

  return d->engine->metric(id);
}

/*!
\reimp
*/
bool QuranPdfWriter::newPage() {
  Q_D(const QuranPdfWriter);

  return d->engine->newPage();
}

/*!
\reimp

\obsolete Use setPageMargins(QMarginsF(l, t, r, b), QPageLayout::Millimeter) instead

\sa setPageMargins()
*/
void QuranPdfWriter::setMargins(const Margins& m) {
  setPageMargins(QMarginsF(m.left, m.top, m.right, m.bottom), QPageLayout::Millimeter);
}

void QuranPdfWriter::beginFormXObject() {
  Q_D(const QuranPdfWriter);

  d->engine->d_func()->beginFormXObject();
}
int QuranPdfWriter::endFormXObject(QRectF bbox) {
  Q_D(const QuranPdfWriter);

  return d->engine->d_func()->endFormXObject(bbox);
}
void QuranPdfWriter::writeRawPDFtoCurrentStream(QString rawPDF) {
  Q_D(const QuranPdfWriter);

  return d->engine->d_func()->writeRawPDFtoCurrentStream(rawPDF);
}
void QuranPdfWriter::addImagetoResources(int objectID) {
  Q_D(const QuranPdfWriter);

  return d->engine->d_func()->addImagetoResources(objectID);
}

struct IndexedPath {
  int index = -1;
  QPainterPath path;
};

static bool readSvgNumber(const QString& s, int& pos, double& out) {
  while (pos < s.size() && (s[pos].isSpace() || s[pos] == ','))
    ++pos;

  static const QRegularExpression re(
      R"(^[+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)");

  QRegularExpressionMatch m = re.match(s.mid(pos));
  if (!m.hasMatch())
    return false;

  out = m.captured(0).toDouble();
  pos += m.capturedLength(0);
  return true;
}

static void skipSvgSeparators(const QString& s, int& pos) {
  while (pos < s.size() && (s[pos].isSpace() || s[pos] == ','))
    ++pos;
}

static QPainterPath svgPathDataToPainterPath(const QString& d) {
  QPainterPath path;

  int pos = 0;
  QChar cmd;
  QPointF cur(0.0, 0.0);
  QPointF start(0.0, 0.0);
  QPointF lastControl(0.0, 0.0);

  while (pos < d.size()) {
    skipSvgSeparators(d, pos);
    if (pos >= d.size())
      break;

    if (d[pos].isLetter()) {
      cmd = d[pos];
      ++pos;
    } else if (cmd.isNull()) {
      break;
    }

    if (cmd == 'Z' || cmd == 'z') {
      path.closeSubpath();
      cur = start;
      continue;
    }

    if (cmd == 'M' || cmd == 'm') {
      double x, y;
      if (!readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y))
        break;

      QPointF p = (cmd == 'm') ? (cur + QPointF(x, y)) : QPointF(x, y);
      path.moveTo(p);
      cur = p;
      start = p;

      // Following pairs after M/m are treated as L/l
      while (true) {
        int saved = pos;
        if (!readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        p = (cmd == 'm') ? (cur + QPointF(x, y)) : QPointF(x, y);
        path.lineTo(p);
        cur = p;
      }
    } else if (cmd == 'L' || cmd == 'l') {
      while (true) {
        double x, y;
        int saved = pos;
        if (!readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        QPointF p = (cmd == 'l') ? (cur + QPointF(x, y)) : QPointF(x, y);
        path.lineTo(p);
        cur = p;
      }
    } else if (cmd == 'H' || cmd == 'h') {
      while (true) {
        double x;
        int saved = pos;
        if (!readSvgNumber(d, pos, x)) {
          pos = saved;
          break;
        }

        cur.setX(cmd == 'h' ? cur.x() + x : x);
        path.lineTo(cur);
      }
    } else if (cmd == 'V' || cmd == 'v') {
      while (true) {
        double y;
        int saved = pos;
        if (!readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        cur.setY(cmd == 'v' ? cur.y() + y : y);
        path.lineTo(cur);
      }
    } else if (cmd == 'C' || cmd == 'c') {
      while (true) {
        double x1, y1, x2, y2, x, y;
        int saved = pos;
        if (!readSvgNumber(d, pos, x1) || !readSvgNumber(d, pos, y1) ||
            !readSvgNumber(d, pos, x2) || !readSvgNumber(d, pos, y2) ||
            !readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        QPointF p1 = (cmd == 'c') ? (cur + QPointF(x1, y1)) : QPointF(x1, y1);
        QPointF p2 = (cmd == 'c') ? (cur + QPointF(x2, y2)) : QPointF(x2, y2);
        QPointF p = (cmd == 'c') ? (cur + QPointF(x, y)) : QPointF(x, y);

        path.cubicTo(p1, p2, p);
        cur = p;
        lastControl = p2;
      }
    } else if (cmd == 'S' || cmd == 's') {
      while (true) {
        double x2, y2, x, y;
        int saved = pos;
        if (!readSvgNumber(d, pos, x2) || !readSvgNumber(d, pos, y2) ||
            !readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        QPointF p1 = cur * 2.0 - lastControl;
        QPointF p2 = (cmd == 's') ? (cur + QPointF(x2, y2)) : QPointF(x2, y2);
        QPointF p = (cmd == 's') ? (cur + QPointF(x, y)) : QPointF(x, y);

        path.cubicTo(p1, p2, p);
        cur = p;
        lastControl = p2;
      }
    } else if (cmd == 'Q' || cmd == 'q') {
      while (true) {
        double x1, y1, x, y;
        int saved = pos;
        if (!readSvgNumber(d, pos, x1) || !readSvgNumber(d, pos, y1) ||
            !readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        QPointF p1 = (cmd == 'q') ? (cur + QPointF(x1, y1)) : QPointF(x1, y1);
        QPointF p = (cmd == 'q') ? (cur + QPointF(x, y)) : QPointF(x, y);

        path.quadTo(p1, p);
        cur = p;
        lastControl = p1;
      }
    } else if (cmd == 'T' || cmd == 't') {
      while (true) {
        double x, y;
        int saved = pos;
        if (!readSvgNumber(d, pos, x) || !readSvgNumber(d, pos, y)) {
          pos = saved;
          break;
        }

        QPointF p1 = cur * 2.0 - lastControl;
        QPointF p = (cmd == 't') ? (cur + QPointF(x, y)) : QPointF(x, y);

        path.quadTo(p1, p);
        cur = p;
        lastControl = p1;
      }
    } else {
      qWarning() << "Unsupported SVG path command:" << cmd;
      break;
    }
  }

  return path;
}

static int parsePathIndex(const QString& id) {
  static const QRegularExpression re(R"(^path_(\d+)$)");
  QRegularExpressionMatch m = re.match(id);
  if (!m.hasMatch())
    return -1;
  return m.captured(1).toInt();
}

std::vector<QPainterPath> loadPathsFromSvg(const QString& fileName) {
  std::vector<QPainterPath> result;
  QVector<IndexedPath> indexed;

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Cannot open SVG file:" << fileName;
    return result;
  }

  QXmlStreamReader xml(&file);

  while (!xml.atEnd()) {
    xml.readNext();

    if (!xml.isStartElement())
      continue;

    if (xml.name() != "path")
      continue;

    const auto attrs = xml.attributes();
    const QString id = attrs.value("id").toString();
    const QString d = attrs.value("d").toString();

    if (id.isEmpty() || d.isEmpty())
      continue;

    int index = parsePathIndex(id);
    if (index < 0)
      continue;

    IndexedPath item;
    item.index = index;
    item.path = svgPathDataToPainterPath(d);
    indexed.push_back(item);
  }

  if (xml.hasError()) {
    qWarning() << "SVG parse error:" << xml.errorString();
    return result;
  }

  std::sort(indexed.begin(), indexed.end(), [](const IndexedPath& a, const IndexedPath& b) {
    return a.index < b.index;
  });

  result.reserve(indexed.size());
  for (const auto& item : indexed)
    result.push_back(item.path);

  return result;
}
#include <QPainter>
#include <QPdfWriter>
#include <QtMath>

static inline qreal mmToPx(qreal mm, qreal dpi) {
  return mm * dpi / 25.4;
}

static inline qreal ptToPx(qreal pt, qreal dpi) {
  return pt * dpi / 72.0;
}

static void drawNoticePage(QPainter& p,
                           const QRectF& page)

{
  p.save();

  QRectF box = page.adjusted(
      page.width() * 0.12,
      page.height() * 0.12,
      -page.width() * 0.12,
      -page.height() * 0.12);

  QFont titleFont("Times New Roman");
  titleFont.setPointSizeF(18);
  titleFont.setBold(true);
  QFont bodyFont("Times New Roman");
  bodyFont.setPointSizeF(11);
  p.setPen(Qt::black);
  p.setFont(titleFont);

  p.drawText(box, Qt::AlignTop | Qt::AlignHCenter,
             "Important Notice");

  QRectF bodyBox = box.adjusted(0, 3000, 0, 0);

  QString version = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
  QString url = "https://github.com/DigitalKhatt/oldmadinafont/mushaf.pdf";

  QString text =
      "This mushaf may contain errors and may be subject to updates.\n\n"
      "To avoid using an outdated or incorrect copy, always refer to "
      "the official latest version:\n\n" +
      url +
      "\n\n"
      "Generation Date: " +
      version +
      "\n\n"
      "To help ensure access to the latest corrected version, please share the official link rather than copies of this mushaf.";

  p.setFont(bodyFont);

  p.drawText(bodyBox,
             Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap,
             text);

  // visual page label

  /*QFont pageFont("Times New Roman");

  pageFont.setPointSizeF(9);

  p.setFont(pageFont);

  p.setPen(QColor(0, 0, 0, 120));

  p.drawText(QRectF(page.left(), page.bottom() - 40, page.width(), 20),
             Qt::AlignCenter,
             "i");*/

  p.restore();
}

static void drawOfficialUrlFooter(
    QPainter& painter,
    const QRectF& fullPagePx,
    int dpi,
    const QString& officialUrl,
    const QString& versionText = QString()) {
  painter.save();

  const qreal sideMargin = mmToPx(10.0, dpi);
  const qreal bottomSpace = mmToPx(0.0, dpi);
  const qreal footerH = mmToPx(2.0, dpi);

  QRectF footerRect(
      fullPagePx.left() + sideMargin,
      fullPagePx.bottom() - footerH - bottomSpace,
      fullPagePx.width() - 2.0 * sideMargin,
      footerH);

  QString text =
      versionText.isEmpty()
          ? QString("Official latest version: %1")
                .arg(officialUrl)
          : QString("Official latest version: %1 | %2")
                .arg(officialUrl, versionText);

  // Small subtle font
  QFont font("Times New Roman");
  font.setPointSizeF(5.5);

  painter.setFont(font);

  QColor color(0, 0, 0);

  // VERY subtle
  color.setAlphaF(0.50);

  painter.setPen(color);

  painter.drawText(
      footerRect,
      Qt::AlignCenter | Qt::AlignVCenter,
      text);

  painter.restore();
}
void QuranPdfWriter::generateQuranPages(QList<QList<LineLayoutInfo>> pages, int lineWidth, QList<QStringList> originalText, double scale, int margin) {
  Q_D(const QuranPdfWriter);

  QPainter painter;

  int surahNameType = 1;

  if (!painter.begin(this)) {
    qDebug() << "generateQuranPages : Cannot begin painter device";
    QMessageBox msgBox;
    msgBox.setText("generateQuranPages : Cannot begin painter device");
    msgBox.exec();
    return;
  }

  QSvgRenderer renderer(QString(":/images/surahframe.svg"));
  QRectF bbox = renderer.viewBoxF();
  bbox = QRectF(550 << OtLayout::SCALEBY, 0, bbox.width() * (188 << OtLayout::SCALEBY), bbox.height() * (160 << OtLayout::SCALEBY));

  beginFormXObject();
  renderer.render(&painter, bbox);
  uint suraFrameObjectId = endFormXObject(bbox);

  auto d_ep = d->engine->d_func();

  auto& glyphs = d_ep->otlayout->glyphs;

  d_ep->getIndex(MyQPdf::GlyphKey{glyphs["endofaya"].charcode, 0, 0});
  for (int i = 0; i < 10; i++) {
    d_ep->getIndex(MyQPdf::GlyphKey{i + 1632, 0, 0});
  }

  d_ep->currenttype3Font = -1;
  if (surahNameType == 2) {
    std::vector<QPainterPath> paths = loadPathsFromSvg(":/fonts/surahs.svg");
    d_ep->generateSurahFontFromSvg(paths);
  } else if (surahNameType == 1) {
    d_ep->generateSurahFont();
  }
  int suraNumber = 0;
  QRectF fullPage = this->pageLayout().fullRectPixels(4800);

  // newPage();
  // drawNoticePage(painter, fullPage);

  for (int p = 0; p < pages.size(); p++) {
    auto page = pages.at(p);
    auto originalPage = originalText.at(p);
    newPage();

    //*d_ep->currentPage << (500 << OtLayout::SCALEBY) << "Tw\n";
    //*d_ep->currentPage << (500 << OtLayout::SCALEBY) << "Tc\n";
    *d_ep->currentPage << "BT\n";

    int currentfont = 0;

    QPoint beginsajda;
    QPoint endsajda;

    for (int l = 0; l < page.size(); l++) {
      auto line = page.at(l);
      auto originalLine = originalPage.at(l);

      hb_position_t currentxPos = (lineWidth) + margin - line.xstartposition;
      hb_position_t currentyPos = line.ystartposition;

      qreal pixelSize = 1000 * line.fontSize;

      int end = -1;

      QPoint lastPos{currentxPos, currentyPos};
      int MCID = 0;

      if (line.type == LineType::Sura) {
        suraNumber++;

        int pageId = d_ep->pages.constLast();
        int ayaFrameyPos = currentyPos - (1100 << OtLayout::SCALEBY);

        auto outlineDest = d_ep->pageMatrix().map(QPoint{currentxPos, ayaFrameyPos});

        MyQPdfEnginePrivate::Dest dest{(uint)pageId, outlineDest.x(), outlineDest.y(), 0};

        MyQPdfEnginePrivate::OutlineEntry outline{QString("%1 (%2)").arg(originalLine).arg(suraNumber), dest};

        d_ep->outlines[d_ep->requestObject()] = outline;

        *d_ep->currentPage << "ET\n";
        *d_ep->currentPage << "q\n1 0 0 1 " << 0 << ayaFrameyPos << "cm\n";
        writeRawPDFtoCurrentStream(QString("/Im%1  Do\nQ\n").arg(suraFrameObjectId));
        addImagetoResources(suraFrameObjectId);
        *d_ep->currentPage << "BT\n";

        if (surahNameType == 1) {
          *d_ep->currentPage << "1 0 0 1 " << 0 << lastPos.y() + -300 << "Tm\n";
          currentfont = d_ep->surahType3Font;
          if (!d_ep->currentPage->fonts.contains(currentfont)) {
            d_ep->currentPage->fonts.append(currentfont);
          }

          int fontSize = 1100;
          double scale = (fontSize / 1000.0);

          *d_ep->currentPage << "/F" << currentfont << fontSize << "Tf\n";

          int unicode = d_ep->surahNumberToUnicode[suraNumber];
          int encoding = unicode - 0xE900;

          char buf[5];

          auto& surahWord = d_ep->surahs[3];
          int surahWordWidth = surahWord.path.boundingRect().width() * scale;

          auto& surah = d_ep->surahs[encoding];
          int surahNameWidth = surah.path.boundingRect().width() * scale;

          auto spaceBetween = 100;

          int startX = (17000 - (surahNameWidth + spaceBetween + surahWordWidth)) / 2;

          *d_ep->currentPage << startX << 0 << "Td <" << MyQPdf::toHex((uchar)encoding, buf) << "> Tj\n";
          *d_ep->currentPage << surahNameWidth + spaceBetween << -0 << "Td <" << MyQPdf::toHex((uchar)3, buf) << "> Tj\n";

          continue;
        }
        if (surahNameType == 2) {
          *d_ep->currentPage << "1 0 0 1 " << 0 << lastPos.y() + -300 << "Tm\n";
          currentfont = d_ep->surahType3Font;
          if (!d_ep->currentPage->fonts.contains(currentfont)) {
            d_ep->currentPage->fonts.append(currentfont);
          }

          int encoding = suraNumber - 1;

          int fontSize = 600;

          if (encoding == 0 || encoding == 1) {
            fontSize = 700;
          }

          double scale = (fontSize / 1000.0);

          *d_ep->currentPage << "/F" << currentfont << fontSize << "Tf\n";

          char buf[5];

          auto& surah = d_ep->surahs[encoding];
          int surahNameWidth = surah.path.boundingRect().width() * scale;

          int startX = (17000 - surahNameWidth) / 2;

          *d_ep->currentPage << startX << 0 << "Td <" << MyQPdf::toHex((uchar)encoding, buf) << "> Tj\n";

          continue;
        } else {
          *d_ep->currentPage << "1 0 0 -1 " << lastPos.x() << lastPos.y() << "Tm\n";
        }
      } else if (line.type == LineType::Bism) {
        *d_ep->currentPage << "1 0 0 -1 " << lastPos.x() << lastPos.y() << "Tm\n";
      } else {
        //*d_ep->currentPage << "1 0 0 -1 " << lastPos.x() << lastPos.y() << "Tm\n";
        *d_ep->currentPage << line.xscale << " 0 0 -1 " << lastPos.x() << lastPos.y() << "Tm\n";
      }

      *d_ep->currentPage << "/P << /MCID " << MCID << ">>\nBDC\n";

      for (int i = 0; i < line.glyphs.size(); i++) {
        char buf[5];

        auto index = d_ep->getIndex({line.glyphs[i].codepoint, line.glyphs[i].lefttatweel, line.glyphs[i].righttatweel});

        if (index.font != currentfont) {
          currentfont = index.font;
          if (!d_ep->currentPage->fonts.contains(currentfont)) {
            d_ep->currentPage->fonts.append(currentfont);
          }

          *d_ep->currentPage << "/F" << currentfont << pixelSize << "Tf\n";
        }

        if (i > end) {
          end = i;
          int currentcluster = line.glyphs[i].cluster;
          int endcluster = -1;
          do {
            while (end + 1 < line.glyphs.size() && line.glyphs[end + 1].cluster == currentcluster)
              ++end;

            if (end < line.glyphs.size() - 1) {
              endcluster = line.glyphs[end + 1].cluster;

              if (originalLine[endcluster].unicode() == 0x06E5 || originalLine[endcluster].unicode() == 0x06E6 || originalLine[endcluster].unicode() == 0x0640) {
                end++;
                currentcluster = endcluster;
                continue;
              } else {
                QString currGlyphName = d_ep->otlayout->glyphNamePerCode[line.glyphs[i].codepoint];
                // GlyphVis& currGlyph = glyphs[currGlyphName];

                QString nextGlyphName = d_ep->otlayout->glyphNamePerCode[line.glyphs[end + 1].codepoint];
                // GlyphVis& nextglyph = glyphs[nextGlyphName];

                if (currGlyphName == "reh.isol" && nextGlyphName == "behshape.init.beforereh") {
                  end++;
                  currentcluster = endcluster;
                  continue;
                }
              }

            } else {
              endcluster = originalLine.size();
            }
            break;
          } while (true);

          *d_ep->currentPage << "/Span << /ActualText <FEFF";
          for (int ii = line.glyphs[i].cluster; ii < endcluster; ++ii) {
            ushort unicode = originalLine[ii].unicode();
            if (unicode == 10)
              unicode = 0x20;
            *d_ep->currentPage << MyQPdf::toHex(unicode, buf);
          }
          *d_ep->currentPage << "> >>\n"
                                "BDC\n";
        }

        // set color
        if (line.glyphs[i].color && line.type != LineType::Sura) {
          int color = line.glyphs[i].color;
          *d_ep->currentPage << ((color >> 24) & 0xff) / 255.0 << ((color >> 16) & 0xff) / 255.0 << ((color >> 8) & 0xff) / 255.0 << "scn\n";
        }

        QPoint pos;
        currentxPos -= line.glyphs[i].x_advance;
        pos.setX(currentxPos + (line.glyphs[i].x_offset));
        pos.setY(currentyPos - (line.glyphs[i].y_offset));

        if (line.glyphs[i].beginsajda) {
          beginsajda = {lastPos.x(), currentyPos};
        } else if (line.glyphs[i].endsajda) {
          endsajda = {pos.x(), currentyPos};
        }

        // if (line.glyphs[i].codepoint != 32) {
        QPoint diff = pos - lastPos;
        lastPos = pos;
        //*d_ep->currentPage << "1 0 0 -1 " << pos.x() << pos.y() << "Tm <" << MyQPdf::toHex((uchar)index.encoding, buf) << "> Tj\n";
        *d_ep->currentPage << diff.x() << -diff.y() << "Td <" << MyQPdf::toHex((uchar)index.encoding, buf) << "> Tj\n";
        //}

        if (line.glyphs[i].color) {
          *d_ep->currentPage << 0.0 << 0.0 << 0.0 << "scn\n";
        }

        if (i == end) {
          *d_ep->currentPage << "EMC\n";
        }
      }
      //*d_ep->currentPage << "/Span << /ActualText <FEFF000A> >>\nBDC\nEMC\n";

      *d_ep->currentPage << "EMC\n";

      if (!beginsajda.isNull() && !endsajda.isNull()) {
        if (beginsajda.y() != endsajda.y()) {
          qDebug() << "Sajda Rule not in the same line";
          // throw "Sajda Rule not in the same line";
          // TODO check this
          beginsajda.setX((lineWidth) + margin - line.xstartposition);
        }

        *d_ep->currentPage << "ET\n";

        QPen pen;
        pen.setWidth(50 << OtLayout::SCALEBY);
        painter.setPen(pen);

        painter.drawLine(QPoint{beginsajda.x(), endsajda.y() - (1100 << OtLayout::SCALEBY)}, QPoint{endsajda.x(), endsajda.y() - (1100 << OtLayout::SCALEBY)});
        beginsajda = QPoint();
        endsajda = QPoint();

        *d_ep->currentPage << "BT\n";
      }
    }
    *d_ep->currentPage << "ET\n";
    // drawOfficialUrlFooter(painter, fullPage, 4800, "https://yourdomain.com/mushaf", "Version 1.3");
  }

  painter.end();
}
