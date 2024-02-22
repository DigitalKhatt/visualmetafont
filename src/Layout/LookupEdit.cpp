#include "hb-ot-layout-gsub-table.hh"
#include "LayoutWindow.h"


#include <QtWidgets>

#include <QTreeView>
#include <QFile>


#include "JustificationContext.h"

//#include "hb-font.hh"
//#include "hb-buffer.hh"
//#include "hb-ft.hh"

#include "font.hpp"

#include "glyph.hpp"




#include "GraphicsViewAdjustment.h"
#include "GraphicsSceneAdjustment.h"

#include "qurantext/quran.h"
//#include <QGLWidget>

#include "GlyphItem.h"

#include "Lookup.h"
#include "GlyphVis.h"
#include "qpoint.h"
#include "automedina/automedina.h"

#include <vector>

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"

#include <iostream>
#include <QPrinter>

#include <math.h> 

#include "to_opentype.h"
#include "Subtable.h"


//#include "hb.hh"
//#include "hb-ot-shape.hh"
//#include "hb-ot-layout-gsub-table.hh"

class ShapeResult
{
public:
  enum class EditMode { Editable, ReadOnly };

  explicit ShapeResult(QPainterPath path = QPainterPath(), float scale = 1) : scale{scale} {
    this->setPath(path);

  }

  void paint(QPainter* painter, const QRect& rect,
    const QPalette& palette, EditMode mode) const {

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(Qt::NoPen);

    painter->setBrush(Qt::black);

    const int yOffset = (rect.height());
    auto rec = path.boundingRect();
    painter->translate(rect.x() + (rect.width() - rec.width()) / 2, rect.y() + (rect.height() - rec.height()) / 2);

    painter->drawPath(path);

    painter->restore();

  }
  QSize sizeHint() const {
    auto rec = path.boundingRect();
    return rec.toRect().size() + QSize(20, 20);
  }
  void setPath(QPainterPath path) {
    this->path = path * transform;
    auto box = this->path.boundingRect();
    this->path = this->path.translated(-box.left(), -box.top());
  }

  QPainterPath getPath() {
    return path;
  }

private:
  float scale;
  QPainterPath path;
  QTransform transform{ scale,0,0,-scale,0,0 };
};
Q_DECLARE_METATYPE(ShapeResult)

class PathDelegate : public QStyledItemDelegate
{

public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override {
    if (index.data().canConvert<ShapeResult>()) {
      ShapeResult shapeResult = qvariant_cast<ShapeResult>(index.data());

      shapeResult.paint(painter, option.rect, option.palette,
        ShapeResult::EditMode::ReadOnly);
    }
    else {
      QStyledItemDelegate::paint(painter, option, index);
    }

  }
  QSize sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const override {
    if (index.data().canConvert<ShapeResult>()) {
      ShapeResult shapeResult = qvariant_cast<ShapeResult>(index.data());
      return shapeResult.sizeHint();
    }
    return QStyledItemDelegate::sizeHint(option, index);
  }


};


