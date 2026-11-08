/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#include "glyphscene.hpp"

#include <qgraphicsview.h>
#include <qmetaobject.h>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <knotcontrolleditem.hpp>

#include "commands.h"
#include "font.hpp"
#include "pairitem.hpp"
#include "qmessagebox.h"
#include "sourcelinks.hpp"

GlyphScene::GlyphScene(QObject* parent) : QGraphicsScene(parent) {  // QGraphicsScene(-1000, -1700, 3000, 2500, parent) {

  pointerPosition = new QLabel("Hello");
  ruler = NULL;
  myMode = MoveItem;
  ismoving = false;
  itemFlags = ItemFlags::None;

  contour = NULL;
  image = NULL;
  guides = NULL;
  m_glyph = NULL;

  // connect(this, &QGraphicsScene::selectionChanged, this, &GlyphScene::selectionChangedSlot);
}
GlyphScene::~GlyphScene() {
}
void GlyphScene::selectionChangedSlot() {
  int i;
  i = 0;
}
void GlyphScene::setGlyph(Glyph* glyph) {
  if (this->m_glyph) {
    disconnect(this->m_glyph, &Glyph::valueChanged, this, &GlyphScene::glyphValueChanged);
  }

  if (m_glyph) disconnect(m_glyph->font, &Font::anchorsVisibilityChanged, this, nullptr);
  this->m_glyph = glyph;
  hiddenAnchors_.clear();
  selectedAnchorKey_.clear();
  qDeleteAll(referenceItems_);
  referenceItems_.clear();
  layers_.clear();

  qDeleteAll(layerContours_);
  layerContours_.clear();

  if (!components.isEmpty()) {
    for (int i = 0; i < components.length(); i++) {
      delete components[i];
    }
    components.clear();
  }

  contour = new ContourItem();
  layerContours_[QString()]=contour;
  contour->setPos(0, 0);
  addItem(contour);
  contour->setGlyph(glyph);

  connect(glyph, &Glyph::valueChanged, this, &GlyphScene::glyphValueChanged, Qt::UniqueConnection);

  if (image != NULL) {
    delete image;
  }

  image = new ImageItem(glyph);
  addItem(image);

  Glyph::ImageInfo imageInfo = glyph->image();
  if (!imageInfo.path.isEmpty()) {
    loadImage(imageInfo);
  }

  Glyph::QHashGlyphComponentInfo localcomponents = glyph->components();
  Glyph::QHashGlyphComponentInfo::iterator comp;
  for (comp = localcomponents.begin(); comp != localcomponents.end(); ++comp) {
    Glyph* compglyph = comp.key();
    // QPainterPath compath = compglyph->getPath();
    // Glyph::ComponentInfo info = comp.value();
    // compath = comp.value().transform.map(compath);

    ComponentItem* compitem = new ComponentItem(glyph, compglyph);
    // compitem->setPath(compath);
    // compitem->setPos(QPointF(info.pos.x(),-info.pos.y()));

    components.append(compitem);

    addItem(compitem);
  }

  if (guides)
    delete guides;

  guides = new GuidesItem(glyph);

  addItem(guides);

  refreshReferences();
  updatePictureDecorations();

  connect(glyph->font, &Font::anchorsVisibilityChanged, this, [this] { emit anchorsChanged(); });
  views().first()->centerOn(contour);
}
QString GlyphScene::pictureName() const { return contour ? contour->pictureName() : QString(); }
QStringList GlyphScene::pictureNames() const {
  auto* final=layerContours_.value(QString());
  return final ? final->pictureNames() : QStringList();
}
bool GlyphScene::pictureHasEditablePaths() const { return contour && contour->hasEditablePaths(); }
bool GlyphScene::hasImage() const { return m_glyph && !m_glyph->image().path.isEmpty(); }
bool GlyphScene::layerVisible(const QString& name) const { return layers_.value(name).visible; }
bool GlyphScene::layerLocked(const QString& name) const { return layers_.value(name).locked; }
bool GlyphScene::activeLayerEditable() const {
  return layerVisible(pictureName()) && !layerLocked(pictureName());
}
QString GlyphScene::itemLayerName(const QGraphicsItem* item) const {
  for (auto* parent=item;parent;parent=parent->parentItem()) {
    if (parent==image) return imageLayerName();
    if (auto* layer=dynamic_cast<const ContourItem*>(parent)) return layer->pictureName();
  }
  return QString(); // Components belong to Final glyph.
}
bool GlyphScene::itemLayerEditable(const QGraphicsItem* item) const {
  const auto name=itemLayerName(item);
  return item && item->isVisible() && item->isEnabled() && layerVisible(name) && !layerLocked(name);
}
void GlyphScene::clearLayerSelection(const QString& name) {
  for (auto* item:selectedItems()) if (itemLayerName(item)==name) item->setSelected(false);
}
void GlyphScene::setLayerVisible(const QString& name, bool visible) {
  if ((!name.isEmpty() && name!=imageLayerName() && !pictureNames().contains(name)) || layerVisible(name)==visible) return;
  QScopedValueRollback<bool> rebuilding(rebuilding_,true);
  clearLayerSelection(name);
  layers_[name].visible=visible;
  refreshReferences();
  updatePictureDecorations();
  emit anchorsChanged();
  emit picturesChanged();
}
void GlyphScene::setLayerLocked(const QString& name, bool locked) {
  if ((!name.isEmpty() && name!=imageLayerName() && !pictureNames().contains(name)) || layerLocked(name)==locked) return;
  QScopedValueRollback<bool> rebuilding(rebuilding_,true);
  clearLayerSelection(name);
  layers_[name].locked=locked;
  refreshReferences();
  updatePictureDecorations();
  emit anchorsChanged();
  emit picturesChanged();
}
void GlyphScene::setFinalPositions(bool enabled) {
  if (finalPositions_==enabled) return;
  QScopedValueRollback<bool> rebuilding(rebuilding_,true);
  clearSelection(); finalPositions_=enabled;
  for (auto* layer:layerContours_) layer->setFinalPositions(enabled);
  refreshReferences(); updatePictureDecorations();
  highlightSourceKeys(highlightedSourceKeys_);
  emit anchorsChanged();
  emit picturesChanged();
}
void GlyphScene::refreshReferences() {
  if (!m_glyph || !contour) return;
  auto* edge=m_glyph->getEdge();
  const auto info=edge ? m_glyph->font->getMPGlyphInfo(edge->charcode) : MPGlyphInfo{};
  auto names=pictureNames(); names.prepend(QString());
  for (auto it=layerContours_.begin();it!=layerContours_.end();) {
    if (!names.contains(it.key())) {
      if (it.value()==contour) contour=layerContours_.value(QString());
      delete it.value(); it=layerContours_.erase(it);
    } else ++it;
  }
  for (const auto& name:names) if (!layerContours_.contains(name)) {
    auto* layer=new ContourItem;
    layerContours_[name]=layer;
    addItem(layer);
    layer->setGlyph(m_glyph,name);
    layer->setFinalPositions(finalPositions_);
    layer->setFillEnabled((itemFlags & ItemFlags::Fill)==ItemFlags::Fill);
  }
  for (auto it=referenceItems_.begin();it!=referenceItems_.end();) {
    if (!names.contains(it.key())) { layers_.remove(it.key()); delete it.value(); it=referenceItems_.erase(it); }
    else ++it;
  }
  for (const auto& name:names) {
    auto*& item=referenceItems_[name];
    if (!item) {
      item=new QGraphicsPathItem;
      item->setTransform(QTransform::fromScale(1,-1));
      item->setZValue(-2);
      item->setAcceptedMouseButtons(Qt::NoButton);
      item->setAcceptHoverEvents(false);
      item->setData(0,"layerReference");
      item->setData(1,name);
      addItem(item);
    }
    const bool visible=layerVisible(name) && (name!=pictureName() || layerLocked(name));
    auto* layer=layerContours_.value(name);
    item->setTransform(layer ? layer->sceneTransform() : QTransform::fromScale(1,-1));
    item->setVisible(visible);
    if (!visible) continue;
    // These are read-only copies of evaluated curves, with no source handles.
    // Both active and reference pictures keep their existing coordinate frame.
    auto* picture=name.isEmpty() ? edge : info.controlledPictures.value(name);
    QPainterPath path; path.setFillRule(Qt::WindingFill);
    for (auto* body=picture ? picture->body : nullptr;body;body=body->next) {
      if (body->type!=mp_fill_code && body->type!=mp_stroked_code) continue;
      auto* fill=reinterpret_cast<mp_fill_object*>(body);
      if (QString::fromUtf8(fill->pre_script)=="begintext") continue;
      path.addPath(Glyph::mp_dump_solved_path(fill->path_p));
    }
    item->setPath(path);
    const QColor color=name.isEmpty() ? QColor(100,105,115) : QColor(45,125,185);
    QPen pen(color); pen.setCosmetic(true);
    item->setPen(pen);
    item->setBrush((itemFlags & ItemFlags::Fill)==ItemFlags::Fill ? QBrush(color) : Qt::NoBrush);
    item->setOpacity(referenceOpacity_);
  }
}
void GlyphScene::updatePictureDecorations() {
  const bool visible=layerVisible(pictureName());
  const bool final=pictureName().isEmpty() && visible;
  for (auto it=layerContours_.cbegin();it!=layerContours_.cend();++it) {
    auto* layer=it.value();
    const bool editable=layerVisible(it.key()) && !layerLocked(it.key());
    layer->setVisible(editable);
    layer->setEnabled(editable);
    layer->setOutlineVisible(layer==contour);
    layer->setZValue(layer==contour ? 1 : 0);
  }
  applyAnchorDisplay();
  if (image) {
    image->setVisible(hasImage() && layerVisible(imageLayerName()));
    const bool editable=layerVisible(imageLayerName()) && !layerLocked(imageLayerName());
    // Disabled graphics items still swallow presses. A locked reference must
    // let them reach the view so its native rubber-band selection can start.
    image->setAcceptedMouseButtons(editable ? Qt::AllButtons : Qt::NoButton);
    image->setEnabled(editable);
  }
  if (guides) guides->setVisible(final);
  for (auto* component:components) {
    component->setVisible(final);
    component->setAcceptedMouseButtons(activeLayerEditable() ? Qt::AllButtons : Qt::NoButton);
    component->setEnabled(activeLayerEditable());
  }
}
void GlyphScene::setPictureName(const QString& name) {
  if (!contour || name==pictureName() || !layerContours_.contains(name)) return;
  QScopedValueRollback<bool> rebuilding(rebuilding_,true);
  clearSelection();
  contour=layerContours_.value(name);
  layers_[pictureName()].visible=true;
  refreshReferences();
  updatePictureDecorations();
  highlightSourceKeys(highlightedSourceKeys_);
  emit anchorsChanged();
  emit picturesChanged(); update();
}
QVector<GlyphScene::AnchorInfo> GlyphScene::anchorInfos() const {
  QMap<QString, AnchorInfo> byKey;
  for (auto layer = layerContours_.cbegin(); layer != layerContours_.cend(); ++layer)
    for (const auto& a : layer.value()->anchorLabelItems) {
      if (!byKey.contains(a.key))
        byKey.insert(a.key, AnchorInfo{a.key, a.text->text(), a.type, a.point, {}});
      byKey[a.key].layers.append(layer.key());
    }
  QVector<AnchorInfo> result;
  for (const auto& info : byKey) result.append(info);
  return result;
}
void GlyphScene::applyAnchorDisplay() {
  QScopedValueRollback<bool> rebuilding(rebuilding_, true);
  for (auto* layer : layerContours_)
    layer->setAnchorDisplayState(hiddenAnchors_, anchorLabelMode_, selectedAnchorKey_);
}
void GlyphScene::setAnchorVisible(const QString& key, bool visible) {
  if (anchorVisible(key) == visible) return;
  if (visible)
    hiddenAnchors_.remove(key);
  else
    hiddenAnchors_.insert(key);
  applyAnchorDisplay();
  emit anchorsChanged();
}
void GlyphScene::showAllAnchors(bool visible) {
  if (visible)
    hiddenAnchors_.clear();
  else
    for (const auto& info : anchorInfos()) hiddenAnchors_.insert(info.key);
  applyAnchorDisplay();
  emit anchorsChanged();
}
void GlyphScene::showOnlyAnchor(const QString& key) {
  for (const auto& info : anchorInfos())
    if (info.key == key)
      hiddenAnchors_.remove(key);
    else
      hiddenAnchors_.insert(info.key);
  applyAnchorDisplay();
  emit anchorsChanged();
  focusAnchor(key);
}
void GlyphScene::setAnchorLabelMode(int mode) {
  mode = qBound(0, mode, 2);
  if (anchorLabelMode_ == mode) return;
  anchorLabelMode_ = mode;
  applyAnchorDisplay();
  emit anchorsChanged();
}
void GlyphScene::focusAnchor(const QString& key) {
  {
    QScopedValueRollback<bool> rebuilding(rebuilding_, true);
    clearSelection();
  }
  highlightSourceKeys({key});
}
void GlyphScene::layoutAnchorLabels() {
  for (auto* layer:layerContours_) if (layer->isVisible()) layer->layoutAnchorLabels();
}
void GlyphScene::highlightSourceKeys(const QStringList& keys) {
  highlightedSourceKeys_=keys;
  QString anchorKey;
  for (const auto& key : keys)
    if (key.startsWith("anchor:")) {
      anchorKey = key;
      break;
    }
  const bool selectionChanged = selectedAnchorKey_ != anchorKey;
  selectedAnchorKey_ = anchorKey;
  if (selectionChanged) applyAnchorDisplay();
  for (auto* item : items()) {
    const auto itemKeys = item->data(SourceLinks::KeysRole).toStringList();
    if (itemKeys.isEmpty()) continue;
    bool highlight = false;
    for (const auto& key : itemKeys)
      if (keys.contains(key)) {
        highlight = true;
        break;
      }
    item->setData(SourceLinks::HighlightRole,highlight);
    const auto kind = item->data(0).toString();
    if (kind == "anchorMarker" || kind == "anchorLeader") {
      QPen pen(highlight ? QColor(190, 85, 0) : QColor(110, 110, 120, 170));
      pen.setCosmetic(true);
      pen.setWidthF(highlight ? 2.5 : 1);
      if (kind == "anchorMarker")
        static_cast<QGraphicsPathItem*>(item)->setPen(pen);
      else
        static_cast<QGraphicsLineItem*>(item)->setPen(pen);
      item->setZValue(highlight ? 5 : (kind == "anchorLeader" ? -1 : 0));
    } else if (kind == "anchorLabel")
      item->setZValue(highlight ? 6 : 0);
    item->update();
  }
  if (selectionChanged) emit anchorSelectionChanged(selectedAnchorKey_);
}

