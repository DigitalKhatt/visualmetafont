#pragma once

#include <QHash>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPainterPath>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QVector>
#include <unordered_map>

#include "GlyphVis.h"
#include "OtLayout.h"

// PDF-Writer / PDFHummus headers
#include "DictionaryContext.h"
#include "EStatusCode.h"
#include "PDFPage.h"
#include "PDFRectangle.h"
#include "PDFWriter.h"
#include "qpagesize.h"

class PageContentContext;
class PDFFormXObject;

class CatalogOutlineExtender : public DocumentContextExtenderAdapter {
 public:
  explicit CatalogOutlineExtender(ObjectIDType outlinesId, ObjectIDType pageLabelsId)
      : m_outlinesId(outlinesId), m_pageLabelsId(pageLabelsId) {}

  PDFHummus::EStatusCode OnCatalogWrite(
      CatalogInformation*,
      DictionaryContext* catalogDictionaryContext,
      ObjectsContext* objectsContext,
      PDFHummus::DocumentContext*) override {
    catalogDictionaryContext->WriteKey("Outlines");
    objectsContext->WriteIndirectObjectReference(m_outlinesId);

    catalogDictionaryContext->WriteKey("PageMode");
    objectsContext->WriteName("UseOutlines");

    if (m_pageLabelsId != -1) {
      catalogDictionaryContext->WriteKey("PageLabels");
      objectsContext->WriteIndirectObjectReference(m_pageLabelsId);
    }

    return PDFHummus::EStatusCode::eSuccess;
  }

 private:
  ObjectIDType m_outlinesId;
  ObjectIDType m_pageLabelsId;
};

class QuranPdfWriterPdfHummus : public QObject {
  Q_OBJECT
 public:
  struct Options {
    QString title = QStringLiteral("The Noble Quran - القرآن الكريم");
    QString creator = QStringLiteral("DigitalKhatt");
    QString officialUrl = QStringLiteral("https://github.com/DigitalKhatt/oldmadinafont");
    QString outputPath;

    int dpi = 4800;
    QPageSize pageSize;

    double userUnitScale = 1.0;  // keep old Qt high-res coordinates

    bool addNoticePage = true;
    bool addFooter = false;
    bool drawRomanIOnNotice = true;

    int surahNameType = 1;                        // 1 = generated surah font, 2 = SVG paths
    QString surahSvgPath = ":/fonts/surahs.svg";  // only used when surahNameType == 2
  };

  explicit QuranPdfWriterPdfHummus(const Options& options,
                                   OtLayout* otlayout,
                                   QObject* parent = nullptr);
  ~QuranPdfWriterPdfHummus() override;

  void setOptions(const Options& options) { m_options = options; }
  const Options& options() const { return m_options; }

  bool generateQuranPages(QList<QList<LineLayoutInfo>> pages,
                          int lineWidth,
                          QList<QStringList> originalText,
                          double scale,
                          int margin = 400 << OtLayout::SCALEBY);

 private:
  struct GlyphKey {
    int code = 0;
    double lefttatweel = 0;
    double righttatweel = 0;
    bool operator==(const GlyphKey& r) const {
      return code == r.code && lefttatweel == r.lefttatweel && righttatweel == r.righttatweel;
    }
  };

  struct GlyphIndex {
    int font = 0;      // Type3 font object id
    int encoding = 0;  // 0..255
  };

  struct Type3FontData {
    QVector<GlyphKey> glyphs;
    QString resourceName;  // /Fxxxx
  };

  struct GlyphPath {
    uint codepoint = 0;
    quint32 glyphIndex = 0;
    QPainterPath path;
  };

  struct PdfObjectRef {
    unsigned long objectID = 0;
    QString resourceName;
  };

  struct HashGlyphKey {
    std::size_t operator()(const GlyphKey& k) const {
      std::size_t h1 = std::hash<int>{}(k.code);
      std::size_t h2 = std::hash<double>{}(k.lefttatweel);
      std::size_t h3 = std::hash<double>{}(k.righttatweel);
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };

  struct PdfOutlineItem {
    QString title;
    int pageIndex;
    double x = 0;
    double y = 0;
    double zoom = 0;  // 0 means keep current zoom
  };

  static std::string toUtf8(const QString& s);
  static std::string pdfName(const QString& name);
  static std::string pdfStringLiteral(const QString& s);
  static std::string pdfHexByte(unsigned char v);
  static std::string pdfHexUShort(ushort v);
  static std::string pdfHexUtf16BE(const QString& text);
  static std::string num(double v);
  static QByteArray pathToPdf(const QPainterPath& path, bool fill = true);

  bool startPdf();
  bool finishPdf();
  bool writeNoticePage();
  void writeFooter(PageContentContext* ctx, double pageW, double pageH, const QString& version);
  void attachUrlToCurrentPage(double x1, double y1, double x2, double y2);

  void seedSpecialGlyphs();
  GlyphIndex getIndex(const GlyphKey& glyphCode);
  QString ensureFontResource(PDFPage* page, int fontObjectId);
  QString ensureFormResource(PDFPage* page, unsigned long formObjectId);

  void generateSurahFont();
  void generateSurahFontFromSvg(const QVector<QPainterPath>& paths);
  QVector<QPainterPath> loadPathsFromSvg(const QString& fileName) const;

  bool writeType3Fonts();
  QByteArray generateGlyphStream(GlyphVis& glyph);
  QByteArray getImageStream(GlyphVis& glyph);
  unsigned long writeFormXObjectFromGlyph(const QString& name, GlyphVis& glyph);
  unsigned long createSurahFrameFormXObject();

  // Low-level object helpers. These are intentionally concentrated here so
  // adapting to a PDF-Writer version difference is localized.
  unsigned long allocateObjectId();
  void writeIndirectDictionaryObject(unsigned long objectID, const QByteArray& dictionaryBody);
  void writeGlyphStream(unsigned long objectID, const QByteArray& streamBody);
  void writeCatalogPageLabels();

  void raw(PageContentContext* ctx, const std::string& s);
  void drawSajdaRule(PageContentContext* ctx, const QPoint& begin, const QPoint& end);

  ObjectIDType writeOutlines(const QVector<PdfOutlineItem>& items);
  ObjectIDType writePageLabels();

 private:
  Options m_options;
  OtLayout* m_otlayout = nullptr;
  PDFWriter m_writer;
  bool m_started = false;

  int m_currentType3Font = -1;
  int m_surahType3Font = -1;
  QMap<int, Type3FontData> m_type3Fonts;
  std::unordered_map<GlyphKey, GlyphIndex, HashGlyphKey> m_glyphsToIndex;
  QVector<GlyphPath> m_surahs;
  std::unordered_map<int, int> m_surahNumberToUnicode;
  QMap<QString, unsigned long> m_xobjects;

  QString m_generationDate;
  QSize pageSizePts;
  QSize pageSizePxs;
  int resolution;
  QVector<PdfOutlineItem> m_outlineItems;
  QVector<ObjectIDType> pageIds;
};
