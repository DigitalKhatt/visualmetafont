/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef MY_QPDF_P_H
#define MY_QPDF_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// #include <QtGui/private/qtguiglobal_p.h>

#ifndef QT_NO_PDF

#include "QtCore/qstring.h"
#include "QtCore/qvector.h"
#include "QtGui/private/qpainter_p.h"
#include "QtGui/private/qstroker_p.h"
#include "QtGui/qmatrix.h"
// #include "qstroker.h"
#include "QtGui/private/qpaintengine_p.h"
#include "qpaintengine.h"
// #include "qfontengine.h"
// #include "qfontsubset.h"
#include <unordered_map>

#include "OtLayout.h"
#include "QPdfWriter"
#include "qpagelayout.h"

struct hb_buffer_t;
class OtLayout;
class GlyphVis;

QT_BEGIN_NAMESPACE

namespace MyQPdf {

const char* qt_real_to_string(qreal val, char* buf);
const char* qt_int_to_string(int val, char* buf);

class ByteStream {
 public:
  // fileBacking means that ByteStream will buffer the contents on disk
  // if the size exceeds a certain threshold. In this case, if a byte
  // array was passed in, its contents may no longer correspond to the
  // ByteStream contents.
  explicit ByteStream(bool fileBacking = false);
  explicit ByteStream(QByteArray* ba, bool fileBacking = false);
  ~ByteStream();
  ByteStream& operator<<(char chr);
  ByteStream& operator<<(const char* str);
  ByteStream& operator<<(const QByteArray& str);
  ByteStream& operator<<(const ByteStream& src);
  ByteStream& operator<<(qreal val);
  ByteStream& operator<<(int val);
  ByteStream& operator<<(const QPointF& p);
  // Note that the stream may be invalidated by calls that insert data.
  QIODevice* stream();
  QByteArray getBA();
  void clear();

  static inline int maxMemorySize() { return 100000000; }
  static inline int chunkSize() { return 10000000; }

 protected:
  void constructor_helper(QIODevice* dev);
  void constructor_helper(QByteArray* ba);

 private:
  void prepareBuffer();

 private:
  QIODevice* dev;
  QByteArray ba;
  bool fileBackingEnabled;
  bool fileBackingActive;
  bool handleDirty;
};

enum PathFlags {
  ClipPath,
  FillPath,
  StrokePath,
  FillAndStrokePath
};
QByteArray generatePath(const QPainterPath& path, const QTransform& matrix, PathFlags flags);
QByteArray generateMatrix(const QTransform& matrix);
QByteArray generateDashes(const QPen& pen);
QByteArray patternForBrush(const QBrush& b);

struct Stroker {
  Stroker();
  void setPen(const QPen& pen, QPainter::RenderHints hints);
  void strokePath(const QPainterPath& path);
  ByteStream* stream;
  bool first;
  QTransform matrix;
  bool cosmeticPen;

 private:
  QStroker basicStroker;
  QDashStroker dashStroker;
  QStrokerOps* stroker;
};

QByteArray ascii85Encode(const QByteArray& input);

const char* toHex(ushort u, char* buffer);
const char* toHex(uchar u, char* buffer);

struct GlyphKey {
  int code;
  double lefttatweel = 0;
  double righttatweel = 0;

  /*
  GlyphKey() {}

  GlyphKey(int code) : code{ code }, lefttatweel{ 0 }, righttatweel{ 0 }{};
  GlyphKey(int code, double lefttatweel, double righttatweel) : code{ code }, lefttatweel{ lefttatweel }, righttatweel{ righttatweel }{};*/

  bool operator==(const GlyphKey& r) const {
    return r.code == code && r.lefttatweel == lefttatweel && r.righttatweel == righttatweel;
  }
};

}  // namespace MyQPdf

namespace std {
template <>
struct hash<MyQPdf::GlyphKey> {
  size_t operator()(const MyQPdf::GlyphKey& r) const {
    return hash<double>{}(r.code) ^ hash<double>{}(r.lefttatweel) ^ hash<double>{}(r.righttatweel);
  }
};

template <>
struct equal_to<MyQPdf::GlyphKey> {
  bool operator()(const MyQPdf::GlyphKey& r, const MyQPdf::GlyphKey& r2) const {
    return r == r2;
  }
};
}  // namespace std

class MyQPdfPage : public MyQPdf::ByteStream {
 public:
  MyQPdfPage();