void GlyphScene::setFillEnable(bool enable) {
  if (enable)
    itemFlags = itemFlags | ItemFlags::Fill;
  else
    itemFlags = itemFlags & ~ItemFlags::Fill;

  for (auto* layer:layerContours_) layer->setFillEnabled(enable);
  refreshReferences();
}
void GlyphScene::glyphValueChanged(QString name, bool structureChanged) {
  QScopedValueRollback<bool> rebuilding(rebuilding_,true);
  /*
  if (name == "image" || name == "imagetransform" || name == "source") {
  Glyph::ImageInfo imageInfo = glyph->property("image").value<Glyph::ImageInfo>();
  if (name == "image" || (name == "source" && structureChanged)) {
  if (!imageInfo.path.isEmpty())
  loadImage(imageInfo);
  }
  else {
  image->setPos(imageInfo.pos);
  image->setTransform(imageInfo.transform);
  }
  }*/

  if (ismoving)
    return;

  Glyph::ImageInfo imageInfo = m_glyph->image();

  if (image->path() == imageInfo.path) {
    auto pos = imageInfo.pos;
    if (m_glyph->getEdge() != nullptr) {
      auto edge = m_glyph->getEdge();
      pos = pos + QPointF(edge->xpart, -edge->ypart);
    }

    image->setPos(pos);
    image->setTransform(imageInfo.transform);
  } else if (imageInfo.path.isEmpty()) {
    image->setPath("");
    image->setPixmap(QPixmap());
  } else {
    loadImage(imageInfo);
  }

  auto* edge=m_glyph->getEdge();
  for (auto* layer:layerContours_) layer->generateedge(edge,structureChanged);
  refreshReferences();
  updatePictureDecorations();
  highlightSourceKeys(highlightedSourceKeys_);

  guides->update();

  for (int i = 0; i < components.length(); i++) {
    components[i]->calculatePath();
  }

  update();
  emit anchorsChanged();
  emit picturesChanged();
}
void GlyphScene::loadImage(Glyph::ImageInfo imageInfo) {
  QString imagePath = imageInfo.path;
  QFileInfo imageFileInfo{imagePath};

  if (imageFileInfo.isRelative()) {
    QDir d = QFileInfo(this->m_glyph->font->path()).absoluteDir();
    QString absolute = d.absolutePath();
    imagePath = absolute + "/" + imageInfo.path;
  }

  QImageReader reader(imagePath);
  reader.setAutoTransform(true);
  const QImage newImage = reader.read();
  if (newImage.isNull()) {
    QMessageBox::information(this->views()[0], QGuiApplication::applicationDisplayName(),
                             tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(imagePath), reader.errorString()));
  }

  image->setPath(imageInfo.path);
  image->setPixmap(QPixmap::fromImage(newImage));

  auto pos = imageInfo.pos;
  if (m_glyph->getEdge() != nullptr) {
    auto edge = m_glyph->getEdge();
    pos = pos + QPointF(edge->xpart, -edge->ypart);
  }

  image->setPos(pos);
  image->setTransform(imageInfo.transform);
}
void GlyphScene::setMode(Mode mode) {
  myMode = mode;

  switch (myMode) {
    case Ruler: {
      views().first()->setDragMode(QGraphicsView::NoDrag);
      break;
    }
    case MoveItem: {
      delete ruler;
      ruler = NULL;
      views().first()->setDragMode(QGraphicsView::RubberBandDrag);
      break;
    }
    case AddPoint: {
      views().first()->setDragMode(QGraphicsView::NoDrag);
      QList<QGraphicsItem*> list = items();
      foreach (QGraphicsItem* item, list) {
        item->setSelected(false);
      }
      break;
    }
  }
}
void GlyphScene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
  QString string = QString("%1, %2")
                       .arg(mouseEvent->scenePos().x())
                       .arg(mouseEvent->scenePos().y());  // Update the cursor position text

  pointerPosition->setText(string);

  if (!(mouseEvent->buttons() & Qt::LeftButton))
    return;

  if (myMode == Ruler && ruler != NULL) {
    QLineF newLine(ruler->line().p1(), mouseEvent->scenePos());
    ruler->setLine(newLine);
  } else if (myMode == MoveItem) {
    // QGraphicsScene::mouseMoveEvent(mouseEvent);

    QGraphicsItem* item = mouseGrabberItem();
    if (!item || item->isBlockedByModalPanel())
      return;

    /*for (int i = 0x1; i <= 0x10; i <<= 1) {
      Qt::MouseButton button = Qt::MouseButton(i);
      mouseEvent->setButtonDownPos(button, mouseGrabberButtonDownPos.value(button, item->d_ptr->genericMapFromScene(mouseEvent->scenePos(), mouseEvent->widget())));
      mouseEvent->setButtonDownScenePos(button, mouseGrabberButtonDownScenePos.value(button, mouseEvent->scenePos()));
      mouseEvent->setButtonDownScreenPos(button, mouseGrabberButtonDownScreenPos.value(button, mouseEvent->screenPos()));
    }*/
    mouseEvent->setPos(item->mapFromScene(mouseEvent->scenePos()));
    mouseEvent->setLastPos(item->mapFromScene(mouseEvent->lastScenePos()));

    QList<QGraphicsItem*> selectedItems;
    selectedItems = this->selectedItems();
    bool movable=false;
    for (auto* selected:selectedItems) if (itemLayerEditable(selected) && selected->flags().testFlag(QGraphicsItem::ItemIsMovable)) { movable=true; break; }
    if (!movable) return; // Computed anchors support navigation, not geometry edits.
    ismoving = true;
    wasMouseMoveEvent = true;
    foreach (QGraphicsItem* item, selectedItems) {
      if (itemLayerEditable(item) && item->flags().testFlag(QGraphicsItem::ItemIsMovable)) {
        mouseEvent->setPos(item->mapFromScene(mouseEvent->scenePos()));
        mouseEvent->setLastPos(item->mapFromScene(mouseEvent->lastScenePos()));
        sendEvent(item, mouseEvent);
      }
    }
    ismoving = false;
    m_glyph->setWidth(m_glyph->width());
  }
}
void GlyphScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
  wasMouseMoveEvent = false;

  // if (mouseEvent->button() != Qt::LeftButton)
  //	return;

  Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();

  bool shift = false;
  bool ctrl = false;
  bool alt = false;

  if (modifiers & Qt::ShiftModifier) {
    shift = true;
  }
  if (modifiers & Qt::ControlModifier) {
    ctrl = true;
  }
  if (modifiers & Qt::AltModifier) {
    alt = true;
  }

  switch (myMode) {
    case Ruler: {
      QLineF line = QLineF(mouseEvent->scenePos(), mouseEvent->scenePos());
      if (ruler == NULL) {
        ruler = new RulerItem(line);
        addItem(ruler);
      } else {
        ruler->setLine(line);
      }
      break;
    }
    case MoveItem: {
      if (shift) {
        QList<QGraphicsItem*> selectedItems;
        selectedItems = this->selectedItems();
        if (selectedItems.count() == 1) {
          KnotItem* knotPoint = qgraphicsitem_cast<KnotItem*>(selectedItems[0]);
          if (knotPoint != NULL) {
            KnotControlledItem* selectdpoint = qgraphicsitem_cast<KnotControlledItem*>(knotPoint->parentItem());
            if (selectdpoint != NULL) {
              // addPointAfterPoint(selectdpoint, mouseEvent->scenePos(),"left");
              mouseEvent->setAccepted(true);
              break;
            }
          }
        } else {
          // addnewPath(mouseEvent->scenePos());
          mouseEvent->setAccepted(true);
          break;
        }
      }

      m_oldValues.clear();
      m_newValues.clear();
      old_controlledPaths.clear();
      new_controlledPaths.clear();
      old_params.clear();
      new_params.clear();

      old_params = m_glyph->params;

      auto mo = m_glyph->metaObject();
      for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        m_oldValues[mo->property(i).name()] = mo->property(i).read(m_glyph);
      }

      QList<QByteArray> dynamicProperties = m_glyph->dynamicPropertyNames();
      for (int i = 0; i < dynamicProperties.length(); i++) {
        QByteArray propname = dynamicProperties[i];
        QVariant val = m_glyph->property(propname);
        m_oldValues[propname] = val;
      }

      for (auto j = m_glyph->controlledPaths.cbegin(); j != m_glyph->controlledPaths.cend(); j++) {
        for (auto h = j.value().cbegin(); h != j.value().cend(); h++) {
          old_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
        }
      }

      connect(m_glyph, &Glyph::valueChanged, this, &GlyphScene::recordGlyphChange);

      QGraphicsScene::mousePressEvent(mouseEvent);
      break;
    }
    case AddPoint: {
      QList<QGraphicsItem*> selectedItems;
      selectedItems = this->selectedItems();
      if (selectedItems.count() == 1) {
        KnotItem* knotPoint = qgraphicsitem_cast<KnotItem*>(selectedItems[0]);
        if (knotPoint != NULL) {
          KnotControlledItem* selectdpoint = qgraphicsitem_cast<KnotControlledItem*>(knotPoint->parentItem());
          if (selectdpoint != NULL) {
            addPointAfterPoint(selectdpoint, mouseEvent->scenePos(), "");
            break;
          }
        }
      }

      QGraphicsScene::mousePressEvent(mouseEvent);

      break;
    }
  }
}
void GlyphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
  Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();

  bool shift = false;
  bool ctrl = false;
  bool alt = false;

  if (modifiers & Qt::ShiftModifier) {
    shift = true;
  }
  if (modifiers & Qt::ControlModifier) {
    ctrl = true;
  }
  if (modifiers & Qt::AltModifier) {
    alt = true;
  }

  if (ruler != 0 && myMode == Ruler) {
  } else {
    if (shift) {
      QList<QGraphicsItem*> selectedItems;
      selectedItems = this->selectedItems();
      QPointF begin = mouseEvent->buttonDownScenePos(Qt::LeftButton);
      QPointF end = mouseEvent->scenePos();
      QLineF line(begin, end);
      QString dir;
      if (line.length() > 20) {
        if (line.angle() > -45 && line.angle() <= 45) {
          dir = "right";
        } else if (line.angle() > 45 && line.angle() <= 135) {
          dir = "up";
        } else if (line.angle() > 135 && line.angle() <= 225) {
          dir = "left";
        } else {
          dir = "down";
        }
      }
      if (selectedItems.count() == 1) {
        KnotItem* knotPoint = qgraphicsitem_cast<KnotItem*>(selectedItems[0]);
        if (knotPoint != NULL) {
          KnotControlledItem* selectdpoint = qgraphicsitem_cast<KnotControlledItem*>(knotPoint->parentItem());
          if (selectdpoint != NULL) {
            addPointAfterPoint(selectdpoint, begin, dir);
            mouseEvent->setAccepted(true);
            return;
          }
        }
      } else {
        addnewPath(begin, dir);
        mouseEvent->setAccepted(true);
        return;
      }
    }

    disconnect(m_glyph, &Glyph::valueChanged, this, &GlyphScene::recordGlyphChange);

    if (wasMouseMoveEvent) {
      for (auto j = m_glyph->controlledPaths.cbegin(); j != m_glyph->controlledPaths.cend(); j++) {
        for (auto h = j.value().cbegin(); h != j.value().cend(); h++) {
          new_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
        }
      }

      if (!m_newValues.isEmpty() || !new_controlledPaths.isEmpty()) {
        GlyphParamsChangeCommand* command = new GlyphParamsChangeCommand(m_glyph, "Item(s) Moved", m_oldValues, m_newValues,
                                                                         old_controlledPaths, new_controlledPaths, old_params, new_params);
        m_glyph->undoStack()->push(command);
      }
    }
  }

  wasMouseMoveEvent = false;

  QGraphicsScene::mouseReleaseEvent(mouseEvent);
}
void GlyphScene::recordGlyphChange(QString name) {
  if (name == "imagetransform") {
    name = "image";
  }

  auto pit = m_glyph->params.find(name);

  if (pit != m_glyph->params.end()) {
    new_params[pit->first] = pit->second;
  } else {
    m_newValues[name] = m_glyph->property(name.toLatin1());
  }
}
void GlyphScene::deletePoint(KnotControlledItem* point) {
  if (!itemLayerEditable(point)) return;
  int numpath = point->m_numsubpath;
  int numpoint = point->m_numpoint;

  if (m_glyph->controlledPaths.contains(numpath) && m_glyph->controlledPaths[numpath].contains(numpoint)) {
    QMap<int, Glyph::Knot*> newcontrolledPaths;
    auto controlledPath = m_glyph->controlledPaths[numpath];
    auto iterator = controlledPath.begin();
    while (iterator != controlledPath.end()) {
      if (iterator.key() < numpoint) {
        newcontrolledPaths[iterator.key()] = controlledPath[iterator.key()];
      } else if (iterator.key() == numpoint) {
      } else {
        newcontrolledPaths[iterator.key() - 1] = controlledPath[iterator.key()];
      }
      ++iterator;
    }

    QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths;
    QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths;

    QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
    while (j.hasNext()) {
      j.next();
      QMapIterator<int, Glyph::Knot*> h(j.value());
      while (h.hasNext()) {
        h.next();
        old_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
      }
    }

    m_glyph->controlledPaths[numpath] = newcontrolledPaths;

    QMapIterator<int, QMap<int, Glyph::Knot*> > jj(m_glyph->controlledPaths);
    while (jj.hasNext()) {
      jj.next();
      QMapIterator<int, Glyph::Knot*> h(jj.value());
      while (h.hasNext()) {
        h.next();
        new_controlledPaths[jj.key()][h.key()] = *m_glyph->controlledPaths[jj.key()][h.key()];
      }
    }

    GlyphPathsChangeCommand* command = new GlyphPathsChangeCommand(m_glyph, "Paths(s) changed", old_controlledPaths, new_controlledPaths);
    m_glyph->undoStack()->push(command);
  }
}
void GlyphScene::addPointAfterPoint(KnotControlledItem* point, QPointF newpoint, QString dir) {
  if (!itemLayerEditable(point)) return;
  int numpath = point->m_numsubpath;
  int numpoint = point->m_numpoint;

  if (m_glyph->controlledPaths.contains(numpath) && m_glyph->controlledPaths[numpath].contains(numpoint)) {
    // Explicit-curve insertion changes the neighbouring handles as well as
    // adding a knot. Snapshot before those changes so undo restores them.
    QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths;
    QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths;
    for (auto path = m_glyph->controlledPaths.cbegin(); path != m_glyph->controlledPaths.cend(); ++path) {
      for (auto knot = path.value().cbegin(); knot != path.value().cend(); ++knot) {
        old_controlledPaths[path.key()][knot.key()] = *knot.value();
      }
    }
    QMap<int, Glyph::Knot*> newcontrolledPaths;
    auto controlledPath = m_glyph->controlledPaths[numpath];
    auto iterator = controlledPath.begin();
    while (iterator != controlledPath.end()) {
      if (iterator.key() < numpoint) {
        newcontrolledPaths[iterator.key()] = controlledPath[iterator.key()];
      } else if (iterator.key() == numpoint) {
        auto currentKnot = controlledPath[numpoint];
        auto nextite = iterator + 1;
        Glyph::Knot* nextknot = nullptr;
        if (nextite != controlledPath.end()) {
          nextknot = controlledPath[nextite.key()];
        }

        newcontrolledPaths[iterator.key()] = controlledPath[iterator.key()];
        Glyph::Knot* newknot = new Glyph::Knot();

        auto edge = m_glyph->getEdge();

        QPointF matrix;

        if (edge) {
          matrix = {edge->xpart, edge->ypart};
        }
        const auto position=m_glyph->pictureNames.isEmpty() && itemLayerName(point).isEmpty()
            ? QPointF(newpoint.x(), -newpoint.y())-matrix : point->mapFromScene(newpoint);
        newknot->expr = std::make_unique<PairPathPointExp>(position);

        Glyph::KnotEntryExit left = {};
        Glyph::KnotEntryExit right = {};
        left.jointtype = Glyph::path_join_tension;
        right.jointtype = Glyph::path_join_tension;
        left.tensionExpr = std::make_unique<LitPathNumericExp>(1);
        if (!dir.isEmpty()) {
          left.type = Glyph::mpgui_given;
          left.dirExpr = std::make_unique<VarMFExpr>(dir, false);
        } else {
          left.type = Glyph::mpgui_open;
        }

        right.tensionExpr = std::make_unique<LitPathNumericExp>(1);
        right.type = Glyph::mpgui_open;

        if (nextknot && currentKnot->rightValue.jointtype == Glyph::path_join_control) {
          // Start both new segments with tension 1 and 1, for the designer
          // to adjust. Clear both old control handles and shared-control
          // flags; retaining either would leave an invalid mixed segment.
          // Endpoint expressions (including their axis parameters) stay intact.
          currentKnot->rightValue = right;
          nextknot->leftValue = right;
        }

        newknot->leftValue = left;
        newknot->rightValue = right;

        newcontrolledPaths[iterator.key() + 1] = newknot;
      } else {
        newcontrolledPaths[iterator.key() + 1] = controlledPath[iterator.key()];
      }
      ++iterator;
    }

    m_glyph->controlledPaths[numpath] = newcontrolledPaths;

    QMapIterator<int, QMap<int, Glyph::Knot*> > jj(m_glyph->controlledPaths);
    while (jj.hasNext()) {
      jj.next();
      QMapIterator<int, Glyph::Knot*> h(jj.value());
      while (h.hasNext()) {
        h.next();
        new_controlledPaths[jj.key()][h.key()] = *m_glyph->controlledPaths[jj.key()][h.key()];
      }
    }

    GlyphPathsChangeCommand* command = new GlyphPathsChangeCommand(m_glyph, "Paths(s) changed", old_controlledPaths, new_controlledPaths);
    m_glyph->undoStack()->push(command);

    selectSourcePoint(numpath,numpoint+1);
  }
}
void GlyphScene::addnewPath(QPointF newpoint, QString dir) {
  if (!activeLayerEditable()) return;
  QMap<int, Glyph::Knot*> newpath;

  Glyph::Knot* newknot = new Glyph::Knot();
  // newknot->isConstant = true;
  Glyph::KnotEntryExit left = {};
  Glyph::KnotEntryExit right = {};
  left.jointtype = Glyph::path_join_tension;
  right.jointtype = Glyph::path_join_tension;
  left.tensionExpr = std::make_unique<LitPathNumericExp>(1);
  if (!dir.isNull()) {
    left.type = Glyph::mpgui_given;
    left.dirExpr = std::make_unique<VarMFExpr>(dir, false);
  } else {
    left.type = Glyph::mpgui_open;
  }
  right.tensionExpr = std::make_unique<LitPathNumericExp>(1);
  right.type = Glyph::mpgui_open;
  newknot->leftValue = left;
  newknot->rightValue = right;

  auto edge = m_glyph->getEdge();

  QPointF matrix;

  if (edge && pictureName().isEmpty()) {
    matrix = {edge->xpart, edge->ypart};
  }
  const auto sourcePoint=pictureName().isEmpty()
      ? QPointF(newpoint.x(),-newpoint.y())-matrix : contour->mapFromScene(newpoint);
  newknot->expr = std::make_unique<PairPathPointExp>(sourcePoint);

  int numpath = m_glyph->controlledPaths.size();
  newpath[0] = newknot;
  newpath[1] = newknot;

  m_glyph->controlledPathNames[numpath] = {pictureName().isEmpty() ? "currentpicture" : pictureName(), "fill"};

  QMap<int, QMap<int, Glyph::Knot> > old_controlledPaths;
  QMap<int, QMap<int, Glyph::Knot> > new_controlledPaths;

  QMapIterator<int, QMap<int, Glyph::Knot*> > j(m_glyph->controlledPaths);
  while (j.hasNext()) {
    j.next();
    QMapIterator<int, Glyph::Knot*> h(j.value());
    while (h.hasNext()) {
      h.next();
      old_controlledPaths[j.key()][h.key()] = *m_glyph->controlledPaths[j.key()][h.key()];
    }
  }

  m_glyph->controlledPaths[numpath] = newpath;

  QMapIterator<int, QMap<int, Glyph::Knot*> > jj(m_glyph->controlledPaths);
  while (jj.hasNext()) {
    jj.next();
    QMapIterator<int, Glyph::Knot*> h(jj.value());
    while (h.hasNext()) {
      h.next();
      new_controlledPaths[jj.key()][h.key()] = *m_glyph->controlledPaths[jj.key()][h.key()];
    }
  }

  GlyphPathsChangeCommand* command = new GlyphPathsChangeCommand(m_glyph, "Paths(s) changed", old_controlledPaths, new_controlledPaths);
  m_glyph->undoStack()->push(command);

  selectSourcePoint(numpath,0);
}
void GlyphScene::selectSourcePoint(int path,int point) {
  const auto key=SourceLinks::pointKey(path,point);
  for (auto* item:items()) if (itemLayerEditable(item) && item->data(SourceLinks::KeysRole).toStringList().contains(key)) {
    item->setSelected(true); break;
  }
}
void GlyphScene::keyPressEvent(QKeyEvent* event) {
  currentPressdKeys.insert(event->key());
  QGraphicsScene::keyPressEvent(event);
}
void GlyphScene::keyReleaseEvent(QKeyEvent* event) {
  currentPressdKeys.remove(event->key());
  QGraphicsScene::keyReleaseEvent(event);
}
