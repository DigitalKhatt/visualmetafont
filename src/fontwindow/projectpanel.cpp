#include "projectpanel.hpp"
#include "mdichild.h"
#include "glyphcellitem.hpp"
#include "glyphwindow.h"
#include <QtWidgets>

ProjectPanel::ProjectPanel(MdiChild* child, QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  auto* title = new QLabel(child->font->project.title(), this);
  title->setWordWrap(true);
  layout->addWidget(title);
  auto* explanation = new QLabel(tr("Groups organize the editor. All glyphs remain in the font."), this);
  explanation->setWordWrap(true);
  layout->addWidget(explanation);
  auto* search = new QLineEdit(this);
  search->setObjectName("projectGlyphSearch");
  search->setPlaceholderText(tr("Search name or U+ code"));
  layout->addWidget(search);
  auto* sources = new QComboBox(this);
  sources->setObjectName("projectGlyphSource");
  sources->addItem(tr("All source files"), "");
  for (const auto& file : child->font->glyphSourceFiles()) sources->addItem(file, file);
  layout->addWidget(sources);
  auto* newFile = new QPushButton(tr("New source file…"), this);
  newFile->setObjectName("newGlyphSourceFile");
  layout->addWidget(newFile);
  connect(newFile, &QPushButton::clicked, this, [this, child, sources] {
    const auto path = QFileDialog::getSaveFileName(this, tr("New glyph source file"), child->font->currentDir() + "/glyphs/new.mp", tr("MetaPost glyph sources (*.mp)"), nullptr, QFileDialog::DontConfirmOverwrite);
    if (path.isEmpty()) return;
    const auto relative = QDir(child->font->currentDir()).relativeFilePath(path);
    if (!child->font->createGlyphFile(relative)) {
      QMessageBox::warning(this, tr("Cannot create source file"), child->font->projectError);
      return;
    }
    const auto file = QDir::cleanPath(relative);
    sources->addItem(file, file); sources->setCurrentIndex(sources->count() - 1);
  });
  auto* groups = new QListWidget(this);
  groups->setObjectName("projectGlyphGroups");
  groups->setSelectionMode(QAbstractItemView::ExtendedSelection);
  auto* all = new QListWidgetItem(tr("All glyphs"), groups);
  all->setData(Qt::UserRole, "");
  const auto descriptors = child->font->project.groups();
  for (const auto& group : descriptors) {
    int count = 0;
    for (auto* glyph : child->font->glyphs)
      if (child->font->project.glyphGroups(glyph->name()).contains(group.id)) ++count;
    auto* item = new QListWidgetItem(QString("%1 (%2)").arg(group.label).arg(count), groups);
    item->setData(Qt::UserRole, group.id);
  }
  auto* ungrouped = new QListWidgetItem(tr("Ungrouped"), groups);
  ungrouped->setData(Qt::UserRole, "__ungrouped__");
  all->setSelected(true);
  layout->addWidget(groups);
  auto* count = new QLabel(this);
  count->setObjectName("projectGlyphCount");
  auto apply = [child, groups, search, sources, count, descriptors] {
    for (int i = 0; i < descriptors.size(); ++i) {
      int members = 0;
      for (auto* glyph : child->font->glyphs)
        members += child->font->project.glyphGroups(glyph->name()).contains(descriptors[i].id);
      groups->item(i + 1)->setText(QString("%1 (%2)").arg(descriptors[i].label).arg(members));
    }
    QStringList selected;
    for (const auto* item : groups->selectedItems()) selected.append(item->data(Qt::UserRole).toString());
    const auto visible = child->filterGlyphs(selected, search->text(), sources->currentData().toString());
    count->setText(tr("%1 of %2 source glyphs").arg(visible).arg(child->font->glyphs.size()));
  };
  connect(groups, &QListWidget::itemSelectionChanged, this, apply);
  connect(search, &QLineEdit::textChanged, this, apply);
  connect(sources, qOverload<int>(&QComboBox::currentIndexChanged), this, apply);
  layout->addWidget(count);
  auto* create = new QPushButton(tr("New glyph…"), this);
  create->setObjectName("newProjectGlyph");
  layout->addWidget(create);
  connect(create, &QPushButton::clicked, this, [this, child, sources, apply] {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("New glyph"));
    auto* form = new QFormLayout(&dialog);
    auto* name = new QLineEdit(&dialog); name->setObjectName("newGlyphName");
    auto* unicode = new QLineEdit(&dialog); unicode->setObjectName("newGlyphUnicode");
    unicode->setPlaceholderText(tr("U+067E, or empty for an unencoded glyph"));
    auto* destination = new QComboBox(&dialog); destination->setObjectName("newGlyphDestination");
    destination->addItems(child->font->glyphSourceFiles());
    if (!sources->currentData().toString().isEmpty()) destination->setCurrentText(sources->currentData().toString());
    form->addRow(tr("Name"), name); form->addRow(tr("Unicode"), unicode); form->addRow(tr("Source file"), destination);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    auto text = unicode->text().trimmed();
    if (text.startsWith("U+", Qt::CaseInsensitive) || text.startsWith("0x", Qt::CaseInsensitive)) text = text.mid(2);
    bool valid = unicode->text().trimmed().isEmpty() || !text.isEmpty();
    const int code = text.isEmpty() ? -1 : text.toInt(&valid, 16);
    if (!valid) { QMessageBox::warning(this, tr("Invalid Unicode"), tr("Enter a hexadecimal Unicode value, or leave it empty.")); return; }
    auto* glyph = child->font->createGlyph(name->text().trimmed(), code, destination->currentText());
    if (!glyph) { QMessageBox::warning(this, tr("Cannot create glyph"), child->font->projectError); return; }
    child->refresh();
    sources->setCurrentIndex(0); apply();
    auto* window = new GlyphWindow(glyph);
    window->setWindowModified(true);
    window->showMaximized();
  });
  auto* move = new QPushButton(tr("Move selected glyphs to file…"), this);
  move->setObjectName("moveProjectGlyphs");
  layout->addWidget(move);
  connect(move, &QPushButton::clicked, this, [this, child, apply] {
    QVector<Glyph*> selected;
    for (auto* item : child->scene()->selectedItems())
      if (auto* cell = qgraphicsitem_cast<GlyphCellItem*>(item)) selected.append(cell->getGlyph());
    if (selected.isEmpty()) { QMessageBox::information(this, tr("Move glyphs"), tr("Select glyph cells first.")); return; }
    bool accepted = false;
    const auto destination = QInputDialog::getItem(this, tr("Move glyphs"), tr("Destination source file"), child->font->glyphSourceFiles(), 0, false, &accepted);
    if (!accepted) return;
    if (!child->font->moveGlyphs(selected, destination)) QMessageBox::warning(this, tr("Cannot move glyphs"), child->font->projectError);
    else { child->refresh(); apply(); }
  });
  auto* assign = new QPushButton(tr("Assign groups to selected glyphs…"), this);
  assign->setObjectName("assignGlyphGroups");
  assign->setEnabled(!descriptors.isEmpty());
  layout->addWidget(assign);
  connect(assign, &QPushButton::clicked, this, [this, child, descriptors, apply] {
    QVector<Glyph*> selected;
    for (auto* item : child->scene()->selectedItems())
      if (auto* cell = qgraphicsitem_cast<GlyphCellItem*>(item)) selected.append(cell->getGlyph());
    if (selected.isEmpty()) {
      QMessageBox::information(this, tr("Glyph groups"), tr("Select glyph cells first."));
      return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Groups for %1 glyph(s)").arg(selected.size()));
    auto* form = new QVBoxLayout(&dialog);
    form->addWidget(new QLabel(tr("Partially checked groups keep each glyph's existing membership."), &dialog));
    QVector<QCheckBox*> checks;
    for (const auto& group : descriptors) {
      auto* check = new QCheckBox(group.label, &dialog);
      check->setTristate(true);
      int members = 0;
      for (auto* glyph : selected)
        members += child->font->project.glyphGroups(glyph->name()).contains(group.id);
      check->setCheckState(members == 0 ? Qt::Unchecked : members == selected.size() ? Qt::Checked : Qt::PartiallyChecked);
      checks.append(check); form->addWidget(check);
    }
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    const auto previous = child->font->project;
    for (auto* glyph : selected) {
      auto membership = previous.glyphGroups(glyph->name());
      for (int i = 0; i < checks.size(); ++i) {
        if (checks[i]->checkState() == Qt::Checked && !membership.contains(descriptors[i].id)) membership.append(descriptors[i].id);
        if (checks[i]->checkState() == Qt::Unchecked) membership.removeAll(descriptors[i].id);
      }
      child->font->project.setGlyphGroups(glyph->name(), membership);
    }
    if (!child->font->saveProjectMetadata()) {
      child->font->project = previous;
      QMessageBox::warning(this, tr("Cannot save project groups"), child->font->projectError);
    }
    apply();
  });
  auto* axesBox = new QGroupBox(tr("OpenType axes"), this);
  auto* axesLayout = new QVBoxLayout(axesBox);
  for (const auto& axis : child->font->project.openTypeAxes()) {
    QStringList targets;
    for (const auto& mapping : axis.mappings) targets.append(mapping.internalParameter);
    auto* label = new QLabel(QString("%1 — %2\n%3 / %4 / %5\n→ %6")
        .arg(axis.tag, axis.label).arg(axis.minimum).arg(axis.defaultValue).arg(axis.maximum)
        .arg(targets.join(", ")), axesBox);
    label->setWordWrap(true);
    axesLayout->addWidget(label);
  }
  auto* exportNote = new QLabel(tr("Project axis mappings are separate from internal parameters. Axis export is configured in the font source."), axesBox);
  exportNote->setWordWrap(true);
  axesLayout->addWidget(exportNote);
  layout->addWidget(axesBox);
  auto* parametersNote = new QLabel(tr("Edit internal design parameters in each glyph's Parameters panel. Weight variation is not designed yet."), this);
  parametersNote->setWordWrap(true);
  layout->addWidget(parametersNote);
  apply();
}
