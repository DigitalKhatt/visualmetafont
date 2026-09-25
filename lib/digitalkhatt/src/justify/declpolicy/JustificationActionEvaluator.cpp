#include <digitalkhatt/justify/declpolicy/JustificationActionEvaluator.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace digitalkhatt::justify {
namespace {

class ActionRun {
 public:
  ActionRun(std::span<const FixedSlotSlot> context, int matchOffset,
            int anchorSlot, JustificationStagingBackend& backend)
      : context_(context),
        matchOffset_(matchOffset),
        anchorSlot_(anchorSlot),
        backend_(backend) {}

  // Slots are addressed relative to the match, so an action may reach past
  // either end of it and into the rest of the subword.  Falling off the
  // subword is not an error: it simply matches nothing, which is what makes a
  // lookahead condition safe to write at a subword boundary.
  const FixedSlotSlot* slot(const JustTarget& target) const {
    const auto index =
        matchOffset_ + (target.slot < 0 ? anchorSlot_ : target.slot);
    if (index < 0 || index >= static_cast<int>(context_.size())) return nullptr;
    return &context_[index];
  }

  // kNoSite when the slot is outside the subword, or the target is an
  // attachment that does not resolve.
  JustificationSiteRef site(const JustTarget& target) const {
    const auto* addressed = slot(target);
    if (addressed == nullptr) return kNoSite;
    if (target.attachment < 0) return addressed->indexInLine;
    return backend_.attachment(
        addressed->indexInLine,
        static_cast<JustAttachmentId>(target.attachment));
  }

  double evaluate(const JustExpr& expression) const {
    std::vector<double> stack;
    stack.reserve(expression.size());
    const auto pop = [&stack] {
      const auto value = stack.back();
      stack.pop_back();
      return value;
    };
    for (const auto& node : expression) {
      switch (node.op) {
        case JustExprOp::Literal:
          stack.push_back(node.literal);
          break;
        case JustExprOp::Underfull:
        case JustExprOp::UnderfullPercent:
        case JustExprOp::InitialUnderfull:
        case JustExprOp::InitialUnderfullPercent:
        case JustExprOp::Stretched:
        case JustExprOp::StretchedPercent: {
          const auto metrics = backend_.lineMetrics();
          double width;
          if (node.op == JustExprOp::InitialUnderfull || node.op == JustExprOp::InitialUnderfullPercent)
            width = metrics.targetWidth - metrics.initialWidth;
          else if (node.op == JustExprOp::Stretched || node.op == JustExprOp::StretchedPercent)
            width = metrics.currentWidth - metrics.initialWidth;
          else
            width = metrics.targetWidth - metrics.currentWidth;
          width = std::max(0.0, width);
          const bool percent = node.op == JustExprOp::UnderfullPercent || node.op == JustExprOp::InitialUnderfullPercent || node.op == JustExprOp::StretchedPercent;
          stack.push_back(percent ? (metrics.targetWidth > 0 ? 100.0 * (width / metrics.targetWidth) : 0.0) : width);
          break;
        }
        case JustExprOp::Attribute: {
          const auto target = site(node.target);
          stack.push_back(target == kNoSite
                              ? 0
                              : backend_.read(target, node.attribute));
          break;
        }
        case JustExprOp::Present: {
          const auto target = site(node.target);
          stack.push_back(target != kNoSite &&
                          backend_.present(target, node.attribute));
          break;
        }
        case JustExprOp::Fact: {
          const auto* addressed = slot(node.target);
          stack.push_back(addressed != nullptr &&
                          (addressed->facts & node.factMask) != 0);
          break;
        }
        case JustExprOp::Not:
          stack.push_back(pop() == 0);
          break;
        case JustExprOp::Select: {
          const auto alternative = pop();
          const auto consequent = pop();
          stack.push_back(pop() != 0 ? consequent : alternative);
          break;
        }
        default: {
          const auto right = pop();
          const auto left = pop();
          stack.push_back(apply(node.op, left, right));
          break;
        }
      }
    }
    if (stack.size() != 1) {
      throw std::runtime_error("justification expression is unbalanced");
    }
    return stack.back();
  }

