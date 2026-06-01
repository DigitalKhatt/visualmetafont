#include "quranpdfwriterpdfhummus.h"

#include <QtCore/qjsondocument.h>
#include <QtGui/qpdfwriter.h>
#include <QtSvg/qsvgrenderer.h>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QPainterPathStroker>
#include <QRawFont>
#include <QRegularExpression>
#include <QTextStream>
#include <QXmlStreamReader>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "DictionaryContext.h"
#include "ObjectsContext.h"
#include "PDFDocumentCopyingContext.h"
#include "PDFFormXObject.h"
#include "PDFPage.h"
#include "PDFPageInput.h"
#include "PDFRectangle.h"
#include "PDFStream.h"
#include "PDFUsedFont.h"
#include "PageContentContext.h"
#include "XObjectContentContext.h"
#include "automedina.h"
#include "glyph.hpp"
#include "qcolor.h"
#include "qdiriterator.h"
#include "qfont.h"
#include "qfontinfo.h"
#include "qjsonobject.h"
#include "qpainter.h"
#include "qstandardpaths.h"

using PDFHummus::eFailure;
using PDFHummus::eSuccess;

static std::unordered_map<int, int> loadSurahNumberToUnicode(const QString& jsonPath) {
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

QuranPdfWriterPdfHummus::QuranPdfWriterPdfHummus(const Options& options,
                                                 OtLayout* otlayout,
                                                 QObject* parent)
    : QObject(parent), m_options(options), m_otlayout(otlayout) {
  pageSizePts = m_options.pageSize.rectPoints().size();
  resolution = options.dpi;
  pageSizePxs = m_options.pageSize.rectPixels(resolution).size();
  this->m_surahNumberToUnicode = loadSurahNumberToUnicode(":/fonts/surahCodes.json");
}

QuranPdfWriterPdfHummus::~QuranPdfWriterPdfHummus() {
  if (m_started)
    finishPdf();
}

std::string QuranPdfWriterPdfHummus::toUtf8(const QString& s) {
  const QByteArray b = s.toUtf8();
  return std::string(b.constData(), b.size());
}

std::string QuranPdfWriterPdfHummus::num(double v) {
  if (std::abs(v) < 0.0000001)
    v = 0;
  std::ostringstream ss;
  ss.setf(std::ios::fixed);
  ss << std::setprecision(4) << v;
  std::string r = ss.str();
  while (r.size() > 1 && r.back() == '0') r.pop_back();
  if (!r.empty() && r.back() == '.') r.pop_back();
  return r;
}

std::string QuranPdfWriterPdfHummus::pdfHexByte(unsigned char v) {
  static const char* h = "0123456789ABCDEF";
  std::string s;
  s += h[(v >> 4) & 0xF];
  s += h[v & 0xF];
  return s;
}

std::string QuranPdfWriterPdfHummus::pdfHexUShort(ushort v) {
  return pdfHexByte((v >> 8) & 0xff) + pdfHexByte(v & 0xff);
}

std::string QuranPdfWriterPdfHummus::pdfHexUtf16BE(const QString& text) {
  std::string out = "FEFF";
  for (int i = 0; i < text.size(); ++i) {
    ushort u = text[i].unicode();
    if (u == 10) u = 0x20;
    out += pdfHexUShort(u);
  }
  return out;
}

std::string QuranPdfWriterPdfHummus::pdfName(const QString& name) {
  QByteArray utf8 = name.toUtf8();
  std::string out;
  for (unsigned char c : utf8) {
    const bool regular = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
    if (regular) {
      out.push_back(char(c));
    } else {
      out.push_back('#');
      out += pdfHexByte(c);
    }
  }
  return out;
}

std::string QuranPdfWriterPdfHummus::pdfStringLiteral(const QString& s) {
  QByteArray b = s.toUtf8();
  std::string out = "(";
  for (char ch : b) {
    if (ch == '(' || ch == ')' || ch == '\\') out.push_back('\\');
    out.push_back(ch);
  }
  out.push_back(')');
  return out;
}

QByteArray QuranPdfWriterPdfHummus::pathToPdf(const QPainterPath& path, bool fill) {
  QByteArray out;
  QTextStream ts(&out, QIODevice::WriteOnly);
  ts.setRealNumberPrecision(3);
  ts.setRealNumberNotation(QTextStream::FixedNotation);

  int start = -1;
  for (int i = 0; i < path.elementCount(); ++i) {
    const auto e = path.elementAt(i);
    if (e.isMoveTo()) {
      if (start >= 0 && path.elementAt(start).x == path.elementAt(i - 1).x && path.elementAt(start).y == path.elementAt(i - 1).y)
        ts << "h\n";
      ts << e.x << ' ' << e.y << " m\n";
      start = i;
    } else if (e.isLineTo()) {
      ts << e.x << ' ' << e.y << " l\n";
    } else if (e.type == QPainterPath::CurveToElement) {
      if (i + 2 >= path.elementCount()) break;
      const auto c1 = path.elementAt(i);
      const auto c2 = path.elementAt(i + 1);
      const auto ep = path.elementAt(i + 2);
      ts << c1.x << ' ' << c1.y << ' '
         << c2.x << ' ' << c2.y << ' '
         << ep.x << ' ' << ep.y << " c\n";
      i += 2;
    }
  }
  if (start >= 0 && path.elementAt(start).x == path.elementAt(path.elementCount() - 1).x && path.elementAt(start).y == path.elementAt(path.elementCount() - 1).y)
    ts << "h\n";
  ts << (fill ? "f\n" : "S\n");
  ts.flush();
  return out;
}

bool QuranPdfWriterPdfHummus::startPdf() {
  if (m_started) return true;
  if (m_options.outputPath.isEmpty()) {
    qWarning() << "QuranPdfWriterPdfHummus: outputPath is empty";
    return false;
  }

  m_generationDate = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss UTC");

  PDFCreationSettings settings(true, true);

  const auto status = m_writer.StartPDF(toUtf8(m_options.outputPath), ePDFVersion17, LogConfiguration::DefaultLogConfiguration(), settings);
  if (status != eSuccess) {
    qWarning() << "PDF-Writer StartPDF failed";
    return false;
  }
  m_started = true;

  InfoDictionary& infoDictionary = m_writer.GetDocumentContext().GetTrailerInformation().GetInfo();

  // 3. Set the Title and Author properties using PDFTextString
  infoDictionary.Title = PDFTextString("Old Madinah Mushaf");
  infoDictionary.Author = PDFTextString("DigitalKhatt Project");
  infoDictionary.Subject = PDFTextString("Experimental Digital Mushaf");
  infoDictionary.Keywords = PDFTextString("Quran,Mushaf,DigitalKhatt,Arabic Typography,Dynamic Justification");
  infoDictionary.Creator = PDFTextString("Visual MetaFont Editor");

  return true;
}

bool QuranPdfWriterPdfHummus::finishPdf() {
  if (!m_started) return true;

  ObjectIDType outlineRoot = writeOutlines(m_outlineItems);
  ObjectIDType pageLabelsID = writePageLabels();

  m_writer.GetDocumentContext().AddDocumentContextExtender(new CatalogOutlineExtender(outlineRoot, pageLabelsID));

  // Must be done before EndPDF. This writes /PageLabels in the catalog.
  // If your PDF-Writer version does not allow catalog extension here, do the
  // page-label step using qpdf/pikepdf after generation.
  writeCatalogPageLabels();

  const auto status = m_writer.EndPDF();
  m_started = false;
  if (status != eSuccess) {
    qWarning() << "PDF-Writer EndPDF failed";
    return false;
  }
  return true;
}

void QuranPdfWriterPdfHummus::raw(PageContentContext* ctx, const std::string& s) {
  ctx->WriteFreeCode(s);
}

unsigned long QuranPdfWriterPdfHummus::allocateObjectId() {
  ObjectsContext& objCtx = m_writer.GetObjectsContext();

  ObjectIDType customObjectId = objCtx.GetInDirectObjectsRegistry().AllocateNewObjectID();

  return customObjectId;
}

void QuranPdfWriterPdfHummus::writeIndirectDictionaryObject(unsigned long objectID,
                                                            const QByteArray& dictionaryBody) {
  ObjectsContext& objects = m_writer.GetObjectsContext();
  objects.StartNewIndirectObject(objectID);
  IByteWriterWithPosition* freeCtx = objects.StartFreeContext();

  freeCtx->Write((const Byte*)dictionaryBody.constData(), dictionaryBody.size());
  objects.EndFreeContext();
  objects.EndIndirectObject();
}
/*
void QuranPdfWriterPdfHummus::writeIndirectStreamObject(unsigned long objectID,
                                                        const QByteArray& dictBody,
                                                        const QByteArray& streamBody) {
  ObjectsContext& objCtx = m_writer.GetObjectsContext();
  objCtx.StartNewIndirectObject(objectID);

  QByteArray header = dictBody;
  if (!header.trimmed().endsWith(">>")) {
    qWarning() << "Stream dictionary must end with >>";
  }

  // The caller must include /Length if a custom compressed stream is required.
  // This default path writes uncompressed streams for predictable glyph debugging.
  QByteArray dict = header;
  dict.replace(">>", QByteArray("/Length ") + QByteArray::number(streamBody.size()) + "\n>>");

  IByteWriterWithPosition* freeCtx = objCtx.StartFreeContext();

  freeCtx->Write((const Byte*)dict.constData(), dict.size());
  static const char begin[] = "stream\n";
  freeCtx->Write((const Byte*)begin, sizeof(begin) - 1);
  freeCtx->Write((const Byte*)streamBody.constData(), streamBody.size());
  static const char end[] = "endstream\n";
  freeCtx->Write((const Byte*)end, sizeof(end) - 1);

  objCtx.EndFreeContext();
  objCtx.EndIndirectObject();
}*/

void QuranPdfWriterPdfHummus::writeGlyphStream(
    unsigned long objectID,
    const QByteArray& streamBody) {
  ObjectsContext& objCtx = m_writer.GetObjectsContext();

  objCtx.StartNewIndirectObject(objectID);

  DictionaryContext* dict = objCtx.StartDictionary();

  PDFStream* stream = objCtx.StartPDFStream(dict, true);

  stream->GetWriteStream()->Write(reinterpret_cast<const Byte*>(streamBody.constData()), streamBody.size());

  objCtx.EndPDFStream(stream);
  // objCtx.EndIndirectObject();
}

void QuranPdfWriterPdfHummus::seedSpecialGlyphs() {
  auto& glyphs = m_otlayout->glyphs;
  getIndex({glyphs["endofaya"].charcode, 0, 0});
  for (int i = 0; i < 10; ++i)
    getIndex({i + 1632, 0, 0});
  // start new font to use the generated font in other fonts
  m_currentType3Font = -1;
}

QuranPdfWriterPdfHummus::GlyphIndex QuranPdfWriterPdfHummus::getIndex(const GlyphKey& glyphCode) {
  auto it = m_glyphsToIndex.find(glyphCode);
  if (it != m_glyphsToIndex.end())
    return it->second;

  if (m_currentType3Font == -1) {
    m_currentType3Font = int(allocateObjectId());
    m_type3Fonts[m_currentType3Font].resourceName = QString("F%1").arg(m_currentType3Font);
  }

  int encoding = m_type3Fonts[m_currentType3Font].glyphs.size();
  if (encoding == 256) {
    m_currentType3Font = int(allocateObjectId());
    m_type3Fonts[m_currentType3Font].resourceName = QString("F%1").arg(m_currentType3Font);
    encoding = 0;
  }

  m_type3Fonts[m_currentType3Font].glyphs.append(glyphCode);
  GlyphIndex index{m_currentType3Font, encoding};
  m_glyphsToIndex.insert({glyphCode, index});
  return index;
}

QString QuranPdfWriterPdfHummus::ensureFontResource(PDFPage* page, int fontObjectId) {
  QString name = m_type3Fonts[fontObjectId].resourceName;
  if (name.isEmpty()) name = QString("F%1").arg(fontObjectId);

  page->GetResourcesDictionary().AddFontMapping(fontObjectId, name.toStdString());
  return name;
}

QString QuranPdfWriterPdfHummus::ensureFormResource(PDFPage* page, unsigned long formObjectId) {
  QString name = QString("Im%1").arg(formObjectId);
  page->GetResourcesDictionary().AddFormXObjectMapping(formObjectId, name.toStdString());

  return name;
}
static qreal roundTo(qreal v, int decimals) {
  qreal factor = std::pow(10.0, decimals);
  return std::round(v * factor) / factor;
}
static QPainterPath roundPath(const QPainterPath& path, int decimals) {
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

void QuranPdfWriterPdfHummus::generateSurahFontFromSvg(const QVector<QPainterPath>& paths) {
  m_surahType3Font = int(allocateObjectId());
  auto& font = m_type3Fonts[m_surahType3Font];
  font.resourceName = QString("F%1").arg(m_surahType3Font);

  for (int i = 0; i < paths.size(); ++i) {
    QTransform t;
    t.scale(100, 100);
    QPainterPath out = t.map(paths[i]);

    auto box = out.boundingRect();

    out.translate(-box.left(), -box.top() - box.height() / 2);

    out = roundPath(out, 2);

    GlyphPath g;
    g.codepoint = i;
    g.glyphIndex = i;
    g.path = out;
    m_surahs.append(g);
    font.glyphs.append({i, 0, 0});
  }
}

void QuranPdfWriterPdfHummus::generateSurahFont() {
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

  m_surahType3Font = int(allocateObjectId());
  auto& font = m_type3Fonts[m_surahType3Font];
  font.resourceName = QString("F%1").arg(m_surahType3Font);

  for (int i = 0; i < chars.size(); ++i) {
    quint32 glyphIndex = glyphIndexes.value(i, 0);
    if (glyphIndex == 0) continue;

    QPainterPath path = surahFont.pathForGlyph(glyphIndex);
    QRectF box = path.boundingRect();
    path.translate(-box.left(), -box.top() - box.height() / 2);

    GlyphPath g;
    g.codepoint = chars[i].unicode();
    g.glyphIndex = glyphIndex;
    g.path = path;
    m_surahs.append(g);

    int unicode = chars[i].unicode();

    font.glyphs.append({unicode, 0, 0});
  }
}

QVector<QPainterPath> QuranPdfWriterPdfHummus::loadPathsFromSvg(const QString& fileName) const {
  struct IndexedPath {
    int index = -1;
    QPainterPath path;
  };
  QVector<IndexedPath> indexed;

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Cannot open SVG file:" << fileName;
    return {};
  }

  auto readNumber = [](const QString& s, int& pos, double& out) -> bool {
    while (pos < s.size() && (s[pos].isSpace() || s[pos] == ',')) ++pos;
    static const QRegularExpression re(R"(^[+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)");
    auto m = re.match(s.mid(pos));
    if (!m.hasMatch()) return false;
    out = m.captured(0).toDouble();
    pos += m.capturedLength(0);
    return true;
  };

  auto parsePath = [&](const QString& d) -> QPainterPath {
    QPainterPath path;
    int pos = 0;
    QChar cmd;
    QPointF cur, start, lastControl;
    auto skip = [&]() { while (pos < d.size() && (d[pos].isSpace() || d[pos] == ',')) ++pos; };
    while (pos < d.size()) {
      skip();
      if (pos >= d.size()) break;
      if (d[pos].isLetter()) cmd = d[pos++];
      if (cmd == 'Z' || cmd == 'z') {
        path.closeSubpath();
        cur = start;
        continue;
      }
      double x, y, x1, y1, x2, y2;
      int saved;
      if (cmd == 'M' || cmd == 'm') {
        if (!readNumber(d, pos, x) || !readNumber(d, pos, y)) break;
        QPointF p = (cmd == 'm') ? cur + QPointF(x, y) : QPointF(x, y);
        path.moveTo(p);
        cur = p;
        start = p;
        while (true) {
          saved = pos;
          if (!readNumber(d, pos, x) || !readNumber(d, pos, y)) {
            pos = saved;
            break;
          }
          p = (cmd == 'm') ? cur + QPointF(x, y) : QPointF(x, y);
          path.lineTo(p);
          cur = p;
        }
      } else if (cmd == 'L' || cmd == 'l') {
        while (true) {
          saved = pos;
          if (!readNumber(d, pos, x) || !readNumber(d, pos, y)) {
            pos = saved;
            break;
          }
          QPointF p = (cmd == 'l') ? cur + QPointF(x, y) : QPointF(x, y);
          path.lineTo(p);
          cur = p;
        }
      } else if (cmd == 'H' || cmd == 'h') {
        while (true) {
          saved = pos;
          if (!readNumber(d, pos, x)) {
            pos = saved;
            break;
          }
          cur.setX(cmd == 'h' ? cur.x() + x : x);
          path.lineTo(cur);
        }
      } else if (cmd == 'V' || cmd == 'v') {
        while (true) {
          saved = pos;
          if (!readNumber(d, pos, y)) {
            pos = saved;
            break;
          }
          cur.setY(cmd == 'v' ? cur.y() + y : y);
          path.lineTo(cur);
        }
      } else if (cmd == 'C' || cmd == 'c') {
        while (true) {
          saved = pos;
          if (!readNumber(d, pos, x1) || !readNumber(d, pos, y1) || !readNumber(d, pos, x2) || !readNumber(d, pos, y2) || !readNumber(d, pos, x) || !readNumber(d, pos, y)) {
            pos = saved;
            break;
          }
          QPointF p1 = (cmd == 'c') ? cur + QPointF(x1, y1) : QPointF(x1, y1);
          QPointF p2 = (cmd == 'c') ? cur + QPointF(x2, y2) : QPointF(x2, y2);
          QPointF p = (cmd == 'c') ? cur + QPointF(x, y) : QPointF(x, y);
          path.cubicTo(p1, p2, p);
          cur = p;
          lastControl = p2;
        }
      } else if (cmd == 'Q' || cmd == 'q') {
        while (true) {
          saved = pos;
          if (!readNumber(d, pos, x1) || !readNumber(d, pos, y1) || !readNumber(d, pos, x) || !readNumber(d, pos, y)) {
            pos = saved;
            break;
          }
          QPointF p1 = (cmd == 'q') ? cur + QPointF(x1, y1) : QPointF(x1, y1);
          QPointF p = (cmd == 'q') ? cur + QPointF(x, y) : QPointF(x, y);
          path.quadTo(p1, p);
          cur = p;
          lastControl = p1;
        }
      } else {
        qWarning() << "Unsupported SVG path command" << cmd;
        break;
      }
    }
    return path;
  };

  QXmlStreamReader xml(&file);
  static const QRegularExpression idRe(R"(^path_(\d+)$)");
  while (!xml.atEnd()) {
    xml.readNext();
    if (!xml.isStartElement() || xml.name() != "path") continue;
    auto attrs = xml.attributes();
    QString id = attrs.value("id").toString();
    QString d = attrs.value("d").toString();
    auto m = idRe.match(id);
    if (!m.hasMatch() || d.isEmpty()) continue;
    indexed.append({m.captured(1).toInt(), parsePath(d)});
  }
  std::sort(indexed.begin(), indexed.end(), [](const IndexedPath& a, const IndexedPath& b) { return a.index < b.index; });
  QVector<QPainterPath> out;
  for (const auto& p : indexed) out.append(p.path);
  return out;
}

QByteArray QuranPdfWriterPdfHummus::getImageStream(GlyphVis& glyph) {
  QByteArray out;
  QTextStream s(&out, QIODevice::WriteOnly);
  s.setRealNumberPrecision(6);

  if (glyph.m_edge) {
    mp_graphic_object* body = glyph.m_edge->body;
    if (body) {
      s << "/DeviceRGB cs\n";
      QPainterPath foreground;
      foreground.setFillRule(Qt::WindingFill);
      do {
        if (body->type == mp_fill_code) {
          auto fillobject = (mp_fill_object*)body;
          QPainterPath subpath = Glyph::mp_dump_solved_path(fillobject->path_p);
          if (fillobject->color_model == mp_rgb_model) {
            QColor rgba(fillobject->color.a_val * 255, fillobject->color.b_val * 255, fillobject->color.c_val * 255);
            s << rgba.redF() << ' ' << rgba.greenF() << ' ' << rgba.blueF() << " sc\n";
            s.flush();
            out += pathToPdf(subpath, true);
          } else if (fillobject->color_model == mp_no_model) {
            foreground.addPath(subpath);
          }
        }
      } while ((body = body->next));
      s << "0 0 0 sc\n";
      s.flush();
      out += pathToPdf(foreground, true);
    }
  }
  s.flush();
  return out;
}

QByteArray QuranPdfWriterPdfHummus::generateGlyphStream(GlyphVis& glyph) {
  QByteArray out;
  QTextStream s(&out, QIODevice::WriteOnly);
  s.setRealNumberPrecision(6);

  if (glyph.name == "endofaya") {
    if (m_xobjects.contains(glyph.name)) {
      s << glyph.width << " 0 d0\n";
      s << "/Im" << m_xobjects[glyph.name] << " Do\n";
    } else {
      out += QByteArray::number(glyph.width) + " 0 d0\n";
      out += getImageStream(glyph);
    }
  } else if (glyph.charcode >= Automedina::AyaNumberCode && glyph.charcode <= Automedina::AyaNumberCode + 286) {
    int ayaNumber = (glyph.charcode - Automedina::AyaNumberCode) + 1;
    int digitheight = 120;
    auto endofayaIndex = getIndex({m_otlayout->glyphs["endofaya"].charcode, 0, 0});
    GlyphVis& endGlyph = m_otlayout->glyphs["endofaya"];

    s << endGlyph.width << " 0 d0\n";
    s << "BT\n";
    s << "/F" << endofayaIndex.font << " 1000 Tf\n";
    if (m_xobjects.contains("endofaya")) {
      s << "ET\n/Im" << m_xobjects["endofaya"] << " Do\nBT\n";
      s << "/F" << endofayaIndex.font << " 1000 Tf\n";
    } else {
      s << "1 0 0 1 0 0 Tm <" << QString::fromStdString(pdfHexByte((unsigned char)endofayaIndex.encoding)) << "> Tj\n";
    }

    auto drawDigit = [&](int digit, int x) {
      GlyphVis& dg = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + digit]];
      auto idx = getIndex({dg.charcode, 0, 0});
      s << "/F" << idx.font << " 1000 Tf\n";
      s << "1 0 0 1 " << x << ' ' << digitheight << " Tm <"
        << QString::fromStdString(pdfHexByte((unsigned char)idx.encoding)) << "> Tj\n";
    };

    if (ayaNumber < 10) {
      GlyphVis& ones = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + ayaNumber]];
      int x = endGlyph.width / 2 - ones.width / 2;
      drawDigit(ayaNumber, x);
    } else if (ayaNumber < 100) {
      int onesD = ayaNumber % 10, tensD = ayaNumber / 10;
      GlyphVis& ones = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + onesD]];
      GlyphVis& tens = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + tensD]];
      int x = endGlyph.width / 2 - (ones.width + tens.width + 40) / 2;
      drawDigit(tensD, x);
      drawDigit(onesD, x + tens.width + 40);
    } else {
      int onesD = ayaNumber % 10;
      int tensD = (ayaNumber / 10) % 10;
      int hundredsD = ayaNumber / 100;
      GlyphVis& ones = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + onesD]];
      GlyphVis& tens = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + tensD]];
      GlyphVis& hundreds = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + hundredsD]];
      int x = endGlyph.width / 2 - (ones.width + tens.width + hundreds.width + 80) / 2;
      drawDigit(hundredsD, x);
      drawDigit(tensD, x + hundreds.width + 40);
      drawDigit(onesD, x + hundreds.width + tens.width + 80);
    }
    s << "ET\n";
  } else if (glyph.name.startsWith("endofaya")) {
    auto colored = glyph.getColoredGlyph();
    if (colored) {
      out += QByteArray::number(colored->width) + " 0 d0\n";
      out += getImageStream(*colored);
    } else {
      s << glyph.width << " 0 " << glyph.bbox.llx << ' ' << glyph.bbox.lly << ' ' << glyph.bbox.urx << ' ' << glyph.bbox.ury << " d1\n";
      s.flush();
      out += pathToPdf(glyph.path, true);
    }
  } else {
    s << glyph.width << " 0 " << glyph.bbox.llx << ' ' << glyph.bbox.lly << ' ' << glyph.bbox.urx << ' ' << glyph.bbox.ury << " d1\n";
    s.flush();
    out += pathToPdf(glyph.path, true);
  }
  s.flush();
  return out;
}

