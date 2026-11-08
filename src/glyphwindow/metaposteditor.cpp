#include "metaposteditor.hpp"

#include <QtWidgets>
#include <QSyntaxHighlighter>
#include <algorithm>
#include <vector>
#include <limits>

namespace {
class MetaPostHighlighter : public QSyntaxHighlighter {
 public:
  explicit MetaPostHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {}
 protected:
  void highlightBlock(const QString& text) override {
    const bool dark = QApplication::palette().color(QPalette::Base).lightness() < 128;
    const QColor comment = dark ? QColor("#8fb887") : QColor("#487441");
    const QColor string = dark ? QColor("#e4ad87") : QColor("#9a4515");
    const QColor keyword = dark ? QColor("#c8a2ed") : QColor("#7934a6");
    const QColor number = dark ? QColor("#9bd4cb") : QColor("#176b69");
    const QColor command = dark ? QColor("#86c6f4") : QColor("#165ca0");
    const QColor parameter = dark ? QColor("#e1c479") : QColor("#805b00");
    static const QSet<QString> keywords = {
      "if", "else", "elseif", "fi", "for", "forsuffixes", "forever", "endfor", "exitif",
      "def", "vardef", "primarydef", "secondarydef", "tertiarydef", "enddef", "expr", "suffix",
      "text", "primary", "secondary", "tertiary", "save", "interim", "input", "endinput", "end",
      "numeric", "pair", "path", "picture", "pen", "boolean", "string", "transform", "color",
      "begingroup", "endgroup", "true", "false", "known", "unknown", "and", "or", "not",
      "tension", "controls", "curl", "cycle", "atleast", "upto", "downto", "step", "until",
      "shifted", "scaled", "xscaled", "yscaled", "rotated", "transformed", "slanted", "withpen",
      "withcolor", "of", "btex", "verbatimtex", "etex"
    };
    static const QSet<QString> commands = {
      "defchar", "enddefchar", "beginchar", "endchar", "fill", "draw", "filldraw", "unfill",
      "clip", "pickup", "addto", "show", "message", "errmessage", "setMarkAnchor", "setCursiveAnchor",
      "setAnchor", "defAnchor", "marksPoint", "marksSibling", "marksOrigin", "marksAdvance",
      "leftjoin", "rightjoin", "leftjoinii", "rightjoinii", "setlanchor", "setranchor", "drawcomponent"
    };
    setCurrentBlockState(0);
    int i = 0;
    if (previousBlockState() == 1) {
      const auto end = QRegularExpression("\\betex\\b").match(text);
      if (!end.hasMatch()) { setFormat(0, text.size(), string); setCurrentBlockState(1); return; }
      setFormat(0, end.capturedStart(), string);
      i = end.capturedStart();
    }
    while (i < text.size()) {
      const int start = i;
      if (text[i] == '%') {
        QTextCharFormat format;
        format.setForeground(comment);
        if (text.mid(i, 2) == "%%") format.setFontWeight(QFont::Bold);
        setFormat(i, text.size() - i, format);
        break;
      }
      if (text[i] == '"') {
        ++i;
        while (i < text.size() && text[i] != '"') ++i;
        if (i < text.size()) ++i;
        setFormat(start, i - start, string);
      } else if (text[i].isDigit() || (text[i] == '.' && i + 1 < text.size() && text[i+1].isDigit())) {
        while (i < text.size() && text[i].isDigit()) ++i;
        if (i < text.size() && text[i] == '.' && (i+1 == text.size() || text[i+1] != '.')) {
          ++i;
          while (i < text.size() && text[i].isDigit()) ++i;
        }
        setFormat(start, i - start, number);
      } else if (text[i].isLetter() || text[i] == '_') {
        while (i < text.size() && (text[i].isLetterOrNumber() || text[i] == '_' ||
            (text[i] == '.' && i+1 < text.size() && text[i+1].isLetterOrNumber()))) ++i;
        const auto token = text.mid(start, i-start);
        if (keywords.contains(token)) setFormat(start, i-start, keyword);
        else if (commands.contains(token)) setFormat(start, i-start, command);
        else if (token.startsWith("z.") || (token.startsWith('z') && token.size() > 1 && token[1].isDigit()))
          setFormat(start, i-start, parameter);
        if (token == "btex" || token == "verbatimtex") {
          const auto end = QRegularExpression("\\betex\\b").match(text, i);
          const int stop = end.hasMatch() ? end.capturedStart() : text.size();
          setFormat(i, stop-i, string);
          i = stop;
          if (!end.hasMatch()) setCurrentBlockState(1);
        }
      } else ++i;
    }
  }
};

class CodeEdit : public QPlainTextEdit {
 public:
  explicit CodeEdit(QWidget* parent) : QPlainTextEdit(parent), gutter_(new QWidget(this)) {
    gutter_->setObjectName("lineNumberArea");
    gutter_->installEventFilter(this);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 2);
    connect(this, &QPlainTextEdit::blockCountChanged, this, [this] { updateGutter(); });
    connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect& rect, int dy) {
      if (dy) gutter_->scroll(0, dy);
      else gutter_->update(0, rect.y(), gutter_->width(), rect.height());
      if (rect.contains(viewport()->rect())) updateGutter();
    });
    updateGutter();
    highlighter_ = new MetaPostHighlighter(document());
  }
 protected:
  void resizeEvent(QResizeEvent* event) override {
    QPlainTextEdit::resizeEvent(event);
    updateGutter();
  }
  void changeEvent(QEvent* event) override {
    QPlainTextEdit::changeEvent(event);
    if (event->type() == QEvent::FontChange) updateGutter();
    if (event->type() == QEvent::PaletteChange) {
      if (highlighter_) highlighter_->rehighlight();
      gutter_->update();
    }
  }
  bool eventFilter(QObject* object, QEvent* event) override {
    if (object == gutter_ && event->type() == QEvent::Paint) {
      QPainter painter(gutter_);
      painter.fillRect(gutter_->rect(), palette().color(QPalette::AlternateBase));
      painter.setPen(palette().color(QPalette::PlaceholderText));
      auto block = firstVisibleBlock();
      while (block.isValid()) {
        const auto bounds = blockBoundingGeometry(block).translated(contentOffset());
        if (bounds.top() > gutter_->height()) break;
        if (block.isVisible() && bounds.bottom() >= 0)
          painter.drawText(0, qRound(bounds.top()), gutter_->width()-7, fontMetrics().height(),
                           Qt::AlignRight, QString::number(block.blockNumber()+1));
        block = block.next();
      }
      return true;
    }
    return QPlainTextEdit::eventFilter(object, event);
  }
 private:
  void updateGutter() {
    const int digits = QString::number(qMax(1, blockCount())).size();
    const int width = 12 + fontMetrics().horizontalAdvance('9') * digits;
    setViewportMargins(width, 0, 0, 0);
    const auto rect = contentsRect();
    gutter_->setGeometry(rect.left(), rect.top(), width, rect.height());
    gutter_->update();
  }
  QWidget* gutter_;
  MetaPostHighlighter* highlighter_ = nullptr;
};