 private:
  static double apply(JustExprOp op, double left, double right) {
    switch (op) {
      case JustExprOp::Add:
        return left + right;
      case JustExprOp::Sub:
        return left - right;
      case JustExprOp::Mul:
        return left * right;
      case JustExprOp::FloorDiv:
        if (right == 0) {
          throw std::runtime_error("justification expression divides by zero");
        }
        if (left < 0 || right < 0) {
          // Truncation and flooring disagree for negatives, and the values this
          // engine works with are non-negative by construction.
          throw std::runtime_error(
              "justification floordiv requires non-negative operands");
        }
        return std::floor(left / right);
      case JustExprOp::Min:
        return std::min(left, right);
      case JustExprOp::Max:
        return std::max(left, right);
      case JustExprOp::Gt:
        return left > right;
      case JustExprOp::Lt:
        return left < right;
      case JustExprOp::Ge:
        return left >= right;
      case JustExprOp::Le:
        return left <= right;
      case JustExprOp::Eq:
        return left == right;
      case JustExprOp::Ne:
        return left != right;
      case JustExprOp::And:
        return left != 0 && right != 0;
      case JustExprOp::Or:
        return left != 0 || right != 0;
      default:
        throw std::runtime_error("unknown justification expression operator");
    }
  }

  std::span<const FixedSlotSlot> context_;
  int matchOffset_;
  int anchorSlot_;
  JustificationStagingBackend& backend_;
};

void runEffects(const std::vector<JustEffect>& effects, const ActionRun& run,
                JustificationStagingBackend& backend, bool& forbidden) {
  for (const auto& effect : effects) {
    if (forbidden) return;
    if (effect.kind == JustEffectKind::Forbid) {
      if (run.evaluate(effect.condition) != 0) forbidden = true;
      continue;
    }
    if (effect.kind == JustEffectKind::When) {
      if (run.evaluate(effect.condition) != 0) {
        runEffects(effect.nested, run, backend, forbidden);
      }
      continue;
    }

    const auto target = run.site(effect.target);
    // An unresolved attachment or a slot outside the subword writes nothing.
    if (target == kNoSite) continue;

    switch (effect.kind) {
      case JustEffectKind::Add: {
        const auto accumulated =
            backend.read(target, effect.attribute) + run.evaluate(effect.value);
        backend.update(target, effect.attribute,
                       std::min(accumulated, effect.clamp));
        break;
      }
      case JustEffectKind::Update:
        backend.update(target, effect.attribute, run.evaluate(effect.value));
        break;
      case JustEffectKind::Clear:
        backend.clear(target);
        break;
      case JustEffectKind::Lookup:
        backend.applyLookup(target, effect.lookup);
        break;
      case JustEffectKind::Replace:
        backend.clear(target);
        for (const auto& write : effect.writes) {
          backend.update(target, write.attribute, run.evaluate(write.value));
        }
        break;
      case JustEffectKind::Vary:
        backend.vary(target, effect.attribute, run.evaluate(effect.value));
        break;
      case JustEffectKind::Forbid:
      case JustEffectKind::When:
        break;  // handled above
    }
  }
}

}  // namespace

FixedSlotActionResult evaluateJustificationAction(
    const CompiledJustificationCatalog& catalog,
    const JustificationAction& action,
    std::span<const FixedSlotSlot> context,
    int matchOffset,
    int anchorSlot,
    int wordIndex,
    JustificationStagingBackend& backend) {
  // The catalog is resolved into the action and the backend at compile time;
  // it stays in the signature for operators that will need it (facts by
  // name) without forcing another interface change then.
  (void)catalog;
  const ActionRun run(context, matchOffset, anchorSlot, backend);

  backend.beginTransaction();
  bool forbidden = false;
  runEffects(action.effects, run, backend, forbidden);
  if (forbidden) {
    backend.abortTransaction();
    return FixedSlotActionResult::Forbidden;
  }
  return backend.commitTransaction(wordIndex);
}

}  // namespace digitalkhatt::justify