unsigned long QuranPdfWriterPdfHummus::writeFormXObjectFromGlyph(const QString& name, GlyphVis& glyph) {
  if (m_xobjects.contains(name)) return m_xobjects[name];

  double matrix[6] = {
      1, 0,
      0, 1,
      0, 0};

  PDFFormXObject* xobjectForm = m_writer.StartFormXObject(PDFRectangle(glyph.bbox.llx, glyph.bbox.lly, glyph.bbox.urx, glyph.bbox.ury), matrix);

  unsigned long id = xobjectForm->GetObjectID();

  QByteArray stream = getImageStream(glyph);

  XObjectContentContext* xobjectContentContext = xobjectForm->GetContentContext();

  xobjectContentContext->WriteFreeCode(stream.toStdString());

  m_writer.EndFormXObjectAndRelease(xobjectForm);

  /*
  QByteArray stream = getImageStream(glyph);
  unsigned long id = allocateObjectId();
  QByteArray dict;
  QTextStream d(&dict, QIODevice::WriteOnly);
  d << "/Type /XObject\n/Subtype /Form\n/FormType 1\n/Matrix [1 0 0 1 0 0]\n";
  d << "/BBox [" << glyph.bbox.llx << ' ' << glyph.bbox.lly << ' ' << glyph.bbox.urx << ' ' << glyph.bbox.ury << "]\n";
  d.flush();
  writeIndirectStreamObject(id, dict, stream);*/

  m_xobjects[name] = id;
  return id;
}

