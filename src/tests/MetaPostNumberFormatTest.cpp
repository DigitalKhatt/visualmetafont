// Standalone Qt regression for glyph expression serialization.
// Run with CTest's visualmetafont.number_format test.
#include "exp.hpp"
#include "pathpointexp.hpp"
#include <QRegularExpression>
#include <iostream>
#include <limits>

int main() {
  const QRegularExpression decimal(QStringLiteral("^-?[0-9]+(?:\\.[0-9]+)?$"));
  for (double value : {0., -0., 1., -1., 123.456789, 4e-5, -1e-6,
                       1.23456789e-30, -1.23456789e20,
                       std::numeric_limits<double>::denorm_min(),
                       std::numeric_limits<double>::max()}) {
    const QString original = QStringLiteral("%1").arg(value);
    const QString source = metapostNumber(value);
    if (!decimal.match(source).hasMatch() || source.toDouble() != original.toDouble()) {
      std::cerr << "Invalid or changed numeric literal: " << original.toStdString()
                << " -> " << source.toStdString() << '\n';
      return 1;
    }
    if (!original.contains(QLatin1Char('e'), Qt::CaseInsensitive) && source != original)
      return 2;
  }
  if (LitNumber(4e-5).toString() != QStringLiteral("0.00004") ||
      LitPathNumericExp(-1e-6).toString() != QStringLiteral("-0.000001") ||
      DirNumber(4e-5).toString() != QStringLiteral("dir 0.00004") ||
      LitPoint(QPointF(4e-5, -1e-6)).toString() != QStringLiteral("(0.00004,-0.000001)"))
    return 3;
  std::cout << "PASS: decimal glyph literals, unchanged numeric values and ordinary formatting\n";
}
