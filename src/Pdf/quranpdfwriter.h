/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file originated from the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QURANPDFWRITER_H
#define QURANPDFWRITER_H

#include <QtCore/qobject.h>
#include <QtGui/qpagedpaintdevice.h>
#include <QtGui/qpagelayout.h>
#include <QtGui/qtguiglobal.h>

#include "OtLayout.h"

class QIODevice;
class QuranPdfWriterPrivate;
class OtLayout;

class QuranPdfWriter : public QObject, public QPagedPaintDevice {
  Q_OBJECT
 public:
  explicit QuranPdfWriter(const QString& filename, OtLayout* otlayout);
  explicit QuranPdfWriter(QIODevice* device, OtLayout* otlayout);
  virtual ~QuranPdfWriter();

  QString title() const;
  void setTitle(const QString& title);

  QString creator() const;
  void setCreator(const QString& creator);

  bool newPage() override;

  void setResolution(int resolution);
  int resolution() const;

  /*
  bool setPageLayout(const QPageLayout &pageLayout);
  bool setPageSize(const QPageSize &pageSize);
  bool setPageOrientation(QPageLayout::Orientation orientation);
  bool setPageMargins(const QMarginsF &margins);
  bool setPageMargins(const QMarginsF &margins, QPageLayout::Unit units);
  QPageLayout pageLayout() const;*/

  void setPageSize(PageSize size) override;
  void setPageSizeMM(const QSizeF& size) override;

  void setMargins(const Margins& m) override;

  void beginFormXObject();
  int endFormXObject(QRectF bbox);
  void writeRawPDFtoCurrentStream(QString rawPDF);
  void addImagetoResources(int objectID);
  void generateQuranPages(QList<QList<LineLayoutInfo>> pages, int lineWidth, QList<QStringList> originalText, double scale, int margin = 400 << OtLayout::SCALEBY);

 protected:
  QPaintEngine* paintEngine() const override;
  int metric(PaintDeviceMetric id) const override;

 private:
  Q_DISABLE_COPY(QuranPdfWriter)
  Q_DECLARE_PRIVATE(QuranPdfWriter)
};

#endif  // QURANPDFWRITER_H
