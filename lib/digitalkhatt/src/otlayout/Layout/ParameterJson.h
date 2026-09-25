#ifndef PARAMETERJSON_H
#define PARAMETERJSON_H

#include <cstdint>
#include <optional>
#include <string>

#include "GlazeJson.h"

using ParameterJsonObject = glz::generic_sorted::object_t;

struct ParameterJsonWriteOptions : glz::opts {
  std::uint8_t indentation_width = 4;
  bool new_lines_in_arrays = true;
};

inline auto writeParameterJson(const ParameterJsonObject& object, std::string& buffer) {
  // Match the former QJsonDocument::Indented output for reviewable GUI saves.
  auto error = glz::write<ParameterJsonWriteOptions{glz::opts{.prettify = true}}>(object, buffer);
  if (!error) buffer.push_back('\n');
  return error;
}

inline const glz::generic_sorted* findJsonValue(const ParameterJsonObject& object,
                                                std::string_view key) {
  const auto found = object.find(key);
  return found == object.end() ? nullptr : &found->second;
}

template <typename T>
std::optional<T> jsonValueAs(const glz::generic_sorted& value) {
  T result{};
  if (glz::read_json(result, value)) return std::nullopt;
  return result;
}

template <typename T>
std::optional<T> jsonValueAs(const ParameterJsonObject& object,
                             std::string_view key) {
  const auto* value = findJsonValue(object, key);
  return value ? jsonValueAs<T>(*value) : std::nullopt;
}

#endif
