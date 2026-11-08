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

#pragma once
#ifndef GLYPHSCENE_H
#define GLYPHSCENE_H
#include <QGraphicsScene>
#include <QMap>
#include <QLabel>
#include "glyph.hpp"
#include "ruleritem.hpp"
#include "contouritem.hpp"
#include "imageitem.hpp"
#include "guidesitem.hpp"
#include "componentitem.hpp"

class KnotControlledItem;

class Glyph;

class GlyphScene : public QGraphicsScene {
	Q_OBJECT
		friend class GlyphView;
	friend class KnotControlledItem;
public:

	enum class ItemFlags {
		None = 0,
		Fill = 4
	};

	GlyphScene(QObject* parent = Q_NULLPTR);
	~GlyphScene();
	enum Mode { MoveItem, Ruler, AddPoint };
	QLabel* pointerPosition;
	void setGlyph(Glyph* glyph);

  static QString imageLayerName() { return QStringLiteral("@image"); }
  bool hasImage() const;
	void setFillEnable(bool  enable);
  void layoutAnchorLabels();
  struct AnchorInfo {
    QString key, name;
    int type;
    QPointF point;
    QStringList layers;
  };
  QVector<AnchorInfo> anchorInfos() const;
  bool anchorVisible(const QString& key) const { return !hiddenAnchors_.contains(key); }
  void setAnchorVisible(const QString& key, bool visible);
  void showAllAnchors(bool visible);
  void showOnlyAnchor(const QString& key);
  int anchorLabelMode() const { return anchorLabelMode_; }
  void setAnchorLabelMode(int mode);
  QString highlightedAnchor() const { return selectedAnchorKey_; }
  void focusAnchor(const QString& key);

  void highlightSourceKeys(const QStringList& keys);
  bool isRebuilding() const { return rebuilding_; }
  QString pictureName() const;
  QStringList pictureNames() const;
  bool pictureHasEditablePaths() const;
  void setPictureName(const QString& name);
  bool layerVisible(const QString& name) const;
  bool layerLocked(const QString& name) const;
  void setLayerVisible(const QString& name, bool visible);
  void setLayerLocked(const QString& name, bool locked);
  bool finalPositions() const { return finalPositions_; }
  void setFinalPositions(bool enabled);

 signals:
  void picturesChanged();
  void anchorsChanged();
  void anchorSelectionChanged(QString key);

 public:

	QSet<int> currentPressdKeys;

	int getControlledPosition(QGraphicsSceneMouseEvent* event) {

		auto scene = this;

		if (scene->currentPressdKeys.contains(Qt::Key_F1)) {
			return 1;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F2)) {
			return 2;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F3)) {
			return 3;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F4)) {
			return 4;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F5)) {
			return 5;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F6)) {
			return 6;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F7)) {
			return 7;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F8)) {
			return 8;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F9)) {
			return 9;
		}
		else if (scene->currentPressdKeys.contains(Qt::Key_F10)) {
			return 10;
		}
		else {
			return 0;
		}

	}

public slots:
	void setMode(Mode mode);


protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) Q_DECL_OVERRIDE;
	void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) Q_DECL_OVERRIDE;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private slots:
	void recordGlyphChange(QString name);
	void glyphValueChanged(QString name, bool structureChanged);
	void selectionChangedSlot();
private:
  bool rebuilding_ = false;
  QSet<QString> hiddenAnchors_;
  QString selectedAnchorKey_;
  int anchorLabelMode_ = 0;
  void applyAnchorDisplay();
  QStringList highlightedSourceKeys_;
  // View state belongs to this editor, never to the glyph source or export.
  struct LayerState { bool visible = true; bool locked = false; };
  QMap<QString, LayerState> layers_;
  QMap<QString, ContourItem*> layerContours_;
  QMap<QString, QGraphicsPathItem*> referenceItems_;
  static constexpr qreal referenceOpacity_ = 0.25;
  bool finalPositions_ = true;
  void refreshReferences();
  bool activeLayerEditable() const;
  QString itemLayerName(const QGraphicsItem* item) const;
  bool itemLayerEditable(const QGraphicsItem* item) const;
  void clearLayerSelection(const QString& name);
  void updatePictureDecorations();
  void selectSourcePoint(int path,int point);

	void addPointAfterPoint(KnotControlledItem* point, QPointF newpoint, QString dir);
	void deletePoint(KnotControlledItem* point);
	void addnewPath(QPointF newpoint, QString dir);

	QMap<QString, QVariant> m_oldValues;
	QMap<QString, QVariant> m_newValues;
	QMap<int, QMap<int, Glyph::Knot> >  old_controlledPaths;
	QMap<int, QMap<int, Glyph::Knot> >  new_controlledPaths;
	Glyph* m_glyph;
	Mode myMode;
	RulerItem* ruler;
	bool ismoving;
	bool wasMouseMoveEvent;
	std::map<QString, Glyph::Param> old_params;
	std::map<QString, Glyph::Param> new_params;
	void loadImage(Glyph::ImageInfo imageInfo);
	ContourItem* contour;

	GuidesItem* guides;
	QVector<ComponentItem*> components;
	ImageItem* image;

	ItemFlags itemFlags;

};

constexpr GlyphScene::ItemFlags operator|(GlyphScene::ItemFlags a, GlyphScene::ItemFlags b)
{
	return static_cast<GlyphScene::ItemFlags>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr GlyphScene::ItemFlags operator&(GlyphScene::ItemFlags a, GlyphScene::ItemFlags b)
{
	return static_cast<GlyphScene::ItemFlags>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr GlyphScene::ItemFlags operator~(GlyphScene::ItemFlags a)
{
	return static_cast<GlyphScene::ItemFlags>(~static_cast<int>(a));
}

#endif // GLYPHSCENE_H