void LayoutWindow::editLookup(QString lookupName) {

  auto lookupIndex = this->m_otlayout->lookupsIndexByName[lookupName];


  Lookup* lookup = this->m_otlayout->lookups[lookupIndex];

  if (lookup->isGsubLookup() || lookup->type != Lookup::chainingpos) return;

  QTableWidget* tableWidget = new	QTableWidget(this);

  tableWidget->horizontalHeader()->hide();
  tableWidget->setColumnCount(1);
  tableWidget->horizontalHeader()->setResizeContentsPrecision(-1);
  tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  tableWidget->setItemDelegate(new PathDelegate);

  int rowIndex = 0;

  //QWidget* tableWidget = new QWidget;
  //QVBoxLayout* layout = new QVBoxLayout(tableWidget);

  hb_font_t* font = m_otlayout->createFont(1);

  for (auto subtable : lookup->subtables) {
    ChainingSubtable* subtableTable = static_cast<ChainingSubtable*>(subtable);

    QVector<QVector<quint32>> backtracks{ 1 };
    QVector< QVector<quint32>> inputs{ 1 };
    QVector< QVector<quint32>> lookaheads{ 1 };

    for (auto& backtrack : subtableTable->compiledRule.backtrack) {
      QVector< QVector<quint32>> newbacktracks;
      for (auto glyphIndex : backtrack) {
        for (auto i_i : backtracks) {
          i_i.append(glyphIndex);
          newbacktracks.append(i_i);
        }
      }
      backtracks = newbacktracks;
    }

    for (auto& input : subtableTable->compiledRule.input) {
      QVector< QVector<quint32>> newinputs;
      for (auto glyphIndex : input) {
        for (auto i_i : inputs) {
          i_i.append(glyphIndex);
          newinputs.append(i_i);
        }
      }
      inputs = newinputs;
    }


    for (auto& lookahead : subtableTable->compiledRule.lookahead) {
      QVector< QVector<quint32>> newlookaheads;
      for (auto glyphIndex : lookahead) {
        for (auto i_i : lookaheads) {
          i_i.append(glyphIndex);
          newlookaheads.append(i_i);
        }
      }
      lookaheads = newlookaheads;
    }

    QVector<QVector<quint32>> oldsequences = backtracks;

    QVector<QVector<quint32>> sequences;
    for (auto& input : inputs) {
      for (auto sequence : oldsequences) {
        sequence.append(input);
        sequences.append(sequence);

      }
    }

    oldsequences = sequences;
    sequences.clear();
    for (auto& lookahead : lookaheads) {
      for (auto sequence : oldsequences) {
        sequence.append(lookahead);
        sequences.append(sequence);
      }
    }

    for (auto& sequence : sequences) {

      if (sequence.size() > 20) {
        std::cout << "size=" << sequence.size() << std::endl;
      }

      hb_buffer_t* buffer = buffer = hb_buffer_create();
      hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
      hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
      hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));

      hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_GLYPHS);

      for (int i = 0; i < sequence.size(); i++) {
        buffer->add(sequence[i], i);
      }



      hb_shape(font, buffer, nullptr, 0);

      unsigned int glyph_count;

      hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
      hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

      qreal scale = (1.0 / 1000) * 100;


      //auto qGraphicsView = new QGraphicsView(tableWidget);


      //qGraphicsView->scale(scale, scale);
      //auto graphicsScene = new QGraphicsScene(tableWidget);
      int currentxPos = 0;
      QPainterPath path;
      path.setFillRule(Qt::WindingFill);
      for (int i = glyph_count - 1; i >= 0; i--) {
        auto codepoint = glyph_info[i].codepoint;
        QString glyphName = m_otlayout->glyphNamePerCode[codepoint];

        if (m_otlayout->glyphs.contains(glyphName)) {
          GlyphVis& glyph = m_otlayout->glyphs[glyphName];

          /*auto glyphItem = new GlyphItem(1, &glyph, m_otlayout, glyph_info[i].lookup_index, glyph_info[i].subtable_index, glyph_info[i].base_codepoint, glyph_info[i].lefttatweel, glyph_info[i].righttatweel);
          glyphItem->setFlag(QGraphicsItem::ItemIsMovable);
          graphicsScene->addItem(glyphItem);*/
          currentxPos -= glyph_pos[i].x_advance;
          QPoint pos(currentxPos + (glyph_pos[i].x_offset), glyph_pos[i].y_offset);

          auto glyphPath = glyph.path;

          if (glyph_info[i].lefttatweel != 0 || glyph_info[i].righttatweel != 0) {
            GlyphParameters parameters{};

            parameters.lefttatweel = glyph_info[i].lefttatweel;
            parameters.righttatweel = glyph_info[i].righttatweel;

            glyphPath = glyph.getAlternate(parameters)->path;
          }

          glyphPath.setFillRule(Qt::WindingFill);

          glyphPath.translate(pos);

          path.addPath(glyphPath);

          //glyphItem->setPos(pos);
        }
      }

      //qGraphicsView->setScene(graphicsScene);

      hb_buffer_destroy(buffer);


      auto pathItem = new QTableWidgetItem();
      pathItem->setData(Qt::DisplayRole, QVariant::fromValue(ShapeResult(path,scale)));

      tableWidget->insertRow(rowIndex);
      tableWidget->setItem(rowIndex, 0, pathItem);


      //layout->addWidget(qGraphicsView);

      rowIndex++;
    }
  }

  //tableWidget->resizeColumnsToContents();
  tableWidget->resizeRowsToContents();

  auto newDock = new QDockWidget(lookupName, this);
  newDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

  newDock->setAttribute(Qt::WA_DeleteOnClose);

  addDockWidget(Qt::LeftDockWidgetArea, newDock);
  viewMenu->addAction(newDock->toggleViewAction());

  /*QScrollArea* scrollArea = new QScrollArea(this);
  scrollArea->setWidget(tableWidget);

  newDock->setWidget(scrollArea);*/

  newDock->setWidget(tableWidget);

  tabifyDockWidget(lookupTree, newDock);
  newDock->show();
  newDock->raise();

  hb_font_destroy(font);

}
