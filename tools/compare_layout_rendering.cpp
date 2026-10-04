// Compare live MetaPost outlines with the precomputed-layout renderer.
// The binary reader and interpolation mirror Tarteel's MushafRendererJSI.cpp,
// binary_reader.h and JsiPage::getGlyphPath (including float conversion).
// Qt rasterizes BOTH paths, so image differences isolate geometry/placement;
// this is not a replacement for testing Skia's rasterizer on a device.
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QSvgGenerator>

#include "MPFont.h"
#include "Layout/GlyphVis.h"
#include "Layout/OtLayout.h"

namespace fs = std::filesystem;
namespace {
using Element = std::vector<double>;
using Contour = std::vector<Element>;
using Outline = std::vector<Contour>;
struct Glyph {
  Outline base;
  std::array<double, 6> limits{};
  std::array<Outline, 6> masters;
};
struct Placement {
  unsigned code = 0, cluster = 0, color = 0;
  int advance = 0, dx = 0, dy = 0;
  std::array<double, 3> axes{};
};
struct Line {
  std::vector<Placement> glyphs;
  unsigned type = 0;
  int x = 0;
  double xscale = 1;
  double fontSize = 1;
};
struct Document {
  unsigned version = 1;
  std::map<unsigned, Glyph> glyphs;
  std::vector<std::vector<Line>> pages;
};

std::string readFile(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
void writeFile(const fs::path& path, const QByteArray& data) {
  QFile file(QString::fromStdString(path.string()));
  if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size())
    throw std::runtime_error("Cannot write " + path.string());
}
QString hash(const fs::path& path) {
  QFile file(QString::fromStdString(path.string()));
  if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot hash " + path.string());
  QCryptographicHash hash(QCryptographicHash::Sha256);
  if (!hash.addData(&file)) throw std::runtime_error("Cannot hash " + path.string());
  return hash.result().toHex();
}

class Reader {
 public:
  std::string bytes;
  size_t offset = 0;
  explicit Reader(const fs::path& path) : bytes(readFile(path)) {}
  unsigned u8() {
    if (offset >= bytes.size()) throw std::runtime_error("Truncated binary at byte " + std::to_string(offset));
    return static_cast<unsigned char>(bytes[offset++]);
  }
  unsigned u16() { unsigned a = u8(); return (a << 8) | u8(); }
  uint32_t u32() { uint32_t a = u16(); return (a << 16) | u16(); }
  int i16() { unsigned v = u16(); return v >= 32768 ? int(v) - 65536 : int(v); }
  double fixed() { return float(std::bit_cast<int32_t>(u32())) / 65536.0f; }
  Outline outline() {
    Outline result(u8());
    for (auto& contour : result) {
      contour.push_back({fixed(), fixed()});
      auto count = u8();
      for (unsigned i = 0; i < count; ++i) {
        Element cubic(6);
        for (auto& value : cubic) value = fixed();
        contour.push_back(std::move(cubic));
      }
    }
    return result;
  }
};
Document readBinary(const fs::path& path) {
  Reader r(path);
  Document doc;
  if (r.bytes.starts_with("DKLY")) {
    r.offset = 4;
    doc.version = r.u16();
    if (doc.version != 2 && doc.version != 3) throw std::runtime_error("Unsupported DKLY version");
  }
  auto count = r.u16();
  for (unsigned i = 0; i < count; ++i) {
    auto code = r.u32();
    if (code > 65535) throw std::runtime_error("Glyph code exceeds Tarteel's uint16 range");
    Glyph glyph;
    glyph.base = r.outline();
    for (int j = 0; j < 4; ++j) glyph.limits[j] = r.fixed();
    for (int j = 0; j < 4; ++j) if (glyph.limits[j]) glyph.masters[j] = r.outline();
    if (doc.version >= 2) {
      for (int j = 4; j < 6; ++j) glyph.limits[j] = r.fixed();
      for (int j = 4; j < 6; ++j) if (glyph.limits[j]) glyph.masters[j] = r.outline();
    }
    for (int j = 0; j < 6; j += 2)
      if (glyph.limits[j] > 0 || glyph.limits[j + 1] < 0)
        throw std::runtime_error("Invalid axis limits");
    if (!doc.glyphs.emplace(code, std::move(glyph)).second)
      throw std::runtime_error("Duplicate glyph code");
  }
  doc.pages.resize(r.u16());
  for (auto& page : doc.pages) {
    page.resize(r.u8());
    for (auto& line : page) {
      line.glyphs.resize(r.u8());
      for (auto& p : line.glyphs) {
        p.code = r.u16(); p.cluster = r.u8(); auto mask = r.u8();
        if (mask & (doc.version >= 2 ? 128 : 192)) throw std::runtime_error("Unsupported placement mask");
        if (mask & 1) p.advance = r.i16();
        if (mask & 2) p.dx = r.i16();
        if (mask & 4) p.dy = r.i16();
        if (mask & 8) p.color = r.u8();
        if (mask & 16) p.axes[0] = r.fixed();
        if (mask & 32) p.axes[1] = r.fixed();
        if (mask & 64) p.axes[2] = r.fixed();
        if (!doc.glyphs.contains(p.code)) throw std::runtime_error("Placement references missing glyph");
      }
      line.type = r.u8(); line.x = r.i16(); line.xscale = r.fixed();
      if (doc.version >= 3) line.fontSize = r.fixed();
      r.u16(); // UTF-16 text length
      if (line.type == 1) r.u8(); // surah index
      if (line.type > 2 || line.xscale <= 0 || line.fontSize <= 0) throw std::runtime_error("Invalid line metadata");
    }
  }
  if (r.offset != r.bytes.size()) throw std::runtime_error("Trailing bytes in layout binary");
  return doc;
}

bool sameTopology(const Outline& a, const Outline& b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].size() != b[i].size()) return false;
    for (size_t j = 0; j < a[i].size(); ++j)
      if (a[i][j].size() != b[i][j].size()) return false;
  }
  return true;
}
double coordinateMax(const Outline& a, const Outline& b) {
  if (!sameTopology(a, b)) return std::numeric_limits<double>::infinity();
  double result = 0;
  for (size_t i = 0; i < a.size(); ++i)
    for (size_t j = 0; j < a[i].size(); ++j)
      for (size_t k = 0; k < a[i][j].size(); ++k)
        result = std::max(result, std::abs(a[i][j][k] - b[i][j][k]));
  return result;
}
Outline jsonOutline(const QJsonArray& shapes) {
  Outline result;
  for (const auto& shape : shapes) {
    Contour contour;
    for (const auto& element : shape.toObject()["path"].toArray()) {
      Element values;
      for (auto value : element.toArray()) values.push_back(value.toDouble());
      contour.push_back(std::move(values));
    }
    result.push_back(std::move(contour));
  }
  return result;
}
std::array<double, 3> clampedAxes(const Glyph& glyph, const Placement& p) {
  auto axes = p.axes;
  for (int axis = 0; axis < 3; ++axis)
    axes[axis] = std::clamp(axes[axis], glyph.limits[2 * axis], glyph.limits[2 * axis + 1]);
  return axes;
}
Outline interpolate(const Glyph& glyph, const std::array<double, 3>& axes) {
  Outline result = glyph.base;
  // Add each master's delta to the DEFAULT, never to the previous axis result.
  for (int axis = 0; axis < 3; ++axis) {
    if (axes[axis] == 0) continue;
    int master = 2 * axis + (axes[axis] > 0);
    if (axis == 2 && glyph.masters[master].empty()) continue; // Tarteel's third-axis guard
    if (!sameTopology(glyph.base, glyph.masters[master]))
      throw std::runtime_error("Incompatible interpolation master topology on axis " + std::to_string(axis));
    double scalar = axes[axis] / glyph.limits[master];
    for (size_t i = 0; i < result.size(); ++i)
      for (size_t j = 0; j < result[i].size(); ++j)
        for (size_t k = 0; k < result[i][j].size(); ++k)
          result[i][j][k] += (glyph.masters[master][i][j][k] - glyph.base[i][j][k]) * scalar;
  }
  // SkPathBuilder takes SkScalar (float), even though interpolation uses double.
  for (auto& contour : result) for (auto& element : contour)
    for (auto& value : element) value = float(value);
  return result;
}
Outline metaPostOutline(const GlyphVis& glyph) {
  Outline result;
  for (auto* object = glyph.mpPath(); object; object = object->next) {
    if (object->type != mp_fill_code)
      throw std::runtime_error("Unsupported MetaPost graphic object in " + glyph.name);
    auto* fill = reinterpret_cast<mp_fill_object*>(object);
    if (fill->color_model != mp_no_model)
      throw std::runtime_error("Internal MetaPost fill colors are not supported: " + glyph.name);
    auto* first = fill->path_p;
    if (!first) continue;
    Contour contour{{first->x_coord, first->y_coord}};
    auto* knot = first;
    do {
      auto* next = knot->next;
      contour.push_back({knot->right_x, knot->right_y, next->left_x, next->left_y, next->x_coord, next->y_coord});
      knot = next;
    } while (knot != first);
    result.push_back(std::move(contour));
  }
  return result;
}

