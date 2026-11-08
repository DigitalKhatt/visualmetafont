#pragma once

#include <QWidget>
#include <QTextCursor>
#include "sourcelinks.hpp"

class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

// The editor owns a draft independently of the last source received from the glyph.
class MetaPostEditor : public QWidget {
  Q_OBJECT
 public:
  explicit MetaPostEditor(QWidget* parent = nullptr);
  QPlainTextEdit* textEdit() const;
  QString toPlainText() const;
  void initializeSource(const QString& source);
  void synchronizeSource(const QString& source);
  void beginExecution();
  bool hasPendingChanges() const;
  bool hasConflict() const { return conflict_; }
  void setPointTopology(const QMap<int,QSet<int>>& topology);
  bool highlightDeclarations(const QStringList& keys);

 public slots:
  void showFind();
  void showReplace();
  void findNext();
  void findPrevious();
  void replaceOne();
  void replaceAll();
  void keepDraft();
  void useDrawing();

 signals:
  void draftStateChanged();
  void declarationSelected(const QStringList& keys);

 private:
  void showSearch(bool replace);
  void search(bool backwards);
  void refreshSearch();
  void updateState();
  void applyText(const QString& source);
  QVector<QTextCursor> matches() const;
  void rebuildSourceIndex();
  void selectDeclarationAtCursor();
  QStringList relatedKeys(const QStringList& keys) const;

  QPlainTextEdit* edit_;
  QWidget* searchBar_;
  QWidget* replaceRow_;
  QLineEdit* find_;
  QLineEdit* replacement_;
  QCheckBox* caseSensitive_;
  QCheckBox* wholeWords_;
  QLabel* matchCount_;
  QLabel* state_;
  QPushButton* keepDraft_;
  QPushButton* useDrawing_;
  QString source_;
  QString incoming_;
  bool conflict_ = false;
  bool updatingSource_ = false;
  QMap<int,QSet<int>> pointTopology_;
  QVector<SourceLinks::Declaration> declarations_;
  QStringList highlightedDeclarations_;
};
