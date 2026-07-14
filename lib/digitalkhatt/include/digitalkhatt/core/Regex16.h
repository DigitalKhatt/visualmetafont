#pragma once

#include "digitalkhatt/core/digitalkahtt_types.h"
#include <memory>
#include <string>

namespace digitalkhatt {

class Regex16Match {
 public:
  Regex16Match();
  Regex16Match(const Regex16Match& other);
  Regex16Match& operator=(const Regex16Match& other);
  Regex16Match(Regex16Match&&) noexcept;
  Regex16Match& operator=(Regex16Match&&) noexcept;
  ~Regex16Match();

  bool hasMatch() const;
  int start() const;
  int end() const;
  int start(int groupNumber) const;
  int end(int groupNumber) const;
  int start(const char* groupName) const;
  int end(const char* groupName) const;
  int lastCapturedIndex() const;
  TextString captured(const char* groupName) const;

 private:
  friend class Regex16;
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

class Regex16 {
 public:
  Regex16();
  explicit Regex16(TextView pattern);
  Regex16(const Regex16& other);
  Regex16& operator=(const Regex16& other);
  Regex16(Regex16&&) noexcept;
  Regex16& operator=(Regex16&&) noexcept;
  ~Regex16();

  bool isValid() const;
  std::string errorString() const;
  int errorOffset() const;

  Regex16Match match(TextView text, int offset = 0) const;

 private:
  friend struct Regex16Match::Impl;
  friend class Regex16Match;
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

Regex16 makeRegex16(TextView pattern);

}  // namespace digitalkhatt
