#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <ranges>
#include <vector>

namespace digitalkhatt {

// Contiguous binary buffer with explicit big-endian integer serialization.
// OpenType tables store all multi-byte integers in big-endian byte order.
class ByteBuffer {
 public:
  using value_type = std::uint8_t;
  using Storage = std::vector<value_type>;

  bool operator==(const ByteBuffer&) const = default;

  [[nodiscard]] std::size_t size() const noexcept { return storage_.size(); }
  [[nodiscard]] bool empty() const noexcept { return storage_.empty(); }
  [[nodiscard]] const value_type* data() const noexcept { return storage_.data(); }
  [[nodiscard]] value_type* data() noexcept { return storage_.data(); }
  [[nodiscard]] std::span<const value_type> bytes() const noexcept { return storage_; }

  void reserve(std::size_t size) { storage_.reserve(size); }
  void clear() noexcept { storage_.clear(); }

  void writeU8(std::uint8_t value) { storage_.push_back(value); }
  void writeI8(std::int8_t value) { writeU8(static_cast<std::uint8_t>(value)); }

  void writeU16(std::uint16_t value) {
    writeU8(static_cast<std::uint8_t>(value >> 8));
    writeU8(static_cast<std::uint8_t>(value));
  }

  void writeI16(std::int16_t value) { writeU16(static_cast<std::uint16_t>(value)); }

  void writeU32(std::uint32_t value) {
    writeU16(static_cast<std::uint16_t>(value >> 16));
    writeU16(static_cast<std::uint16_t>(value));
  }

  void writeI32(std::int32_t value) { writeU32(static_cast<std::uint32_t>(value)); }

  void writeU64(std::uint64_t value) {
    writeU32(static_cast<std::uint32_t>(value >> 32));
    writeU32(static_cast<std::uint32_t>(value));
  }

  void writeI64(std::int64_t value) { writeU64(static_cast<std::uint64_t>(value)); }

  void append(std::span<const value_type> bytes) {
    storage_.insert(storage_.end(), bytes.begin(), bytes.end());
  }

  void append(const ByteBuffer& other) { append(other.bytes()); }

  template <typename Integer>
    requires std::is_integral_v<Integer>
  ByteBuffer& operator<<(Integer value) {
    if constexpr (sizeof(Integer) == 1)
      writeU8(static_cast<std::uint8_t>(value));
    else if constexpr (sizeof(Integer) == 2)
      writeU16(static_cast<std::uint16_t>(value));
    else if constexpr (sizeof(Integer) == 4)
      writeU32(static_cast<std::uint32_t>(value));
    else if constexpr (sizeof(Integer) == 8)
      writeU64(static_cast<std::uint64_t>(value));
    return *this;
  }

  template <std::ranges::input_range Range>
    requires std::is_integral_v<std::ranges::range_value_t<Range>>
  ByteBuffer& operator<<(const Range& values) {
    for (auto value : values) *this << value;
    return *this;
  }

  void replace(std::size_t offset, std::size_t width,
               const ByteBuffer& replacement) {
    if (width != replacement.size())
      throw std::invalid_argument("ByteBuffer replacement must preserve size");
    checkPatchRange(offset, width);
    std::copy(replacement.bytes().begin(), replacement.bytes().end(),
              storage_.begin() + offset);
  }

  void patchU16(std::size_t offset, std::uint16_t value) {
    checkPatchRange(offset, sizeof(value));
    storage_[offset] = static_cast<std::uint8_t>(value >> 8);
    storage_[offset + 1] = static_cast<std::uint8_t>(value);
  }

  void patchU32(std::size_t offset, std::uint32_t value) {
    checkPatchRange(offset, sizeof(value));
    storage_[offset] = static_cast<std::uint8_t>(value >> 24);
    storage_[offset + 1] = static_cast<std::uint8_t>(value >> 16);
    storage_[offset + 2] = static_cast<std::uint8_t>(value >> 8);
    storage_[offset + 3] = static_cast<std::uint8_t>(value);
  }

  void padTo(std::size_t alignment, std::uint8_t value = 0) {
    if (alignment == 0) throw std::invalid_argument("ByteBuffer alignment must not be zero");
    const auto remainder = size() % alignment;
    if (remainder != 0) storage_.insert(storage_.end(), alignment - remainder, value);
  }

 private:
  void checkPatchRange(std::size_t offset, std::size_t width) const {
    if (offset > size() || width > size() - offset) {
      throw std::out_of_range("ByteBuffer patch exceeds buffer size");
    }
  }

  Storage storage_;
};

}  // namespace digitalkhatt
