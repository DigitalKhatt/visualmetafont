// tajweed_service.h
//
// C++ port of tajweed.service.ts (Angular/TypeScript) using PCRE2-16 and
// std::u16string. The regex patterns, capture-group names and the two match
// loops in applyTajweedForText() are translated statement-for-statement from
// the original TS so that behavior (including quirks/dead branches) matches
// exactly.
//
// Requires PCRE2 built with 16-bit support AND variable-length lookbehind
// support (PCRE2 >= 10.43, released Feb 2023). Several lookbehind assertions
// in this pattern set are genuinely variable length (e.g. `(?<=[X][marks]*)`)
// and are NOT expressible as a fixed-length-per-branch lookbehind, so an
// older PCRE2 will fail to compile the pattern with "lookbehind assertion is
// not fixed length". Build/link against a recent PCRE2 (10.44+ recommended).
//
// Dependency note: the original TS file imports `LineType`,
// `MushafLayoutType`, and `QuranTextService` from './qurantext.service'.
// That file was not provided, so this header declares a minimal interface,
// `IQuranTextService`, that captures exactly the surface area
// applyTajweedByPage() uses:
//   - textService.quranText[pageIndex][lineIndex]   -> quranText()
//   - textService.getLineInfo(pageIndex, lineIndex)  -> getLineInfo()
//   - textService.mushafType                          -> mushafType()
// Adapt/implement IQuranTextService against your real Quran text service.

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "digitalkhatt/core/digitalkahtt_types.h"

namespace digitalkhatt::tajweed {

enum class MushafLayoutType {
  Madinah15Lines,
  IndoPak15Lines,
};

// --- TajweedService ----------------------------------------------------------

class CompiledTajweedRegex;  // pimpl, wraps a compiled pcre2_code

class TajweedService {
 public:
  TajweedService();
  ~TajweedService();

  TajweedService(const TajweedService&) = delete;
  TajweedService& operator=(const TajweedService&) = delete;

  // Mirrors: setTajweed: (pos: number, tajweed: string) => void
  // The original JS occasionally calls setTajweed(pos, undefined) to clear
  // a previously-set tag (see the qalqala-removal branch); that is
  // represented here as std::nullopt.
  using SetTajweedFn = std::function<void(int pos, std::optional<std::string> tajweed)>;
  using ResetIndexFn = std::function<void()>;

  void applyTajweedForText(const std::u16string& text,
                           const SetTajweedFn& setTajweed,
                           const ResetIndexFn& resetIndex,
                           bool isIndopak);

  // Returns one map per line: map[column] = tajweed tag (or nullopt if
  // explicitly cleared).
  PageTajweedResult applyTajweedByPage(const std::vector<LineToJustify>& lines, bool isIndopak);

 private:
  std::unique_ptr<CompiledTajweedRegex> tafkhimRE_;
  std::unique_ptr<CompiledTajweedRegex> othersREMadinah_;
  std::unique_ptr<CompiledTajweedRegex> othersREIndoPak_;
};

}  // namespace digitalkhatt::tajweed
