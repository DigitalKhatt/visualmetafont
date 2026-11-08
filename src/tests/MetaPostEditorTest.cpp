#include <QtWidgets>
#include <QtTest/QTest>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "metaposteditor.hpp"
#include "font.hpp"
#include "glyph.hpp"
#include "glyphwindow.h"
#include "glyphview.hpp"
#include "pairitem.hpp"
#include "knotitem.hpp"
#include "sourcelinks.hpp"

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
static void write(const QString& path, const QByteArray& data) {
  QFile file(path);
  require(file.open(QIODevice::WriteOnly) && file.write(data) == data.size(), "Write fixture");
}
static void replace(QPlainTextEdit* edit, const QString& from, const QString& to) {
  auto cursor = edit->document()->find(from);
  require(!cursor.isNull(), "Find text to edit");
  cursor.insertText(to);
  edit->setTextCursor(cursor);
}
static QColor colourAt(QTextDocument* document, int block, int position) {
  for (const auto& format : document->findBlockByNumber(block).layout()->formats())
    if (position >= format.start && position < format.start+format.length)
      return format.format.foreground().color();
  return {};
}
static QStringList linkedSource(QPlainTextEdit* code) {
  QStringList spans;
  for (const auto& selection:code->extraSelections())
    if (selection.format.property(QTextFormat::UserProperty)=="sourceDeclaration") spans.append(selection.cursor.selectedText());
  return spans;
}
static QGraphicsItem* itemWithKey(GlyphScene* scene,const QString& key,const QString& tag={}) {
  for (auto* item:scene->items())
    if (item->data(SourceLinks::KeysRole).toStringList().contains(key) && (tag.isEmpty() || item->data(0).toString()==tag)) return item;
  return nullptr;
}
static void sourceIndex() {
  const QString source=R"(z.p = (1,2);
pair indirect;
indirect := z.p;
% setMarkAnchor(commented,z.p);
message "setMarkAnchor(string,z.p);";
setCursiveAnchor(join,z.p,EntryAnchorRTL);
setMarkAnchor(curried)(z.p);
setCursiveAnchor(exit)(z.p)(ExitAnchorRTL);
setMarkAnchor(top,marksPoint(indirect,(0,0)));
%%beginpaths
fill (0,0) .. controls (1,2) and (3,4) .. (20,0) -- cycle;
fill (0,0) -- (20,0) -- cycle;
%%endpaths
)";
  const QMap<int,QSet<int>> topology{{0,{0,1}},{1,{0,1}}};
  const auto declarations=SourceLinks::declarations(source,topology);
  const auto find=[&](const QString& key) {
    for (const auto& d:declarations) if (d.key==key) return d;
    throw std::runtime_error(("Missing source link: "+key).toStdString());
  };
  require(find(SourceLinks::anchorKey("join",4)).key=="anchor:4:join", "RTL cursive anchor type is part of its identity");
  require(find(SourceLinks::anchorKey("curried",1)).related.contains(SourceLinks::parameterKey("z.p")) &&
      find(SourceLinks::anchorKey("exit",5)).related.contains(SourceLinks::parameterKey("z.p")), "Separate MetaPost argument groups resolve to the same anchor identities");
  require(find(SourceLinks::anchorKey("top",1)).related.contains(SourceLinks::parameterKey("z.p")), "Follow indirect anchor parameter dependencies");
  for (const auto& d:declarations)
    require(d.key!=SourceLinks::anchorKey("commented",1) && d.key!=SourceLinks::anchorKey("string",1), "Ignore declarations inside comments and strings");
  const auto a=find(SourceLinks::pointKey(0,0)), b=find(SourceLinks::pointKey(1,0));
  require(a.begin!=b.begin && source.mid(a.begin,a.end-a.begin)==source.mid(b.begin,b.end-b.begin), "Repeated coordinates use path identities");
  const auto right=find(SourceLinks::pointKey(0,0,"right")), left=find(SourceLinks::pointKey(0,1,"left"));
  require(source.mid(right.begin,right.end-right.begin)=="(1,2)" && source.mid(left.begin,left.end-left.begin)=="(3,4)", "Map individual control expressions");
  auto changed=source; changed.replace("fill (0,0) -- (20,0) -- cycle", "fill (0,0) -- (20,0) -- (30,0) -- cycle");
  for (const auto& d:SourceLinks::declarations(changed,topology))
    require(!d.key.startsWith("point:1:"), "An unexecuted topology change cannot select a different rendered point");
}
static void standalone() {
  MetaPostEditor editor;
  editor.resize(900,700); editor.show();
  auto* code = editor.textEdit();
  editor.initializeSource("alpha Alpha alphabet\nalpha\n");
  editor.showReplace();
  auto* find = editor.findChild<QLineEdit*>("findText");
  auto* replacement = editor.findChild<QLineEdit*>("replacementText");
  auto* words = editor.findChild<QCheckBox*>("wholeWords");
  auto* matchCase = editor.findChild<QCheckBox*>("matchCase");
  find->setText("alpha");
  code->moveCursor(QTextCursor::Start);
  editor.findNext(); require(code->textCursor().selectionStart() == 0, "Find first match");
  editor.findNext(); require(code->textCursor().selectionStart() == 6, "Case insensitive next match");
  editor.findPrevious(); require(code->textCursor().selectionStart() == 0, "Previous match");
  editor.findPrevious(); require(code->textCursor().selectionStart() == 21, "Previous wraps");
  words->setChecked(true); matchCase->setChecked(true);
  replacement->setText("beta");
  editor.replaceAll();
  require(editor.toPlainText() == "beta Alpha alphabet\nbeta\n", "Replace all respects case and whole words");
  code->undo(); require(editor.toPlainText() == "alpha Alpha alphabet\nalpha\n", "Replace all is one undo step");
  code->redo(); require(editor.toPlainText().startsWith("beta"), "Replace all redo");
  editor.showFind(); find->setText("missing"); editor.findNext();
  find->setText("beta"); replacement->setText("gamma");
  code->moveCursor(QTextCursor::Start); editor.findNext(); editor.replaceOne();
  require(editor.toPlainText().startsWith("gamma Alpha"), "Replace selected occurrence");
  code->undo(); require(editor.toPlainText().startsWith("beta Alpha"), "Single replacement Undo");
  require(editor.findChild<QLabel*>("codeState")->text().contains("Unexecuted"), "Draft status visible");
  require(editor.findChild<QWidget*>("lineNumberArea")->width() > 0, "Line number gutter present");
  QString many;
  for (int i=0; i<150; ++i) many += QString("line %1\n").arg(i);
  editor.initializeSource(many);
  QApplication::processEvents();
  code->moveCursor(QTextCursor::End); code->ensureCursorVisible();
  QApplication::processEvents();
  require(code->verticalScrollBar()->value() > 0, "Editor scrolls long source");
  require(editor.findChild<QWidget*>("lineNumberArea")->width() > code->fontMetrics().horizontalAdvance("999"),
      "Gutter grows for three digit line numbers");

  editor.initializeSource("% fill 123\nsetMarkAnchor(marks_topmarks,z.marktop);\nmessage \"% fill 123\";\nx := .5 .. 20;\nbtex\nhello % TeX\netex\n");
  QApplication::processEvents();
  const auto comment = colourAt(code->document(),0,0);
  const auto command = colourAt(code->document(),1,0);
  const auto parameter = colourAt(code->document(),1,30);
  const auto string = colourAt(code->document(),2,9);
  require(comment.isValid() && command.isValid() && parameter.isValid() && string.isValid(), "Syntax categories are highlighted");
  require(comment != command && string != comment && parameter != command, "Comments, strings, commands and parameters differ");
  require(colourAt(code->document(),2,18) == string, "Percent inside string is not a comment");
  require(colourAt(code->document(),5,0) == string, "Embedded TeX retains state across lines");
  require(!code->document()->isUndoAvailable() && !editor.hasPendingChanges(), "Highlighting never changes text or undo");

  const QString base = "header\nz.point = (10,20);\nbody A\nfooter\n";
  editor.initializeSource(base);
  replace(code,"body A","body B");
  auto selection = code->document()->find("body B"); code->setTextCursor(selection);
  const int cursor = selection.position(), anchor = selection.anchor();
  const auto drawing = QString(base).replace("(10,20)","(30,40)");
  editor.synchronizeSource(drawing);
  require(editor.toPlainText() == QString(drawing).replace("body A","body B"), "Merge disjoint graphical and unexecuted code edits");
  require(!editor.hasConflict() && editor.hasPendingChanges(), "Merged draft remains unexecuted");
  require(code->textCursor().position() == cursor && code->textCursor().anchor() == anchor, "Preserve selection outside changed lines");
  code->undo(); require(editor.toPlainText() == QString(base).replace("body A","body B"), "Undo synchronized drawing edit");
  code->undo(); require(editor.toPlainText() == base, "Previous typing undo survives synchronization");
  code->redo(); code->redo();
  editor.synchronizeSource(base);
  require(editor.toPlainText() == QString(base).replace("body A","body B"), "Graphical undo preserves pending draft");
  const auto typed = editor.toPlainText();
  editor.synchronizeSource(base);
  require(editor.toPlainText() == typed, "Repeated notifications leave draft intact");

  editor.initializeSource(base);
  replace(code,"(10,20)","(12,24)");
  const auto draft = editor.toPlainText();
  const int undoSteps = code->document()->availableUndoSteps();
  editor.synchronizeSource(drawing);
  require(editor.hasConflict() && editor.toPlainText() == draft, "Conflicting draft is never overwritten");
  require(code->document()->availableUndoSteps() == undoSteps, "Conflict leaves text undo untouched");
  editor.useDrawing();
  require(!editor.hasConflict() && !editor.hasPendingChanges() && editor.toPlainText() == drawing, "Choose drawing explicitly");
  code->undo(); require(editor.toPlainText() == draft, "Discarded conflict draft remains recoverable through Undo");
  editor.synchronizeSource(drawing);
  editor.keepDraft(); require(!editor.hasConflict() && editor.toPlainText() == draft, "Choose draft explicitly");

  // Boundary cases exercise insertion/deletion at EOF and around blank lines.
  const QStringList examples{"", "a", "a\n", "a\nb", "a\nb\n", "\na\n", "a\n\nb\n", "a\nβ\nب"};
  for (const auto& before : examples) for (const auto& after : examples) {
    editor.initializeSource(before); editor.synchronizeSource(after);
    require(editor.toPlainText() == after && !editor.hasConflict(), "Exact synchronization including EOF and Unicode");
    if (before != after) { code->undo(); require(editor.toPlainText() == before, "Exact synchronization Undo"); }
  }
  for (int seed=0; seed<40; ++seed) {
    QString before, after;
    for (int i=0; i<30; ++i) {
      before += QString("line %1\n").arg(i);
      if ((i+seed)%7 == 0) after += "inserted\n";
      if ((i+seed)%5 != 0) after += QString("line %1%2\n").arg(i).arg((i+seed)%3 ? "" : " changed");
    }
    editor.initializeSource(before); editor.synchronizeSource(after);
    require(editor.toPlainText() == after, "Multiple separated insertion/deletion/replacement edits");
    code->undo(); require(editor.toPlainText() == before, "Multiple synchronized edits undo atomically");
  }
}
static void integration(const QString& screenshot) {
  QTemporaryDir temporary;
  const QDir dir(temporary.path());
  write(dir.filePath("test.mp"),"lineheight:=88;penwidth:=88;nuqta:=123.2;showAnchors:=1;\n");
  write(dir.filePath("glyphs.mp"),R"(defchar(editor.test,123,300,-1,-1);
%%beginparams
z.p = (40,130);
%%beginpaths
fill (0,0)--(180,0)--(180,100)--(0,100)--cycle;
%%endpaths
%%beginbody
pair indirect;
indirect := z.p;
setMarkAnchor(top,z.p);
setMarkAnchor(shared,indirect);
enddefchar;
)");
  Font font;
  require(font.loadFile(dir.filePath("test.mp")),"Load integration font");
  QApplication::restoreOverrideCursor();
  auto* glyph = font.glyphs.first();
  std::unique_ptr<GlyphWindow> window(new GlyphWindow(glyph));
  window->resize(1450,950); window->show(); QApplication::processEvents();
  auto* editor = window->findChild<MetaPostEditor*>();
  auto* execute = window->findChild<QPushButton*>("executeCode");
  auto* code = editor->textEdit();
  const auto original = glyph->source();
  window->activateWindow(); code->setFocus(); QApplication::processEvents();
  auto* view = window->findChild<GlyphView*>();
  auto* parameter=itemWithKey(view->getScene(),SourceLinks::parameterKey("z.p"));
  require(parameter,"Parameter has a stable source identity");
  auto declaration=code->document()->find("z.p =");
  code->setTextCursor(declaration);
  require(parameter->data(SourceLinks::HighlightRole).toBool(), "Selecting a parameter declaration highlights its point");
  auto* shared=itemWithKey(view->getScene(),SourceLinks::anchorKey("shared",1),"anchorMarker");
  require(shared && shared->data(SourceLinks::HighlightRole).toBool(), "Shared and indirect anchors highlight from their common parameter");
  require(view->scene()->selectedItems().isEmpty(), "Source navigation does not change drag selection");
  const int cursorPosition=code->textCursor().position(), cursorAnchor=code->textCursor().anchor();
  view->setFocus(); view->scene()->clearSelection(); parameter->setSelected(true);
  require(linkedSource(code).join('\n').contains("z.p ="), "Selecting graphical parameter highlights its assignment");
  require(code->textCursor().position()==cursorPosition && code->textCursor().anchor()==cursorAnchor,
      "Graphical selection preserves text cursor and selection");
  auto* literal=itemWithKey(view->getScene(),SourceLinks::pointKey(0,1));
  require(literal,"Outline knot has a stable path/point identity");
  view->scene()->clearSelection(); literal->setSelected(true);
  require(linkedSource(code).contains("(180,0)"), "Selecting an outline point highlights its path expression");
  code->setFocus(); auto pointCode=code->document()->find("(180,0)"); code->setTextCursor(pointCode);
  require(literal->data(SourceLinks::HighlightRole).toBool(), "Selecting a path expression highlights its outline point");
  auto* anchorLabel=itemWithKey(view->getScene(),SourceLinks::anchorKey("top",1),"anchorLabel");
  require(anchorLabel && anchorLabel->isVisible(),"Computed anchor label is visible");
  view->scene()->clearSelection(); view->setFocus();
  QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,
      anchorLabel->deviceTransform(view->viewportTransform()).map(anchorLabel->boundingRect().center()).toPoint());
  require(linkedSource(code).join('\n').contains("setMarkAnchor(top,z.p)"), "Clicking computed anchor label highlights its declaration");
  const auto labelPosition=anchorLabel->deviceTransform(view->viewportTransform()).map(anchorLabel->boundingRect().center()).toPoint();
  QTest::mousePress(view->viewport(),Qt::LeftButton,Qt::NoModifier,labelPosition);
  QGraphicsSceneMouseEvent labelMove(QEvent::GraphicsSceneMouseMove);
  labelMove.setScenePos(view->mapToScene(labelPosition+QPoint(8,8)));
  labelMove.setLastScenePos(view->mapToScene(labelPosition)); labelMove.setButtons(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&labelMove);
  QTest::mouseRelease(view->viewport(),Qt::LeftButton,Qt::NoModifier,labelPosition+QPoint(8,8));
  require(!window->isWindowModified() && glyph->source()==original && !code->document()->isUndoAvailable(),
      "Bidirectional navigation changes neither source, undo nor dirty state");
  require(glyph->undoStack()->count()==0,"Computed anchor label drag creates no geometry undo entry");
  if (!screenshot.isEmpty()) require(window->grab().save(screenshot+".links.png"),"Save linked-selection preview");
  code->setFocus();
  QTest::keySequence(code, QKeySequence::Find);
  require(editor->findChild<QLineEdit*>("findText")->hasFocus(), "Find shortcut opens the editor search bar");
  QTest::keyClick(editor->findChild<QLineEdit*>("findText"), Qt::Key_Escape);
  require(code->hasFocus(), "Escape closes search and returns focus to code");
  replace(code,"setMarkAnchor(top,z.p);","setMarkAnchor(top,z.p);\n% unfinished code stays here");
  require(window->isWindowModified(), "Unexecuted text changes participate in close protection");
  QTest::keySequence(code,QKeySequence::Undo);
  require(editor->toPlainText() == original, "Undo shortcut edits code when source editor has focus");
  QTest::keySequence(code,QKeySequence::Redo);
  require(editor->toPlainText().contains("unfinished"), "Redo shortcut restores unexecuted code");
  // Exercise the real graphical point movement, not just the source synchronization API.
  PairItem* editable = nullptr;
  for (auto* item : view->scene()->items()) if (auto* pair = dynamic_cast<PairItem*>(item))
    if (pair->toolTip().contains("z.p")) editable = pair;
  require(editable,"Graphical anchor parameter is editable");
  const auto start = editable->mapToScene(QPointF());
  const auto end = editable->mapToScene(QPointF(20,20));
  QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
  press.setScenePos(start); press.setButton(Qt::LeftButton); press.setButtons(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&press);
  QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
  move.setScenePos(end); move.setLastScenePos(start); move.setButtons(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&move);
  QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
  release.setScenePos(end); release.setLastScenePos(end); release.setButton(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&release);
  require(editor->toPlainText().contains("unfinished code stays here") && editor->toPlainText().contains("(60,150)"),
      "Actual graphical property changes merge into pending source");
  require(!editor->hasConflict() && execute->isEnabled(), "Disjoint edits allow execution");
  for (auto* action : window->findChildren<QAction*>()) if (action->shortcut() == QKeySequence::Save) action->trigger();
  require(editor->toPlainText().contains("unfinished") && window->statusBar()->currentMessage().contains("unexecuted"),
      "Save cannot silently omit pending source changes");
  view->setFocus(); QApplication::processEvents();
  QTest::keySequence(view,QKeySequence::Undo);
  require(editor->toPlainText().contains("(40,130)") && editor->toPlainText().contains("unfinished"), "Actual graphical undo keeps draft");
  QTest::keySequence(view,QKeySequence::Redo);
  editor->textEdit()->setFocus(); QApplication::processEvents();
  require(window->findChildren<QAction*>().size() > 0, "Actions available");
  QTest::keySequence(code,QKeySequence::Replace);
  require(editor->findChild<QLineEdit*>("replacementText")->isVisible(), "Replace shortcut expands replacement controls");
  editor->findChild<QLineEdit*>("findText")->setText("setMarkAnchor");
  editor->findChild<QLineEdit*>("replacementText")->setText("setMarkAnchor");
  QApplication::processEvents();
  if (!screenshot.isEmpty()) require(window->grab().save(screenshot),"Save editor screenshot");
  window->executeCommands();
  require(!editor->hasConflict() && !editor->hasPendingChanges(), "Executing own draft accepts normalized source without conflict");
  require(glyph->source().contains("unfinished code stays here"), "Executed body comments retained");
  require(code->document()->isUndoAvailable(), "Execution preserves text history");
  const auto current = glyph->source();
  replace(code,"(60,150)","(70,160)");
  glyph->setPropertyWithUndo("z.p",QPointF(80,170));
  const auto conflicted = editor->toPlainText();
  require(editor->hasConflict() && !execute->isEnabled(), "Actual same-parameter conflict blocks Execute");
  const int index = glyph->undoStack()->index();
  window->executeCommands();
  require(glyph->undoStack()->index() == index && editor->toPlainText() == conflicted,
      "Programmatic Execute cannot overwrite conflicting drawing or code");
  editor->keepDraft();
  require(execute->isEnabled(), "Explicit conflict choice enables Execute");
  window->executeCommands();
  require(glyph->source().contains("(70,160)") && !editor->hasPendingChanges(), "Explicitly chosen draft executes");
  glyph->undoStack()->undo();
  require(editor->toPlainText().contains("(80,170)") && !editor->hasConflict(), "Drawing undo after execution synchronizes");
  require(original != current, "Fixture genuinely changed model");
}
static void realFontLinks(const QString& project,const QString& screenshot) {
  Font font;
  require(font.loadFile(project),"Load real font for read-only source navigation");
  QApplication::restoreOverrideCursor();
  // The final form reuses the isolated form's drawing macro.
  for (const auto& name:QStringList{"noon.isol.expa","noon.fina.expa.afterbeh"}) {
    auto* glyph=font.glyphperName.value(name);
    require(glyph,"Real noon glyph exists");
    const auto before=glyph->source();
    std::unique_ptr<GlyphWindow> window(new GlyphWindow(glyph));
    window->resize(1500,950); window->show(); window->activateWindow(); QApplication::processEvents();
    auto* editor=window->findChild<MetaPostEditor*>(); auto* code=editor->textEdit();
    auto* view=window->findChild<GlyphView*>();
    code->setFocus();
    auto declaration=code->document()->find(name=="noon.isol.expa" ? "z.marktop.offset.noon.isol.expa =" : "z.marktop.offset.noon.fina.expa.afterbeh =");
    require(!declaration.isNull(),"Real upper-mark offset declaration exists");
    code->setTextCursor(declaration);
    auto* anchor=itemWithKey(view->getScene(),SourceLinks::anchorKey("marks_topmarks",1),"anchorMarker");
    require(anchor && anchor->data(SourceLinks::HighlightRole).toBool(),"Real computed anchor follows its placement expression and authored offset");
    if (name=="noon.isol.expa") {
      auto* point=itemWithKey(view->getScene(),SourceLinks::pointKey(0,0));
      require(point,"Real parameterized outline point has a source identity");
      view->setFocus(); view->scene()->clearSelection(); point->setSelected(true);
      require(!linkedSource(code).isEmpty(),"Real parameterized outline expression maps to its declaration");
    }
    require(glyph->source()==before && !editor->hasPendingChanges() && !window->isWindowModified() && glyph->undoStack()->count()==0,
        "Real-font inspection preserves source and undo state");
    if (!screenshot.isEmpty()) require(window->grab().save(screenshot+"."+name+".png"),"Save real-font navigation preview");
  }
}
int main(int argc, char** argv) {
  QApplication application(argc,argv);
  try {
    standalone();
    sourceIndex();
    integration(argc>1 ? QString::fromLocal8Bit(argv[1]) : QString());
    if (argc>2) realFontLinks(QString::fromLocal8Bit(argv[2]),QString::fromLocal8Bit(argv[1]));
    std::cout << "MetaPost editor checks passed\n";
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
