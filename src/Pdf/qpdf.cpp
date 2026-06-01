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

#include "qpdf.h"

#ifndef QT_NO_PDF

// #include "qplatformdefs.h"
#include <qdebug.h>
#include <qfile.h>
#include <qtemporaryfile.h>
// #include <private/qmath_p.h>
#include <qmath.h>
// #include <private/qpainter_p.h>
#include <qnumeric.h>
#include <qpainter.h>
// #include "private/qfont_p.h"
#include <qimagewriter.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <iostream>

#include "GlyphVis.h"
#include "QByteArrayOperator.h"
#include "QtCore/qdatetime.h"
#include "automedina/automedina.h"
#include "glyph.hpp"
#include "hb.hh"
#include "qbuffer.h"
#include "qfont.h"
#include "qrawfont.h"
#include "qurl.h"

extern "C" {
#include "mplibps.h"
}

#ifndef QT_NO_COMPRESS
#include <zlib.h>
#endif

#ifdef QT_NO_COMPRESS
static const bool do_compress = false;
#else
static const bool do_compress = true;
#endif

// might be helpful for smooth transforms of images
// Can't use it though, as gs generates completely wrong images if this is true.
static const bool interpolateImages = false;

QT_BEGIN_NAMESPACE

inline QPaintEngine::PaintEngineFeatures qt_pdf_decide_features() {
  QPaintEngine::PaintEngineFeatures f = QPaintEngine::AllFeatures;
  f &= ~(QPaintEngine::PorterDuff | QPaintEngine::PerspectiveTransform | QPaintEngine::ObjectBoundingModeGradients | QPaintEngine::ConicalGradientFill);
  return f;
}

const char* qt_real_to_string_4(qreal val, char* buf) {
  const char* ret = buf;

  if (!qIsFinite(val) || std::abs(val) > std::numeric_limits<quint32>::max()) {
    *(buf++) = '0';
    *(buf++) = ' ';
    *buf = 0;
    return ret;
  }

  if (val < 0) {
    *(buf++) = '-';
    val = -val;
  }

  qreal intPartReal;
  qreal frac = std::modf(val, &intPartReal);
  quint32 ival = (quint32)intPartReal;

  int ifrac = (int)std::round(frac * 10000.0);
  if (ifrac == 10000) {
    ++ival;
    ifrac = 0;
  }

  // integer part
  char tmp[32];
  int i = 0;
  if (ival == 0) {
    *(buf++) = '0';
  } else {
    while (ival) {
      tmp[i++] = char('0' + (ival % 10));
      ival /= 10;
    }
    while (i)
      *(buf++) = tmp[--i];
  }

  // fractional part (trim trailing zeros)
  if (ifrac) {
    char fracDigits[4];
    for (int j = 3; j >= 0; --j) {
      fracDigits[j] = char('0' + (ifrac % 10));
      ifrac /= 10;
    }

    int last = 3;
    while (last >= 0 && fracDigits[last] == '0')
      --last;

    *(buf++) = '.';
    for (int j = 0; j <= last; ++j)
      *(buf++) = fracDigits[j];
  }

  *(buf++) = ' ';
  *buf = 0;
  return ret;
}

const char* qt_real_to_string_compact(qreal val, char* buf) {
  const char* ret = buf;

  if (!qIsFinite(val) || std::abs(val) > std::numeric_limits<quint32>::max()) {
    *(buf++) = '0';
    *(buf++) = ' ';
    *buf = 0;
    return ret;
  }

  if (val < 0) {
    *(buf++) = '-';
    val = -val;
  }

  qreal intPartReal;
  qreal frac = std::modf(val, &intPartReal);
  quint32 intPart = static_cast<quint32>(intPartReal);

  // Round to 9 decimal digits
  quint32 fracPart = static_cast<quint32>(std::round(frac * 1000000000.0));
  if (fracPart == 1000000000u) {
    ++intPart;
    fracPart = 0;
  }

  // Write integer part
  char rev[32];
  int n = 0;
  if (intPart == 0) {
    *(buf++) = '0';
  } else {
    while (intPart) {
      rev[n++] = char('0' + (intPart % 10));
      intPart /= 10;
    }
    while (n) {
      *(buf++) = rev[--n];
    }
  }

  // Write fractional part without trailing zeros
  if (fracPart != 0) {
    char fracDigits[9];
    for (int i = 8; i >= 0; --i) {
      fracDigits[i] = char('0' + (fracPart % 10));
      fracPart /= 10;
    }

    int last = 8;
    while (last >= 0 && fracDigits[last] == '0')
      --last;

    *(buf++) = '.';
    for (int i = 0; i <= last; ++i)
      *(buf++) = fracDigits[i];
  }

  *(buf++) = ' ';
  *buf = 0;
  return ret;
}

/* also adds a space at the end of the number */
const char* qt_real_to_string_original(qreal val, char* buf) {
  const char* ret = buf;

  if (qIsNaN(val)) {
    *(buf++) = '0';
    *(buf++) = ' ';
    *buf = 0;
    return ret;
  }

  if (val < 0) {
    *(buf++) = '-';
    val = -val;
  }
  unsigned int ival = (unsigned int)val;
  qreal frac = val - (qreal)ival;

  int ifrac = (int)(frac * 1000000000);
  if (ifrac == 1000000000) {
    ++ival;
    ifrac = 0;
  }
  char output[256];
  int i = 0;
  while (ival) {
    output[i] = '0' + (ival % 10);
    ++i;
    ival /= 10;
  }
  int fact = 100000000;
  if (i == 0) {
    *(buf++) = '0';
  } else {
    while (i) {
      *(buf++) = output[--i];
      fact /= 10;
      ifrac /= 10;
    }
  }

  if (ifrac) {
    *(buf++) = '.';
    while (fact) {
      *(buf++) = '0' + ((ifrac / fact) % 10);
      fact /= 10;
    }
  }
  *(buf++) = ' ';
  *buf = 0;
  return ret;
}

const char* MyQPdf::qt_real_to_string(qreal val, char* buf) {
  return qt_real_to_string_original(val, buf);
}
const char* MyQPdf::qt_int_to_string(int val, char* buf) {
  const char* ret = buf;
  if (val < 0) {
    *(buf++) = '-';
    val = -val;
  }
  char output[256];
  int i = 0;
  while (val) {
    output[i] = '0' + (val % 10);
    ++i;
    val /= 10;
  }
  if (i == 0) {
    *(buf++) = '0';
  } else {
    while (i)
      *(buf++) = output[--i];
  }
  *(buf++) = ' ';
  *buf = 0;
  return ret;
}

namespace MyQPdf {
ByteStream::ByteStream(QByteArray* byteArray, bool fileBacking)
    : dev(new QBuffer(byteArray)),
      fileBackingEnabled(fileBacking),
      fileBackingActive(false),
      handleDirty(false) {
  dev->open(QIODevice::ReadWrite | QIODevice::Append);
}

ByteStream::ByteStream(bool fileBacking)
    : dev(new QBuffer(&ba)),
      fileBackingEnabled(fileBacking),
      fileBackingActive(false),
      handleDirty(false) {
  dev->open(QIODevice::ReadWrite);
}

ByteStream::~ByteStream() {
  delete dev;
}

ByteStream& ByteStream::operator<<(char chr) {
  if (handleDirty) prepareBuffer();
  dev->write(&chr, 1);
  return *this;
}

ByteStream& ByteStream::operator<<(const char* str) {
  if (handleDirty) prepareBuffer();
  dev->write(str, strlen(str));
  return *this;
}

ByteStream& ByteStream::operator<<(const QByteArray& str) {
  if (handleDirty) prepareBuffer();
  dev->write(str);
  return *this;
}

ByteStream& ByteStream::operator<<(const ByteStream& src) {
  Q_ASSERT(!src.dev->isSequential());
  if (handleDirty) prepareBuffer();
  // We do play nice here, even though it looks ugly.
  // We save the position and restore it afterwards.
  ByteStream& s = const_cast<ByteStream&>(src);
  qint64 pos = s.dev->pos();
  s.dev->reset();
  while (!s.dev->atEnd()) {
    QByteArray buf = s.dev->read(chunkSize());
    dev->write(buf);
  }
  s.dev->seek(pos);
  return *this;
}

ByteStream& ByteStream::operator<<(qreal val) {
  char buf[256];
  qt_real_to_string(val, buf);
  *this << buf;
  return *this;
}

ByteStream& ByteStream::operator<<(int val) {
  char buf[256];
  qt_int_to_string(val, buf);
  *this << buf;
  return *this;
}

ByteStream& ByteStream::operator<<(const QPointF& p) {
  char buf[256];
  qt_real_to_string(p.x(), buf);
  *this << buf;
  qt_real_to_string(p.y(), buf);
  *this << buf;
  return *this;
}

QIODevice* ByteStream::stream() {
  dev->reset();
  handleDirty = true;
  return dev;
}

QByteArray ByteStream::getBA() {
  return this->ba;
}

void ByteStream::clear() {
  dev->open(QIODevice::ReadWrite | QIODevice::Truncate);
}

void ByteStream::constructor_helper(QByteArray* ba) {
  delete dev;
  dev = new QBuffer(ba);
  dev->open(QIODevice::ReadWrite);
}

void ByteStream::prepareBuffer() {
  Q_ASSERT(!dev->isSequential());
  qint64 size = dev->size();
  if (fileBackingEnabled && !fileBackingActive && size > maxMemorySize()) {
    // Switch to file backing.
    QTemporaryFile* newFile = new QTemporaryFile;
    newFile->open();
    dev->reset();
    while (!dev->atEnd()) {
      QByteArray buf = dev->read(chunkSize());
      newFile->write(buf);
    }
    delete dev;
    dev = newFile;
    ba.clear();
    fileBackingActive = true;
  }
  if (dev->pos() != size) {
    dev->seek(size);
    handleDirty = false;
  }
}
}  // namespace MyQPdf

#define QT_PATH_ELEMENT(elm)

QByteArray MyQPdf::generatePath(const QPainterPath& path, const QTransform& matrix, PathFlags flags) {
  QByteArray result;
  if (!path.elementCount())
    return result;

  ByteStream s(&result);

  int start = -1;
  for (int i = 0; i < path.elementCount(); ++i) {
    const QPainterPath::Element& elm = path.elementAt(i);
    switch (elm.type) {
      case QPainterPath::MoveToElement:
        if (start >= 0 && path.elementAt(start).x == path.elementAt(i - 1).x && path.elementAt(start).y == path.elementAt(i - 1).y)
          s << "h\n";
        s << matrix.map(QPointF(elm.x, elm.y)) << "m\n";
        start = i;
        break;
      case QPainterPath::LineToElement:
        s << matrix.map(QPointF(elm.x, elm.y)) << "l\n";
        break;
      case QPainterPath::CurveToElement:
        Q_ASSERT(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement);
        Q_ASSERT(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement);
        s << matrix.map(QPointF(elm.x, elm.y))
          << matrix.map(QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y))
          << matrix.map(QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y))
          << "c\n";
        i += 2;
        break;
      default:
        qFatal("MyQPdf::generatePath(), unhandled type: %d", elm.type);
    }
  }
  if (start >= 0 && path.elementAt(start).x == path.elementAt(path.elementCount() - 1).x && path.elementAt(start).y == path.elementAt(path.elementCount() - 1).y)
    s << "h\n";

  Qt::FillRule fillRule = path.fillRule();

  const char* op = "";
  switch (flags) {
    case ClipPath:
      op = (fillRule == Qt::WindingFill) ? "W n\n" : "W* n\n";
      break;
    case FillPath:
      op = (fillRule == Qt::WindingFill) ? "f\n" : "f*\n";
      break;
    case StrokePath:
      op = "S\n";
      break;
    case FillAndStrokePath:
      op = (fillRule == Qt::WindingFill) ? "B\n" : "B*\n";
      break;
  }
  s << op;
  return result;
}

QByteArray MyQPdf::generateMatrix(const QTransform& matrix) {
  QByteArray result;
  ByteStream s(&result);
  s << matrix.m11()
    << matrix.m12()
    << matrix.m21()
    << matrix.m22()
    << matrix.dx()
    << matrix.dy()
    << "cm\n";
  return result;
}

QByteArray MyQPdf::generateDashes(const QPen& pen) {
  QByteArray result;
  ByteStream s(&result);
  s << '[';

  QVector<qreal> dasharray = pen.dashPattern();
  qreal w = pen.widthF();
  if (w < 0.001)
    w = 1;
  for (int i = 0; i < dasharray.size(); ++i) {
    qreal dw = dasharray.at(i) * w;
    if (dw < 0.0001) dw = 0.0001;
    s << dw;
  }
  s << ']';
  s << pen.dashOffset() * w;
  s << " d\n";
  return result;
}

static const char* const pattern_for_brush[] = {
    0,  // NoBrush
    0,  // SolidPattern
    "0 J\n"
    "6 w\n"
    "[] 0 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "0 4 m\n"
    "8 4 l\n"
    "S\n",  // Dense1Pattern

    "0 J\n"
    "2 w\n"
    "[6 2] 1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[] 0 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[6 2] -3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n",  // Dense2Pattern

    "0 J\n"
    "2 w\n"
    "[6 2] 1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 2] -1 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[6 2] -3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n",  // Dense3Pattern

    "0 J\n"
    "2 w\n"
    "[2 2] 1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 2] -1 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[2 2] 1 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n",  // Dense4Pattern

    "0 J\n"
    "2 w\n"
    "[2 6] -1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 2] 1 d\n"
    "2 0 m\n"
    "2 8 l\n"
    "6 0 m\n"
    "6 8 l\n"
    "S\n"
    "[2 6] 3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n",  // Dense5Pattern

    "0 J\n"
    "2 w\n"
    "[2 6] -1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n"
    "[2 6] 3 d\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n",  // Dense6Pattern

    "0 J\n"
    "2 w\n"
    "[2 6] -1 d\n"
    "0 0 m\n"
    "0 8 l\n"
    "8 0 m\n"
    "8 8 l\n"
    "S\n",  // Dense7Pattern

    "1 w\n"
    "0 4 m\n"
    "8 4 l\n"
    "S\n",  // HorPattern

    "1 w\n"
    "4 0 m\n"
    "4 8 l\n"
    "S\n",  // VerPattern

    "1 w\n"
    "4 0 m\n"
    "4 8 l\n"
    "0 4 m\n"
    "8 4 l\n"
    "S\n",  // CrossPattern

    "1 w\n"
    "-1 5 m\n"
    "5 -1 l\n"
    "3 9 m\n"
    "9 3 l\n"
    "S\n",  // BDiagPattern

    "1 w\n"
    "-1 3 m\n"
    "5 9 l\n"
    "3 -1 m\n"
    "9 5 l\n"
    "S\n",  // FDiagPattern

    "1 w\n"
    "-1 3 m\n"
    "5 9 l\n"
    "3 -1 m\n"
    "9 5 l\n"
    "-1 5 m\n"
    "5 -1 l\n"
    "3 9 m\n"
    "9 3 l\n"
    "S\n",  // DiagCrossPattern
};

