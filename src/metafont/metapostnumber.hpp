#ifndef METAPOSTNUMBER_HPP
#define METAPOSTNUMBER_HPP

#include <QString>

// The glyph grammar and MetaPost require decimal literals, not e notation.
// Expand Qt's existing representation without changing its precision or
// rounding tiny coordinates to zero.
inline QString metapostNumber(double value) {
  QString text = QString::number(value);
  const int exponentIndex = text.indexOf(QLatin1Char('e'), 0, Qt::CaseInsensitive);
  if (exponentIndex < 0) return text;

  const int exponent = text.mid(exponentIndex + 1).toInt();
  QString digits = text.left(exponentIndex);
  QString sign;
  if (digits.startsWith(QLatin1Char('-'))) {
    sign = QStringLiteral("-");
    digits.remove(0, 1);
  }
  int decimal = digits.indexOf(QLatin1Char('.'));
  if (decimal < 0) decimal = digits.size();
  digits.remove(QLatin1Char('.'));
  decimal += exponent;
  if (decimal <= 0)
    return sign + QStringLiteral("0.") + QString(-decimal, QLatin1Char('0')) + digits;
  if (decimal >= digits.size())
    return sign + digits + QString(decimal - digits.size(), QLatin1Char('0'));
  digits.insert(decimal, QLatin1Char('.'));
  return sign + digits;
}

#endif // METAPOSTNUMBER_HPP