  QVector<uint> images;
  QVector<uint> graphicStates;
  QVector<uint> patterns;
  QVector<uint> fonts;
  QVector<uint> annotations;

  void streamImage(int w, int h, int object);

  QSize pageSize;

 private:
};

class ObjectStream : public MyQPdf::ByteStream {
 public:
  ObjectStream();
  int objectnumber;
  QVector<uint> objectNumbers;
  QVector<uint> byteOffsets;

  void addObject(QByteArray data, uint objectNumber);

 private:
  int currentposition = 0;
};

class QPdfWriter;
class MyQPdfEnginePrivate;

struct Line {
  int begin;
  int end;
  int nbspaces;
  int charswidth;
};

struct GlyphIndex {
  int font;
  int encoding;
};

struct GlyphPath {
  uint codepoint;
  quint32 glyphIndex;
  QPainterPath path;
};

class MyQPdfEngine : public QPaintEngine {
  Q_DECLARE_PRIVATE(MyQPdfEngine)
  friend class QuranPdfWriter;

 public:
  MyQPdfEngine(OtLayout* otlayout);
  MyQPdfEngine(MyQPdfEnginePrivate& d);
  ~MyQPdfEngine() {}

  void setOutputFilename(const QString& filename);

  void setResolution(int resolution);
  int resolution() const;

  // reimplementations QPaintEngine
  bool begin(QPaintDevice* pdev) Q_DECL_OVERRIDE;
  bool end() Q_DECL_OVERRIDE;

  void drawPoints(const QPointF* points, int pointCount) Q_DECL_OVERRIDE;
  void drawLines(const QLineF* lines, int lineCount) Q_DECL_OVERRIDE;
  void drawRects(const QRectF* rects, int rectCount) Q_DECL_OVERRIDE;
  void drawPolygon(const QPointF* points, int pointCount, PolygonDrawMode mode) Q_DECL_OVERRIDE;
  void drawPath(const QPainterPath& path) Q_DECL_OVERRIDE;

  // void drawTextItem(const QPointF &p, const QTextItem &textItem) Q_DECL_OVERRIDE;

  void drawPixmap(const QRectF& rectangle, const QPixmap& pixmap, const QRectF& sr) Q_DECL_OVERRIDE;
  void drawImage(const QRectF& r, const QImage& pm, const QRectF& sr,
                 Qt::ImageConversionFlags flags = Qt::AutoColor) Q_DECL_OVERRIDE;
  void drawTiledPixmap(const QRectF& rectangle, const QPixmap& pixmap, const QPointF& point) Q_DECL_OVERRIDE;

  void drawHyperlink(const QRectF& r, const QUrl& url);

  void updateState(const QPaintEngineState& state) Q_DECL_OVERRIDE;

  int metric(QPaintDevice::PaintDeviceMetric metricType) const;
  Type type() const Q_DECL_OVERRIDE;
  // end reimplementations QPaintEngine

  // Printer stuff...
  bool newPage();

  // Page layout stuff
  void setPageLayout(const QPageLayout& pageLayout);
  void setPageSize(const QPageSize& pageSize);
  void setPageOrientation(QPageLayout::Orientation orientation);
  void setPageMargins(const QMarginsF& margins, QPageLayout::Unit units = QPageLayout::Point);

  QPageLayout pageLayout() const;

  void setPen();
  void setBrush();
  void setupGraphicsState(QPaintEngine::DirtyFlags flags);

  void beginFormXObject();
  int endFormXObject(QRectF bbox);
  void writeRawPDFtoCurrentStream(QString rawPDF);
  void addImagetoResources(int objectID);

 protected:
  // QScopedPointer<MyQPdfEnginePrivate> d_ptr;

 private:
  void updateClipPath(const QPainterPath& path, Qt::ClipOperation op);
};

class MyQPdfEnginePrivate : public QPaintEnginePrivate {
  Q_DECLARE_PUBLIC(MyQPdfEngine)

  friend class MyQPdfEngine;
  friend class QuranPdfWriter;

 public:
  MyQPdfEnginePrivate(OtLayout* otlayout);
  ~MyQPdfEnginePrivate();

  inline uint requestObject() { return currentObject++; }

  void writeHeader();
  void writeTail();

  int addImage(const QImage& image, bool* bitmap, qint64 serial_no);
  int addConstantAlphaObject(int brushAlpha, int penAlpha = 255);
  int addBrushPattern(const QTransform& matrix, bool* specifyColor, int* gStateObject);