// Same font bootstrap as compare_extreme_lines; no GUI or justification pass.
std::size_t registerGlyphSources(MPFont& font, const fs::path& glyphsPath) {
  const auto glyphs = readFile(glyphsPath);
  const std::string reset =
      "params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;";
  std::size_t count = 0;
  std::size_t pos = 0;
  while (pos < glyphs.size()) {
    const auto beginPos = glyphs.find("beginchar", pos);
    const auto defPos = glyphs.find("defchar", pos);
    const bool isDef = defPos != std::string::npos &&
                       (beginPos == std::string::npos || defPos < beginPos);
    const auto start = isDef ? defPos : beginPos;
    if (start == std::string::npos) break;
    const std::string_view endMarker = isDef ? "enddefchar;" : "endchar;";
    const auto end = glyphs.find(endMarker, start);
    if (end == std::string::npos)
      throw std::runtime_error("Unterminated glyph near byte " +
                               std::to_string(start));
    const auto blockEnd = end + endMarker.size();
    const auto block = glyphs.substr(start, blockEnd - start);
    const auto open = block.find('(');
    const auto comma1 = block.find(',', open);
    const auto comma2 = block.find(',', comma1 + 1);
    if (open == std::string::npos || comma1 == std::string::npos ||
        comma2 == std::string::npos)
      throw std::runtime_error("Invalid glyph header near byte " +
                               std::to_string(start));
    const auto name = block.substr(open + 1, comma1 - open - 1);
    const auto unicode =
        std::stoi(block.substr(comma1 + 1, comma2 - comma1 - 1));
    font.registerGlyphSource(name, block, isDef ? "defchar" : "beginchar",
                             unicode);
    font.execute(reset + block);
    ++count;
    pos = blockEnd;
  }
  return count;
}