QByteArray MyQPdf::patternForBrush(const QBrush& b) {
  int style = b.style();
  if (style > Qt::DiagCrossPattern)
    return QByteArray();
  return pattern_for_brush[style];
}

static void moveToHook(qfixed x, qfixed y, void* data) {
  MyQPdf::Stroker* t = (MyQPdf::Stroker*)data;
  if (!t->first)
    *t->stream << "h\n";
  if (!t->cosmeticPen)
    t->matrix.map(x, y, &x, &y);
  *t->stream << x << y << "m\n";
  t->first = false;
}

static void lineToHook(qfixed x, qfixed y, void* data) {
  MyQPdf::Stroker* t = (MyQPdf::Stroker*)data;
  if (!t->cosmeticPen)
    t->matrix.map(x, y, &x, &y);
  *t->stream << x << y << "l\n";
}

static void cubicToHook(qfixed c1x, qfixed c1y,
                        qfixed c2x, qfixed c2y,
                        qfixed ex, qfixed ey,
                        void* data) {
  MyQPdf::Stroker* t = (MyQPdf::Stroker*)data;
  if (!t->cosmeticPen) {
    t->matrix.map(c1x, c1y, &c1x, &c1y);
    t->matrix.map(c2x, c2y, &c2x, &c2y);
    t->matrix.map(ex, ey, &ex, &ey);
  }
  *t->stream << c1x << c1y
             << c2x << c2y
             << ex << ey
             << "c\n";
}

MyQPdf::Stroker::Stroker()
    : stream(0),
      first(true),
      dashStroker(&basicStroker) {
  stroker = &basicStroker;
  basicStroker.setMoveToHook(moveToHook);
  basicStroker.setLineToHook(lineToHook);
  basicStroker.setCubicToHook(cubicToHook);
  cosmeticPen = true;
  basicStroker.setStrokeWidth(.1);
}

void MyQPdf::Stroker::setPen(const QPen& pen, QPainter::RenderHints hints) {
  if (pen.style() == Qt::NoPen) {
    stroker = 0;
    return;
  }
  qreal w = pen.widthF();
  bool zeroWidth = w < 0.0001;
  cosmeticPen = qt_pen_is_cosmetic(pen, hints);
  if (zeroWidth)
    w = .1;

  basicStroker.setStrokeWidth(w);
  basicStroker.setCapStyle(pen.capStyle());
  basicStroker.setJoinStyle(pen.joinStyle());
  basicStroker.setMiterLimit(pen.miterLimit());

  QVector<qreal> dashpattern = pen.dashPattern();
  if (zeroWidth) {
    for (int i = 0; i < dashpattern.size(); ++i)
      dashpattern[i] *= 10.;
  }
  if (!dashpattern.isEmpty()) {
    dashStroker.setDashPattern(dashpattern);
    dashStroker.setDashOffset(pen.dashOffset());
    stroker = &dashStroker;
  } else {
    stroker = &basicStroker;
  }
}

void MyQPdf::Stroker::strokePath(const QPainterPath& path) {
  if (!stroker)
    return;
  first = true;

  stroker->strokePath(path, this, cosmeticPen ? matrix : QTransform());
  *stream << "h f\n";
}

QByteArray MyQPdf::ascii85Encode(const QByteArray& input) {
  int isize = input.size() / 4 * 4;
  QByteArray output;
  output.resize(input.size() * 5 / 4 + 7);
  char* out = output.data();
  const uchar* in = (const uchar*)input.constData();
  for (int i = 0; i < isize; i += 4) {
    uint val = (((uint)in[i]) << 24) + (((uint)in[i + 1]) << 16) + (((uint)in[i + 2]) << 8) + (uint)in[i + 3];
    if (val == 0) {
      *out = 'z';
      ++out;
    } else {
      char base[5];
      base[4] = val % 85;
      val /= 85;
      base[3] = val % 85;
      val /= 85;
      base[2] = val % 85;
      val /= 85;
      base[1] = val % 85;
      val /= 85;
      base[0] = val % 85;
      *(out++) = base[0] + '!';
      *(out++) = base[1] + '!';
      *(out++) = base[2] + '!';
      *(out++) = base[3] + '!';
      *(out++) = base[4] + '!';
    }
  }
  // write the last few bytes
  int remaining = input.size() - isize;
  if (remaining) {
    uint val = 0;
    for (int i = isize; i < input.size(); ++i)
      val = (val << 8) + in[i];
    val <<= 8 * (4 - remaining);
    char base[5];
    base[4] = val % 85;
    val /= 85;
    base[3] = val % 85;
    val /= 85;
    base[2] = val % 85;
    val /= 85;
    base[1] = val % 85;
    val /= 85;
    base[0] = val % 85;
    for (int i = 0; i < remaining + 1; ++i)
      *(out++) = base[i] + '!';
  }
  *(out++) = '~';
  *(out++) = '>';
  output.resize(out - output.data());
  return output;
}

const char* MyQPdf::toHex(ushort u, char* buffer) {
  int i = 3;
  while (i >= 0) {
    ushort hex = (u & 0x000f);
    if (hex < 0x0a)
      buffer[i] = '0' + hex;
    else
      buffer[i] = 'A' + (hex - 0x0a);
    u = u >> 4;
    i--;
  }
  buffer[4] = '\0';
  return buffer;
}

const char* MyQPdf::toHex(uchar u, char* buffer) {
  int i = 1;
  while (i >= 0) {
    ushort hex = (u & 0x000f);
    if (hex < 0x0a)
      buffer[i] = '0' + hex;
    else
      buffer[i] = 'A' + (hex - 0x0a);
    u = u >> 4;
    i--;
  }
  buffer[2] = '\0';
  return buffer;
}

MyQPdfPage::MyQPdfPage()
    : MyQPdf::ByteStream(true)  // Enable file backing
{
}

void MyQPdfPage::streamImage(int w, int h, int object) {
  *this << w << "0 0 " << -h << "0 " << h << "cm /Im" << object << " Do\n";
  if (!images.contains(object))
    images.append(object);
}

ObjectStream::ObjectStream()
    : MyQPdf::ByteStream(true)  // Enable file backing
{
}

void ObjectStream::addObject(QByteArray data, uint objectNumber) {
  byteOffsets.append(currentposition);
  *this << data;
  currentposition += data.length();

  objectNumbers.append(objectNumber);
}

MyQPdfEngine::MyQPdfEngine(MyQPdfEnginePrivate& dd)
    : QPaintEngine(dd, qt_pdf_decide_features()) {
}

MyQPdfEngine::MyQPdfEngine(OtLayout* otlayout)
    : MyQPdfEngine(*new MyQPdfEnginePrivate(otlayout)) {
}

void MyQPdfEngine::setOutputFilename(const QString& filename) {
  Q_D(MyQPdfEngine);
  d->outputFileName = filename;
}

void MyQPdfEngine::drawPoints(const QPointF* points, int pointCount) {
  if (!points)
    return;

  Q_D(MyQPdfEngine);
  QPainterPath p;
  for (int i = 0; i != pointCount; ++i) {
    p.moveTo(points[i]);
    p.lineTo(points[i] + QPointF(0, 0.001));
  }

  bool hadBrush = d->hasBrush;
  d->hasBrush = false;
  drawPath(p);
  d->hasBrush = hadBrush;
}

void MyQPdfEngine::drawLines(const QLineF* lines, int lineCount) {
  if (!lines)
    return;

  Q_D(MyQPdfEngine);
  QPainterPath p;
  for (int i = 0; i != lineCount; ++i) {
    p.moveTo(lines[i].p1());
    p.lineTo(lines[i].p2());
  }
  bool hadBrush = d->hasBrush;
  d->hasBrush = false;
  drawPath(p);
  d->hasBrush = hadBrush;
}

void MyQPdfEngine::drawRects(const QRectF* rects, int rectCount) {
  if (!rects)
    return;

  Q_D(MyQPdfEngine);

  if (d->clipEnabled && d->allClipped)
    return;
  if (!d->hasPen && !d->hasBrush)
    return;

  QBrush penBrush = d->pen.brush();
  if (d->simplePen || !d->hasPen) {
    // draw strokes natively in this case for better output
    if (!d->simplePen && !d->stroker.matrix.isIdentity())
      *d->currentPage << "q\n"
                      << MyQPdf::generateMatrix(d->stroker.matrix);
    for (int i = 0; i < rectCount; ++i)
      *d->currentPage << rects[i].x() << rects[i].y() << rects[i].width() << rects[i].height() << "re\n";
    *d->currentPage << (d->hasPen ? (d->hasBrush ? "B\n" : "S\n") : "f\n");
    if (!d->simplePen && !d->stroker.matrix.isIdentity())
      *d->currentPage << "Q\n";
  } else {
    QPainterPath p;
    for (int i = 0; i != rectCount; ++i)
      p.addRect(rects[i]);
    drawPath(p);
  }
}

void MyQPdfEngine::drawPolygon(const QPointF* points, int pointCount, PolygonDrawMode mode) {
  Q_D(MyQPdfEngine);

  if (!points || !pointCount)
    return;

  bool hb = d->hasBrush;
  QPainterPath p;

  switch (mode) {
    case OddEvenMode:
      p.setFillRule(Qt::OddEvenFill);
      break;
    case ConvexMode:
    case WindingMode:
      p.setFillRule(Qt::WindingFill);
      break;
    case PolylineMode:
      d->hasBrush = false;
      break;
    default:
      break;
  }

  p.moveTo(points[0]);
  for (int i = 1; i < pointCount; ++i)
    p.lineTo(points[i]);

  if (mode != PolylineMode)
    p.closeSubpath();
  drawPath(p);

  d->hasBrush = hb;
}

void MyQPdfEngine::drawPath(const QPainterPath& p) {
  Q_D(MyQPdfEngine);

  if (d->clipEnabled && d->allClipped)
    return;
  if (!d->hasPen && !d->hasBrush)
    return;

  if (d->simplePen) {
    // draw strokes natively in this case for better output
    *d->currentPage << MyQPdf::generatePath(p, QTransform(), d->hasBrush ? MyQPdf::FillAndStrokePath : MyQPdf::StrokePath);
  } else {
    if (d->hasBrush)
      *d->currentPage << MyQPdf::generatePath(p, d->stroker.matrix, MyQPdf::FillPath);
    if (d->hasPen) {
      *d->currentPage << "q\n";
      QBrush b = d->brush;
      d->brush = d->pen.brush();
      setBrush();
      d->stroker.strokePath(p);
      *d->currentPage << "Q\n";
      d->brush = b;
    }
  }
}

void MyQPdfEngine::drawPixmap(const QRectF& rectangle, const QPixmap& pixmap, const QRectF& sr) {
  if (sr.isEmpty() || rectangle.isEmpty() || pixmap.isNull())
    return;
  Q_D(MyQPdfEngine);

  QBrush b = d->brush;

  QRect sourceRect = sr.toRect();
  QPixmap pm = sourceRect != pixmap.rect() ? pixmap.copy(sourceRect) : pixmap;
  QImage image = pm.toImage();
  bool bitmap = true;
  const int object = d->addImage(image, &bitmap, pm.cacheKey());
  if (object < 0)
    return;

  *d->currentPage << "q\n/GSa gs\n";
  *d->currentPage
      << MyQPdf::generateMatrix(QTransform(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                                           rectangle.x(), rectangle.y()) *
                                (d->simplePen ? QTransform() : d->stroker.matrix));
  if (bitmap) {
    // set current pen as d->brush
    d->brush = d->pen.brush();
  }
  setBrush();
  d->currentPage->streamImage(image.width(), image.height(), object);
  *d->currentPage << "Q\n";

  d->brush = b;
}

void MyQPdfEngine::drawImage(const QRectF& rectangle, const QImage& image, const QRectF& sr, Qt::ImageConversionFlags) {
  if (sr.isEmpty() || rectangle.isEmpty() || image.isNull())
    return;
  Q_D(MyQPdfEngine);

  QRect sourceRect = sr.toRect();
  QImage im = sourceRect != image.rect() ? image.copy(sourceRect) : image;
  bool bitmap = true;
  const int object = d->addImage(im, &bitmap, im.cacheKey());
  if (object < 0)
    return;

  *d->currentPage << "q\n/GSa gs\n";
  *d->currentPage
      << MyQPdf::generateMatrix(QTransform(rectangle.width() / sr.width(), 0, 0, rectangle.height() / sr.height(),
                                           rectangle.x(), rectangle.y()) *
                                (d->simplePen ? QTransform() : d->stroker.matrix));
  setBrush();
  d->currentPage->streamImage(im.width(), im.height(), object);
  *d->currentPage << "Q\n";
}

void MyQPdfEngine::drawTiledPixmap(const QRectF& rectangle, const QPixmap& pixmap, const QPointF& point) {
  Q_D(MyQPdfEngine);

  bool bitmap = (pixmap.depth() == 1);
  QBrush b = d->brush;
  QPointF bo = d->brushOrigin;
  bool hp = d->hasPen;
  d->hasPen = false;
  bool hb = d->hasBrush;
  d->hasBrush = true;

  d->brush = QBrush(pixmap);
  if (bitmap)
    // #### fix bitmap case where we have a brush pen
    d->brush.setColor(d->pen.color());

  d->brushOrigin = -point;
  *d->currentPage << "q\n";
  setBrush();

  drawRects(&rectangle, 1);
  *d->currentPage << "Q\n";

  d->hasPen = hp;
  d->hasBrush = hb;
  d->brush = b;
  d->brushOrigin = bo;
}
/*
void MyQPdfEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
  Q_D(MyQPdfEngine);

  if (!d->hasPen || (d->clipEnabled && d->allClipped))
    return;

  if (d->stroker.matrix.type() >= QTransform::TxProject) {
    QPaintEngine::drawTextItem(p, textItem);
    return;
  }

  *d->currentPage << "q\n";
  if(!d->simplePen)
    *d->currentPage << MyQPdf::generateMatrix(d->stroker.matrix);

  bool hp = d->hasPen;
  d->hasPen = false;
  QBrush b = d->brush;
  d->brush = d->pen.brush();
  setBrush();

  const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
  Q_ASSERT(ti.fontEngine->type() != QFontEngine::Multi);
  d->drawTextItem(p, ti);
  d->hasPen = hp;
  d->brush = b;
  *d->currentPage << "Q\n";
}*/