unsigned long QuranPdfWriterPdfHummus::createSurahFrameFormXObject() {
  auto path = m_otlayout->font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/surahframe.pdf";
  QPdfWriter writer(outputFileName);

  QSvgRenderer renderer(QString(":/images/surahframe.svg"));

  double pageWidthPts = pageSizePts.width();

  double marginPts = pageWidthPts * (8.5 / 256.0);
  double frameWidth = pageWidthPts - 2 * marginPts;
  double frameHeight = frameWidth * (26.0 / 239);

  QPageSize pageSize{{pageWidthPts, frameHeight}, QPageSize::Point, "MedianQuranBook"};
  double scale = 4800.0 / 72;
  QRectF bbox = QRectF(marginPts * scale, 0, frameWidth * scale, frameHeight * scale);
  QPageLayout pageLayout{pageSize, QPageLayout::Portrait, QMarginsF(0, 0, 0, 0)};
  writer.setPageLayout(pageLayout);
  writer.setResolution(4800 << OtLayout::SCALEBY);

  QPainter painter;

  if (!painter.begin(&writer)) {
    qDebug() << "generateQuranPages : Cannot begin painter device";

    return -1;
  }

  // QPainter painter(&writer);

  renderer.render(&painter, bbox);

  if (!painter.end()) {
    qWarning() << "Painter end failed";
  }

  auto copyingContext = m_writer.CreatePDFCopyingContext(outputFileName.toStdString());

  EStatusCodeAndObjectIDType ret = copyingContext->CreateFormXObjectFromPDFPage(0, EPDFPageBox::ePDFPageBoxMediaBox);

  if (ret.first == EStatusCode::eFailure) {
    qWarning() << "createSurahFrameFormXObject failed";
  }

  return ret.second;
}

