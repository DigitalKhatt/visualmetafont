#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>

#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>
#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>

namespace digitalkhatt::justify {

// Opaque handle for a position an action can write to.  The engine never
// interprets it; the backend maps it to whatever it stores state against (a
// character index in the line today, a glyph parameter record later).
using JustificationSiteRef = int;
inline constexpr JustificationSiteRef kNoSite = -1;

struct JustificationLineMetrics {
  // Committed widths in the measurement font's 1000-unit em, including
  // spacing changes accepted by earlier recipe steps. Not trial widths.
  double targetWidth = 0;
  double currentWidth = 0;
  // Initial shaped width before any spacing/features/stages in the recipe.
  double initialWidth = 0;
};

// The whole font- and HarfBuzz-facing surface an action needs.  The evaluator
// lives in this library and knows nothing about OpenType features, so a backend
// that writes glyph parameters instead of cvXX values is a drop-in replacement
// driving the same catalog.
class JustificationStagingBackend {
 public:
  virtual ~JustificationStagingBackend() = default;

  // One action is one transaction: stage every effect, then commit or abort.
  virtual void beginTransaction() = 0;
  virtual void abortTransaction() = 0;
  virtual FixedSlotActionResult commitTransaction(int wordIndex) = 0;

  // Reads observe staged state and must not create entries.
  virtual bool present(JustificationSiteRef site,
                       JustAttributeId attribute) const = 0;
  virtual double read(JustificationSiteRef site,
                      JustAttributeId attribute) const = 0;

  // Only queried by actions that explicitly use line-width expressions.
  virtual JustificationLineMetrics lineMetrics() const {
    throw std::logic_error("line-width expressions require line metrics");
  }

  // Merges into the site, keeping its other attributes and their order.
  virtual void update(JustificationSiteRef site, JustAttributeId attribute,
                      double value) = 0;
  // Drops everything staged at the site.  A backend's staged state is a
  // description replayed from scratch on every measurement, never a
  // committed mutation, so undoing an application is meaningful.
  virtual void clear(JustificationSiteRef site) = 0;

  // Applies a named one-to-one GSUB lookup to the glyph currently occupying
  // this site. The live staging backend stores the resulting glyph ID
  // beside the native parameters.
  virtual void applyLookup(JustificationSiteRef, std::string_view) {
    throw std::logic_error("lookup effects require a direct-substitution backend");
  }

  // Optional glyph-state inspection used while collecting a candidate. The
  // policy layer remains independent of HarfBuzz: a backend may expose its
  // glyph identifier, or leave it unresolved while retaining the lookup name.
  virtual std::optional<std::uint32_t> glyph(
      JustificationSiteRef) const {
    return std::nullopt;
  }

  // Exact width change of the currently staged transaction relative to its
  // committed word. Candidate collection calls this before aborting. A
  // policy-only backend may leave the value unavailable.
  virtual std::optional<double> measureStagedWidthDelta(int) {
    return std::nullopt;
  }

  // Records a continuous native-parameter endpoint. Ordinary fixed/discrete
  // transactions reject it; candidate collection overrides this method
  // and keeps the current staged value as the range's minimum endpoint.
  virtual void vary(JustificationSiteRef, JustAttributeId, double) {
    throw std::logic_error("vary requires candidate_pool range collection");
  }

  // Resolves a declared attachment from an anchor, or kNoSite.
  virtual JustificationSiteRef attachment(JustificationSiteRef anchor,
                                          JustAttachmentId attachment) const = 0;
};

}  // namespace digitalkhatt::justify