void MyQPdfEngine::drawHyperlink(const QRectF& r, const QUrl& url) {
  Q_D(MyQPdfEngine);

  const uint annot = d->addXrefEntry(-1);
  const QByteArray urlascii = url.toEncoded();
  int len = urlascii.size();
  QVarLengthArray<char> url_esc;
  url_esc.reserve(len + 1);
  for (int j = 0; j < len; j++) {
    if (urlascii[j] == '(' || urlascii[j] == ')' || urlascii[j] == '\\')
      url_esc.append('\\');
    url_esc.append(urlascii[j]);
  }
  url_esc.append('\0');

  char buf[256];
  const QRectF rr = d->pageMatrix().mapRect(r);
  d->xprintf("<<\n/Type /Annot\n/Subtype /Link\n/Rect [");
  d->xprintf("%s ", MyQPdf::qt_real_to_string(rr.left(), buf));
  d->xprintf("%s ", MyQPdf::qt_real_to_string(rr.top(), buf));
  d->xprintf("%s ", MyQPdf::qt_real_to_string(rr.right(), buf));
  d->xprintf("%s", MyQPdf::qt_real_to_string(rr.bottom(), buf));
  d->xprintf("]\n/Border [0 0 0]\n/A <<\n");
  d->xprintf("/Type /Action\n/S /URI\n/URI (%s)\n", url_esc.constData());
  d->xprintf(">>\n>>\n");
  d->xprintf("endobj\n");
  d->currentPage->annotations.append(annot);
}

void MyQPdfEngine::updateState(const QPaintEngineState& state) {
  Q_D(MyQPdfEngine);

  QPaintEngine::DirtyFlags flags = state.state();

  if (flags & DirtyTransform)
    d->stroker.matrix = state.transform();

  if (flags & DirtyPen) {
    d->pen = state.pen();
    d->hasPen = d->pen.style() != Qt::NoPen;
    // d->stroker.setPen(d->pen, state.renderHints());
    QBrush penBrush = d->pen.brush();
    bool oldSimple = d->simplePen;
    d->simplePen = (d->hasPen && (penBrush.style() == Qt::SolidPattern) && penBrush.isOpaque() && d->opacity == 1.0);
    if (oldSimple != d->simplePen)
      flags |= DirtyTransform;
  } else if (flags & DirtyHints) {
    // d->stroker.setPen(d->pen, state.renderHints());
  }
  if (flags & DirtyBrush) {
    d->brush = state.brush();
    if (d->brush.color().alpha() == 0 && d->brush.style() == Qt::SolidPattern)
      d->brush.setStyle(Qt::NoBrush);
    d->hasBrush = d->brush.style() != Qt::NoBrush;
  }
  if (flags & DirtyBrushOrigin) {
    d->brushOrigin = state.brushOrigin();
    flags |= DirtyBrush;
  }
  if (flags & DirtyOpacity) {
    d->opacity = state.opacity();
    if (d->simplePen && d->opacity != 1.0) {
      d->simplePen = false;
      flags |= DirtyTransform;
    }
  }

  bool ce = d->clipEnabled;
  if (flags & DirtyClipPath) {
    d->clipEnabled = true;
    updateClipPath(state.clipPath(), state.clipOperation());
  } else if (flags & DirtyClipRegion) {
    d->clipEnabled = true;
    QPainterPath path;
    for (const QRect& rect : state.clipRegion())
      path.addRect(rect);
    updateClipPath(path, state.clipOperation());
    flags |= DirtyClipPath;
  } else if (flags & DirtyClipEnabled) {
    d->clipEnabled = state.isClipEnabled();
  }

  if (ce != d->clipEnabled)
    flags |= DirtyClipPath;
  else if (!d->clipEnabled)
    flags &= ~DirtyClipPath;

  setupGraphicsState(flags);
}

void MyQPdfEngine::setupGraphicsState(QPaintEngine::DirtyFlags flags) {
  Q_D(MyQPdfEngine);
  if (flags & DirtyClipPath)
    flags |= DirtyTransform | DirtyPen | DirtyBrush;

  if (flags & DirtyTransform) {
    *d->currentPage << "Q\n";
    flags |= DirtyPen | DirtyBrush;
  }

  if (flags & DirtyClipPath) {
    *d->currentPage << "Q q\n";

    d->allClipped = false;
    if (d->clipEnabled && !d->clips.isEmpty()) {
      for (int i = 0; i < d->clips.size(); ++i) {
        if (d->clips.at(i).isEmpty()) {
          d->allClipped = true;
          break;
        }
      }
      if (!d->allClipped) {
        for (int i = 0; i < d->clips.size(); ++i) {
          *d->currentPage << MyQPdf::generatePath(d->clips.at(i), QTransform(), MyQPdf::ClipPath);
        }
      }
    }
  }

  if (flags & DirtyTransform) {
    *d->currentPage << "q\n";
    if (d->simplePen && !d->stroker.matrix.isIdentity())
      *d->currentPage << MyQPdf::generateMatrix(d->stroker.matrix);
  }
  if (flags & DirtyBrush)
    setBrush();
  if (d->simplePen && (flags & DirtyPen))
    setPen();
}

extern QPainterPath qt_regionToPath(const QRegion& region);

void MyQPdfEngine::updateClipPath(const QPainterPath& p, Qt::ClipOperation op) {
  Q_D(MyQPdfEngine);
  QPainterPath path = d->stroker.matrix.map(p);
  // qDebug() << "updateClipPath: " << d->stroker.matrix << p.boundingRect() << path.boundingRect() << op;

  if (op == Qt::NoClip) {
    d->clipEnabled = false;
    d->clips.clear();
  } else if (op == Qt::ReplaceClip) {
    d->clips.clear();
    d->clips.append(path);
  } else if (op == Qt::IntersectClip) {
    d->clips.append(path);
  } else {  // UniteClip
    // ask the painter for the current clipping path. that's the easiest solution
    path = painter()->clipPath();
    path = d->stroker.matrix.map(path);
    d->clips.clear();
    d->clips.append(path);
  }
}

void MyQPdfEngine::setPen() {
  Q_D(MyQPdfEngine);
  if (d->pen.style() == Qt::NoPen)
    return;
  QBrush b = d->pen.brush();
  Q_ASSERT(b.style() == Qt::SolidPattern && b.isOpaque());

  QColor rgba = b.color();
  if (d->grayscale) {
    qreal gray = qGray(rgba.rgba()) / 255.;
    *d->currentPage << gray << gray << gray;
  } else {
    *d->currentPage << rgba.redF()
                    << rgba.greenF()
                    << rgba.blueF();
  }
  *d->currentPage << "SCN\n";

  *d->currentPage << d->pen.widthF() << "w ";

  int pdfCapStyle = 0;
  switch (d->pen.capStyle()) {
    case Qt::FlatCap:
      pdfCapStyle = 0;
      break;
    case Qt::SquareCap:
      pdfCapStyle = 2;
      break;
    case Qt::RoundCap:
      pdfCapStyle = 1;
      break;
    default:
      break;
  }
  *d->currentPage << pdfCapStyle << "J ";

  int pdfJoinStyle = 0;
  switch (d->pen.joinStyle()) {
    case Qt::MiterJoin:
    case Qt::SvgMiterJoin:
      *d->currentPage << qMax(qreal(1.0), d->pen.miterLimit()) << "M ";
      pdfJoinStyle = 0;
      break;
    case Qt::BevelJoin:
      pdfJoinStyle = 2;
      break;
    case Qt::RoundJoin:
      pdfJoinStyle = 1;
      break;
    default:
      break;
  }
  *d->currentPage << pdfJoinStyle << "j ";

  *d->currentPage << MyQPdf::generateDashes(d->pen);
}

void MyQPdfEngine::setBrush() {
  Q_D(MyQPdfEngine);
  Qt::BrushStyle style = d->brush.style();
  if (style == Qt::NoBrush)
    return;

  bool specifyColor;
  int gStateObject = 0;
  int patternObject = d->addBrushPattern(d->stroker.matrix, &specifyColor, &gStateObject);
  if (!patternObject && !specifyColor)
    return;

  *d->currentPage << (patternObject ? "/PCSp cs " : "/CSp cs ");
  if (specifyColor) {
    QColor rgba = d->brush.color();
    if (d->grayscale) {
      qreal gray = qGray(rgba.rgba()) / 255.;
      *d->currentPage << gray << gray << gray;
    } else {
      *d->currentPage << rgba.redF()
                      << rgba.greenF()
                      << rgba.blueF();
    }
  }
  if (patternObject)
    *d->currentPage << "/Pat" << patternObject;
  *d->currentPage << "scn\n";

  if (gStateObject)
    *d->currentPage << "/GState" << gStateObject << "gs\n";
  else
    *d->currentPage << "/GSa gs\n";
}

bool MyQPdfEngine::newPage() {
  Q_D(MyQPdfEngine);
  if (!isActive())
    return false;
  d->newPage();

  setupGraphicsState(DirtyBrush | DirtyPen | DirtyClipPath);
  QFile* outfile = qobject_cast<QFile*>(d->outDevice);
  if (outfile && outfile->error() != QFile::NoError)
    return false;
  return true;
}

QPaintEngine::Type MyQPdfEngine::type() const {
  return QPaintEngine::Pdf;
}

void MyQPdfEngine::setResolution(int resolution) {
  Q_D(MyQPdfEngine);
  d->resolution = resolution;
}

int MyQPdfEngine::resolution() const {
  Q_D(const MyQPdfEngine);
  return d->resolution;
}

void MyQPdfEngine::setPageLayout(const QPageLayout& pageLayout) {
  Q_D(MyQPdfEngine);
  d->m_pageLayout = pageLayout;
}

void MyQPdfEngine::setPageSize(const QPageSize& pageSize) {
  Q_D(MyQPdfEngine);
  d->m_pageLayout.setPageSize(pageSize);
}

void MyQPdfEngine::setPageOrientation(QPageLayout::Orientation orientation) {
  Q_D(MyQPdfEngine);
  d->m_pageLayout.setOrientation(orientation);
}

void MyQPdfEngine::setPageMargins(const QMarginsF& margins, QPageLayout::Unit units) {
  Q_D(MyQPdfEngine);
  d->m_pageLayout.setUnits(units);
  d->m_pageLayout.setMargins(margins);
}

QPageLayout MyQPdfEngine::pageLayout() const {
  Q_D(const MyQPdfEngine);
  return d->m_pageLayout;
}
void MyQPdfEngine::beginFormXObject() {
  Q_D(MyQPdfEngine);

  d->beginFormXObject();
}
int MyQPdfEngine::endFormXObject(QRectF bbox) {
  Q_D(MyQPdfEngine);

  return d->endFormXObject(bbox);
}
void MyQPdfEngine::writeRawPDFtoCurrentStream(QString rawPDF) {
  Q_D(MyQPdfEngine);

  d->writeRawPDFtoCurrentStream(rawPDF);
}
void MyQPdfEngine::addImagetoResources(int objectID) {
  Q_D(MyQPdfEngine);

  d->addImagetoResources(objectID);
}

// Metrics are in Device Pixels
int MyQPdfEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const {
  Q_D(const MyQPdfEngine);
  int val;
  switch (metricType) {
    case QPaintDevice::PdmWidth:
      val = d->m_pageLayout.paintRectPixels(d->resolution).width();
      break;
    case QPaintDevice::PdmHeight:
      val = d->m_pageLayout.paintRectPixels(d->resolution).height();
      break;
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmDpiY:
      val = d->resolution;
      break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
      val = 1200;
      break;
    case QPaintDevice::PdmWidthMM:
      val = qRound(d->m_pageLayout.paintRect(QPageLayout::Millimeter).width());
      break;
    case QPaintDevice::PdmHeightMM:
      val = qRound(d->m_pageLayout.paintRect(QPageLayout::Millimeter).height());
      break;
    case QPaintDevice::PdmNumColors:
      val = INT_MAX;
      break;
    case QPaintDevice::PdmDepth:
      val = 32;
      break;
    case QPaintDevice::PdmDevicePixelRatio:
      val = 1;
      break;
    case QPaintDevice::PdmDevicePixelRatioScaled:
      val = 1 * QPaintDevice::devicePixelRatioFScale();
      break;
    default:
      qWarning("QPdfWriter::metric: Invalid metric command");
      return 0;
  }
  return val;
}

std::unordered_map<int, int> loadSurahNumberToUnicode(const QString& jsonPath) {
  std::unordered_map<int, int> surahNumberToUnicode;

  QFile file(jsonPath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open file:" << jsonPath;
    return surahNumberToUnicode;
  }

  const QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    qWarning() << "JSON parse error:" << parseError.errorString();
    return surahNumberToUnicode;
  }

  if (!doc.isObject()) {
    qWarning() << "Root JSON is not an object";
    return surahNumberToUnicode;
  }

  const QJsonObject obj = doc.object();

  for (auto it = obj.begin(); it != obj.end(); ++it) {
    bool ok = false;
    const int surahNumber = it.key().toInt(&ok);
    if (!ok) {
      qWarning() << "Invalid surah key:" << it.key();
      continue;
    }

    if (!it.value().isDouble()) {
      qWarning() << "Value is not numeric for key:" << it.key();
      continue;
    }

    const int unicodeValue = it.value().toInt();
    surahNumberToUnicode[surahNumber] = unicodeValue;
  }

  return surahNumberToUnicode;
}

MyQPdfEnginePrivate::MyQPdfEnginePrivate(OtLayout* otlayout)
    : clipEnabled(false), allClipped(false), hasPen(true), hasBrush(false), simplePen(false), outDevice(0), ownsDevice(false), embedFonts(true), grayscale(false), m_pageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(10, 10, 10, 10)) {
  resolution = 1200;
  currentObject = 1;
  currentPage = 0;
  currentObjectStream = new ObjectStream();
  stroker.stream = 0;

  streampos = 0;

  stream = new QDataStream;

  this->otlayout = otlayout;

  this->surahNumberToUnicode = loadSurahNumberToUnicode(":/fonts/surahCodes.json");
}

bool MyQPdfEngine::begin(QPaintDevice* pdev) {
  Q_D(MyQPdfEngine);
  d->pdev = pdev;

  if (!d->outDevice) {
    if (!d->outputFileName.isEmpty()) {
      QFile* file = new QFile(d->outputFileName);
      if (!file->open(QFile::WriteOnly | QFile::Truncate)) {
        delete file;
        return false;
      }
      d->outDevice = file;
    } else {
      return false;
    }
    d->ownsDevice = true;
  }

  d->currentObject = 1;

  d->currentPage = new MyQPdfPage;
  d->stroker.stream = d->currentPage;
  d->opacity = 1.0;

  d->stream->setDevice(d->outDevice);

  d->streampos = 0;
  d->hasPen = true;
  d->hasBrush = false;
  d->clipEnabled = false;
  d->allClipped = false;

  d->xrefPositions.clear();
  d->objectStreamPositions.clear();
  d->pageRoot = 0;
  d->catalog = 0;
  d->info = 0;
  d->graphicsState = 0;
  d->patternColorSpace = 0;
  d->simplePen = false;

  d->pages.clear();
  d->imageCache.clear();
  d->alphaCache.clear();

  setActive(true);
  d->writeHeader();

  // newPage();

  return true;
}

