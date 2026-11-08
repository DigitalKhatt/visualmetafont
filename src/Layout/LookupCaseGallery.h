#pragma once
#include <QAbstractScrollArea>
#include <QCache>
#include <QPainterPath>
#include <QSet>
#include <QTimer>
#include <functional>
#include <limits>

struct LookupCaseThumbnail {
  QString label, error;
  QPainterPath path;
  bool applied = true;
  QSet<QString> glyphDependencies;
};

// Virtual gallery: even "All" retains only visible previews and a bounded cache.
class LookupCaseGallery : public QAbstractScrollArea {
  Q_OBJECT
 public:
  using Renderer = std::function<LookupCaseThumbnail(quint64)>;
  explicit LookupCaseGallery(QWidget* parent = nullptr);
  void setCases(quint64 count, Renderer renderer);
  void invalidatePreviews();
  void invalidateGlyph(const QString& glyph);
  quint64 count() const { return count_; }
  quint64 currentIndex() const { return current_; }
  void setCurrentIndex(quint64 index);
  int cachedPreviewCount() const { return cache_.size(); }
  bool visiblePreviewsReady() const;
 signals:
  void currentChanged(quint64 index);

 protected:
  void paintEvent(QPaintEvent*) override;
  void resizeEvent(QResizeEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void keyPressEvent(QKeyEvent*) override;
  void wheelEvent(QWheelEvent*) override;

 private:
  void updateScrollRange();
  void setFirstRow(quint64 row);
  void renderPending();
  bool visible(quint64 index) const;
  quint64 rowCount() const;
  int visibleRows() const;
  int cellWidth() const;
  static constexpr quint64 Invalid = std::numeric_limits<quint64>::max();
  static constexpr int TileWidth = 240, TileHeight = 190;
  quint64 count_ = 0, current_ = Invalid, firstRow_ = 0;
  int columns_ = 1;
  bool syncingScroll_ = false;
  Renderer renderer_;
  QCache<quint64, LookupCaseThumbnail> cache_{16 * 1024 * 1024};
  QSet<quint64> pending_, stale_;
  QTimer renderTimer_;
};