bool QuranPdfWriterPdfHummus::writeType3Fonts() {
  auto& glyphs = m_otlayout->glyphs;
  auto& endOfAyaGlyph = glyphs["endofaya"];
  int ayaFont = getIndex({endOfAyaGlyph.charcode, 0, 0}).font;

  auto coloredEnd = endOfAyaGlyph.getColoredGlyph();
  if (coloredEnd)
    writeFormXObjectFromGlyph("endofaya", *coloredEnd);
  else
    writeFormXObjectFromGlyph("endofaya", endOfAyaGlyph);

  for (auto it = m_type3Fonts.begin(); it != m_type3Fonts.end(); ++it) {
    int fontObjectID = it.key();
    Type3FontData& fontData = it.value();
    bool isSurahFont = (fontObjectID == m_surahType3Font);

    unsigned long charProcsID = allocateObjectId();
    unsigned long encodingID = allocateObjectId();
    unsigned long resourcesID = allocateObjectId();
    unsigned long descriptorID = allocateObjectId();

    QByteArray widths;
    QByteArray encoding;
    QByteArray charProcs;
    QTextStream w(&widths, QIODevice::WriteOnly);
    QTextStream e(&encoding, QIODevice::WriteOnly);
    QTextStream cp(&charProcs, QIODevice::WriteOnly);

    w << "[";
    e << "<< /Type /Encoding\n/Differences [0";
    cp << "<<";

    double maxwidth = 0;
    qreal llx = INT_MAX, lly = INT_MAX, urx = INT_MIN, ury = INT_MIN;

    for (int i = 0; i < fontData.glyphs.size(); ++i) {
      GlyphKey glyphCode = fontData.glyphs[i];
      QString glyphName;
      QByteArray glyphStream;
      double glyphWidth = 0;
      QRectF box;

      if (!isSurahFont) {
        glyphName = m_otlayout->glyphNamePerCode[glyphCode.code];
        GlyphVis* glyph = &glyphs[glyphName];
        if (glyphCode.lefttatweel != 0 || glyphCode.righttatweel != 0) {
          GlyphParameters params{};
          params.lefttatweel = glyphCode.lefttatweel;
          params.righttatweel = glyphCode.righttatweel;
          glyph = glyph->getAlternate(params);
          glyphName = QString("%1%2%3").arg(glyphName).arg(params.lefttatweel).arg(params.righttatweel);
        }
        glyphWidth = glyph->width;
        box = QRectF(glyph->bbox.llx, glyph->bbox.lly, glyph->bbox.urx - glyph->bbox.llx, glyph->bbox.ury - glyph->bbox.lly);
        glyphStream = generateGlyphStream(*glyph);
      } else {
        glyphName = QString("surah_%1").arg(glyphCode.code);
        const GlyphPath& surah = m_surahs[i];
        box = surah.path.boundingRect();
        glyphWidth = std::ceil(box.width());

        QByteArray local;
        QTextStream gs(&local, QIODevice::WriteOnly);
        gs << glyphWidth << " 0 " << std::ceil(box.left()) << ' ' << std::ceil(box.top()) << ' '
           << std::ceil(box.right()) << ' ' << std::ceil(box.bottom()) << " d1\n";
        gs.flush();
        local += pathToPdf(surah.path, true);
        glyphStream = local;
      }

      w << glyphWidth << ' ';
      e << " /" << QString::fromStdString(pdfName(glyphName));
      maxwidth = std::max(maxwidth, glyphWidth);
      llx = std::min(llx, box.left());
      lly = std::min(lly, box.top());
      urx = std::max(urx, box.right());
      ury = std::max(ury, box.bottom());

      unsigned long glyphObj = allocateObjectId();
      cp << " /" << QString::fromStdString(pdfName(glyphName)) << ' ' << glyphObj << " 0 R";
      writeGlyphStream(glyphObj, glyphStream);
    }

    w << "]";
    e << "]\n>>\n";
    cp << "\n>>\n";

    w.flush();
    e.flush();
    cp.flush();

    writeIndirectDictionaryObject(charProcsID, charProcs);
    writeIndirectDictionaryObject(encodingID, encoding);

    QByteArray res;
    QTextStream rs(&res, QIODevice::WriteOnly);
    rs << "<<\n";
    if (fontObjectID != ayaFont) {
      rs << "/Font << /F" << ayaFont << ' ' << ayaFont << " 0 R >>\n";
    }
    rs << "/XObject <<\n";
    for (auto xit = m_xobjects.begin(); xit != m_xobjects.end(); ++xit) {
      rs << "/Im" << xit.value() << ' ' << xit.value() << " 0 R\n";
    }
    rs << ">>\n>>\n";
    rs.flush();
    writeIndirectDictionaryObject(resourcesID, res);

    QByteArray desc;
    QTextStream ds(&desc, QIODevice::WriteOnly);
    ds.setRealNumberNotation(QTextStream::FixedNotation);
    ds << "<< /Type /FontDescriptor\n";
    ds << "/FontName /quranmedinafont" << fontObjectID << "\n";
    ds << "/FontFamily (QuranMedinaFont)\n/FontStretch /Normal\n/FontWeight 400\n/Flags 12\n";
    ds << "/FontBBox [" << llx << ' ' << lly << ' ' << urx << ' ' << ury << "]\n";
    ds << "/ItalicAngle 0\n/Ascent 1000\n/Descent 200\n/Leading 1750\n/AvgWidth 500\n/MaxWidth " << int(maxwidth) << "\n>>\n";
    ds.flush();
    writeIndirectDictionaryObject(descriptorID, desc);

    QByteArray fontDict;
    QTextStream fd(&fontDict, QIODevice::WriteOnly);
    fd.setRealNumberNotation(QTextStream::FixedNotation);
    fd << "<< /Type /Font\n/Subtype /Type3\n";
    fd << "/Encoding " << encodingID << " 0 R\n";
    fd << "/CharProcs " << charProcsID << " 0 R\n";
    fd << "/Resources " << resourcesID << " 0 R\n";
    fd << "/FontDescriptor " << descriptorID << " 0 R\n";
    fd << "/Name /quranmedinafont" << fontObjectID << "\n";
    fd << "/FirstChar 0\n/LastChar " << (fontData.glyphs.size() - 1) << "\n";
    fd << "/FontMatrix [0.001 0 0 0.001 0 0]\n";
    fd << "/FontBBox [" << llx << ' ' << lly << ' ' << urx << ' ' << ury << "]\n";
    fd << "/Widths " << widths << "\n>>\n";
    fd.flush();
    writeIndirectDictionaryObject(fontObjectID, fontDict);
  }
  return true;
}