bool MyQPdfEngine::end() {
  Q_D(MyQPdfEngine);
  d->writeTail();

  d->stream->unsetDevice();

  // qDeleteAll(d->fonts);
  // d->fonts.clear();
  delete d->currentPage;
  d->currentPage = 0;

  if (d->outDevice && d->ownsDevice) {
    d->outDevice->close();
    delete d->outDevice;
    d->outDevice = 0;
  }

  if (d->currentObjectStream != nullptr)
    delete d->currentObjectStream;

  setActive(false);
  return true;
}

MyQPdfEnginePrivate::~MyQPdfEnginePrivate() {
  // qDeleteAll(fonts);
  delete currentPage;
  delete stream;
}

void MyQPdfEnginePrivate::writeHeader() {
  addXrefEntry(0, false);

  xprintf("%%PDF-1.7\n");
  xprintf("%%%c%c%c%c\n", 'A' + 128, 'M' + 128, 'I' + 128, 'N' + 128);

  catalog = requestObject();
  pageRoot = requestObject();
  outlinesid = requestObject();
  structTreeRootobjectId = requestObject();

  writeInfo();

  QByteArray ba;

  xprintf(ba,
          "<<\n"
          "/Type /Catalog\n"
          "/Lang (Arabic)\n"
          "/Pages %d 0 R\n"
          "/Outlines %d 0 R\n"
          //"/StructTreeRoot %d 0 R\n"
          "/MarkInfo << /Marked true >>\n"
          "/ViewerPreferences <<\n"
          "/Direction /R2L\n"
          ">>\n"
          ">>\n",
          pageRoot, outlinesid, structTreeRootobjectId);

  addObject(ba, catalog);

  ba.clear();

  // graphics state
  graphicsState = requestObject();
  xprintf(ba,
          "<<\n"
          "/Type /ExtGState\n"
          "/SA true\n"
          "/SM 0.02\n"
          "/ca 1.0\n"
          "/CA 1.0\n"
          "/AIS false\n"
          "/SMask /None"
          ">>\n");
  addObject(ba, graphicsState);
  ba.clear();

  // color space for pattern
  patternColorSpace = requestObject();
  xprintf(ba, "[/Pattern /DeviceRGB]\n");
  addObject(ba, patternColorSpace);
}

void MyQPdfEnginePrivate::writeInfo() {
  info = requestObject();
  QByteArray ba;
  xprintf(ba, "<<\n/Title ");
  printString(ba, "The Noble Quran - القرآن الكريم");
  xprintf(ba, "\n/Author ");
  printString(ba, "Amine Anane");
  xprintf(ba, "\n/Creator ");
  printString(ba, "VisualMetaFont 0.1");
  xprintf(ba, "\n/Producer ");
  printString(ba, QString::fromLatin1("VisualMetaFont 0.1"));
  QDateTime now = QDateTime::currentDateTime();
  QTime t = now.time();
  QDate d = now.date();
  xprintf(ba, "\n/CreationDate (D:%d%02d%02d%02d%02d%02d",
          d.year(),
          d.month(),
          d.day(),
          t.hour(),
          t.minute(),
          t.second());
  int offset = now.offsetFromUtc();
  int hours = (offset / 60) / 60;
  int mins = (offset / 60) % 60;
  if (offset < 0)
    xprintf(ba, "-%02d'%02d')\n", -hours, -mins);
  else if (offset > 0)
    xprintf(ba, "+%02d'%02d')\n", hours, mins);
  else
    xprintf("Z)\n");
  xprintf(ba, ">>\n");

  addObject(ba, info);
}
void MyQPdfEnginePrivate::writeStructure() {
  QByteArray ba;

  // StructTreeRoot
  int parentTreeobjectId = requestObject();
  int SErootId = requestObject();

  ba.clear();
  xprintf(ba,
          "<< /Type /StructTreeRoot\n"
          "/K [%d 0 R]\n"
          "/ParentTree %d 0 R\n"
          "/ParentTreeNextKey 1\n"
          ">>\n",
          SErootId, parentTreeobjectId);
  addObject(ba, structTreeRootobjectId);

  // root StructElem
  ba.clear();
  xprintf(ba,
          "<< /Type /StructElem\n"
          "/S /P\n"
          "/P %d 0 R\n"
          "/Pg %d 0 R\n"
          //"/ActualText <FEFF0623064E>\n"
          "/K 0\n"
          ">>\n",
          structTreeRootobjectId, pages.at(0));
  addObject(ba, SErootId);

  // ParentTree
  ba.clear();
  int firstpagestructparents = requestObject();
  xprintf(ba,
          "<<\n"
          "/Nums [\n"
          "0 %d 0 R\n"
          "] >>\n",
          firstpagestructparents);
  addObject(ba, parentTreeobjectId);

  // first page ParentTree
  ba.clear();
  xprintf(ba,
          "[\n"
          "%d 0 R\n"
          "]\n",
          SErootId);
  addObject(ba, firstpagestructparents);
}
void MyQPdfEnginePrivate::writeOutlines() {
  QByteArray data;
  MyQPdf::ByteStream s(&data);
  s << "<<\n";
  s << "/Type /Outlines\n";
  if (outlines.size() > 0) {
    s << "/First " << (int)outlines.keys().constFirst() << "0 R\n";
    s << "/Last " << (int)outlines.keys().constLast() << "0 R\n";
    s << "/Count " << outlines.keys().size() << "\n";
  }
  s << ">>\n";

  addObject(data, outlinesid);

  QMapIterator<uint, OutlineEntry> i(outlines);

  while (i.hasNext()) {
    s.clear();
    bool hasprevious = i.hasPrevious();
    int previousKey = 0;
    if (hasprevious) {
      previousKey = i.key();
    }
    i.next();

    auto dest = i.value().dest;

    QByteArray title;

    printString(title, i.value().title);

    s << "<<\n";
    s << "/Title " << title << "\n";
    s << "/Parent " << outlinesid << "0 R\n";
    if (hasprevious) {
      s << "/Prev " << previousKey << "0 R\n";
    }
    if (i.hasNext()) {
      s << "/Next " << (int)i.peekNext().key() << "0 R\n";
    }

    s << "/Dest [" << (int)i.value().dest.pageID << "0 R /XYZ " << (int)dest.left << (int)dest.top << "0]\n";

    s << ">>\n";

    addObject(data, i.key());
  }
}

void MyQPdfEnginePrivate::writePageRoot() {
  // addXrefEntry(pageRoot);
  QByteArray pageRootBA;

  xprintf(pageRootBA,
          "<<\n"
          "/Type /Pages\n"
          "/Kids \n"
          "[\n");
  int size = pages.size();
  for (int i = 0; i < size; ++i)
    xprintf(pageRootBA, "%d 0 R\n", pages[i]);
  xprintf(pageRootBA, "]\n");

  // xprintf("/Group <</S /Transparency /I true /K false>>\n");
  xprintf(pageRootBA, "/Count %d\n", pages.size());

  xprintf(pageRootBA,
          "/ProcSet [/PDF /Text /ImageB /ImageC]\n"
          ">>\n");
  //"endobj\n");

  addObject(pageRootBA, pageRoot);
}
void MyQPdfEnginePrivate::addObject(QByteArray data, uint objectNumber) {
  currentObjectStream->addObject(data, objectNumber);
}
void MyQPdfEnginePrivate::writeObjectStream() {
  if (currentObjectStream == nullptr || currentObjectStream->objectNumbers.size() == 0)
    return;

  uint objectStreamObjectNum = requestObject();

  QByteArray objectStreamContent;

  for (int i = 0; i < currentObjectStream->objectNumbers.size(); ++i) {
    xprintf(objectStreamContent, "%d %d ", currentObjectStream->objectNumbers[i], currentObjectStream->byteOffsets[i]);
    addXrefEntryForObjectStream(currentObjectStream->objectNumbers[i], objectStreamObjectNum, i);
  }

  int first = objectStreamContent.length();
  objectStreamContent.append(currentObjectStream->getBA());

  objectStreamContent = getCompressed(objectStreamContent);

  addXrefEntry(objectStreamObjectNum);
  xprintf(
      "<<\n"
      "/Type /ObjStm\n"
      "/Length %d\n"
      "/N %d\n"
      "/First %d\n",
      objectStreamContent.length(), currentObjectStream->objectNumbers.length(), first);
  if (do_compress)
    xprintf("/Filter /FlateDecode\n");
  xprintf(
      ">>\n"
      "stream\n");

  // QIODevice *content = currentPage->stream();
  // int len = writeCompressed(content);
  write(objectStreamContent);
  xprintf(
      "\nendstream\n"
      "endobj\n");
}
void MyQPdfEnginePrivate::writePage() {
  if (pages.empty())
    return;

  *currentPage << "Q Q\n";

  uint pageStream = requestObject();
  uint resources = requestObject();
  // uint annots = requestObject();

  QByteArray pageDict;
  QByteArray pageResources;

  xprintf(pageDict,
          "<<\n"
          "/Type /Page\n"
          "/Parent %d 0 R\n"
          "/Contents %d 0 R\n"
          "/Resources %d 0 R\n"
          //"/Annots %d 0 R\n"
          "/MediaBox [0 0 %d %d]\n"
          "/StructParents %d\n"
          ">>\n",
          //	"endobj\n",
          pageRoot, pageStream, resources,  // annots,
          // make sure we use the pagesize from when we started the page, since the user may have changed it
          currentPage->pageSize.width(), currentPage->pageSize.height(), pages.size() - 1);

  xprintf(pageResources,
          "<<\n"
          "/ColorSpace <<\n"
          "/PCSp %d 0 R\n"
          "/CSp /DeviceRGB\n"
          "/CSpg /DeviceGray\n"
          ">>\n"
          "/ExtGState <<\n"
          "/GSa %d 0 R\n",
          patternColorSpace, graphicsState);

  for (int i = 0; i < currentPage->graphicStates.size(); ++i)
    xprintf(pageResources, "/GState%d %d 0 R\n", currentPage->graphicStates.at(i), currentPage->graphicStates.at(i));
  xprintf(pageResources, ">>\n");

  if (currentPage->patterns.size() > 0) {
    xprintf(pageResources, "/Pattern <<\n");
    for (int i = 0; i < currentPage->patterns.size(); ++i)
      xprintf(pageResources, "/Pat%d %d 0 R\n", currentPage->patterns.at(i), currentPage->patterns.at(i));
    xprintf(pageResources, ">>\n");
  }

  if (currentPage->fonts.size() > 0) {
    xprintf(pageResources, "/Font <<\n");
    for (int i = 0; i < currentPage->fonts.size(); ++i)
      xprintf(pageResources, "/F%d %d 0 R\n", currentPage->fonts[i], currentPage->fonts[i]);
    xprintf(pageResources, ">>\n");
  }

  if (currentPage->images.size() > 0) {
    xprintf(pageResources, "/XObject <<\n");
    for (int i = 0; i < currentPage->images.size(); ++i) {
      xprintf(pageResources, "/Im%d %d 0 R\n", currentPage->images.at(i), currentPage->images.at(i));
    }
    xprintf(pageResources, ">>\n");
  }

  xprintf(pageResources, ">>\n");

  addObject(pageDict, pages.constLast());
  addObject(pageResources, resources);

  /*
  QByteArray objectStreamContent;

  xprintf(objectStreamContent, "%d %d %d %d\n", pages.constLast(), 0, resources, pageDict.length());

  int first = objectStreamContent.length();
  objectStreamContent.append(pageDict);
  objectStreamContent.append(pageResources);
  objectStreamContent = getCompressed(objectStreamContent);


  uint objectStreamObjectNum = requestObject();

  addXrefEntry(objectStreamObjectNum);
  xprintf("<<\n"
    "/Type /ObjStm\n"
    "/Length %d\n"
    "/N 2\n"
    "/First %d\n", objectStreamContent.length(), first);
  if (do_compress)
    xprintf("/Filter /FlateDecode\n");
  xprintf(">>\n"
    "stream\n");

  //QIODevice *content = currentPage->stream();
  //int len = writeCompressed(content);
  write(objectStreamContent);
  xprintf("\nendstream\n"
    "endobj\n");

  addXrefEntryForObjectStream(pages.constLast(), objectStreamObjectNum, 0);
  addXrefEntryForObjectStream(resources, objectStreamObjectNum, 1);*/

  auto ba = getCompressed(currentPage->getBA());

  addXrefEntry(pageStream);
  xprintf(
      "<<\n"
      "/Length %d\n",
      ba.length());  // object number for stream length object
  if (do_compress)
    xprintf("/Filter /FlateDecode\n");

  xprintf(">>\n");
  xprintf("stream\n");
  // QIODevice *content = currentPage->stream();
  // int len = writeCompressed(content);
  write(ba);
  xprintf(
      "endstream\n"
      "endobj\n");
}
/*
void MyQPdfEnginePrivate::writeTail()
{
  writePage();
  //writeFonts();
  writeType3Fonts();
  writePageRoot();
  addXrefEntry(xrefPositions.size(), false);
  xprintf("xref\n"
    "0 %d\n"
    "%010d 65535 f \n", xrefPositions.size() - 1, xrefPositions[0]);

  for (int i = 1; i < xrefPositions.size() - 1; ++i)
    xprintf("%010d 00000 n \n", xrefPositions[i]);

  xprintf("trailer\n"
    "<<\n"
    "/Size %d\n"
    "/Info %d 0 R\n"
    "/Root %d 0 R\n"
    ">>\n"
    "startxref\n%d\n"
    "%%%%EOF\n",
    xrefPositions.size() - 1, info, catalog, xrefPositions.constLast());
}*/

void MyQPdfEnginePrivate::writeTail() {
  writePage();
  // writeFonts();
  writeType3Fonts();

  writePageRoot();
  writeOutlines();
  if (pages.size() > 0) {
    writeStructure();
  }

  writeObjectStream();

  addXrefEntry(xrefPositions.size(), true);
  /*
  xprintf("xref\n"
    "0 %d\n"
    "%010d 65535 f \n", xrefPositions.size() - 1, xrefPositions[0]);

  for (int i = 1; i < xrefPositions.size() - 1; ++i)
    xprintf("%010d 00000 n \n", xrefPositions[i]);

  xprintf("trailer\n"
    "<<\n"
    "/Size %d\n"
    "/Info %d 0 R\n"
    "/Root %d 0 R\n"
    ">>\n"
    "startxref\n%d\n"
    "%%%%EOF\n",
    xrefPositions.size() - 1, info, catalog, xrefPositions.constLast());

    */

  QByteArray content;
  content << quint8(0) << quint32(0) << quint16(0xFFFF);
  for (int i = 1; i < xrefPositions.size(); ++i) {
    if (xrefPositions[i] < 0) {
      content << quint8(2) << quint32(-xrefPositions[i]) << quint16(objectStreamPositions[i]);
    } else {
      content << quint8(1) << quint32(xrefPositions[i]) << quint16(0);
    }
  }

  content = getCompressed(content);

  xprintf(
      "<<\n"
      "/Type /XRef\n"
      "/Size %d\n"
      "/Info %d 0 R\n"
      "/Root %d 0 R\n"
      "/W [1 4 2]\n"
      "/Index [0 %d]\n"
      "/Length %d\n",
      xrefPositions.size(), info, catalog, xrefPositions.size(), content.length());
  if (do_compress)
    xprintf("/Filter /FlateDecode\n");

  xprintf(
      ">>\n"
      "stream\n");
  write(content);
  xprintf(
      "endstream\n"
      "endobj\n"
      "startxref\n%d\n"
      "%%%%EOF\n",
      xrefPositions.constLast());
}