void initializeFont(MPFont& font, const fs::path& project,
                    const fs::path& resources) {
  std::string source{"MPGUI:=1;"};
  source += readFile(resources / "mfplain.mp");
  source += readFile(resources / "mpost.mp");
  source += readFile(resources / "vmf.mp");
  source += readFile(project);
  font.initialize(std::move(source), project);
  const auto count = registerGlyphSources(font, project.parent_path() / "glyphs.mp");
  std::cout << "Registered " << count << " glyph sources\n";
}

QPointF point(const Element& e, size_t i) { return {e[i], e[i + 1]}; }
QPointF bezier(QPointF start, const Element& e, double t) {
  double u = 1 - t;
  return start * (u*u*u) + point(e, 0) * (3*u*u*t) + point(e, 2) * (3*u*t*t) + point(e, 4) * (t*t*t);
}
Outline positioned(Outline outline, QPointF origin, double outlineScale, double xscale) {
  for (auto& contour : outline) for (auto& e : contour)
    for (size_t k = 0; k < e.size(); k += 2) {
      e[k] = (origin.x() + e[k] * outlineScale) * xscale;
      e[k + 1] = origin.y() + e[k + 1] * outlineScale;
    }
  return outline;
}
QPainterPath painterPath(const Outline& outline) {
  QPainterPath path;
  path.setFillRule(Qt::WindingFill); // SkPath's default and PDF's `f` operator
  for (const auto& contour : outline) {
    if (contour.empty()) continue;
    path.moveTo(point(contour[0], 0));
    for (size_t j = 1; j < contour.size(); ++j)
      path.cubicTo(point(contour[j], 0), point(contour[j], 2), point(contour[j], 4));
  }
  return path;
}
QJsonObject curveError(const Outline& a, const Outline& b, int samples) {
  QJsonObject result{{"matchingTopology", sameTopology(a, b)}};
  if (!sameTopology(a, b)) return result;
  double sum = 0, maximum = 0, maxT = 0;
  int maxContour = -1, maxSegment = -1;
  qint64 count = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].empty()) continue;
    QPointF startA = point(a[i][0], 0), startB = point(b[i][0], 0);
    for (size_t j = 1; j < a[i].size(); ++j) {
      for (int s = 0; s <= samples; ++s) {
        double t = double(s) / samples;
        auto delta = bezier(startA, a[i][j], t) - bezier(startB, b[i][j], t);
        double squared = QPointF::dotProduct(delta, delta);
        sum += squared; ++count;
        if (squared > maximum) { maximum = squared; maxContour = int(i); maxSegment = int(j) - 1; maxT = t; }
      }
      // The next cubic starts at the PREVIOUS ENDPOINT, not its first control.
      startA = point(a[i][j], 4); startB = point(b[i][j], 4);
    }
  }
  result["sampleCount"] = double(count);
  result["rms"] = count ? std::sqrt(sum / count) : 0;
  result["maximum"] = std::sqrt(maximum);
  result["maximumContour"] = maxContour;
  result["maximumSegment"] = maxSegment;
  result["maximumT"] = maxT;
  result["maximumCoordinateDelta"] = coordinateMax(a, b);
  return result;
}
struct Segment { QPointF a, b; };
std::vector<Segment> flattened(const Outline& outline, int samples) {
  std::vector<Segment> result;
  for (const auto& contour : outline) {
    if (contour.empty()) continue;
    auto start = point(contour[0], 0);
    for (size_t j = 1; j < contour.size(); ++j) {
      auto prev = start;
      for (int s = 1; s <= samples; ++s) {
        auto next = bezier(start, contour[j], double(s) / samples);
        result.push_back({prev, next}); prev = next;
      }
      start = point(contour[j], 4);
    }
    auto first = point(contour[0], 0);
    if (start != first) result.push_back({start, first});
  }
  return result;
}
QPointF closestPoint(QPointF p, const Segment& s) {
  auto d = s.b - s.a;
  double denom = QPointF::dotProduct(d, d);
  double t = denom ? std::clamp(QPointF::dotProduct(p - s.a, d) / denom, 0.0, 1.0) : 0;
  return s.a + d * t;
}
QJsonArray xy(QPointF p) { return {p.x(), p.y()}; }
QJsonObject gap(const Outline& a, const Outline& b, int samples) {
  auto pathA = painterPath(a), pathB = painterPath(b);
  if (pathA.isEmpty() || pathB.isEmpty()) return {{"status", "empty outline"}};
  if (pathA.intersects(pathB)) return {{"status", "filled paths intersect"}, {"distance", 0}};
  auto segA = flattened(a, samples), segB = flattened(b, samples);
  double best = std::numeric_limits<double>::infinity();
  QPointF from, to;
  auto consider = [&](QPointF p, QPointF q, bool reverse) {
    auto d = p - q; double squared = QPointF::dotProduct(d, d);
    if (squared < best) { best = squared; from = reverse ? q : p; to = reverse ? p : q; }
  };
  for (const auto& s : segA) for (const auto& t : segB) {
    double dx = std::max({0.0, std::min(s.a.x(), s.b.x()) - std::max(t.a.x(), t.b.x()),
                        std::min(t.a.x(), t.b.x()) - std::max(s.a.x(), s.b.x())});
    double dy = std::max({0.0, std::min(s.a.y(), s.b.y()) - std::max(t.a.y(), t.b.y()),
                        std::min(t.a.y(), t.b.y()) - std::max(s.a.y(), s.b.y())});
    if (dx*dx + dy*dy >= best) continue;
    consider(s.a, closestPoint(s.a, t), false); consider(s.b, closestPoint(s.b, t), false);
    consider(t.a, closestPoint(t.a, s), true); consider(t.b, closestPoint(t.b, s), true);
  }
  return {{"status", "sampled contour clearance"}, {"distance", std::sqrt(best)}, {"from", xy(from)}, {"to", xy(to)}};
}
struct RenderedGlyph {
  int index;
  unsigned color;
  Outline exact, app, unit;
};
const std::array<QRgb, 9> palette = {0xff000000, 0xff00a650, 0xff006694, 0xffb4b4b4,
  0xff00adef, 0xffc38a08, 0xfff47216, 0xffec008c, 0xff8c0000};