struct Edit { int begin, end; QStringList lines; };
// Line edits make the merge deliberately conservative: simultaneous edits to
// the same assignment are a conflict, even if they affect different characters.
std::vector<Edit> differences(const QStringList& base, const QStringList& next) {
  int prefix = 0, oldEnd = base.size(), newEnd = next.size();
  while (prefix < oldEnd && prefix < newEnd && base[prefix] == next[prefix]) ++prefix;
  while (oldEnd > prefix && newEnd > prefix && base[oldEnd-1] == next[newEnd-1]) { --oldEnd; --newEnd; }
  const int n = oldEnd-prefix, m = newEnd-prefix;
  if (!n && !m) return {};
  // Bound memory for unusually large pasted documents; a coarse edit remains safe.
  if (qint64(n+1)*(m+1) > 2000000) return {{prefix, oldEnd, next.mid(prefix, m)}};
  std::vector<int> lengths((n+1)*(m+1));
  auto cell = [&](int i, int j) -> int& { return lengths[i*(m+1)+j]; };
  for (int i=n-1; i>=0; --i) for (int j=m-1; j>=0; --j)
    cell(i,j) = base[prefix+i] == next[prefix+j] ? 1+cell(i+1,j+1) : qMax(cell(i+1,j),cell(i,j+1));
  std::vector<Edit> edits;
  int i=0, j=0;
  while (i<n || j<m) {
    if (i<n && j<m && base[prefix+i] == next[prefix+j]) { ++i; ++j; continue; }
    Edit edit{prefix+i, prefix+i, {}};
    while (i<n || j<m) {
      if (i<n && j<m && base[prefix+i] == next[prefix+j]) break;
      if (j<m && (i==n || cell(i,j+1) >= cell(i+1,j))) edit.lines.append(next[prefix+j++]);
      else ++i;
    }
    edit.end = prefix+i;
    edits.push_back(std::move(edit));
  }
  return edits;
}
bool merge(const QString& baseText, const QString& draft, const QString& incoming, QString& result) {
  const auto base = baseText.split('\n');
  auto edits = differences(base, draft.split('\n'));
  const auto incomingEdits = differences(base, incoming.split('\n'));
  for (const auto& change : incomingEdits) {
    bool identical = false;
    for (const auto& edit : edits) {
      if (edit.begin == change.begin && edit.end == change.end && edit.lines == change.lines) { identical = true; break; }
      const bool insertion = edit.begin == edit.end || change.begin == change.end;
      if (insertion ? qMax(edit.begin,change.begin) <= qMin(edit.end,change.end)
                    : qMax(edit.begin,change.begin) < qMin(edit.end,change.end)) return false;
    }
    if (!identical) edits.push_back(change);
  }
  std::sort(edits.begin(), edits.end(), [](const Edit& a, const Edit& b) { return a.begin > b.begin; });
  auto lines = base;
  for (const auto& edit : edits) {
    for (int i=edit.end; i>edit.begin; --i) lines.removeAt(i-1);
    for (int i=edit.lines.size()-1; i>=0; --i) lines.insert(edit.begin, edit.lines[i]);
  }
  result = lines.join('\n');
  return true;
}
}