int MyQPdfEnginePrivate::addXrefEntry(int object, bool printostr) {
  if (object < 0)
    object = requestObject();

  if (object >= xrefPositions.size())
    xrefPositions.resize(object + 1);

  xrefPositions[object] = streampos;
  if (printostr)
    xprintf("%d 0 obj\n", object);

  return object;
}
int MyQPdfEnginePrivate::addXrefEntryForObjectStream(int object, int parentStream, int index) {
  if (object < 0)
    object = requestObject();

  if (object >= xrefPositions.size())
    xrefPositions.resize(object + 1);

  xrefPositions[object] = -parentStream;
  objectStreamPositions[object] = index;

  return object;
}

void MyQPdfEnginePrivate::printString(const QString& string) {
  // The 'text string' type in PDF is encoded either as PDFDocEncoding, or
  // Unicode UTF-16 with a Unicode byte order mark as the first character
  // (0xfeff), with the high-order byte first.
  QByteArray array;

  printString(array, string);

  write(array);
}

void MyQPdfEnginePrivate::printString(QByteArray& byteArray, const QString& string) {
  // The 'text string' type in PDF is encoded either as PDFDocEncoding, or
  // Unicode UTF-16 with a Unicode byte order mark as the first character
  // (0xfeff), with the high-order byte first.
  byteArray.append("(\xfe\xff");
  const ushort* utf16 = string.utf16();

  for (int i = 0; i < string.size(); ++i) {
    char part[2] = {char((*(utf16 + i)) >> 8), char((*(utf16 + i)) & 0xff)};
    for (int j = 0; j < 2; ++j) {
      if (part[j] == '(' || part[j] == ')' || part[j] == '\\')
        byteArray.append('\\');
      byteArray.append(part[j]);
    }
  }
  byteArray.append(')');
}

// For strings up to 10000 bytes only !
void MyQPdfEnginePrivate::xprintf(const char* fmt, ...) {
  if (!stream)
    return;

  const int msize = 10000;
  char buf[msize];

  va_list args;
  va_start(args, fmt);
  int bufsize = qvsnprintf(buf, msize, fmt, args);

  Q_ASSERT(bufsize < msize);

  va_end(args);

  stream->writeRawData(buf, bufsize);
  streampos += bufsize;
}

void MyQPdfEnginePrivate::xprintf(QByteArray& byteArray, const char* fmt, ...) {
  const int msize = 10000;
  char buf[msize];

  va_list args;
  va_start(args, fmt);
  int bufsize = qvsnprintf(buf, msize, fmt, args);

  Q_ASSERT(bufsize < msize);

  va_end(args);

  byteArray.append(buf, bufsize);
}

int MyQPdfEnginePrivate::writeImage(const QByteArray& data, int width, int height, int depth,
                                    int maskObject, int softMaskObject, bool dct, bool isMono) {
  int image = addXrefEntry(-1);
  xprintf(
      "<<\n"
      "/Type /XObject\n"
      "/Subtype /Image\n"
      "/Width %d\n"
      "/Height %d\n",
      width, height);

  if (depth == 1) {
    if (!isMono) {
      xprintf(
          "/ImageMask true\n"
          "/Decode [1 0]\n");
    } else {
      xprintf(
          "/BitsPerComponent 1\n"
          "/ColorSpace /DeviceGray\n");
    }
  } else {
    xprintf(
        "/BitsPerComponent 8\n"
        "/ColorSpace %s\n",
        (depth == 32) ? "/DeviceRGB" : "/DeviceGray");
  }
  if (maskObject > 0)
    xprintf("/Mask %d 0 R\n", maskObject);
  if (softMaskObject > 0)
    xprintf("/SMask %d 0 R\n", softMaskObject);

  int lenobj = requestObject();
  xprintf("/Length %d 0 R\n", lenobj);
  if (interpolateImages)
    xprintf("/Interpolate true\n");
  int len = 0;
  if (dct) {
    // qDebug("DCT");
    xprintf("/Filter /DCTDecode\n>>\nstream\n");
    write(data);
    len = data.length();
  } else {
    if (do_compress)
      xprintf("/Filter /FlateDecode\n>>\nstream\n");
    else
      xprintf(">>\nstream\n");
    len = writeCompressed(data);
  }
  xprintf(
      "endstream\n"
      "endobj\n");
  addXrefEntry(lenobj);
  xprintf(
      "%d\n"
      "endobj\n",
      len);
  return image;
}

struct QGradientBound {
  qreal start;
  qreal stop;
  int function;
  bool reverse;
};
Q_DECLARE_TYPEINFO(QGradientBound, Q_PRIMITIVE_TYPE);

int MyQPdfEnginePrivate::createShadingFunction(const QGradient* gradient, int from, int to, bool reflect, bool alpha) {
  QGradientStops stops = gradient->stops();
  if (stops.isEmpty()) {
    stops << QGradientStop(0, Qt::black);
    stops << QGradientStop(1, Qt::white);
  }
  if (stops.at(0).first > 0)
    stops.prepend(QGradientStop(0, stops.at(0).second));
  if (stops.at(stops.size() - 1).first < 1)
    stops.append(QGradientStop(1, stops.at(stops.size() - 1).second));

  QVector<int> functions;
  const int numStops = stops.size();
  functions.reserve(numStops - 1);
  for (int i = 0; i < numStops - 1; ++i) {
    int f = addXrefEntry(-1);
    QByteArray data;
    MyQPdf::ByteStream s(&data);
    s << "<<\n"
         "/FunctionType 2\n"
         "/Domain [0 1]\n"
         "/N 1\n";
    if (alpha) {
      s << "/C0 [" << stops.at(i).second.alphaF() << "]\n"
                                                     "/C1 ["
        << stops.at(i + 1).second.alphaF() << "]\n";
    } else {
      s << "/C0 [" << stops.at(i).second.redF() << stops.at(i).second.greenF() << stops.at(i).second.blueF() << "]\n"
                                                                                                                "/C1 ["
        << stops.at(i + 1).second.redF() << stops.at(i + 1).second.greenF() << stops.at(i + 1).second.blueF() << "]\n";
    }
    s << ">>\n"
         "endobj\n";
    write(data);
    functions << f;
  }

  QVector<QGradientBound> gradientBounds;
  gradientBounds.reserve((to - from) * (numStops - 1));

  for (int step = from; step < to; ++step) {
    if (reflect && step % 2) {
      for (int i = numStops - 1; i > 0; --i) {
        QGradientBound b;
        b.start = step + 1 - qBound(qreal(0.), stops.at(i).first, qreal(1.));
        b.stop = step + 1 - qBound(qreal(0.), stops.at(i - 1).first, qreal(1.));
        b.function = functions.at(i - 1);
        b.reverse = true;
        gradientBounds << b;
      }
    } else {
      for (int i = 0; i < numStops - 1; ++i) {
        QGradientBound b;
        b.start = step + qBound(qreal(0.), stops.at(i).first, qreal(1.));
        b.stop = step + qBound(qreal(0.), stops.at(i + 1).first, qreal(1.));
        b.function = functions.at(i);
        b.reverse = false;
        gradientBounds << b;
      }
    }
  }

  // normalize bounds to [0..1]
  qreal bstart = gradientBounds.at(0).start;
  qreal bend = gradientBounds.at(gradientBounds.size() - 1).stop;
  qreal norm = 1. / (bend - bstart);
  for (int i = 0; i < gradientBounds.size(); ++i) {
    gradientBounds[i].start = (gradientBounds[i].start - bstart) * norm;
    gradientBounds[i].stop = (gradientBounds[i].stop - bstart) * norm;
  }

  int function;
  if (gradientBounds.size() > 1) {
    function = addXrefEntry(-1);
    QByteArray data;
    MyQPdf::ByteStream s(&data);
    s << "<<\n"
         "/FunctionType 3\n"
         "/Domain [0 1]\n"
         "/Bounds [";
    for (int i = 1; i < gradientBounds.size(); ++i)
      s << gradientBounds.at(i).start;
    s << "]\n"
         "/Encode [";
    for (int i = 0; i < gradientBounds.size(); ++i)
      s << (gradientBounds.at(i).reverse ? "1 0 " : "0 1 ");
    s << "]\n"
         "/Functions [";
    for (int i = 0; i < gradientBounds.size(); ++i)
      s << gradientBounds.at(i).function << "0 R ";
    s << "]\n"
         ">>\n"
         "endobj\n";
    write(data);
  } else {
    function = functions.at(0);
  }
  return function;
}

int MyQPdfEnginePrivate::generateLinearGradientShader(const QLinearGradient* gradient, const QTransform& matrix, bool alpha) {
  QPointF start = gradient->start();
  QPointF stop = gradient->finalStop();
  QPointF offset = stop - start;
  Q_ASSERT(gradient->coordinateMode() == QGradient::LogicalMode);

  int from = 0;
  int to = 1;
  bool reflect = false;
  switch (gradient->spread()) {
    case QGradient::PadSpread:
      break;
    case QGradient::ReflectSpread:
      reflect = true;
      Q_FALLTHROUGH();
    case QGradient::RepeatSpread: {
      // calculate required bounds
      QRectF pageRect = m_pageLayout.fullRectPixels(resolution);
      QTransform inv = matrix.inverted();
      QPointF page_rect[4] = {inv.map(pageRect.topLeft()),
                              inv.map(pageRect.topRight()),
                              inv.map(pageRect.bottomLeft()),
                              inv.map(pageRect.bottomRight())};

      qreal length = offset.x() * offset.x() + offset.y() * offset.y();

      // find the max and min values in offset and orth direction that are needed to cover
      // the whole page
      from = INT_MAX;
      to = INT_MIN;
      for (int i = 0; i < 4; ++i) {
        qreal off = ((page_rect[i].x() - start.x()) * offset.x() + (page_rect[i].y() - start.y()) * offset.y()) / length;
        from = qMin(from, qFloor(off));
        to = qMax(to, qCeil(off));
      }

      stop = start + to * offset;
      start = start + from * offset;
      break;
    }
  }

  int function = createShadingFunction(gradient, from, to, reflect, alpha);

  QByteArray shader;
  MyQPdf::ByteStream s(&shader);
  s << "<<\n"
       "/ShadingType 2\n"
       "/ColorSpace "
    << (alpha ? "/DeviceGray\n" : "/DeviceRGB\n") << "/AntiAlias true\n"
                                                     "/Coords ["
    << start.x() << start.y() << stop.x() << stop.y() << "]\n"
                                                         "/Extend [true true]\n"
                                                         "/Function "
    << function << "0 R\n"
                   ">>\n"
                   "endobj\n";
  int shaderObject = addXrefEntry(-1);
  write(shader);

  return shaderObject;
}

int MyQPdfEnginePrivate::generateRadialGradientShader(const QRadialGradient* gradient, const QTransform& matrix, bool alpha) {
  QPointF p1 = gradient->center();
  qreal r1 = gradient->centerRadius();
  QPointF p0 = gradient->focalPoint();
  qreal r0 = gradient->focalRadius();

  Q_ASSERT(gradient->coordinateMode() == QGradient::LogicalMode);

  int from = 0;
  int to = 1;
  bool reflect = false;
  switch (gradient->spread()) {
    case QGradient::PadSpread:
      break;
    case QGradient::ReflectSpread:
      reflect = true;
      Q_FALLTHROUGH();
    case QGradient::RepeatSpread: {
      Q_ASSERT(qFuzzyIsNull(r0));  // QPainter emulates if this is not 0

      QRectF pageRect = m_pageLayout.fullRectPixels(resolution);
      QTransform inv = matrix.inverted();
      QPointF page_rect[4] = {inv.map(pageRect.topLeft()),
                              inv.map(pageRect.topRight()),
                              inv.map(pageRect.bottomLeft()),
                              inv.map(pageRect.bottomRight())};

      // increase to until the whole page fits into it
      bool done = false;
      while (!done) {
        QPointF center = QPointF(p0.x() + to * (p1.x() - p0.x()), p0.y() + to * (p1.y() - p0.y()));
        double radius = r0 + to * (r1 - r0);
        double r2 = radius * radius;
        done = true;
        for (int i = 0; i < 4; ++i) {
          QPointF off = page_rect[i] - center;
          if (off.x() * off.x() + off.y() * off.y() > r2) {
            ++to;
            done = false;
            break;
          }
        }
      }
      p1 = QPointF(p0.x() + to * (p1.x() - p0.x()), p0.y() + to * (p1.y() - p0.y()));
      r1 = r0 + to * (r1 - r0);
      break;
    }
  }

  int function = createShadingFunction(gradient, from, to, reflect, alpha);

  QByteArray shader;
  MyQPdf::ByteStream s(&shader);
  s << "<<\n"
       "/ShadingType 3\n"
       "/ColorSpace "
    << (alpha ? "/DeviceGray\n" : "/DeviceRGB\n") << "/AntiAlias true\n"
                                                     "/Domain [0 1]\n"
                                                     "/Coords ["
    << p0.x() << p0.y() << r0 << p1.x() << p1.y() << r1 << "]\n"
                                                           "/Extend [true true]\n"
                                                           "/Function "
    << function << "0 R\n"
                   ">>\n"
                   "endobj\n";
  int shaderObject = addXrefEntry(-1);
  write(shader);
  return shaderObject;
}

int MyQPdfEnginePrivate::generateGradientShader(const QGradient* gradient, const QTransform& matrix, bool alpha) {
  switch (gradient->type()) {
    case QGradient::LinearGradient:
      return generateLinearGradientShader(static_cast<const QLinearGradient*>(gradient), matrix, alpha);
    case QGradient::RadialGradient:
      return generateRadialGradientShader(static_cast<const QRadialGradient*>(gradient), matrix, alpha);
    case QGradient::ConicalGradient:
      Q_UNIMPLEMENTED();  // ### Implement me!
      break;
    case QGradient::NoGradient:
      break;
  }
  return 0;
}

