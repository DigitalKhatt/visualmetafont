#include "LookupCaseGallery.h"

#include <QtWidgets>
#include <algorithm>
#include <climits>
#include <cmath>

LookupCaseGallery::LookupCaseGallery(QWidget* parent) : QAbstractScrollArea(parent) {
  setObjectName("patternCaseGallery");
  setFocusPolicy(Qt::StrongFocus);
  setMinimumWidth(300);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  viewport()->setAutoFillBackground(true);
  auto palette = viewport()->palette();
  palette.setColor(QPalette::Window, Qt::white);
  viewport()->setPalette(palette);
  renderTimer_.setSingleShot(true);
  connect(&renderTimer_, &QTimer::timeout, this, &LookupCaseGallery::renderPending);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
    if (syncingScroll_) return;
    const auto maximum = rowCount() > quint64(visibleRows()) ? rowCount() - visibleRows() : 0;
    firstRow_ = maximum <= INT_MAX ? quint64(value)
                : value == INT_MAX ? maximum
                                   : quint64(static_cast<long double>(value) * maximum / INT_MAX);
    pending_.clear();
    viewport()->update();
  });
}
int LookupCaseGallery::cellWidth() const { return std::max(1, viewport()->width() / columns_); }
int LookupCaseGallery::visibleRows() const {
  return std::max(1, viewport()->height() / TileHeight);
}
quint64 LookupCaseGallery::rowCount() const { return count_ ? (count_ - 1) / columns_ + 1 : 0; }
void LookupCaseGallery::updateScrollRange() {
  const auto maximum = rowCount() > quint64(visibleRows()) ? rowCount() - visibleRows() : 0;
  firstRow_ = std::min(firstRow_, maximum);
  syncingScroll_ = true;
  verticalScrollBar()->setRange(0, int(std::min<quint64>(maximum, INT_MAX)));
  verticalScrollBar()->setPageStep(visibleRows());
  verticalScrollBar()->setValue(maximum <= INT_MAX
                                    ? int(firstRow_)
                                    : int(static_cast<long double>(firstRow_) * INT_MAX / maximum));
  syncingScroll_ = false;
}
void LookupCaseGallery::setCases(quint64 count, Renderer renderer) {
  renderTimer_.stop();
  renderer_ = std::move(renderer);
  count_ = count;
  current_ = Invalid;
  firstRow_ = 0;
  cache_.clear();
  stale_.clear();
  pending_.clear();
  updateScrollRange();
  viewport()->update();
  if (count_) setCurrentIndex(0);
}
void LookupCaseGallery::invalidatePreviews() {
  cache_.clear();
  stale_.clear();
  pending_.clear();
  renderTimer_.stop();
  viewport()->update();
}
void LookupCaseGallery::invalidateGlyph(const QString& glyph) {
  for (auto key : cache_.keys())
    if (cache_.object(key)->glyphDependencies.contains(glyph)) {
      stale_.insert(key);
      if (visible(key)) pending_.insert(key);
    }
  if (!pending_.isEmpty() && !renderTimer_.isActive()) renderTimer_.start(0);
  viewport()->update();
}
void LookupCaseGallery::setFirstRow(quint64 row) {
  firstRow_ = row;
  pending_.clear();
  updateScrollRange();
  viewport()->update();
}
void LookupCaseGallery::setCurrentIndex(quint64 index) {
  if (index >= count_ || index == current_) return;
  current_ = index;
  const auto row = index / columns_;
  if (row < firstRow_)
    setFirstRow(row);
  else if (row >= firstRow_ + quint64(visibleRows()))
    setFirstRow(row - visibleRows() + 1);
  viewport()->update();
  emit currentChanged(index);
}
bool LookupCaseGallery::visible(quint64 index) const {
  const auto row = index / columns_;
  return index < count_ && row >= firstRow_ && row - firstRow_ <= quint64(visibleRows());
}
void LookupCaseGallery::renderPending() {
  QElapsedTimer elapsed;
  elapsed.start();
  // Yield to mouse, scroll and window events between small batches.
  while (!pending_.isEmpty()) {
    const auto index = *pending_.begin();
    pending_.remove(index);
    if (!visible(index) || !renderer_) continue;
    auto result = std::make_unique<LookupCaseThumbnail>();
    try {
      *result = renderer_(index);
    } catch (const std::exception& e) {
      result->error = QString::fromUtf8(e.what()).section('\n', 0, 0);
    }
    const int cost = int(std::min<qsizetype>(
        16 * 1024 * 1024,
        std::max<qsizetype>(1, result->path.elementCount() * int(sizeof(QPainterPath::Element)) +
                                   2 * (result->label.size() + result->error.size()) + 256)));
    cache_.insert(index, result.release(), cost);
    stale_.remove(index);
    if (elapsed.elapsed() >= 8) break;
  }
  viewport()->update();
  if (!pending_.isEmpty()) renderTimer_.start(0);
}
bool LookupCaseGallery::visiblePreviewsReady() const {
  for (int row = 0; row <= visibleRows(); ++row) {
    const auto absoluteRow = firstRow_ + quint64(row);
    if (absoluteRow >= rowCount()) break;
    const auto begin = absoluteRow * quint64(columns_);
    for (int column = 0; column < columns_ && quint64(column) < count_ - begin; ++column)
      if (!cache_.contains(begin + column) || stale_.contains(begin + column)) return false;
  }
  return true;
}
void LookupCaseGallery::paintEvent(QPaintEvent*) {
  QPainter painter(viewport());
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(viewport()->rect(), Qt::white);
  if (!count_) {
    painter.drawText(viewport()->rect(), Qt::AlignCenter, tr("No cases"));
    return;
  }
  const int width = cellWidth();
  for (int row = 0; row <= visibleRows(); ++row) {
    const auto absoluteRow = firstRow_ + quint64(row);
    if (absoluteRow >= rowCount()) break;
    const auto begin = absoluteRow * quint64(columns_);
    for (int column = 0; column < columns_ && quint64(column) < count_ - begin; ++column) {
      const auto index = begin + column;
      const QRect cell(column * width, row * TileHeight, width, TileHeight);
      painter.fillRect(cell.adjusted(3, 3, -3, -3),
                       index == current_ ? QColor("#e8f2fc") : QColor("#fafbfc"));
      painter.setPen(index == current_ ? QColor("#2777b8") : QColor("#d9dfe7"));
      painter.drawRect(cell.adjusted(3, 3, -3, -3));
      auto* thumbnail = cache_.object(index);
      if (stale_.contains(index)) pending_.insert(index);
      painter.setPen(QColor("#536170"));
      painter.drawText(cell.adjusted(12, 7, -12, -7), Qt::AlignTop | Qt::AlignLeft,
                       QString::number(index + 1));
      if (!thumbnail) {
        painter.drawText(cell, Qt::AlignCenter, tr("Rendering…"));
        pending_.insert(index);
        continue;
      }
      const auto ink = thumbnail->path.boundingRect();
      if (!ink.isEmpty()) {
        const QRectF area = cell.adjusted(12, 28, -12, -43);
        const double scale =
            std::min({.09, area.width() / ink.width(), area.height() / ink.height()});
        painter.save();
        painter.translate(area.center());
        painter.scale(scale, -scale);
        painter.translate(-ink.center());
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.drawPath(thumbnail->path);
        painter.restore();
      }
      painter.setPen(thumbnail->error.isEmpty()
                         ? (thumbnail->applied ? QColor("#253447") : QColor("#9a650a"))
                         : QColor("#b02a37"));
      const auto text = thumbnail->error.isEmpty() ? thumbnail->label : thumbnail->error;
      const QRect caption = cell.adjusted(10, TileHeight - 39, -10, -7);
      painter.drawText(caption, Qt::AlignCenter | Qt::TextWordWrap, text);
    }
  }
  if (!pending_.isEmpty() && !renderTimer_.isActive()) renderTimer_.start(0);
}
void LookupCaseGallery::resizeEvent(QResizeEvent* event) {
  const auto first = firstRow_ * quint64(columns_);
  QAbstractScrollArea::resizeEvent(event);
  columns_ = std::max(1, viewport()->width() / TileWidth);
  firstRow_ = first / columns_;
  pending_.clear();
  updateScrollRange();
  viewport()->update();
}
void LookupCaseGallery::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    const int column = event->pos().x() / cellWidth();
    const auto row = firstRow_ + quint64(std::max(0, event->pos().y() / TileHeight));
    if (column < columns_ && row < rowCount()) {
      const auto begin = row * quint64(columns_);
      if (quint64(column) < count_ - begin) setCurrentIndex(begin + column);
    }
    setFocus();
    event->accept();
    return;
  }
  QAbstractScrollArea::mousePressEvent(event);
}
void LookupCaseGallery::keyPressEvent(QKeyEvent* event) {
  if (current_ == Invalid) return;
  quint64 next = current_;
  const auto down = [&](quint64 n) { return current_ + std::min(n, count_ - 1 - current_); };
  switch (event->key()) {
    case Qt::Key_Right:
      next = down(1);
      break;
    case Qt::Key_Left:
      next = current_ ? current_ - 1 : 0;
      break;
    case Qt::Key_Down:
      next = down(columns_);
      break;
    case Qt::Key_Up:
      next = current_ >= quint64(columns_) ? current_ - columns_ : 0;
      break;
    case Qt::Key_Home:
      next = 0;
      break;
    case Qt::Key_End:
      next = count_ - 1;
      break;
    case Qt::Key_PageDown:
      next = down(quint64(columns_) * visibleRows());
      break;
    case Qt::Key_PageUp: {
      const auto n = quint64(columns_) * visibleRows();
      next = current_ >= n ? current_ - n : 0;
      break;
    }
    default:
      QAbstractScrollArea::keyPressEvent(event);
      return;
  }
  setCurrentIndex(next);
  event->accept();
}
void LookupCaseGallery::wheelEvent(QWheelEvent* event) {
  const int delta =
      event->angleDelta().y() != 0 ? event->angleDelta().y() : event->pixelDelta().y();
  if (delta) {
    const auto amount = quint64(std::max(1, std::abs(delta) / 120));
    if (delta > 0)
      setFirstRow(firstRow_ > amount ? firstRow_ - amount : 0);
    else {
      const auto maximum = rowCount() > quint64(visibleRows()) ? rowCount() - visibleRows() : 0;
      setFirstRow(firstRow_ + std::min(amount, maximum - firstRow_));
    }
  }
  event->accept();
}