static QString findFontPath(const QString& fontFamilyName) {
  QStringList fontDirs;  // = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
  fontDirs << "/Library/Fonts"
           << "/System/Library/Fonts"
           << "/System/Library/Fonts/Supplemental";

  for (const QString& dir : fontDirs) {
    // Recursively scan directories for common font files
    QDirIterator it(dir, QStringList() << "*.ttf" << "*.otf" << "*.ttc", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString filePath = it.next();

      if (filePath.contains(fontFamilyName))
        return filePath;

      // Load the font file into a temporary ID to verify its family name
      /*int id = QFontDatabase::addApplicationFont(filePath);
      if (id != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (families.contains(fontFamilyName, Qt::CaseInsensitive)) {
          return filePath;  // Found the file matching your font family name
        }
      }*/
    }
  }
  return QString();  // Not found
}

bool QuranPdfWriterPdfHummus::writeNoticePage() {
  QString fontFilePath = findFontPath("Arial.ttf");

  // Note: If rawFont doesn't expose the direct path string in your Qt build flavor,
  // the safest fallback using Qt's resource loop is checking the family compatibility:
  if (fontFilePath.isEmpty()) {
    qWarning() << "Could not resolve physical font file via Qt!";
    return -1;
  }

  QString fontFileBoldPath = findFontPath("Arial Bold.ttf");

  // Note: If rawFont doesn't expose the direct path string in your Qt build flavor,
  // the safest fallback using Qt's resource loop is checking the family compatibility:
  if (fontFileBoldPath.isEmpty()) {
    qWarning() << "Could not resolve physical bold font file via Qt!";
    return -1;
  }

  QSize pageSizePts = this->pageSizePts;
  //.transposed();

  PDFPage* page = new PDFPage();

  page->SetMediaBox(PDFRectangle(0, 0, pageSizePts.width(), pageSizePts.height()));
  PageContentContext* ctx = m_writer.StartPageContentContext(page);

  PageContentContext* contentContext = m_writer.StartPageContentContext(page);

  // Use it directly in PDF-Writer
  PDFUsedFont* regularFont = m_writer.GetFontForFile(fontFilePath.toStdString());
  PDFUsedFont* boldFont = m_writer.GetFontForFile(fontFileBoldPath.toStdString());

  std::string text = "Important Notice";
  double fontSize = 16.0;

  auto textDimensions = boldFont->CalculateTextDimensions(text, fontSize);

  double top = pageSizePts.height();

  double currentY = top - textDimensions.height - 20;

  AbstractContentContext::TextOptions textOptions(boldFont, fontSize, AbstractContentContext::eRGB, 0);
  contentContext->WriteText((pageSizePts.width() - textDimensions.width) / 2, currentY, text, textOptions);

  currentY -= 10;

  PDFRectangle urlLinkBox;

  QStringList lines;
  lines << "This mushaf is an experimental development version produced as part";
  lines << "of the DigitalKhatt project and is not yet considered a final release.";
  lines << "The text, typography, and layout are still undergoing review and refinement.";
  lines << "Consequently, this edition may contain errors and may be updated periodically.";
  lines << "For the latest version, updates, and corrections, or to report an issue,";
  lines << "please consult the official project page:";
  // lines << "please always consult the official source:";
  lines << m_options.officialUrl;
  lines << "";
  lines << QString("Generation Date: %1").arg(m_generationDate);
  // lines << "";
  // lines << "To help ensure that readers have access to the latest corrections and";
  // lines << "improvements, please share the official link rather than copies of this mushaf.";

  for (const QString& line : lines) {
    text = line.toStdString();
    fontSize = 6;
    textDimensions = regularFont->CalculateTextDimensions(text, fontSize);
    bool isOfficialUrl = line == m_options.officialUrl;
    if (isOfficialUrl) {
      textOptions = AbstractContentContext::TextOptions(regularFont, fontSize, AbstractContentContext::eRGB, 0x0000FF);
    } else {
      textOptions = AbstractContentContext::TextOptions(regularFont, fontSize);
    }

    double xPos = (pageSizePts.width() - textDimensions.width) / 2;
    currentY = currentY - textDimensions.height - 10;
    contentContext->WriteText(xPos, currentY, text, textOptions);

    if (isOfficialUrl) {
      urlLinkBox = PDFRectangle(xPos + textDimensions.xMin, currentY + textDimensions.yMin, xPos + textDimensions.xMax, currentY + textDimensions.yMax);
    }
  }

  m_writer.EndPageContentContext(contentContext);

  m_writer.AttachURLLinktoCurrentPage(m_options.officialUrl.toStdString(), urlLinkBox);

  auto ret = m_writer.WritePageReleaseAndReturnPageID(page);

  pageIds.push_back(ret.second);

  double outlineDestX = 0;
  double outlineDestY = pageSizePts.height();

  m_outlineItems.append(PdfOutlineItem{"Important Notice",
                                       0,
                                       outlineDestX,
                                       outlineDestY,
                                       0});

  return ret.first == eSuccess;
}