void draw(QPainter& painter, const std::vector<RenderedGlyph>& glyphs, int mode, const QTransform& transform, bool tajweed) {
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setTransform(transform);
  for (const auto& glyph : glyphs) {
    QColor color = QColor::fromRgb(palette[tajweed ? glyph.color : 0]);
    if (mode == 3) {
      painter.fillPath(painterPath(glyph.exact), QColor(220, 0, 100, 150));
      painter.fillPath(painterPath(glyph.app), QColor(0, 160, 220, 150));
    } else {
      painter.fillPath(painterPath(mode == 0 ? glyph.exact : mode == 1 ? glyph.app : glyph.unit), color);
    }
  }
}
QImage raster(const std::vector<RenderedGlyph>& glyphs, int mode, const QTransform& transform, QSize size, bool tajweed) {
  QImage image(size, QImage::Format_ARGB32);
  if (image.isNull()) throw std::runtime_error("Cannot allocate render image");
  image.fill(Qt::white);
  QPainter painter(&image); draw(painter, glyphs, mode, transform, tajweed);
  return image;
}
void saveImage(const QImage& image, const fs::path& path) {
  if (!image.save(QString::fromStdString(path.string()))) throw std::runtime_error("Cannot save " + path.string());
}
void svg(const fs::path& path, const std::vector<RenderedGlyph>& glyphs, int mode, const QTransform& transform, QSize size, bool tajweed) {
  QSvgGenerator generator;
  generator.setFileName(QString::fromStdString(path.string()));
  generator.setSize(size); generator.setViewBox(QRect(QPoint(), size));
  generator.setTitle("MetaPost / Tarteel geometry comparison");
  QPainter painter;
  if (!painter.begin(&generator)) throw std::runtime_error("Cannot write SVG");
  painter.fillRect(QRect(QPoint(), size), Qt::white);
  draw(painter, glyphs, mode, transform, tajweed);
}
// Small analytic checks guard against errors in the measuring tool itself.
void selfTest() {
  auto check = [](bool ok, const char* message) {
    if (!ok) throw std::runtime_error(std::string("Self-test failed: ") + message);
  };
  auto near = [&](double a, double b, const char* message) { check(std::abs(a - b) < 1e-6, message); };
  Outline zero{{{0,0}, {0,0,0,0,0,0}}};
  Outline raisedStart{{{8,0}, {0,0,0,0,0,0}}};
  auto error = curveError(zero, raisedStart, 2); // distances at t=0,.5,1: 8,1,0
  near(error["maximum"].toDouble(), 8, "cubic must start at its move/previous endpoint");
  near(error["rms"].toDouble(), std::sqrt(65.0 / 3), "sampled RMS");
  auto scaled = positioned(raisedStart, {10,20}, 2, 3);
  near(scaled[0][0][0], (10 + 8 * 2) * 3, "outline scale after glyph origin");
  near(scaled[0][0][1], 20, "font scale does not multiply mark offset");
  auto translated = positioned(raisedStart, {3,4}, 1, 1);
  near(curveError(raisedStart, translated, 16)["rms"].toDouble(), 5, "translation RMS");
  check(!curveError(zero, {}, 16)["matchingTopology"].toBool(), "topology mismatch");
  Glyph glyph; glyph.base = zero; glyph.limits = {-2,2,-4,4,0,20};
  for (int axis = 0; axis < 6; ++axis) glyph.masters[axis] = positioned(zero, {double(axis + 1),0}, 1, 1);
  auto combined = interpolate(glyph, {1,-2,10});
  near(combined[0][0][0], 1 + 1.5 + 3, "independent left/right/third deltas");
  Placement placement; placement.axes = {-9,9,25};
  check(clampedAxes(glyph, placement) == std::array<double,3>{-2,4,20}, "axis clamping");
  auto rectangle = [](double x, double y, double w, double h) {
    return Outline{{{x,y}, {x,y,x+w,y,x+w,y}, {x+w,y,x+w,y+h,x+w,y+h},
      {x+w,y+h,x,y+h,x,y+h}, {x,y+h,x,y,x,y}}};
  };
  auto rect = rectangle(0,0,10,10);
  near(gap(rect, rectangle(14,0,10,10), 8)["distance"].toDouble(-1), 4, "disjoint clearance");
  near(gap(rect, rectangle(2,2,2,2), 8)["distance"].toDouble(-1), 0, "filled containment");
  near(gap(rect, rectangle(5,0,10,10), 8)["distance"].toDouble(-1), 0, "filled intersection");
  std::cout << "Analytic interpolation, cubic RMS, clamping and clearance checks passed\n";
}
const char* help = R"(Usage: digitalkhatt_compare_layout_rendering --project FONT.mp --binary LAYOUT.bin [options]
  --glyph-map FILE.json    Generate Layout Info JSON from the SAME export (names).
                          Default: PROJECT_DIR/output/PROJECT_NAME.json
  --page N --line N        One-based page/line; default 1/2 (first Basmala).
  --glyphs FIRST:LAST      Inclusive ZERO-based placement indices; default whole line.
  --gap A:B               Measure minimum filled-outline clearance; repeatable.
                          A and B are ZERO-based indices in the original line;
                          both must be inside the selected glyph range.
  --pixels-per-unit X     Raster scale; default 1. SVGs remain zoomable.
  --metapost-scale X      Override live outline scale; default: binary line.fontSize.
                          Use the PDF's line.fontSize to reproduce PDF geometry.
  --samples N             Uniform t intervals per cubic; default 256, max 8192.
  --self-test             Run analytic interpolation/curve/gap checks and exit.
  --monochrome            Disable stored Tajweed colors (metrics always monochrome).
  --output DIR            Default layout-rendering-comparison.
  --resources DIR         MetaPost resource directory override.

