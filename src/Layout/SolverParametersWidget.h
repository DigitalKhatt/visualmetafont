#pragma once

#include <QWidget>

#include <digitalkhatt/layout/OptParams.h>

class QTimer;

// Dock-hosted panel for tuning digitalkhatt::layout::OptParams live and
// toggling individual XPBD constraints on/off to see their isolated effect
// on the current page. Edits the referenced OptParams in place and emits
// parametersChanged() (debounced) so the caller can re-render.
class SolverParametersWidget : public QWidget {
  Q_OBJECT

 public:
  explicit SolverParametersWidget(digitalkhatt::layout::OptParams& params, QWidget* parent = nullptr);

 signals:
  void parametersChanged();

 private:
  digitalkhatt::layout::OptParams& m_params;
  QTimer* m_debounce;

  void scheduleChanged();
};