void QuranPdfWriterPdfHummus::writeFooter(PageContentContext* ctx, double pageW, double pageH, const QString& version) {
  QString text = QString("Official latest version: %1 | %2").arg(m_options.officialUrl, version);
  raw(ctx, "q\n0 0 0 rg\n/GSfooter gs\nBT\n/F1 220 Tf\n");
  raw(ctx, "1 0 0 1 " + num(pageW * 0.17) + " " + num(pageH * 0.015) + " Tm " + pdfStringLiteral(text) + " Tj\n");
  raw(ctx, "ET\nQ\n");
}

void QuranPdfWriterPdfHummus::drawSajdaRule(PageContentContext* ctx, const QPoint& begin, const QPoint& end) {
  int y = end.y() - (1100 << OtLayout::SCALEBY);
  int penWidth = 50 << OtLayout::SCALEBY;
  raw(ctx, "q\n" + std::to_string(penWidth) + " w\n");
  raw(ctx, std::to_string(begin.x()) + " " + std::to_string(y) + " m\n");
  raw(ctx, std::to_string(end.x()) + " " + std::to_string(y) + " l\nS\nQ\n");
}

void QuranPdfWriterPdfHummus::writeCatalogPageLabels() {
  // PDF-Writer does not have a first-class page-label helper. The cleanest
  // production path is pikepdf/qpdf post-processing. This placeholder is left
  // here deliberately; do not call it unless you extend DocumentContext catalog.
  // Desired catalog object entry:
  // /PageLabels << /Nums [ 0 << /S /r /St 1 >> 1 << /S /D /St 1 >> ] >>
}