Replays the binary's Force/justification result; does not rejustify or change assets.
Binary v1 and DKLY v2/v3 (third axis and line fontSize) supported. Surah-title lines use a
separate app font and are rejected. Both images use Qt's nonzero-fill rasterizer;
Tarteel geometry is reproduced, but device Skia rasterization is not exercised.
Outputs: metapost/tarteel/metapost-unit/overlay PNG + SVG, difference.png,
report.json with hashes, axes, origins, curve errors and optional gaps; index.html.
)";
std::pair<int,int> pair(const std::string& value) {
  auto colon = value.find(':');
  if (colon == std::string::npos) throw std::runtime_error("Expected A:B");
  auto integer = [](const std::string& s) { size_t end; int n = std::stoi(s, &end); if (end != s.size()) throw std::runtime_error("Invalid integer"); return n; };
  return {integer(value.substr(0, colon)), integer(value.substr(colon + 1))};
}
int integerOption(const std::string& value) {
  size_t end;
  int result = std::stoi(value, &end);
  if (end != value.size()) throw std::runtime_error("Invalid integer: " + value);
  return result;
}
double realOption(const std::string& value) {
  size_t end;
  double result = std::stod(value, &end);
  if (end != value.size()) throw std::runtime_error("Invalid number: " + value);
  return result;
}
} // namespace

