#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

// Minimal, Qt-free reader for the .dat page files written by the Qt desktop
// tool (visualmetafont/src/Layout/LayoutWindow.cpp) via QDataStream: 32-bit
// big-endian lengths/ints, 64-bit big-endian doubles, and QString/QList
// encoded as a length/count prefix followed by that many big-endian UTF-16
// code units / recursively-encoded elements. This only relies on that
// documented, stable Qt stream layout -- it does not link Qt.
class QDataStreamReader {
 public:
  explicit QDataStreamReader(std::vector<std::uint8_t> bytes) : data_(std::move(bytes)) {}

  std::uint32_t readUInt32() {
    require(4);
    std::uint32_t value = (std::uint32_t(data_[pos_]) << 24) |
                          (std::uint32_t(data_[pos_ + 1]) << 16) |
                          (std::uint32_t(data_[pos_ + 2]) << 8) |
                          std::uint32_t(data_[pos_ + 3]);
    pos_ += 4;
    return value;
  }

  std::int32_t readInt32() { return static_cast<std::int32_t>(readUInt32()); }

  double readDouble() {
    require(8);
    std::uint64_t bits = 0;
    for (int i = 0; i < 8; ++i) bits = (bits << 8) | data_[pos_ + i];
    pos_ += 8;
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
  }

  // Qt QString layout: uint32 byte length (0xFFFFFFFF for a null string),
  // then that many bytes of big-endian UTF-16.
  std::u16string readString() {
    std::uint32_t byteLength = readUInt32();
    if (byteLength == 0xFFFFFFFFu) return {};
    require(byteLength);
    std::u16string result;
    result.reserve(byteLength / 2);
    for (std::uint32_t i = 0; i < byteLength; i += 2) {
      char16_t ch = static_cast<char16_t>((std::uint16_t(data_[pos_ + i]) << 8) |
                                          std::uint16_t(data_[pos_ + i + 1]));
      result.push_back(ch);
    }
    pos_ += byteLength;
    return result;
  }

  // Qt QList<T> layout: uint32 count followed by that many elements.
  template <typename T, typename ItemReader>
  std::vector<T> readList(ItemReader readItem) {
    std::uint32_t count = readUInt32();
    std::vector<T> result;
    result.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) result.push_back(readItem(*this));
    return result;
  }

 private:
  void require(std::size_t n) const {
    if (pos_ + n > data_.size()) throw std::runtime_error("QDataStreamReader: unexpected end of data");
  }

  std::vector<std::uint8_t> data_;
  std::size_t pos_ = 0;
};

inline std::vector<std::uint8_t> readBinaryFile(const std::string& fileName) {
  std::ifstream stream(fileName, std::ios::binary);
  if (!stream) return {};
  stream.seekg(0, std::ios::end);
  auto length = static_cast<std::size_t>(stream.tellg());
  stream.seekg(0, std::ios::beg);
  std::vector<std::uint8_t> buffer(length);
  stream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(length));
  return buffer;
}