bool QuranPdfWriterPdfHummus::generateQuranPages(QList<QList<LineLayoutInfo>> pages,
                                                 int lineWidth,
                                                 QList<QStringList> originalText,
                                                 double scale,
                                                 int margin) {
  Q_UNUSED(scale);
  if (!m_otlayout) {
    qWarning() << "QuranPdfWriterPdfHummus: m_otlayout is null";
    return false;
  }
  if (!startPdf()) return false;

  seedSpecialGlyphs();

  if (m_options.surahNameType == 2) {
    generateSurahFontFromSvg(loadPathsFromSvg(m_options.surahSvgPath));
  } else if (m_options.surahNameType == 1) {
    generateSurahFont();
  }

  unsigned long suraFrameObjectId = createSurahFrameFormXObject();

  // First pass: enumerate all glyphs used in pages so Type3 fonts are complete
  // before page resources refer to them.
  for (const auto& page : pages) {
    for (const auto& line : page) {
      for (const auto& g : line.glyphs)
        getIndex({g.codepoint, g.lefttatweel, g.righttatweel});
    }
  }
  writeType3Fonts();

  if (m_options.addNoticePage && !writeNoticePage())
    return false;

  int suraNumber = 0;

  for (int p = 0; p < pages.size(); ++p) {
    PDFPage* pdfPage = new PDFPage();

    pdfPage->SetMediaBox(PDFRectangle(0, 0, pageSizePts.width(), pageSizePts.height()));

    PageContentContext* ctx = m_writer.StartPageContentContext(pdfPage);

    qreal lscale = 72. / resolution;
    ctx->cm(lscale, 0, 0, -lscale, 0, pageSizePts.height());

    const auto page = pages.at(p);
    const auto originalPage = originalText.at(p);

    // Ensure default font resource for notice/footer if you add it with real Helvetica.
    // In production, map /F1 to a Type1 font or use PDF-Writer text API.

    raw(ctx, "BT\n");
    int currentFont = 0;
    QPoint beginsajda;
    QPoint endsajda;

    for (int l = 0; l < page.size(); ++l) {
      const auto line = page.at(l);
      const auto originalLine = originalPage.at(l);

      hb_position_t currentxPos = lineWidth + margin - line.xstartposition;
      hb_position_t currentyPos = line.ystartposition;
      qreal pixelSize = 1000 * line.fontSize;
      QPoint lastPos{currentxPos, currentyPos};
      int end = -1;
      int MCID = 0;

      if (line.type == LineType::Sura) {
        suraNumber++;
        int ayaFrameyPos = currentyPos - (1100 << OtLayout::SCALEBY);
        raw(ctx, "ET\nq\n");
        ctx->cm(4800.0 / 72, 0, 0, 4800.0 / 72, 0, ayaFrameyPos);
        QString frameName = ensureFormResource(pdfPage, suraFrameObjectId);
        raw(ctx, "/" + toUtf8(frameName) + " Do\nQ\nBT\n");

        double outlineDestX = 0;
        double outlineDestY = pageSizePts.height() - (72.0 / resolution) * ayaFrameyPos;

        m_outlineItems.append(PdfOutlineItem{QString("%1 (%2)").arg(originalLine).arg(suraNumber),
                                             pageIds.size(),
                                             outlineDestX,
                                             outlineDestY,
                                             0});

        if (m_options.surahNameType == 1) {
          currentFont = m_surahType3Font;
          QString fName = ensureFontResource(pdfPage, currentFont);
          int fontSize = 1100;
          double sc = fontSize / 1000.0;
          int unicode = m_surahNumberToUnicode[suraNumber];
          int encoding = unicode - 0xE900;

          int surahWordWidth = (m_surahs.size() > 3 ? m_surahs[3].path.boundingRect().width() : 0) * sc;
          int surahNameWidth = (encoding >= 0 && encoding < m_surahs.size() ? m_surahs[encoding].path.boundingRect().width() : 0) * sc;
          int spaceBetween = 100;
          int startX = (17000 - (surahNameWidth + spaceBetween + surahWordWidth)) / 2;

          raw(ctx, "1 0 0 1 0 " + std::to_string(lastPos.y() - 300) + " Tm\n");
          raw(ctx, "/" + toUtf8(fName) + " " + std::to_string(fontSize) + " Tf\n");
          raw(ctx, std::to_string(startX) + " 0 Td <" + pdfHexByte((unsigned char)encoding) + "> Tj\n");
          raw(ctx, std::to_string(surahNameWidth + spaceBetween) + " 0 Td <" + pdfHexByte((unsigned char)3) + "> Tj\n");
          continue;
        }

        if (m_options.surahNameType == 2) {
          currentFont = m_surahType3Font;
          QString fName = ensureFontResource(pdfPage, currentFont);
          int encoding = suraNumber - 1;
          int fontSize = (encoding == 0 || encoding == 1) ? 700 : 600;
          double sc = fontSize / 1000.0;
          int surahNameWidth = (encoding >= 0 && encoding < m_surahs.size() ? m_surahs[encoding].path.boundingRect().width() : 0) * sc;
          int startX = (17000 - surahNameWidth) / 2;
          raw(ctx, "1 0 0 1 0 " + std::to_string(lastPos.y() - 300) + " Tm\n");
          raw(ctx, "/" + toUtf8(fName) + " " + std::to_string(fontSize) + " Tf\n");
          raw(ctx, std::to_string(startX) + " 0 Td <" + pdfHexByte((unsigned char)encoding) + "> Tj\n");
          continue;
        }

        raw(ctx, "1 0 0 -1 " + std::to_string(lastPos.x()) + " " + std::to_string(lastPos.y()) + " Tm\n");
      } else if (line.type == LineType::Bism) {
        raw(ctx, "1 0 0 -1 " + std::to_string(lastPos.x()) + " " + std::to_string(lastPos.y()) + " Tm\n");
      } else {
        raw(ctx, std::to_string(line.xscale) + " 0 0 -1 " + std::to_string(lastPos.x()) + " " + std::to_string(lastPos.y()) + " Tm\n");
      }

      raw(ctx, "/P << /MCID " + std::to_string(MCID) + " >>\nBDC\n");

      for (int i = 0; i < line.glyphs.size(); ++i) {
        auto index = getIndex({line.glyphs[i].codepoint, line.glyphs[i].lefttatweel, line.glyphs[i].righttatweel});
        if (index.font != currentFont) {
          currentFont = index.font;
          QString fName = ensureFontResource(pdfPage, currentFont);
          raw(ctx, "/" + toUtf8(fName) + " " + std::to_string(pixelSize) + " Tf\n");
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
              ushort u = originalLine[endcluster].unicode();
              if (u == 0x06E5 || u == 0x06E6 || u == 0x0640) {
                end++;
                currentcluster = endcluster;
                continue;
              }
              QString currGlyphName = m_otlayout->glyphNamePerCode[line.glyphs[i].codepoint];
              QString nextGlyphName = m_otlayout->glyphNamePerCode[line.glyphs[end + 1].codepoint];
              if (currGlyphName == "reh.isol" && nextGlyphName == "behshape.init.beforereh") {
                end++;
                currentcluster = endcluster;
                continue;
              }
            } else {
              endcluster = originalLine.size();
            }
            break;
          } while (true);

          QString actualText = originalLine.mid(line.glyphs[i].cluster, endcluster - line.glyphs[i].cluster);
          raw(ctx, "/Span << /ActualText <" + pdfHexUtf16BE(actualText) + "> >>\nBDC\n");
        }

        if (line.glyphs[i].color && line.type != LineType::Sura) {
          int color = line.glyphs[i].color;
          double r = ((color >> 24) & 0xff) / 255.0;
          double g = ((color >> 16) & 0xff) / 255.0;
          double b = ((color >> 8) & 0xff) / 255.0;
          raw(ctx, num(r) + " " + num(g) + " " + num(b) + " rg\n");
        }

        currentxPos -= line.glyphs[i].x_advance;
        QPoint pos;
        pos.setX(currentxPos + line.glyphs[i].x_offset);
        pos.setY(currentyPos - line.glyphs[i].y_offset);

        if (line.glyphs[i].beginsajda)
          beginsajda = {lastPos.x(), currentyPos};
        else if (line.glyphs[i].endsajda)
          endsajda = {pos.x(), currentyPos};

        QPoint diff = pos - lastPos;
        lastPos = pos;
        raw(ctx, std::to_string(diff.x()) + " " + std::to_string(-diff.y()) + " Td <" + pdfHexByte((unsigned char)index.encoding) + "> Tj\n");

        if (line.glyphs[i].color)
          raw(ctx, "0 0 0 rg\n");
        if (i == end)
          raw(ctx, "EMC\n");
      }

      raw(ctx, "EMC\n");

      if (!beginsajda.isNull() && !endsajda.isNull()) {
        if (beginsajda.y() != endsajda.y()) {
          qDebug() << "Sajda Rule not in the same line";
          beginsajda.setX(lineWidth + margin - line.xstartposition);
        }
        raw(ctx, "ET\n");
        drawSajdaRule(ctx, beginsajda, endsajda);
        beginsajda = QPoint();
        endsajda = QPoint();
        raw(ctx, "BT\n");
      }
    }

    raw(ctx, "ET\n");
    if (m_options.addFooter)
      writeFooter(ctx, pageSizePxs.width(), pageSizePxs.height(), m_generationDate);

    m_writer.EndPageContentContext(ctx);

    auto ret = m_writer.WritePageReleaseAndReturnPageID(pdfPage);

    pageIds.push_back(ret.second);

    if (ret.first != eSuccess) {
      qWarning() << "WritePageAndRelease failed for page" << p;
      return false;
    }
  }

  return finishPdf();
}

