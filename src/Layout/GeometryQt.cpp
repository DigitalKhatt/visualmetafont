#include "GeometryQt.h"

#include <cmath>

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLineF>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QVBoxLayout>

using namespace geometry;

QPainterPath toQPainterPath(const geometry::GeometrySet& geomSet) {
  QPainterPath path;

  for (const auto& P : geomSet.polys()) {
    const size_t n = P.size();
    if (n == 0) continue;

    // Move to first point
    path.moveTo(P[0].x, P[0].y);

    // Draw edges
    for (size_t i = 1; i < n; ++i) {
      path.lineTo(P[i].x, P[i].y);
    }

    path.closeSubpath();
  }

  return path;
}

void debugDistance(const geometry::GeometrySet& A, const geometry::GeometrySet& B,
                   const std::string& nameA, const std::string& nameB,
                   const geometry::GSContact& gsContact) {
  const auto geometrySet = A.scaledY(-1);
  auto otherGeometrySet = B.scaledY(-1);

  QPointF pA{gsContact.contact.pA.x, -gsContact.contact.pA.y};
  QPointF pB{gsContact.contact.pB.x, -gsContact.contact.pB.y};

  auto normal = gsContact.contact.normal;

  Vec2 dirA = gsContact.contact.pA + (normal * -10);
  Vec2 dirB = gsContact.contact.pB + (normal * 10);

  QPointF dirPA{dirA.x, -dirA.y};
  QPointF dirPB{dirB.x, -dirB.y};

  QLineF lineA(pA, dirPA);
  QLineF lineB(pB, dirPB);

  auto* view = new QGraphicsView();
  QGraphicsScene scene;

  scene.addPath(toQPainterPath(geometrySet), QPen(Qt::black));
  scene.addPath(toQPainterPath(otherGeometrySet), QPen(Qt::black));

  if (gsContact.polyA != static_cast<size_t>(-1) && gsContact.polyB != static_cast<size_t>(-1)) {
    auto& tt = geometrySet.polys()[gsContact.polyA];
    auto& tt2 = otherGeometrySet.polys()[gsContact.polyB];

    for (auto& p : tt) {
      QPainterPath pp;
      pp.addEllipse(QPointF(p.x, p.y), 2, 2);
      scene.addPath(pp, QPen(Qt::red));
    }

    for (auto& p : tt2) {
      QPainterPath pp;
      pp.addEllipse(QPointF(p.x, p.y), 2, 2);
      scene.addPath(pp, QPen(Qt::red));
    }

    GeometrySet gg{std::vector<Poly>{tt}};
    GeometrySet gg2{std::vector<Poly>{tt2}};

    scene.addPath(toQPainterPath(gg), QPen(Qt::red));
    scene.addPath(toQPainterPath(gg2), QPen(Qt::red));
  }

  QLineF line(pA, pB);

  QPen pen(Qt::magenta, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

  scene.addLine(line, pen);
  scene.addLine(lineA, QPen(Qt::blue));

  qreal arrowSize = 10;
  double angle = std::acos(line.dx() / line.length());
  if (line.dy() >= 0)
    angle = 2 * M_PI - angle;

  QPointF arrowP1 = line.p2() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                        cos(angle + M_PI - M_PI / 3) * arrowSize);
  QPointF arrowP2 = line.p2() + QPointF(sin(angle + M_PI + M_PI / 3) * arrowSize,
                                        cos(angle + M_PI + M_PI / 3) * arrowSize);

  QPolygonF head;
  head << line.p2() << arrowP1 << arrowP2;

  QPainterPath circle1;
  circle1.addEllipse(pA, 5, 5);

  QPainterPath circle2;
  circle2.addEllipse(pB, 5, 5);

  scene.addPath(circle1, QPen(Qt::blue));
  scene.addLine(lineA, QPen(Qt::blue));
  scene.addPath(circle2, QPen(Qt::green));

  view->setScene(&scene);
  view->setRenderHints(QPainter::Antialiasing |
                       QPainter::TextAntialiasing);

  view->setMinimumSize(1500, 1000);

  view->scale(2, 2);

  QDialog box;
  box.setWindowTitle(QString("Graphics Preview: %1 vs %2")
                         .arg(QString::fromStdString(nameA), QString::fromStdString(nameB)));

  QVBoxLayout* layout = new QVBoxLayout(&box);
  layout->addWidget(view);

  // Optional: give the box a reasonable minimum width
  box.setMinimumWidth(1500);
  box.setMinimumHeight(1200);

  // Show the message box
  box.exec();
}