int MyQPdfEnginePrivate::gradientBrush(const QBrush& b, const QTransform& matrix, int* gStateObject) {
  const QGradient* gradient = b.gradient();

  if (!gradient || gradient->coordinateMode() != QGradient::LogicalMode)
    return 0;

  QRectF pageRect = m_pageLayout.fullRectPixels(resolution);

  QTransform m = b.transform() * matrix;
  int shaderObject = generateGradientShader(gradient, m);

  QByteArray str;
  MyQPdf::ByteStream s(&str);
  s << "<<\n"
       "/Type /Pattern\n"
       "/PatternType 2\n"
       "/Shading "
    << shaderObject << "0 R\n"
                       "/Matrix ["
    << m.m11()
    << m.m12()
    << m.m21()
    << m.m22()
    << m.dx()
    << m.dy() << "]\n";
  s << ">>\n"
       "endobj\n";

  int patternObj = addXrefEntry(-1);
  write(str);
  currentPage->patterns.append(patternObj);

  if (!b.isOpaque()) {
    bool ca = true;
    QGradientStops stops = gradient->stops();
    int a = stops.at(0).second.alpha();
    for (int i = 1; i < stops.size(); ++i) {
      if (stops.at(i).second.alpha() != a) {
        ca = false;
        break;
      }
    }
    if (ca) {
      *gStateObject = addConstantAlphaObject(stops.at(0).second.alpha());
    } else {
      int alphaShaderObject = generateGradientShader(gradient, m, true);

      QByteArray content;
      MyQPdf::ByteStream c(&content);
      c << "/Shader" << alphaShaderObject << "sh\n";

      QByteArray form;
      MyQPdf::ByteStream f(&form);
      f << "<<\n"
           "/Type /XObject\n"
           "/Subtype /Form\n"
           "/BBox [0 0 "
        << pageRect.width() << pageRect.height() << "]\n"
                                                    "/Group <</S /Transparency >>\n"
                                                    "/Resources <<\n"
                                                    "/Shading << /Shader"
        << alphaShaderObject << alphaShaderObject << "0 R >>\n"
                                                     ">>\n";

      f << "/Length " << content.length() << "\n"
                                             ">>\n"
                                             "stream\n"
        << content
        << "endstream\n"
           "endobj\n";

      int softMaskFormObject = addXrefEntry(-1);
      write(form);
      *gStateObject = addXrefEntry(-1);
      xprintf(
          "<< /SMask << /S /Alpha /G %d 0 R >> >>\n"
          "endobj\n",
          softMaskFormObject);
      currentPage->graphicStates.append(*gStateObject);
    }
  }

  return patternObj;
}

int MyQPdfEnginePrivate::addConstantAlphaObject(int brushAlpha, int penAlpha) {
  if (brushAlpha == 255 && penAlpha == 255)
    return 0;
  int object = alphaCache.value(QPair<uint, uint>(brushAlpha, penAlpha), 0);
  if (!object) {
    object = addXrefEntry(-1);
    QByteArray alphaDef;
    MyQPdf::ByteStream s(&alphaDef);
    s << "<<\n/ca " << (brushAlpha / qreal(255.)) << '\n';
    s << "/CA " << (penAlpha / qreal(255.)) << "\n>>";
    xprintf("%s\nendobj\n", alphaDef.constData());
    alphaCache.insert(QPair<uint, uint>(brushAlpha, penAlpha), object);
  }
  if (currentPage->graphicStates.indexOf(object) < 0)
    currentPage->graphicStates.append(object);

  return object;
}

int MyQPdfEnginePrivate::addBrushPattern(const QTransform& m, bool* specifyColor, int* gStateObject) {
  int paintType = 2;  // Uncolored tiling
  int w = 8;
  int h = 8;

  *specifyColor = true;
  *gStateObject = 0;

  QTransform matrix = m;
  matrix.translate(brushOrigin.x(), brushOrigin.y());
  if (!xobjectState) {
    matrix = matrix * pageMatrix();
  }

  // qDebug() << brushOrigin << matrix;

  Qt::BrushStyle style = brush.style();
  if (style == Qt::LinearGradientPattern || style == Qt::RadialGradientPattern) {  // && style <= Qt::ConicalGradientPattern) {
    *specifyColor = false;
    return gradientBrush(brush, matrix, gStateObject);
  }

  if ((!brush.isOpaque() && brush.style() < Qt::LinearGradientPattern) || opacity != 1.0)
    *gStateObject = addConstantAlphaObject(qRound(brush.color().alpha() * opacity),
                                           qRound(pen.color().alpha() * opacity));

  int imageObject = -1;
  QByteArray pattern = MyQPdf::patternForBrush(brush);
  if (pattern.isEmpty()) {
    if (brush.style() != Qt::TexturePattern)
      return 0;
    QImage image = brush.textureImage();
    bool bitmap = true;
    imageObject = addImage(image, &bitmap, image.cacheKey());
    if (imageObject != -1) {
      QImage::Format f = image.format();
      if (f != QImage::Format_MonoLSB && f != QImage::Format_Mono) {
        paintType = 1;  // Colored tiling
        *specifyColor = false;
      }
      w = image.width();
      h = image.height();
      QTransform m(w, 0, 0, -h, 0, h);
      MyQPdf::ByteStream s(&pattern);
      s << MyQPdf::generateMatrix(m);
      s << "/Im" << imageObject << " Do\n";
    }
  }

  QByteArray str;
  MyQPdf::ByteStream s(&str);
  s << "<<\n"
       "/Type /Pattern\n"
       "/PatternType 1\n"
       "/PaintType "
    << paintType << "\n"
                    "/TilingType 1\n"
                    "/BBox [0 0 "
    << w << h << "]\n"
                 "/XStep "
    << w << "\n"
            "/YStep "
    << h << "\n"
            "/Matrix ["
    << matrix.m11()
    << matrix.m12()
    << matrix.m21()
    << matrix.m22()
    << matrix.dx()
    << matrix.dy() << "]\n"
                      "/Resources \n<< ";  // open resource tree
  if (imageObject > 0) {
    s << "/XObject << /Im" << imageObject << ' ' << imageObject << "0 R >> ";
  }
  s << ">>\n"
       "/Length "
    << pattern.length() << "\n"
                           ">>\n"
                           "stream\n"
    << pattern
    << "endstream\n"
       "endobj\n";

  int patternObj = addXrefEntry(-1);
  write(str);
  currentPage->patterns.append(patternObj);
  return patternObj;
}

static inline bool is_monochrome(const QVector<QRgb>& colorTable) {
  return colorTable.size() == 2 && colorTable.at(0) == QColor(Qt::black).rgba() && colorTable.at(1) == QColor(Qt::white).rgba();
}

/*!
 * Adds an image to the pdf and return the pdf-object id. Returns -1 if adding the image failed.
 */
int MyQPdfEnginePrivate::addImage(const QImage& img, bool* bitmap, qint64 serial_no) {
  if (img.isNull())
    return -1;

  int object = imageCache.value(serial_no);
  if (object)
    return object;

  QImage image = img;
  QImage::Format format = image.format();
  if (image.depth() == 1 && *bitmap && is_monochrome(img.colorTable())) {
    if (format == QImage::Format_MonoLSB)
      image = image.convertToFormat(QImage::Format_Mono);
    format = QImage::Format_Mono;
  } else {
    *bitmap = false;
    if (format != QImage::Format_RGB32 && format != QImage::Format_ARGB32) {
      image = image.convertToFormat(QImage::Format_ARGB32);
      format = QImage::Format_ARGB32;
    }
  }

  int w = image.width();
  int h = image.height();
  int d = image.depth();

  if (format == QImage::Format_Mono) {
    int bytesPerLine = (w + 7) >> 3;
    QByteArray data;
    data.resize(bytesPerLine * h);
    char* rawdata = data.data();
    for (int y = 0; y < h; ++y) {
      memcpy(rawdata, image.constScanLine(y), bytesPerLine);
      rawdata += bytesPerLine;
    }
    object = writeImage(data, w, h, d, 0, 0, false, is_monochrome(img.colorTable()));
  } else {
    QByteArray softMaskData;
    bool dct = false;
    QByteArray imageData;
    bool hasAlpha = false;
    bool hasMask = false;

    if (QImageWriter::supportedImageFormats().contains("jpeg") && !grayscale) {
      QBuffer buffer(&imageData);
      QImageWriter writer(&buffer, "jpeg");
      writer.setQuality(94);
      writer.write(image);
      dct = true;

      if (format != QImage::Format_RGB32) {
        softMaskData.resize(w * h);
        uchar* sdata = (uchar*)softMaskData.data();
        for (int y = 0; y < h; ++y) {
          const QRgb* rgb = (const QRgb*)image.constScanLine(y);
          for (int x = 0; x < w; ++x) {
            uchar alpha = qAlpha(*rgb);
            *sdata++ = alpha;
            hasMask |= (alpha < 255);
            hasAlpha |= (alpha != 0 && alpha != 255);
            ++rgb;
          }
        }
      }
    } else {
      imageData.resize(grayscale ? w * h : 3 * w * h);
      uchar* data = (uchar*)imageData.data();
      softMaskData.resize(w * h);
      uchar* sdata = (uchar*)softMaskData.data();
      for (int y = 0; y < h; ++y) {
        const QRgb* rgb = (const QRgb*)image.constScanLine(y);
        if (grayscale) {
          for (int x = 0; x < w; ++x) {
            *(data++) = qGray(*rgb);
            uchar alpha = qAlpha(*rgb);
            *sdata++ = alpha;
            hasMask |= (alpha < 255);
            hasAlpha |= (alpha != 0 && alpha != 255);
            ++rgb;
          }
        } else {
          for (int x = 0; x < w; ++x) {
            *(data++) = qRed(*rgb);
            *(data++) = qGreen(*rgb);
            *(data++) = qBlue(*rgb);
            uchar alpha = qAlpha(*rgb);
            *sdata++ = alpha;
            hasMask |= (alpha < 255);
            hasAlpha |= (alpha != 0 && alpha != 255);
            ++rgb;
          }
        }
      }
      if (format == QImage::Format_RGB32)
        hasAlpha = hasMask = false;
    }
    int maskObject = 0;
    int softMaskObject = 0;
    if (hasAlpha) {
      softMaskObject = writeImage(softMaskData, w, h, 8, 0, 0);
    } else if (hasMask) {
      // dither the soft mask to 1bit and add it. This also helps PDF viewers
      // without transparency support
      int bytesPerLine = (w + 7) >> 3;
      QByteArray mask(bytesPerLine * h, 0);
      uchar* mdata = (uchar*)mask.data();
      const uchar* sdata = (const uchar*)softMaskData.constData();
      for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
          if (*sdata)
            mdata[x >> 3] |= (0x80 >> (x & 7));
          ++sdata;
        }
        mdata += bytesPerLine;
      }
      maskObject = writeImage(mask, w, h, 1, 0, 0);
    }
    object = writeImage(imageData, w, h, grayscale ? 8 : 32,
                        maskObject, softMaskObject, dct);
  }
  imageCache.insert(serial_no, object);
  return object;
}

QTransform MyQPdfEnginePrivate::pageMatrix() const {
  qreal scale = 72. / resolution;
  QTransform tmp(scale, 0.0, 0.0, -scale, 0.0, m_pageLayout.fullRectPoints().height());

  if (m_pageLayout.mode() != QPageLayout::FullPageMode) {
    QRect r = m_pageLayout.paintRectPixels(resolution);
    tmp.translate(r.left(), r.top());
  }
  return tmp;
}

void MyQPdfEnginePrivate::newPage() {
  if (currentPage && currentPage->pageSize.isEmpty())
    currentPage->pageSize = m_pageLayout.fullRectPoints().size();
  writePage();

  if (currentPage)
    delete currentPage;

  currentPage = new MyQPdfPage;
  currentPage->pageSize = m_pageLayout.fullRectPoints().size();
  stroker.stream = currentPage;
  pages.append(requestObject());

  *currentPage << "/GSa gs /CSp cs /CSp CS\n"
               << MyQPdf::generateMatrix(pageMatrix())
               << "q q\n";
}
void MyQPdfEnginePrivate::writeRawPDFtoCurrentStream(QString rawPDF) {
  if (currentPage) {
    *currentPage << rawPDF.toLatin1().constData();
  }
}
void MyQPdfEnginePrivate::addImagetoResources(int objectID) {
  if (currentPage) {
    currentPage->images.append(objectID);
  }
}
void MyQPdfEnginePrivate::beginFormXObject() {
  if (currentPage) {
    writePage();
    delete currentPage;
  }

  currentPage = new MyQPdfPage;
  stroker.stream = currentPage;

  *currentPage << "/GSa gs /CSp cs /CSp CS\n"
               << "q q\n";

  xobjectState = true;
}
int MyQPdfEnginePrivate::endFormXObject(QRectF bbox) {
  *currentPage << "Q Q\n";

  uint resources = requestObject();

  QByteArray xobjectResources;

  xprintf(xobjectResources,
          "<<\n"
          "/ColorSpace <<\n"
          "/PCSp %d 0 R\n"
          "/CSp /DeviceRGB\n"
          "/CSpg /DeviceGray\n"
          ">>\n"
          "/ExtGState <<\n"
          "/GSa %d 0 R\n",
          patternColorSpace, graphicsState);

  for (int i = 0; i < currentPage->graphicStates.size(); ++i)
    xprintf(xobjectResources, "/GState%d %d 0 R\n", currentPage->graphicStates.at(i), currentPage->graphicStates.at(i));
  xprintf(xobjectResources, ">>\n");

  if (currentPage->patterns.size() > 0) {
    xprintf(xobjectResources, "/Pattern <<\n");
    for (int i = 0; i < currentPage->patterns.size(); ++i)
      xprintf(xobjectResources, "/Pat%d %d 0 R\n", currentPage->patterns.at(i), currentPage->patterns.at(i));
    xprintf(xobjectResources, ">>\n");
  }

  if (currentPage->fonts.size() > 0) {
    xprintf(xobjectResources, "/Font <<\n");
    for (int i = 0; i < currentPage->fonts.size(); ++i)
      xprintf(xobjectResources, "/F%d %d 0 R\n", currentPage->fonts[i], currentPage->fonts[i]);
    xprintf(xobjectResources, ">>\n");
  }

  if (currentPage->images.size() > 0) {
    xprintf(xobjectResources, "/XObject <<\n");
    for (int i = 0; i < currentPage->images.size(); ++i) {
      xprintf(xobjectResources, "/Im%d %d 0 R\n", currentPage->images.at(i), currentPage->images.at(i));
    }
    xprintf(xobjectResources, ">>\n");
  }

  xprintf(xobjectResources, ">>\n");

  addObject(xobjectResources, resources);

  uint xobjectStream = addXrefEntry(-1);

  auto data = getCompressed(currentPage->getBA());

  xprintf(
      "<<\n"
      "/Type /XObject\n"
      "/Subtype /Form\n"
      "/FormType 1\n"
      "/Matrix [1 0 0 1 0 0]\n"
      "/Resources %d 0 R\n"
      "/BBox [%f %f %f %f]\n"
      "/Length %d\n",
      resources, bbox.left(), bbox.top(), bbox.right(), bbox.bottom(), data.length());
  if (do_compress)
    xprintf("/Filter /FlateDecode\n");

  xprintf(">>\n");
  xprintf("stream\n");
  write(data);
  write(
      "endstream\n"
      "endobj\n");

  delete currentPage;

  currentPage = nullptr;

  xobjectState = false;

  return xobjectStream;
}