int main(int argc, char** argv) {
  QCoreApplication application(argc, argv);
  try {
    fs::path project, binary, names, output = "layout-rendering-comparison", resources = DIGITALKHATT_METAFONT_RESOURCES;
    int pageIndex = 1, lineIndex = 2, first = 0, last = -1, samples = 256;
    double pixelsPerUnit = 1;
    std::optional<double> mpScaleOverride;
    bool tajweed = true;
    std::vector<std::pair<int,int>> gaps;
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--self-test") { selfTest(); return 0; }
      if (arg == "--help" || arg == "-h") { std::cout << help; return 0; }
      if (arg == "--monochrome") { tajweed = false; continue; }
      if (++i >= argc) throw std::runtime_error("Missing value for " + arg);
      std::string value = argv[i];
      if (arg == "--project") project = fs::absolute(value);
      else if (arg == "--binary") binary = fs::absolute(value);
      else if (arg == "--glyph-map") names = fs::absolute(value);
      else if (arg == "--output") output = fs::absolute(value);
      else if (arg == "--resources") resources = fs::absolute(value);
      else if (arg == "--page") pageIndex = integerOption(value);
      else if (arg == "--line") lineIndex = integerOption(value);
      else if (arg == "--samples") samples = integerOption(value);
      else if (arg == "--pixels-per-unit") pixelsPerUnit = realOption(value);
      else if (arg == "--metapost-scale") mpScaleOverride = realOption(value);
      else if (arg == "--glyphs") { auto range = pair(value); first = range.first; last = range.second; if (last < first) throw std::runtime_error("Reversed glyph range"); }
      else if (arg == "--gap") gaps.push_back(pair(value));
      else throw std::runtime_error("Unknown option " + arg);
    }
    if (project.empty() || binary.empty()) throw std::runtime_error(help);
    output = fs::absolute(output);
    if (!std::isfinite(pixelsPerUnit) || pixelsPerUnit <= 0 || (mpScaleOverride && (!std::isfinite(*mpScaleOverride) || *mpScaleOverride <= 0)) || samples < 1 || samples > 8192)
      throw std::runtime_error("Invalid scale or sample count");
    if (names.empty()) names = project.parent_path() / "output" / (project.stem().string() + ".json");
    auto doc = readBinary(binary);
    if (pageIndex < 1 || pageIndex > int(doc.pages.size()) || lineIndex < 1 || lineIndex > int(doc.pages[pageIndex - 1].size()))
      throw std::runtime_error("Page/line outside binary");
    const auto& line = doc.pages[pageIndex - 1][lineIndex - 1];
    const double mpScale = mpScaleOverride.value_or(line.fontSize);
    if (line.type == 1) throw std::runtime_error("Surah-title rendering uses Tarteel's external icomoon font; select a text line");
    if (last == -1) last = int(line.glyphs.size()) - 1;
    if (first < 0 || last < first || last >= int(line.glyphs.size())) throw std::runtime_error("Invalid glyph selection");
    for (auto [a,b] : gaps) if (a < first || a > last || b < first || b > last) throw std::runtime_error("Gap glyphs must be inside selected range");
    QJsonParseError error;
    auto json = QJsonDocument::fromJson(QByteArray::fromStdString(readFile(names)), &error);
    if (error.error != QJsonParseError::NoError || !json.isObject()) throw std::runtime_error("Invalid glyph-map JSON");
    auto nameMap = json.object()["glyphs"].toObject();

    MPFont font;
    initializeFont(font, project, resources);
    OtLayout layout(&font, true, true);
    layout.useNormAxisValues = false;
    layout.quantizeGlyphAdvances = true;
    layout.loadLookupFile("features.fea");
    QJsonArray glyphReports, warnings;
    std::vector<RenderedGlyph> rendered;
    double cursor = 0;
    QRectF bounds;
    bool haveBounds = false;
    for (int i = 0; i <= last; ++i) {
      const auto& p = line.glyphs[i]; cursor -= p.advance;
      if (i < first) continue;
      const auto& g = doc.glyphs.at(p.code);
      auto mapped = nameMap[QString::number(p.code)].toObject();
      auto name = mapped["name"].toString().toStdString();
      if (name.empty()) throw std::runtime_error("No glyph name for code " + std::to_string(p.code));
      // A wrong JSON can silently map a dynamic code to an unrelated glyph.
      if (coordinateMax(g.base, jsonOutline(mapped["default"].toArray())) > 0.02)
        throw std::runtime_error("Glyph-map/binary default mismatch for " + name + "; use JSON from the same export");
      auto source = layout.glyphs.find(name);
      if (source == layout.glyphs.end()) throw std::runtime_error("Live font missing glyph " + name);
      double defaultDelta = coordinateMax(g.base, metaPostOutline(source->second));
      if (defaultDelta > 0.02) warnings.append(QString::fromStdString("Live default differs from binary: " + name));
      auto axes = clampedAxes(g, p);
      GlyphParameters parameters{};
      parameters.lefttatweel = axes[0]; parameters.righttatweel = axes[1]; parameters.third = axes[2];
      auto* alternate = source->second.getAlternate(parameters);
      if (!alternate) throw std::runtime_error("No MetaPost alternate for " + name);
      auto exact = metaPostOutline(*alternate), app = interpolate(g, axes);
      QPointF origin(cursor + p.dx, p.dy);
      if (p.color >= palette.size()) throw std::runtime_error("Unknown Tajweed color");
      RenderedGlyph item{i, p.color, positioned(exact, origin, mpScale, line.xscale),
        positioned(app, origin, line.fontSize, line.xscale), positioned(exact, origin, 1, line.xscale)};
      for (const auto* outline : {&item.exact, &item.app, &item.unit}) {
        auto path = painterPath(*outline);
        if (path.isEmpty()) continue;
        bounds = haveBounds ? bounds.united(path.boundingRect()) : path.boundingRect(); haveBounds = true;
      }
      glyphReports.append(QJsonObject{{"index", i}, {"codepoint", int(p.code)}, {"name", QString::fromStdString(name)},
        {"cluster", int(p.cluster)}, {"tajweedColor", int(p.color)}, {"advance", p.advance}, {"offset", QJsonArray{p.dx, p.dy}},
        {"originBeforeLineScale", xy(origin)}, {"requestedAxes", QJsonArray{p.axes[0], p.axes[1], p.axes[2]}},
        {"clampedAxes", QJsonArray{axes[0], axes[1], axes[2]}},
        {"liveDefaultMaximumCoordinateDelta", std::isfinite(defaultDelta) ? QJsonValue(defaultDelta) : QJsonValue()},
        {"outlineError", curveError(exact, app, samples)}, {"positionedError", curveError(item.exact, item.app, samples)}});
      rendered.push_back(std::move(item));
    }
    if (!haveBounds) throw std::runtime_error("Selected glyphs have no visible paths");
    double width = std::ceil(bounds.width() * pixelsPerUnit) + 40, height = std::ceil(bounds.height() * pixelsPerUnit) + 40;
    if (width > 32767 || height > 32767 || width * height > 40000000)
      throw std::runtime_error("Image too large; reduce --pixels-per-unit or select --glyphs");
    QSize size{int(width), int(height)};
    QTransform transform(pixelsPerUnit, 0, 0, -pixelsPerUnit,
                         20 - bounds.left() * pixelsPerUnit, 20 + bounds.bottom() * pixelsPerUnit);
    fs::create_directories(output);
    const std::array<std::string,4> stems{"metapost", "tarteel", "metapost-unit", "overlay"};
    for (int mode = 0; mode < 4; ++mode) {
      saveImage(raster(rendered, mode, transform, size, tajweed), output / (stems[mode] + ".png"));
      svg(output / (stems[mode] + ".svg"), rendered, mode, transform, size, tajweed);
    }
    auto exactImage = raster(rendered, 0, transform, size, false), appImage = raster(rendered, 1, transform, size, false);
    QImage difference(size, QImage::Format_ARGB32); difference.fill(Qt::white);
    double pixelSum = 0; qint64 changed = 0;
    for (int y = 0; y < size.height(); ++y) for (int x = 0; x < size.width(); ++x) {
      int a = 255 - qRed(exactImage.pixel(x,y)), b = 255 - qRed(appImage.pixel(x,y));
      int d = std::abs(a - b); pixelSum += d * d;
      if (d) ++changed;
      difference.setPixel(x,y, a > b ? qRgb(255, 255-d, 255-d) : qRgb(255-d, 255-d, 255));
    }
    saveImage(difference, output / "difference.png");
    QJsonArray gapReports;
    for (auto [a,b] : gaps) {
      const auto& ga = rendered[a - first]; const auto& gb = rendered[b - first];
      gapReports.append(QJsonObject{{"glyphA", a}, {"glyphB", b}, {"metapost", gap(ga.exact, gb.exact, samples)},
        {"tarteel", gap(ga.app, gb.app, samples)}, {"metapostUnit", gap(ga.unit, gb.unit, samples)}});
    }
    QJsonObject inputs;
    for (const auto& [key, path] : std::vector<std::pair<QString, fs::path>>{{"binary", binary}, {"glyphMap", names},
        {"project", project}, {"glyphSources", project.parent_path() / "glyphs.mp"}, {"features", project.parent_path() / "features.fea"}})
      inputs[key] = QJsonObject{{"path", QString::fromStdString(path.string())}, {"sha256", hash(path)}};
    QJsonObject report{{"schemaVersion", 1}, {"binaryVersion", int(doc.version)}, {"inputs", inputs}, {"page", pageIndex},
      {"line", lineIndex}, {"lineX", line.x}, {"lineXScale", line.xscale}, {"lineFontSize", line.fontSize}, {"metapostScale", mpScale}, {"pixelsPerUnit", pixelsPerUnit},
      {"samplesPerCubic", samples + 1}, {"imageWidth", size.width()}, {"imageHeight", size.height()},
      {"rasterizer", "Qt QPainter (both paths); app geometry mirrored, not device Skia"},
      {"rmsDefinition", "Euclidean distance at corresponding cubic t values, uniform t, including both endpoints; font units, not arc-length weighted"},
      {"gapDefinition", "Minimum filled-outline clearance in positioned line units; sampled cubic segments, approximate, not vertical or raster gap"},
      {"frame", "Shared crop, origin at line start; y up. Global page translation omitted equally from both paths."},
      {"changedPixels", double(changed)}, {"pixelRms0To255", std::sqrt(pixelSum / (width * height))},
      {"warnings", warnings}, {"glyphs", glyphReports}, {"gaps", gapReports}};
    writeFile(output / "report.json", QJsonDocument(report).toJson());
    QByteArray html = R"HTML(<!doctype html><meta charset="utf-8"><title>Layout rendering comparison</title>
<style>body{font:16px system-ui;margin:24px}img{max-width:100%;background:white;border:1px solid #ddd}pre{white-space:pre-wrap}button{margin:4px;padding:8px}#view{overflow:auto}#view img.zoom{max-width:none}</style>
<h1>MetaPost / Tarteel geometry comparison</h1><p>Identical binary placements and Qt rasterization. MetaPost evaluates live glyphs; Tarteel interpolates stored masters. This does not test device Skia.</p>
<p><button onclick="show('metapost')">MetaPost</button><button onclick="show('tarteel')">Tarteel</button><button onclick="show('metapost-unit')">MetaPost at scale 1</button><button onclick="show('overlay')">Overlay</button><button onclick="show('difference')">Pixel difference</button><button onclick="document.querySelector('img').classList.toggle('zoom')">Actual pixels / fit</button></p>
<p id="label">overlay: magenta = MetaPost, cyan = Tarteel. Pixel difference: red = more MetaPost ink, blue = more Tarteel ink.</p>
<div id="view"><img src="overlay.png"></div><p><a href="report.json">Full measurements and input hashes</a> · <a href="overlay.svg">Zoomable SVG overlay</a></p><h2>Run details</h2><pre>REPORT</pre>
<script>function show(name){document.querySelector('img').src=name+'.png';document.getElementById('label').textContent=name+(name==='difference'?' — red: more MetaPost ink; blue: more Tarteel ink':'');}</script>)HTML";
    html.replace("REPORT", QString::fromUtf8(QJsonDocument(report).toJson()).toHtmlEscaped().toUtf8());
    writeFile(output / "index.html", html);
    std::cout << "Wrote " << fs::absolute(output) << "\n" << rendered.size() << " glyphs; " << changed
              << " differing pixels; " << warnings.size() << " source warnings\n";
    for (const auto& item : gapReports) std::cout << QJsonDocument(item.toObject()).toJson(QJsonDocument::Compact).constData() << '\n';
    return 0;
  } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << '\n'; return 1; }
}
