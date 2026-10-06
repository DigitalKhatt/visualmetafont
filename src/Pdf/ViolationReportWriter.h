#pragma once
#include <QString>
#include <digitalkhatt/pdf/ViolationReportWriter.h>

// Qt facade; geometry, CSV and PDF generation are implemented by the library.
class ViolationReportWriter : public digitalkhatt::pdf::ViolationReportWriter {
 public:
  bool write(const std::vector<Page>& pages, const QString& pdf, const QString& csv,
             const digitalkhatt::layout::OptParams& params = {}) {
    return digitalkhatt::pdf::ViolationReportWriter::write(pages, pdf.toStdString(), csv.toStdString(), params);
  }
  bool writeSummary(std::vector<WordEntry>& entries, const QString& pdf,
                    const std::vector<std::string>& notes = {}) {
    return digitalkhatt::pdf::ViolationReportWriter::writeSummary(entries, pdf.toStdString(), notes);
  }
};