int MyQPdfEnginePrivate::writeCompressed(QIODevice* dev) {
#ifndef QT_NO_COMPRESS
  if (do_compress) {
    int size = MyQPdfPage::chunkSize();
    int sum = 0;
    ::z_stream zStruct;
    zStruct.zalloc = Z_NULL;
    zStruct.zfree = Z_NULL;
    zStruct.opaque = Z_NULL;
    if (::deflateInit(&zStruct, Z_DEFAULT_COMPRESSION) != Z_OK) {
      qWarning("QPdfStream::writeCompressed: Error in deflateInit()");
      return sum;
    }
    zStruct.avail_in = 0;
    QByteArray in, out;
    out.resize(size);
    while (!dev->atEnd() || zStruct.avail_in != 0) {
      if (zStruct.avail_in == 0) {
        in = dev->read(size);
        zStruct.avail_in = in.size();
        zStruct.next_in = reinterpret_cast<unsigned char*>(in.data());
        if (in.size() <= 0) {
          qWarning("QPdfStream::writeCompressed: Error in read()");
          ::deflateEnd(&zStruct);
          return sum;
        }
      }
      zStruct.next_out = reinterpret_cast<unsigned char*>(out.data());
      zStruct.avail_out = out.size();
      if (::deflate(&zStruct, 0) != Z_OK) {
        qWarning("QPdfStream::writeCompressed: Error in deflate()");
        ::deflateEnd(&zStruct);
        return sum;
      }
      int written = out.size() - zStruct.avail_out;
      stream->writeRawData(out.constData(), written);
      streampos += written;
      sum += written;
    }
    int ret;
    do {
      zStruct.next_out = reinterpret_cast<unsigned char*>(out.data());
      zStruct.avail_out = out.size();
      ret = ::deflate(&zStruct, Z_FINISH);
      if (ret != Z_OK && ret != Z_STREAM_END) {
        qWarning("QPdfStream::writeCompressed: Error in deflate()");
        ::deflateEnd(&zStruct);
        return sum;
      }
      int written = out.size() - zStruct.avail_out;
      stream->writeRawData(out.constData(), written);
      streampos += written;
      sum += written;
    } while (ret == Z_OK);

    ::deflateEnd(&zStruct);

    return sum;
  } else
#endif
  {
    QByteArray arr;
    int sum = 0;
    while (!dev->atEnd()) {
      arr = dev->read(MyQPdfPage::chunkSize());
      stream->writeRawData(arr.constData(), arr.size());
      streampos += arr.size();
      sum += arr.size();
    }
    return sum;
  }
}

int MyQPdfEnginePrivate::writeCompressed(const char* src, int len) {
#ifndef QT_NO_COMPRESS
  if (do_compress) {
    uLongf destLen = len + len / 100 + 13;  // zlib requirement
    Bytef* dest = new Bytef[destLen];
    if (Z_OK == ::compress(dest, &destLen, (const Bytef*)src, (uLongf)len)) {
      stream->writeRawData((const char*)dest, destLen);
    } else {
      qWarning("QPdfStream::writeCompressed: Error in compress()");
      destLen = 0;
    }
    delete[] dest;
    len = destLen;
  } else
#endif
  {
    stream->writeRawData(src, len);
  }
  streampos += len;
  return len;
}

QByteArray MyQPdfEnginePrivate::getCompressed(QByteArray src) {
  int len = src.size();
  if (do_compress) {
    uLongf destLen = len + len / 100 + 13;  // zlib requirement
    // Bytef* dest = new Bytef[destLen];
    QByteArray dest;
    dest.resize(destLen);
    if (Z_OK == ::compress((Bytef*)dest.data(), &destLen, (const Bytef*)src.constData(), (uLongf)len)) {
      // stream->writeRawData((const char*)dest, destLen);
    } else {
      qWarning("QPdfStream::writeCompressed: Error in compress()");
      destLen = 0;
    }
    dest.resize(destLen);
    return dest;
  } else

  {
    return src;
  }
}

GlyphIndex MyQPdfEnginePrivate::getIndex(MyQPdf::GlyphKey glyphCode) {
  if (glyphsToIndex.find(glyphCode) != glyphsToIndex.end()) {
    return glyphsToIndex[glyphCode];
  }

  if (currenttype3Font == -1) {
    currenttype3Font = requestObject();
  }

  int encoding = fonttoToglyphs[currenttype3Font].size();

  if (encoding == 256) {
    currenttype3Font = requestObject();
    encoding = 0;
  }
  fonttoToglyphs[currenttype3Font].append(glyphCode);

  GlyphIndex index{currenttype3Font, encoding};

  glyphsToIndex.insert({glyphCode, index});

  return index;
}

#include <QPainterPathStroker>

QPainterPath makeBold(const QPainterPath& path, qreal amount) {
  QPainterPathStroker stroker;
  stroker.setWidth(amount);             // thickness of bold
  stroker.setJoinStyle(Qt::RoundJoin);  // better for glyphs
  stroker.setCapStyle(Qt::RoundCap);

  QPainterPath stroke = stroker.createStroke(path);

  // Union original + stroke
  return path.united(stroke);
}

static qreal roundTo(qreal v, int decimals) {
  qreal factor = std::pow(10.0, decimals);
  return std::round(v * factor) / factor;
}

QPainterPath roundPath(const QPainterPath& path, int decimals) {
  QPainterPath out;
  out.setFillRule(path.fillRule());

  int i = 0;

  while (i < path.elementCount()) {
    const auto e = path.elementAt(i);

    if (e.isMoveTo()) {
      out.moveTo(roundTo(e.x, decimals),
                 roundTo(e.y, decimals));
      ++i;
    } else if (e.isLineTo()) {
      out.lineTo(roundTo(e.x, decimals),
                 roundTo(e.y, decimals));
      ++i;
    } else if (e.type == QPainterPath::CurveToElement) {
      if (i + 2 >= path.elementCount())
        break;

      const auto c1 = path.elementAt(i);
      const auto c2 = path.elementAt(i + 1);
      const auto end = path.elementAt(i + 2);

      out.cubicTo(
          roundTo(c1.x, decimals), roundTo(c1.y, decimals),
          roundTo(c2.x, decimals), roundTo(c2.y, decimals),
          roundTo(end.x, decimals), roundTo(end.y, decimals));

      i += 3;
    } else {
      ++i;
    }
  }

  return out;
}
static qreal snap(qreal v, qreal grid) {
  return std::round(v / grid) * grid;
}

void MyQPdfEnginePrivate::generateSurahFontFromSvg(const std::vector<QPainterPath>& paths) {
  surahType3Font = requestObject();
  auto& type3Font = fonttoToglyphs[surahType3Font];
  for (int i = 0; i < paths.size(); ++i) {
    const QPainterPath& path = paths[i];

    QTransform t;

    t.scale(100, 100);

    QPainterPath out = t.map(path);

    // out = makeBold(out, 25.0);

    // out = roundPath(out, 2);

    auto box = out.boundingRect();

    out.translate(-box.left(), -box.top() - box.height() / 2);

    out = roundPath(out, 2);

    GlyphPath g;
    g.codepoint = i;
    g.glyphIndex = i;
    g.path = out;

    surahs.append(g);

    MyQPdf::GlyphKey key{i};

    type3Font.append(key);
  }
}

void MyQPdfEnginePrivate::generateSurahFont() {
  const QString surahFontPath = ":/fonts/icomoon.ttf";

  QRawFont surahFont(surahFontPath, 1024.0);
  if (!surahFont.isValid()) {
    qCritical() << "Failed to load raw font:" << surahFontPath;
    return;
  }

  QVector<QChar> chars;
  for (uint cp = 0xE900; cp <= 0xE972; ++cp)
    chars.append(QChar(cp));

  int maxGlyphs = chars.size();  // safe upper bound for PUA
  QVector<quint32> glyphIndexes(maxGlyphs);

  int numGlyphs = maxGlyphs;

  bool ok = surahFont.glyphIndexesForChars(
      chars.constData(),
      chars.size(),
      glyphIndexes.data(),
      &numGlyphs);

  if (!ok) {
    qWarning() << "glyphIndexesForChars failed";
    return;
  }

  surahType3Font = requestObject();
  auto& type3Font = fonttoToglyphs[surahType3Font];

  for (int i = 0; i < chars.size(); ++i) {
    quint32 glyphIndex = glyphIndexes.value(i, 0);

    if (glyphIndex == 0) {
      // missing glyph → skip or keep empty
      continue;
    }

    QPainterPath path = surahFont.pathForGlyph(glyphIndex);

    QRectF box = path.boundingRect();

    // QTransform t;

    // 1) remove left bearing
    // t.translate(-box.left(), box.top() - box.height() / 2);

    // 2) flip Y
    // t.scale(1.0, -1.0);

    // path = t.map(path);

    path.translate(-box.left(), -box.top() - box.height() / 2);

    GlyphPath g;
    g.codepoint = chars[i].unicode();
    g.glyphIndex = glyphIndex;
    g.path = path;

    surahs.append(g);

    MyQPdf::GlyphKey key{chars[i].unicode()};

    type3Font.append(key);
  }
}

