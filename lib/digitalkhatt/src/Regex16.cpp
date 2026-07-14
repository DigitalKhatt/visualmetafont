#define PCRE2_CODE_UNIT_WIDTH 16
#include <pcre2.h>

#include "digitalkhatt/core/Regex16.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace digitalkhatt {

struct Regex16::Impl {
  pcre2_code* code = nullptr;
  TextString pattern;
  // Raw PCRE2 diagnostic text (UTF-16, as PCRE2 returns it). Captured as-is,
  // with no conversion; errorString() only narrows it to std::string lazily,
  // on the rare path where a caller actually asks for it.
  TextString error;
  int errorOffset = -1;

  Impl() = default;
  explicit Impl(TextView pat) : pattern(pat) {
    int err = 0;
    PCRE2_SIZE errOff = 0;
    code = pcre2_compile(reinterpret_cast<PCRE2_SPTR16>(pattern.data()),
                         pattern.size(),
                         PCRE2_UTF | PCRE2_UCP,
                         &err,
                         &errOff,
                         nullptr);
    if (!code) {
      PCRE2_UCHAR16 buffer[256]{};
      pcre2_get_error_message(err, buffer, 256);
      size_t len = 0;
      while (len < 256 && buffer[len] != 0) ++len;
      error.assign(reinterpret_cast<const char16_t*>(buffer), len);
      errorOffset = static_cast<int>(errOff);
      return;
    }
    // Equivalent spirit to OptimizeOnFirstUsageOption, but eager and harmless if JIT is unavailable.
    pcre2_jit_compile(code, PCRE2_JIT_COMPLETE);
  }
  ~Impl() {
    if (code) pcre2_code_free(code);
  }

  Impl(const Impl& other) : Impl(other.pattern) {}
};

struct Regex16Match::Impl {
  const Regex16::Impl* re = nullptr;
  TextView text;
  pcre2_match_data* data = nullptr;
  bool ok = false;
  int matchCount = 0;
  int offset = 0;

  Impl() = default;
  Impl(const Regex16::Impl* regex, TextView t, int matchOffset)
      : re(regex), text(t), offset(matchOffset) {
    if (!re || !re->code) return;
    data = pcre2_match_data_create_from_pattern(re->code, nullptr);
    int rc = pcre2_match(re->code,
                         reinterpret_cast<PCRE2_SPTR16>(text.data()),
                         text.size(),
                         static_cast<PCRE2_SIZE>(offset),
                         0,
                         data,
                         nullptr);
    ok = rc >= 0;
    matchCount = ok ? rc : 0;
  }
  Impl(const Impl& other) : Impl(other.re, other.text, other.offset) {}
  ~Impl() {
    if (data) pcre2_match_data_free(data);
  }

  int groupNumber(const char* name) const {
    if (!re || !re->code) return -1;
    TextString n;
    for (const char* p = name; *p; ++p) n.push_back(static_cast<char16_t>(*p));
    n.push_back(u'\0');
    return pcre2_substring_number_from_name(re->code,
                                            reinterpret_cast<PCRE2_SPTR16>(n.data()));
  }

  int startNumber(int n) const {
    if (!ok || !data || n < 0) return -1;
    PCRE2_SIZE* ov = pcre2_get_ovector_pointer(data);
    if (!ov) return -1;
    uint32_t count = pcre2_get_ovector_count(data);
    if (static_cast<uint32_t>(n) >= count) return -1;
    PCRE2_SIZE v = ov[2 * n];
    return v == PCRE2_UNSET ? -1 : static_cast<int>(v);
  }

  int endNumber(int n) const {
    if (!ok || !data || n < 0) return -1;
    PCRE2_SIZE* ov = pcre2_get_ovector_pointer(data);
    if (!ov) return -1;
    uint32_t count = pcre2_get_ovector_count(data);
    if (static_cast<uint32_t>(n) >= count) return -1;
    PCRE2_SIZE v = ov[2 * n + 1];
    return v == PCRE2_UNSET ? -1 : static_cast<int>(v);
  }
};