  void drawTextItem(const QPointF& p, const QTextItemInt& ti);

  QTransform pageMatrix() const;

  void newPage();
  void beginFormXObject();
  int endFormXObject(QRectF bbox);
  void writeRawPDFtoCurrentStream(QString rawPDF);
  void addImagetoResources(int objectID);

  int currentObject;

  MyQPdfPage* currentPage;
  ObjectStream* currentObjectStream;
  MyQPdf::Stroker stroker;

  QPointF brushOrigin;
  QBrush brush;
  QPen pen;
  QVector<QPainterPath> clips;
  bool clipEnabled;
  bool allClipped;
  bool hasPen;
  bool hasBrush;
  bool simplePen;
  qreal opacity;
  bool xobjectState = false;

  // QHash<QFontEngine::FaceId, QFontSubset *> fonts;

  QPaintDevice* pdev;

  // the device the output is in the end streamed to.
  QIODevice* outDevice;
  bool ownsDevice;

  // printer options
  QString outputFileName;
  QString title;
  QString creator;
  bool embedFonts;
  int resolution;
  bool grayscale;

  // Page layout: size, orientation and margins
  QPageLayout m_pageLayout;

  OtLayout* otlayout;

  struct Dest {
    uint pageID;
    int left;
    int top;
    int zoom;
  };

  struct OutlineEntry {
    QString title;
    Dest dest;
  };

 private:
  int gradientBrush(const QBrush& b, const QTransform& matrix, int* gStateObject);
  int generateGradientShader(const QGradient* gradient, const QTransform& matrix, bool alpha = false);
  int generateLinearGradientShader(const QLinearGradient* lg, const QTransform& matrix, bool alpha);
  int generateRadialGradientShader(const QRadialGradient* gradient, const QTransform& matrix, bool alpha);
  int createShadingFunction(const QGradient* gradient, int from, int to, bool reflect, bool alpha);

  void writeInfo();
  void writePageRoot();
  // void writeFonts();
  // void embedFont(QFontSubset *font);

  QVector<int> xrefPositions;
  QHash<int, int> objectStreamPositions;
  QDataStream* stream;
  int streampos;

  int writeImage(const QByteArray& data, int width, int height, int depth,
                 int maskObject, int softMaskObject, bool dct = false, bool isMono = false);
  void writePage();
  void writeObjectStream();
  void writeOutlines();
  void writeStructure();
  void addObject(QByteArray data, uint objectNumber);

  int addXrefEntry(int object, bool printostr = true);
  int addXrefEntryForObjectStream(int object, int parentStream, int index);
  void printString(const QString& string);
  void printString(QByteArray& byteArray, const QString& string);
  void xprintf(const char* fmt, ...);
  void xprintf(QByteArray& byteArray, const char* fmt, ...);
  void generateSurahFont();
  void generateSurahFontFromSvg(const std::vector<QPainterPath>& paths);

  inline void write(const QByteArray& data) {
    stream->writeRawData(data.constData(), data.size());
    streampos += data.size();
  }

  int writeCompressed(const char* src, int len);
  QByteArray getCompressed(QByteArray src);
  inline int writeCompressed(const QByteArray& data) { return writeCompressed(data.constData(), data.length()); }
  int writeCompressed(QIODevice* dev);

  // various PDF objects
  int pageRoot, catalog, info, graphicsState, patternColorSpace, outlinesid, structTreeRootobjectId;
  QVector<uint> pages;
  QHash<qint64, uint> imageCache;
  QHash<QPair<uint, uint>, uint> alphaCache;

  // Quran specific

  QByteArray generateGlyph(GlyphVis& glyph);
  void writeType3Fonts();
  GlyphIndex getIndex(MyQPdf::GlyphKey glyph);
  QByteArray getImageStream(GlyphVis& glyph);
  int writeXobjectForm(GlyphVis& glyph, QString name);

  int currenttype3Font = -1;
  int surahType3Font = -1;
  std::unordered_map<MyQPdf::GlyphKey, GlyphIndex> glyphsToIndex;
  QHash<int, QVector<MyQPdf::GlyphKey>> fonttoToglyphs;
  QHash<QString, int> XObjects;
  QMap<uint, OutlineEntry> outlines;
  QVector<GlyphPath> surahs;
  std::unordered_map<int, int> surahNumberToUnicode;
};

QT_END_NAMESPACE

#endif  // QT_NO_PDF

#endif  // MY_QPDF_P_H