int MyQPdfEnginePrivate::writeXobjectForm(GlyphVis& glyph, QString name) {
  if (XObjects.contains(name)) {
    return XObjects[name];
  }
  QByteArray data = getCompressed(getImageStream(glyph));

  int xform = addXrefEntry(-1);
  xprintf(
      "<<\n"
      "/Type /XObject\n"
      "/Subtype /Form\n"
      "/FormType 1\n"
      "/Matrix [1 0 0 1 0 0]\n"
      "/BBox [%f %f %f %f]\n"
      "/Length %d\n",
      glyph.bbox.llx, glyph.bbox.lly, glyph.bbox.urx, glyph.bbox.ury, data.length());
  if (do_compress)
    xprintf("/Filter /FlateDecode\n");

  xprintf(">>\n");
  xprintf("stream\n");
  write(data);
  write(
      "endstream\n"
      "endobj\n");

  XObjects.insert(name, xform);

  return xform;
}
void MyQPdfEnginePrivate::writeType3Fonts() {
  auto glyphs = otlayout->glyphs;

  auto& endOfAyaGlyph = glyphs["endofaya"];

  auto ayafont = getIndex({endOfAyaGlyph.charcode, 0, 0}).font;

  auto coloredGlyph = endOfAyaGlyph.getColoredGlyph();

  if (coloredGlyph) {
    writeXobjectForm(*coloredGlyph, "endofaya");
  } else {
    writeXobjectForm(endOfAyaGlyph, "endofaya");
  }

  // auto bytearray

  for (auto font = fonttoToglyphs.constBegin(); font != fonttoToglyphs.constEnd(); ++font) {
    char buf[5];
    int CharProcs = requestObject();
    int Encoding = requestObject();
    int resources = requestObject();
    int FontDescriptor = requestObject();
    bool generateCmap = false;

    QByteArray arrayWidthByteArray;
    QByteArray encodingByteArray;
    QByteArray charProcsByteArray;
    QByteArray fontdictionayByteArray;
    QByteArray FontDescriptorByteArray;
    QByteArray bfcharByteArray;

    MyQPdf::ByteStream arrayWidthStream(&arrayWidthByteArray);
    MyQPdf::ByteStream encodingByteStream(&encodingByteArray);
    MyQPdf::ByteStream charProcsByteStream(&charProcsByteArray);
    MyQPdf::ByteStream fontdictionayByteStream(&fontdictionayByteArray);
    MyQPdf::ByteStream FontDescriptorByteStream(&FontDescriptorByteArray);
    MyQPdf::ByteStream bfcharByteArrayByteStream(&bfcharByteArray);

    encodingByteStream << "<< /Type /Encoding\n/Differences [0";
    charProcsByteStream << "<<";
    arrayWidthStream << "[";
    int totalcmap = 0;

    double maxwidth = 0;

    bool isSurahFont = surahType3Font == font.key();

    GlyphVis::BBox bbox{INT_MAX, INT_MAX, INT_MIN, INT_MIN};
    for (int i = 0; i < font->size(); i++) {
      auto glyphCode = font->at(i);
      QString glyphName;
      QByteArray steamDataByteArray;

      if (!isSurahFont) {
        glyphName = otlayout->glyphNamePerCode[glyphCode.code];
        GlyphVis* glyph = &glyphs[glyphName];

        // if (glyphCode.lefttatweel >= 0.0001 || glyphCode.righttatweel >= 0.0001) {
        if (glyphCode.lefttatweel != 0 || glyphCode.righttatweel != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = glyphCode.lefttatweel;
          parameters.righttatweel = glyphCode.righttatweel;

          // std::cout << glyphName.toStdString() << "," << glyphCode.lefttatweel << "," << glyphCode.righttatweel << "\n";

          glyph = glyph->getAlternate(parameters);

          glyphName = QString("%1%2%3").arg(glyphName).arg(parameters.lefttatweel).arg(parameters.righttatweel);
        }
        arrayWidthStream << glyph->width;

        bbox.llx = qMin(bbox.llx, glyph->bbox.llx);
        bbox.lly = qMin(bbox.lly, glyph->bbox.lly);
        bbox.urx = qMax(bbox.urx, glyph->bbox.urx);
        bbox.ury = qMax(bbox.ury, glyph->bbox.ury);
        maxwidth = qMax(maxwidth, glyph->width);

        encodingByteStream << " /" << glyphName.toLatin1();

        steamDataByteArray = getCompressed(generateGlyph(*glyph));
      } else {
        glyphName = QString("surah_%1").arg(glyphCode.code);
        auto& surah = surahs[i];

        auto boundingRect = surah.path.boundingRect();

        auto llx = std::ceil(boundingRect.left());
        auto lly = std::ceil(boundingRect.top());
        auto urx = std::ceil(boundingRect.right());
        auto ury = std::ceil(boundingRect.bottom());
        auto width = std::ceil(boundingRect.width());

        if (lly >= ury) {
          std::cout << "Error" << std::endl;
        }

        arrayWidthStream << width;

        // Qt rectangle convention top <= bottom
        // Since it is flipped ( y from bottom to top) than lly = box.top() and ury = box.bottom()

        bbox.llx = qMin(bbox.llx, llx);
        bbox.lly = qMin(bbox.lly, lly);
        bbox.urx = qMax(bbox.urx, urx);
        bbox.ury = qMax(bbox.ury, ury);
        maxwidth = qMax(maxwidth, width);

        encodingByteStream << " /" << glyphName.toLatin1();

        QByteArray localSteamDataByteArray;
        MyQPdf::ByteStream localSteamDataByteStream(&localSteamDataByteArray);

        localSteamDataByteStream << width << 0 << llx << lly << urx << ury << "d1\n";
        localSteamDataByteStream << MyQPdf::generatePath(surah.path, stroker.matrix, MyQPdf::FillPath);

        steamDataByteArray = getCompressed(localSteamDataByteArray);
      }

      int streamobj = requestObject();

      charProcsByteStream << " /" << glyphName.toLatin1() << " " << streamobj << "0 R";

      addXrefEntry(streamobj);
      xprintf(
          "<<\n"
          "/Length %d\n",
          steamDataByteArray.length());
      if (do_compress)
        xprintf("/Filter /FlateDecode\n");

      xprintf(">>\n");
      xprintf("stream\n");
      write(steamDataByteArray);
      write(
          "endstream\n"
          "endobj\n");

      if (generateCmap) {
        ushort unicode = glyphCode.code;
        if (glyphCode.code >= 0xDFFF) {
          unicode = 0x200B;
        }
        if (glyphCode.code < 0xDFFF) {
          totalcmap++;
          bfcharByteArrayByteStream << "<" << MyQPdf::toHex((uchar)i, buf) << "> ";
          bfcharByteArrayByteStream << "<" << MyQPdf::toHex(unicode, buf) << ">\n";
        }
      }
    }
    arrayWidthStream << "]";

    // cmap stream
    int cmap = 0;

    if (generateCmap) {
      cmap = requestObject();
      QByteArray cmapByteArray;
      MyQPdf::ByteStream cmapByteStream(&cmapByteArray);

      cmapByteStream << "1 begincodespacerange\n";
      cmapByteStream << "<00> <" << MyQPdf::toHex((uchar)(font->size() - 1), buf) << ">\n";
      cmapByteStream << "endcodespacerange\n";
      cmapByteStream << totalcmap << " beginbfchar\n";
      cmapByteStream << bfcharByteArray;
      cmapByteStream << "endbfchar\n";

      cmapByteStream << "endcmap\nCMapName currentdict /CMap defineresource pop\nend\nend\n";
      // cmapByteStream << "%%EndResource\n" << "%%EOF\n\n";

      cmapByteArray = getCompressed(cmapByteArray);
      ;
      addXrefEntry(cmap);
      xprintf(
          "<<\n"
          "/Length %d\n",
          cmapByteArray.length());
      if (do_compress)
        xprintf("/Filter /FlateDecode\n");

      xprintf(">>\n");
      xprintf("stream\n");
      write(cmapByteArray);
      write(
          "endstream\n"
          "endobj\n");
    }

    /// addXrefEntry(CharProcs);
    charProcsByteStream << "\n>>\n";  // endobj\n";
    addObject(charProcsByteArray, CharProcs);
    // write(charProcsByteArray);

    // addXrefEntry(Encoding);
    encodingByteStream << "]\n>>\n";  // endobj\n";
    addObject(encodingByteArray, Encoding);
    // write(encodingByteArray);

    // addXrefEntry(font.key());
    fontdictionayByteStream << "<< /Type /Font\n";
    fontdictionayByteStream << "/Subtype /Type3\n";
    fontdictionayByteStream << "/Encoding " << Encoding << "0 R\n";
    fontdictionayByteStream << "/CharProcs " << CharProcs << "0 R\n";
    fontdictionayByteStream << "/Resources " << resources << "0 R\n";
    fontdictionayByteStream << "/FontDescriptor " << FontDescriptor << "0 R\n";
    if (generateCmap) {
      fontdictionayByteStream << "/ToUnicode " << cmap << "0 R\n";
    }

    fontdictionayByteStream << "/Name /quranmedinafont" << font.key() << "\n";
    fontdictionayByteStream << "/FirstChar 0\n";
    fontdictionayByteStream << "/LastChar " << font.value().size() - 1 << "\n";
    fontdictionayByteStream << "/FontMatrix [0.001 0 0 0.001 0 0]\n";
    fontdictionayByteStream << "/FontBBox [" << bbox.llx << bbox.lly << bbox.urx << bbox.ury << "]\n";
    fontdictionayByteStream << "/Widths " << arrayWidthByteArray << "\n";
    fontdictionayByteStream << ">>\n";  // endobj\n";
    // write(fontdictionayByteArray);
    addObject(fontdictionayByteArray, font.key());

    FontDescriptorByteStream << "<< /Type /FontDescriptor\n";
    FontDescriptorByteStream << "/FontName /quranmedinafont" << font.key() << "\n";
    FontDescriptorByteStream << "/FontFamily (QuranMedinaFont)" << "\n";
    FontDescriptorByteStream << "/FontStretch /Normal" << "\n";
    FontDescriptorByteStream << "/FontWeight 400" << "\n";
    FontDescriptorByteStream << "/Flags 12" << "\n";
    FontDescriptorByteStream << "/FontBBox [" << bbox.llx << bbox.lly << bbox.urx << bbox.ury << "]\n";
    FontDescriptorByteStream << "/ItalicAngle 0" << "\n";
    FontDescriptorByteStream << "/Ascent 1000" << "\n";
    FontDescriptorByteStream << "/Descent 200" << "\n";
    FontDescriptorByteStream << "/Leading 1750" << "\n";
    FontDescriptorByteStream << "/AvgWidth 500" << "\n";
    FontDescriptorByteStream << "/MaxWidth " << (int)maxwidth << "\n";
    FontDescriptorByteStream << ">>\n";

    addObject(FontDescriptorByteArray, FontDescriptor);

    QByteArray resourcesBA;

    if (font.key() != ayafont) {
      // addXrefEntry(resources);
      xprintf(resourcesBA, "<<\n");

      xprintf(resourcesBA, "/Font <<\n");
      xprintf(resourcesBA, "/F%d %d 0 R\n", ayafont, ayafont);
      xprintf(resourcesBA, ">>\n");

      xprintf(resourcesBA, "/XObject <<\n");
      for (auto xform : XObjects) {
        xprintf(resourcesBA, "/Im%d %d 0 R\n", xform, xform);
      }
      xprintf(resourcesBA, ">>\n");

      xprintf(resourcesBA, ">>\n");
      //"endobj\n");
    } else {
      // addXrefEntry(resources);
      xprintf(resourcesBA, "<<\n");

      xprintf(resourcesBA, "/XObject <<\n");
      for (auto xform : XObjects) {
        xprintf(resourcesBA, "/Im%d %d 0 R\n", xform, xform);
      }
      xprintf(resourcesBA, ">>\n");

      xprintf(resourcesBA, ">>\n");
      //"endobj\n");
    }

    addObject(resourcesBA, resources);
  }
}
QByteArray MyQPdfEnginePrivate::getImageStream(GlyphVis& glyph) {
  QByteArray steamDataByteArray;
  MyQPdf::ByteStream steamDataByteStream(&steamDataByteArray);

  if (glyph.m_edge) {
    mp_graphic_object* body = glyph.m_edge->body;
    if (body) {
      steamDataByteStream << "/DeviceRGB cs\n";
      QPainterPath foregroudpath;
      foregroudpath.setFillRule(Qt::FillRule::WindingFill);
      do {
        switch (body->type) {
          case mp_fill_code: {
            auto fillobject = (mp_fill_object*)body;
            QPainterPath subpath = Glyph::mp_dump_solved_path(fillobject->path_p);
            if (fillobject->color_model == mp_rgb_model) {
              // painter.setBrush(QColor(fillobject->color.a_val, fillobject->color.b_val, fillobject->color.c_val));

              QColor rgba(fillobject->color.a_val * 255, fillobject->color.b_val * 255, fillobject->color.c_val * 255);
              steamDataByteStream << rgba.redF() << rgba.greenF() << rgba.blueF() << "sc\n";
              steamDataByteStream << MyQPdf::generatePath(subpath, stroker.matrix, MyQPdf::FillPath);
            } else if (fillobject->color_model == mp_no_model) {
              foregroudpath.addPath(subpath);
            }

            break;
          }
          default:
            break;
        }

      } while (body = body->next);

      QColor rgba = Qt::black;
      steamDataByteStream << rgba.redF() << rgba.greenF() << rgba.blueF() << "sc\n";
      steamDataByteStream << MyQPdf::generatePath(foregroudpath, stroker.matrix, MyQPdf::FillPath);
    }
  }

  return steamDataByteArray;
}
QByteArray MyQPdfEnginePrivate::generateGlyph(GlyphVis& glyph) {
  QByteArray steamDataByteArray;
  MyQPdf::ByteStream steamDataByteStream(&steamDataByteArray);

  if (glyph.name == "endofaya") {  //||  glyph->name == "rubelhizb" glyph->name == "placeofsajdah" ||
    if (XObjects.contains(glyph.name)) {
      steamDataByteStream << glyph.width << 0 << "d0\n";
      int xobjectnum = XObjects[glyph.name];
      steamDataByteStream << "/Im" << xobjectnum << " Do\n";
    } else {
      auto bytearray = getImageStream(glyph);
      steamDataByteStream << glyph.width << 0 << "d0\n"
                          << bytearray;
    }

  } else if (glyph.charcode >= Automedina::AyaNumberCode && glyph.charcode <= Automedina::AyaNumberCode + 286) {
    int ayaNumber = (glyph.charcode - Automedina::AyaNumberCode) + 1;

    int digitheight = 120;

    auto endofayaIndex = getIndex({otlayout->glyphs["endofaya"].charcode, 0, 0});

    steamDataByteStream << otlayout->glyphs["endofaya"].width << 0 << "d0\n";
    char buf[5];

    if (XObjects.contains("endofaya")) {
      steamDataByteStream << "/Im" << XObjects["endofaya"] << "Do\n";
      steamDataByteStream << "BT\n";
      steamDataByteStream << "/F" << endofayaIndex.font << 1000 << "Tf\n";
    } else {
      steamDataByteStream << "BT\n";
      steamDataByteStream << "/F" << endofayaIndex.font << 1000 << "Tf\n";
      steamDataByteStream << "1 0 0 1 0 0 Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)endofayaIndex.encoding, buf) << "> Tj\n";
    }

    if (ayaNumber < 10) {
      auto& onesglyph = otlayout->glyphs[otlayout->glyphNamePerCode[1632 + ayaNumber]];
      auto oneglyphIndex = getIndex({onesglyph.charcode, 0, 0});

      auto position = otlayout->glyphs["endofaya"].width / 2 - (onesglyph.width) / 2;

      if (oneglyphIndex.font != endofayaIndex.font) {
        steamDataByteStream << "/F" << oneglyphIndex.font << 1000 << "Tf\n";
      }
      steamDataByteStream << "1 0 0 1 " << position << digitheight << "Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)oneglyphIndex.encoding, buf) << "> Tj\n";

    } else if (ayaNumber < 100) {
      int onesdigit = ayaNumber % 10;
      int tensdigit = ayaNumber / 10;

      auto& onesglyph = otlayout->glyphs[otlayout->glyphNamePerCode[1632 + onesdigit]];
      auto& tensglyph = otlayout->glyphs[otlayout->glyphNamePerCode[1632 + tensdigit]];

      auto oneglyphIndex = getIndex({onesglyph.charcode, 0, 0});
      auto tensglyphIndex = getIndex({tensglyph.charcode, 0, 0});

      auto position = otlayout->glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + 40) / 2;
      // painter.translate(position, digitheight);

      if (tensglyphIndex.font != endofayaIndex.font) {
        steamDataByteStream << "/F" << oneglyphIndex.font << 1000 << "Tf\n";
      }
      steamDataByteStream << "1 0 0 1 " << position << digitheight << "Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)tensglyphIndex.encoding, buf) << "> Tj\n";

      if (oneglyphIndex.font != tensglyphIndex.font) {
        steamDataByteStream << "/F" << oneglyphIndex.font << 1000 << "Tf\n";
      }

      steamDataByteStream << "1 0 0 1 " << position + tensglyph.width + 40 << digitheight << "Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)oneglyphIndex.encoding, buf) << "> Tj\n";

    } else {
      int onesdigit = ayaNumber % 10;
      int tensdigit = (ayaNumber / 10) % 10;
      int hundredsdigit = ayaNumber / 100;

      auto& onesglyph = otlayout->glyphs[otlayout->glyphNamePerCode[1632 + onesdigit]];
      auto& tensglyph = otlayout->glyphs[otlayout->glyphNamePerCode[1632 + tensdigit]];
      auto& hundredsglyph = otlayout->glyphs[otlayout->glyphNamePerCode[1632 + hundredsdigit]];

      auto oneglyphIndex = getIndex({onesglyph.charcode, 0, 0});
      auto tensglyphIndex = getIndex({tensglyph.charcode, 0, 0});
      auto hundredsglyphIndex = getIndex({hundredsglyph.charcode, 0, 0});

      auto position = otlayout->glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + hundredsglyph.width + 80) / 2;
      // painter.translate(position, digitheight);
      if (hundredsglyphIndex.font != endofayaIndex.font) {
        steamDataByteStream << "/F" << hundredsglyphIndex.font << 1000 << "Tf\n";
      }
      steamDataByteStream << "1 0 0 1 " << position << digitheight << "Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)hundredsglyphIndex.encoding, buf) << "> Tj\n";

      if (hundredsglyphIndex.font != tensglyphIndex.font) {
        steamDataByteStream << "/F" << tensglyphIndex.font << 1000 << "Tf\n";
      }

      steamDataByteStream << "1 0 0 1 " << position + hundredsglyph.width + 40 << digitheight << "Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)tensglyphIndex.encoding, buf) << "> Tj\n";

      if (oneglyphIndex.font != tensglyphIndex.font) {
        steamDataByteStream << "/F" << oneglyphIndex.font << 1000 << "Tf\n";
      }

      steamDataByteStream << "1 0 0 1 " << position + hundredsglyph.width + 40 + tensglyph.width + 40 << digitheight << "Tm\n";
      steamDataByteStream << "<" << MyQPdf::toHex((uchar)oneglyphIndex.encoding, buf) << "> Tj\n";
    }

    steamDataByteStream << "ET\n";
  } else if (glyph.name.startsWith("endofaya")) {
    auto coloredGlyph = glyph.getColoredGlyph();
    if (coloredGlyph) {
      auto bytearray = getImageStream(*coloredGlyph);
      steamDataByteStream << coloredGlyph->width << 0 << "d0\n";
      steamDataByteStream << bytearray;
    } else {
      steamDataByteStream << glyph.width << 0 << glyph.bbox.llx << glyph.bbox.lly << glyph.bbox.urx << glyph.bbox.ury << "d1\n";
      steamDataByteStream << MyQPdf::generatePath(glyph.path, stroker.matrix, MyQPdf::FillPath);
    }

  } else {
    steamDataByteStream << glyph.width << 0 << glyph.bbox.llx << glyph.bbox.lly << glyph.bbox.urx << glyph.bbox.ury << "d1\n";
    steamDataByteStream << MyQPdf::generatePath(glyph.path, stroker.matrix, MyQPdf::FillPath);
  }

  return steamDataByteArray;
}

QT_END_NAMESPACE

#endif  // QT_NO_PDF
