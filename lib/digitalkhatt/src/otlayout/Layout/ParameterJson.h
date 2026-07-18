#ifndef PARAMETERJSON_H
#define PARAMETERJSON_H

#include <optional>

#include "GlazeJson.h"

using ParameterJsonObject = glz::generic_sorted::object_t;

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