MetaPostEditor::MetaPostEditor(QWidget* parent) : QWidget(parent) {
  setObjectName("metaPostEditor");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  searchBar_ = new QWidget(this);
  auto* searchLayout = new QVBoxLayout(searchBar_);
  searchLayout->setContentsMargins(0,0,0,0);
  auto* findRow = new QHBoxLayout;
  find_ = new QLineEdit(searchBar_);
  find_->setObjectName("findText"); find_->setPlaceholderText(tr("Find"));
  matchCount_ = new QLabel(searchBar_);
  auto* previous = new QToolButton(searchBar_);
  previous->setText(QString::fromUtf8("↑")); previous->setToolTip(tr("Previous match"));
  auto* next = new QToolButton(searchBar_);
  next->setText(QString::fromUtf8("↓")); next->setToolTip(tr("Next match"));
  auto* close = new QToolButton(searchBar_);
  close->setText(QString::fromUtf8("×")); close->setToolTip(tr("Close search (Escape)"));
  findRow->addWidget(find_, 1);
  findRow->addWidget(previous); findRow->addWidget(next); findRow->addWidget(close);
  searchLayout->addLayout(findRow);
  auto* options = new QHBoxLayout;
  caseSensitive_ = new QCheckBox(tr("Match case"), searchBar_);
  caseSensitive_->setObjectName("matchCase");
  wholeWords_ = new QCheckBox(tr("Whole words"), searchBar_);
  wholeWords_->setObjectName("wholeWords");
  options->addWidget(caseSensitive_); options->addWidget(wholeWords_); options->addStretch(); options->addWidget(matchCount_);
  searchLayout->addLayout(options);
  replaceRow_ = new QWidget(searchBar_);
  auto* replaceLayout = new QHBoxLayout(replaceRow_);
  replaceLayout->setContentsMargins(0,0,0,0);
  replacement_ = new QLineEdit(replaceRow_);
  replacement_->setObjectName("replacementText"); replacement_->setPlaceholderText(tr("Replace with"));
  auto* replace = new QPushButton(tr("Replace"), replaceRow_);
  auto* replaceAllButton = new QPushButton(tr("Replace all"), replaceRow_);
  replaceLayout->addWidget(replacement_,1); replaceLayout->addWidget(replace); replaceLayout->addWidget(replaceAllButton);
  searchLayout->addWidget(replaceRow_);
  layout->addWidget(searchBar_);
  searchBar_->hide();
  edit_ = new CodeEdit(this);
  edit_->setObjectName("metaPostCode");
  layout->addWidget(edit_,1);
  state_ = new QLabel(this);
  state_->setObjectName("codeState"); state_->setWordWrap(true);
  layout->addWidget(state_);
  auto* conflictRow = new QHBoxLayout;
  keepDraft_ = new QPushButton(tr("Keep my code"), this);
  keepDraft_->setObjectName("keepCode");
  keepDraft_->setToolTip(tr("Keep the draft. Executing it will replace the conflicting drawing changes."));
  useDrawing_ = new QPushButton(tr("Use drawing code"), this);
  useDrawing_->setObjectName("useDrawing");
  useDrawing_->setToolTip(tr("Load the current drawing source. The previous draft remains recoverable with text Undo."));
  conflictRow->addWidget(keepDraft_); conflictRow->addWidget(useDrawing_); conflictRow->addStretch();
  layout->addLayout(conflictRow);
  connect(find_, &QLineEdit::textChanged, this, &MetaPostEditor::refreshSearch);
  connect(caseSensitive_, &QCheckBox::toggled, this, &MetaPostEditor::refreshSearch);
  connect(wholeWords_, &QCheckBox::toggled, this, &MetaPostEditor::refreshSearch);
  connect(find_, &QLineEdit::returnPressed, this, &MetaPostEditor::findNext);
  connect(previous, &QToolButton::clicked, this, &MetaPostEditor::findPrevious);
  connect(next, &QToolButton::clicked, this, &MetaPostEditor::findNext);
  connect(replace, &QPushButton::clicked, this, &MetaPostEditor::replaceOne);
  connect(replaceAllButton, &QPushButton::clicked, this, &MetaPostEditor::replaceAll);
  connect(close, &QToolButton::clicked, this, [this] { searchBar_->hide(); refreshSearch(); edit_->setFocus(); });
  connect(keepDraft_, &QPushButton::clicked, this, &MetaPostEditor::keepDraft);
  connect(useDrawing_, &QPushButton::clicked, this, &MetaPostEditor::useDrawing);
  connect(edit_, &QPlainTextEdit::textChanged, this, [this] { rebuildSourceIndex(); refreshSearch(); updateState(); });
  connect(edit_, &QPlainTextEdit::cursorPositionChanged, this, &MetaPostEditor::refreshSearch);
  connect(edit_, &QPlainTextEdit::cursorPositionChanged, this, &MetaPostEditor::selectDeclarationAtCursor);
  connect(edit_, &QPlainTextEdit::selectionChanged, this, &MetaPostEditor::selectDeclarationAtCursor);
  connect(qApp,&QApplication::focusChanged,this,[this](QWidget*,QWidget* now) {
    if (now==edit_) selectDeclarationAtCursor();
  });
  const auto shortcut = [this](QKeySequence key, auto callback) {
    auto* action = new QAction(this);
    action->setShortcut(key); action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(action); connect(action, &QAction::triggered, this, callback);
  };
  shortcut(QKeySequence(Qt::Key_Escape), [this] {
    if (!searchBar_->isHidden()) { searchBar_->hide(); refreshSearch(); edit_->setFocus(); }
  });
  updateState();
}
QPlainTextEdit* MetaPostEditor::textEdit() const { return edit_; }
QString MetaPostEditor::toPlainText() const { return edit_->toPlainText(); }
bool MetaPostEditor::hasPendingChanges() const { return toPlainText() != (conflict_ ? incoming_ : source_); }
void MetaPostEditor::beginExecution() {
  // The submitted draft is now the baseline for normalization by Glyph::source().
  // This is called only after the window has checked for an unresolved conflict.
  source_ = incoming_ = toPlainText();
}
void MetaPostEditor::initializeSource(const QString& source) {
  QScopedValueRollback<bool> updating(updatingSource_,true);
  source_ = incoming_ = source; conflict_ = false;
  edit_->setPlainText(source); updateState();
}
void MetaPostEditor::synchronizeSource(const QString& incoming) {
  incoming_ = incoming;
  QString merged;
  if (toPlainText() == incoming) { source_ = incoming; conflict_ = false; }
  else if (merge(source_, toPlainText(), incoming, merged)) {
    source_ = incoming; conflict_ = false; applyText(merged);
  } else conflict_ = true;
  updateState();
}
void MetaPostEditor::keepDraft() { source_ = incoming_; conflict_ = false; updateState(); }
void MetaPostEditor::useDrawing() {
  source_ = incoming_; conflict_ = false; applyText(source_); updateState();
}
void MetaPostEditor::applyText(const QString& source) {
  QScopedValueRollback<bool> updating(updatingSource_,true);
  const auto old = toPlainText();
  if (old == source) return;
  const auto edits = differences(old.split('\n'), source.split('\n'));
  auto selection = edit_->textCursor();
  const int vertical = edit_->verticalScrollBar()->value(), horizontal = edit_->horizontalScrollBar()->value();
  auto cursor = edit_->textCursor();
  cursor.beginEditBlock();
  // Apply small edits backwards so cursor tracking and text undo remain intact.
  const auto oldLines = old.split('\n');
  for (const auto& edit : std::vector<Edit>(edits.rbegin(), edits.rend())) {
    int start = 0, end = 0;
    for (int i=0; i<edit.begin; ++i) start += oldLines[i].size()+1;
    end = start;
    for (int i=edit.begin; i<edit.end; ++i) end += oldLines[i].size()+1;
    QString replacement = edit.lines.join('\n');
    if (edit.end < oldLines.size()) { if (!edit.lines.empty()) replacement += '\n'; }
    else if (edit.begin > 0) { --start; if (!edit.lines.empty()) replacement.prepend('\n'); }
    cursor.setPosition(qMin(start,old.size()));
    cursor.setPosition(qMin(end,old.size()), QTextCursor::KeepAnchor);
    cursor.insertText(replacement);
  }
  cursor.endEditBlock();
  edit_->setTextCursor(selection);
  edit_->verticalScrollBar()->setValue(vertical); edit_->horizontalScrollBar()->setValue(horizontal);
}
void MetaPostEditor::updateState() {
  const bool pending = hasPendingChanges();
  state_->setText(conflict_ ? tr("Code and drawing changed the same lines. Your draft is preserved. Choose a version before executing.")
      : pending ? tr("Unexecuted code changes — Execute to update the drawing. Execute before saving.") : QString());
  state_->setVisible(pending || conflict_);
  keepDraft_->setVisible(conflict_); useDrawing_->setVisible(conflict_);
  emit draftStateChanged();
}
void MetaPostEditor::showSearch(bool replace) {
  if (edit_->textCursor().hasSelection() && !edit_->textCursor().selectedText().contains(QChar::ParagraphSeparator))
    find_->setText(edit_->textCursor().selectedText());
  searchBar_->show(); replaceRow_->setVisible(replace);
  find_->setFocus(); find_->selectAll(); refreshSearch();
}
void MetaPostEditor::showFind() { showSearch(false); }
void MetaPostEditor::showReplace() { showSearch(true); }
QVector<QTextCursor> MetaPostEditor::matches() const {
  QVector<QTextCursor> result;
  if (find_->text().isEmpty()) return result;
  QTextDocument::FindFlags flags;
  if (caseSensitive_->isChecked()) flags |= QTextDocument::FindCaseSensitively;
  if (wholeWords_->isChecked()) flags |= QTextDocument::FindWholeWords;
  auto cursor = edit_->document()->find(find_->text(), 0, flags);
  while (!cursor.isNull()) {
    result.append(cursor);
    cursor = edit_->document()->find(find_->text(), cursor.selectionEnd(), flags);
  }
  return result;
}
void MetaPostEditor::refreshSearch() {
  QList<QTextEdit::ExtraSelection> selections;
  QTextEdit::ExtraSelection line;
  line.cursor = edit_->textCursor(); line.cursor.clearSelection();
  line.format.setBackground(palette().color(QPalette::AlternateBase));
  line.format.setProperty(QTextFormat::FullWidthSelection, true); selections.append(line);
  if (!searchBar_->isHidden()) {
    const auto found = matches();
    matchCount_->setText(find_->text().isEmpty() ? QString()
        : found.size() == 1 ? tr("1 match") : tr("%1 matches").arg(found.size()));
    for (const auto& cursor : found) {
      QTextEdit::ExtraSelection match;
      match.cursor = cursor;
      QColor colour = palette().color(QPalette::Highlight); colour.setAlpha(85);
      match.format.setBackground(colour); selections.append(match);
    }
  }
  for (const auto& declaration:declarations_) if (highlightedDeclarations_.contains(declaration.key)) {
    QTextEdit::ExtraSelection linked;
    linked.cursor=QTextCursor(edit_->document()); linked.cursor.setPosition(declaration.begin);
    linked.cursor.setPosition(declaration.end,QTextCursor::KeepAnchor);
    linked.format.setBackground(QColor(255,193,70,95));
    linked.format.setProperty(QTextFormat::UserProperty,"sourceDeclaration");
    selections.append(linked);
  }
  edit_->setExtraSelections(selections);
}
void MetaPostEditor::setPointTopology(const QMap<int,QSet<int>>& topology) {
  if (pointTopology_==topology) return;
  pointTopology_=topology;
  rebuildSourceIndex(); refreshSearch();
}
void MetaPostEditor::rebuildSourceIndex() {
  declarations_=SourceLinks::declarations(toPlainText(),pointTopology_);
}
QStringList MetaPostEditor::relatedKeys(const QStringList& keys) const {
  auto result=keys;
  for (const auto& declaration:declarations_) {
    if (keys.contains(declaration.key)) result+=declaration.related;
    if (declaration.key.startsWith("anchor:") || declaration.key.startsWith("point:"))
      for (const auto& key:keys) if (declaration.related.contains(key)) { result.append(declaration.key); break; }
  }
  result.removeDuplicates(); return result;
}
bool MetaPostEditor::highlightDeclarations(const QStringList& keys) {
  highlightedDeclarations_.clear();
  int first=-1;
  // Item metadata lists its precise location first, then named-parameter fallbacks.
  for (const auto& key:keys) {
    for (const auto& declaration:declarations_) if (declaration.key==key) {
      highlightedDeclarations_.append(key);
      if (first<0) first=declaration.begin;
    }
    if (!highlightedDeclarations_.isEmpty()) break;
  }
  if (first<0 && !keys.isEmpty() && keys.first().startsWith("point:")) {
    auto fallback=keys.first().section(':',0,2)+":point";
    for (const auto& declaration:declarations_) if (declaration.key==fallback) {
      highlightedDeclarations_.append(fallback); first=declaration.begin; break;
    }
  }
  refreshSearch();
  if (first>=0) {
    const auto block=edit_->document()->findBlock(first);
    const int top=edit_->cursorForPosition(QPoint(0,0)).blockNumber();
    const int rows=qMax(1,edit_->viewport()->height()/edit_->fontMetrics().height());
    if (block.blockNumber()<top || block.blockNumber()>=top+rows)
      edit_->verticalScrollBar()->setValue(qMax(0,block.blockNumber()-2));
  }
  return first>=0;
}
void MetaPostEditor::selectDeclarationAtCursor() {
  if (updatingSource_ || !edit_->hasFocus()) return;
  const auto cursor=edit_->textCursor();
  const int start=cursor.selectionStart(), end=cursor.selectionEnd();
  QStringList keys;
  int smallest=std::numeric_limits<int>::max();
  for (const auto& declaration:declarations_) {
    if (cursor.hasSelection() ? start<declaration.end && end>declaration.begin
                              : start>=declaration.begin && start<declaration.end) {
      const int length=declaration.end-declaration.begin;
      if (!cursor.hasSelection() && length>smallest) continue;
      if (!cursor.hasSelection() && length<smallest) keys.clear();
      keys.append(declaration.key); smallest=length;
    }
  }
  keys.removeDuplicates(); highlightedDeclarations_=keys;
  refreshSearch(); emit declarationSelected(relatedKeys(keys));
}
void MetaPostEditor::search(bool backwards) {
  if (find_->text().isEmpty()) { showFind(); return; }
  QTextDocument::FindFlags flags;
  if (caseSensitive_->isChecked()) flags |= QTextDocument::FindCaseSensitively;
  if (wholeWords_->isChecked()) flags |= QTextDocument::FindWholeWords;
  if (backwards) flags |= QTextDocument::FindBackward;
  auto cursor = edit_->textCursor();
  const int from = backwards ? cursor.selectionStart() : cursor.selectionEnd();
  auto found = edit_->document()->find(find_->text(), from, flags);
  if (found.isNull()) found = edit_->document()->find(find_->text(), backwards ? edit_->document()->characterCount()-1 : 0, flags);
  if (!found.isNull()) { edit_->setTextCursor(found); edit_->ensureCursorVisible(); }
  refreshSearch();
}
void MetaPostEditor::findNext() { search(false); }
void MetaPostEditor::findPrevious() { search(true); }
void MetaPostEditor::replaceOne() {
  const auto selection = edit_->textCursor();
  for (const auto& match : matches())
    if (match.selectionStart() == selection.selectionStart() && match.selectionEnd() == selection.selectionEnd()) {
      auto cursor = selection; cursor.insertText(replacement_->text()); edit_->setTextCursor(cursor); findNext(); return;
    }
  findNext();
}
void MetaPostEditor::replaceAll() {
  const auto found = matches();
  if (found.isEmpty()) return;
  auto cursor = edit_->textCursor(); cursor.beginEditBlock();
  for (auto it = found.crbegin(); it != found.crend(); ++it) {
    cursor.setPosition(it->selectionStart()); cursor.setPosition(it->selectionEnd(),QTextCursor::KeepAnchor);
    cursor.insertText(replacement_->text());
  }
  cursor.endEditBlock(); refreshSearch();
}