Regex16::Regex16() : impl_(std::make_unique<Impl>()) {}
Regex16::Regex16(TextView pattern) : impl_(std::make_unique<Impl>(pattern)) {}
Regex16::Regex16(const Regex16& other) : impl_(std::make_unique<Impl>(*other.impl_)) {}
Regex16& Regex16::operator=(const Regex16& other) {
  if (this != &other) impl_ = std::make_unique<Impl>(*other.impl_);
  return *this;
}
Regex16::Regex16(Regex16&&) noexcept = default;
Regex16& Regex16::operator=(Regex16&&) noexcept = default;
Regex16::~Regex16() = default;

bool Regex16::isValid() const { return impl_ && impl_->code; }
std::string Regex16::errorString() const {
  if (!impl_) return "no regex implementation";
  // PCRE2 diagnostic messages are always plain ASCII, so a direct per-code-unit
  // narrowing cast is exact here -- no surrogate-pair decoding required.
  std::string result;
  result.reserve(impl_->error.size());
  for (char16_t ch : impl_->error) result.push_back(static_cast<char>(ch));
  return result;
}
int Regex16::errorOffset() const { return impl_ ? impl_->errorOffset : -1; }
Regex16Match Regex16::match(TextView text, int offset) const {
  Regex16Match m;
  m.impl_ = std::make_unique<Regex16Match::Impl>(impl_.get(), text, offset);
  return m;
}

Regex16Match::Regex16Match() : impl_(std::make_unique<Impl>()) {}
Regex16Match::Regex16Match(const Regex16Match& other)
    : impl_(other.impl_ ? std::make_unique<Impl>(*other.impl_) : std::make_unique<Impl>()) {}
Regex16Match& Regex16Match::operator=(const Regex16Match& other) {
  if (this != &other) {
    impl_ = other.impl_ ? std::make_unique<Impl>(*other.impl_) : std::make_unique<Impl>();
  }
  return *this;
}
Regex16Match::Regex16Match(Regex16Match&&) noexcept = default;
Regex16Match& Regex16Match::operator=(Regex16Match&&) noexcept = default;
Regex16Match::~Regex16Match() = default;

bool Regex16Match::hasMatch() const { return impl_ && impl_->ok; }
int Regex16Match::start() const { return impl_ ? impl_->startNumber(0) : -1; }
int Regex16Match::end() const { return impl_ ? impl_->endNumber(0) : -1; }
int Regex16Match::start(int groupNumber) const {
  return impl_ ? impl_->startNumber(groupNumber) : -1;
}
int Regex16Match::end(int groupNumber) const {
  return impl_ ? impl_->endNumber(groupNumber) : -1;
}
int Regex16Match::start(const char* groupName) const {
  if (!impl_) return -1;
  return impl_->startNumber(impl_->groupNumber(groupName));
}
int Regex16Match::end(const char* groupName) const {
  if (!impl_) return -1;
  return impl_->endNumber(impl_->groupNumber(groupName));
}
int Regex16Match::lastCapturedIndex() const {
  return impl_ && impl_->ok ? impl_->matchCount - 1 : -1;
}
TextString Regex16Match::captured(const char* groupName) const {
  if (!impl_ || !impl_->ok) return {};
  int s = start(groupName);
  int e = end(groupName);
  if (s < 0 || e < s) return {};
  return TextString(impl_->text.substr(static_cast<size_t>(s), static_cast<size_t>(e - s)));
}

Regex16 makeRegex16(TextView pattern) {
  Regex16 re(pattern);
  if (!re.isValid()) {
    throw std::runtime_error("PCRE2-16 error at offset " + std::to_string(re.errorOffset()) + ": " + re.errorString());
  }
  return re;
}

}  // namespace digitalkhatt