static QByteArray pdfUtf16BEHexString(const QString& s) {
  QByteArray out;
  out += "<FEFF";

  for (int i = 0; i < s.size(); ++i) {
    ushort u = s.at(i).unicode();

    char buf[5];
    snprintf(buf, sizeof(buf), "%04X", u);
    out += buf;
  }

  out += ">";
  return out;
}
static std::string toUtf16BERawBytes(const QString& s) {
  std::string bytes;

  // BOM
  bytes.push_back(char(0xFE));
  bytes.push_back(char(0xFF));

  for (int i = 0; i < s.size(); ++i) {
    ushort u = s.at(i).unicode();

    bytes.push_back(char((u >> 8) & 0xFF));
    bytes.push_back(char(u & 0xFF));
  }

  return bytes;
}

ObjectIDType QuranPdfWriterPdfHummus::writePageLabels() {
  if (!m_options.addNoticePage)
    return -1;

  ObjectsContext& objCtx = m_writer.GetObjectsContext();

  ObjectIDType pageLabelsId = objCtx.GetInDirectObjectsRegistry().AllocateNewObjectID();

  objCtx.StartNewIndirectObject(pageLabelsId);

  DictionaryContext* dict =
      objCtx.StartDictionary();

  dict->WriteKey("Nums");

  objCtx.StartArray();

  // page 0 -> i
  objCtx.WriteInteger(0);

  DictionaryContext* roman =
      objCtx.StartDictionary();

  roman->WriteKey("S");
  objCtx.WriteName("r");

  roman->WriteKey("St");
  objCtx.WriteInteger(1);

  objCtx.EndDictionary(roman);

  // page 1 -> 1
  objCtx.WriteInteger(1);

  DictionaryContext* decimal =
      objCtx.StartDictionary();

  decimal->WriteKey("S");
  objCtx.WriteName("D");

  decimal->WriteKey("St");
  objCtx.WriteInteger(1);

  objCtx.EndDictionary(decimal);

  objCtx.EndArray();

  objCtx.EndDictionary(dict);
  objCtx.EndIndirectObject();

  return pageLabelsId;
}

ObjectIDType QuranPdfWriterPdfHummus::writeOutlines(const QVector<PdfOutlineItem>& items) {
  ObjectsContext& objCtx = m_writer.GetObjectsContext();

  ObjectIDType outlinesRootId = objCtx.GetInDirectObjectsRegistry()
                                    .AllocateNewObjectID();

  QVector<ObjectIDType> itemIds;
  for (int i = 0; i < items.size(); ++i) {
    itemIds.push_back(
        objCtx.GetInDirectObjectsRegistry().AllocateNewObjectID());
  }

  // Root outline object
  objCtx.StartNewIndirectObject(outlinesRootId);
  DictionaryContext* root = objCtx.StartDictionary();

  root->WriteKey("Type");
  objCtx.WriteName("Outlines");

  if (!itemIds.isEmpty()) {
    root->WriteKey("First");
    objCtx.WriteIndirectObjectReference(itemIds.first());

    root->WriteKey("Last");
    objCtx.WriteIndirectObjectReference(itemIds.last());

    root->WriteKey("Count");
    objCtx.WriteInteger(items.size());
  }

  objCtx.EndDictionary(root);
  objCtx.EndIndirectObject();

  // Child outline entries
  for (int i = 0; i < items.size(); ++i) {
    objCtx.StartNewIndirectObject(itemIds[i]);

    DictionaryContext* d = objCtx.StartDictionary();

    d->WriteKey("Title");
    /*QByteArray titleHex = pdfUtf16BEHexString(items[i].title);

    IByteWriterWithPosition* w = objCtx.StartFreeContext();
    w->Write(reinterpret_cast<const Byte*>(titleHex.constData()), titleHex.size());
    objCtx.EndFreeContext();*/

    std::string titleBytes = toUtf16BERawBytes(items[i].title);

    d->WriteHexStringValue(titleBytes);

    d->WriteKey("Parent");
    objCtx.WriteIndirectObjectReference(outlinesRootId);

    if (i > 0) {
      d->WriteKey("Prev");
      objCtx.WriteIndirectObjectReference(itemIds[i - 1]);
    }

    if (i + 1 < items.size()) {
      d->WriteKey("Next");
      objCtx.WriteIndirectObjectReference(itemIds[i + 1]);
    }

    d->WriteKey("Dest");
    objCtx.StartArray();

    objCtx.WriteIndirectObjectReference(pageIds[items[i].pageIndex]);
    objCtx.WriteName("XYZ");
    objCtx.WriteDouble(items[i].x);
    objCtx.WriteDouble(items[i].y);
    objCtx.WriteDouble(items[i].zoom);

    objCtx.EndArray();

    objCtx.EndDictionary(d);
    objCtx.EndIndirectObject();
  }

  return outlinesRootId;
}
