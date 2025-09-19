#pragma once

#include <functional>
#include <string>
#include <algorithm>

namespace esphome {
namespace dtu {

/// Trim string inplace from the begin.
/// @param predicate to test trimmed characters, by default checks with isspace.
template<typename... Predicate> constexpr inline void str_ltrim_ref(std::string &s, Predicate... predicate) {
  if constexpr (sizeof...(Predicate) == 1) {
#if __cplusplus >= 202002L
    s.erase(s.cbegin(), std::ranges::find_if_not(s, std::forward<Predicate>(predicate)...));
#else
    s.erase(s.cbegin(), std::find_if_not(s.cbegin(), s.cend(), std::forward<Predicate>(predicate)...));
#endif
  } else if constexpr (sizeof...(Predicate) == 0) {
#if __cplusplus >= 202002L
    s.erase(s.cbegin(), std::ranges::find_if_not(s, [](char c) { return std::isspace(c); }));
#else
    s.erase(s.cbegin(), std::find_if_not(s.cbegin(), s.cend(), [](char c) { return std::isspace(c); }));
#endif
  } else {
    static_assert(sizeof...(Predicate) > 1, "invalid predicate parameter");
  }
}

/// Trim string inplace from then end.
/// @param predicate to test trimmed characters, by default checks with isspace.
template<typename... Predicate> constexpr inline void str_rtrim_ref(std::string &s, Predicate... predicate) {
  if constexpr (sizeof...(Predicate) == 1) {
#if __cplusplus >= 202002L
    s.erase(std::ranges::find_if_not(s.rbegin(), s.rend(), std::forward<Predicate>(predicate)...).base(), s.end());
#else
    s.erase(std::find_if_not(s.rbegin(), s.rend(), std::forward<Predicate>(predicate)...).base(), s.end());
#endif
  } else if constexpr (sizeof...(Predicate) == 0) {
#if __cplusplus >= 202002L
    s.erase(std::ranges::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());
#else
    s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());
#endif
  } else {
    static_assert(sizeof...(Predicate) > 1, "invalid predicate parameter");
  }
}

/// Trim string inplace from then both sides.
/// @param predicate to test trimmed characters, by default checks with isspace.
template<typename... Predicate> constexpr inline void str_trim_ref(std::string &s, Predicate... predicate) {
  str_rtrim_ref(s, std::forward<Predicate>(predicate)...);
  str_ltrim_ref(s, std::forward<Predicate>(predicate)...);
}

/// Trim string from the begin.
/// @param predicate to test trimmed characters, by default checks with isspace.
template<typename... Predicate> inline std::string str_ltrim(const std::string &s, Predicate... predicate) {
  auto ref = s;
  str_ltrim_ref(ref, std::forward<Predicate>(predicate)...);
  return ref;
}

/// Trim string from then end.
/// @param predicate to test trimmed characters, by default checks with isspace.
template<typename... Predicate> inline std::string str_rtrim(const std::string &s, Predicate... predicate) {
  auto ref = s;
  str_rtrim_ref(ref, std::forward<Predicate>(predicate)...);
  return ref;
}

/// Trim string from then both sides.
/// @param predicate to test trimmed characters, by default checks with isspace.
template<typename... Predicate> inline std::string str_trim(const std::string &s, Predicate... predicate) {
  auto ref = s;
  str_trim_ref(ref, std::forward<Predicate>(predicate)...);
  return ref;
}

/// Split the string into parts by separator.
/// Default splits by space.
std::vector<std::string> str_split(const std::string &s, char delimiter = ' ');

/// Split the string into parts by separator.
std::vector<std::string> str_split(const std::string &s, const char *delimiter);

/// Split the string into parts by separator.
inline std::vector<std::string> str_split(const std::string &s, const std::string &delimiter) {
  return str_split(s, delimiter.c_str());
}

}  // namespace dtu
}  // namespace esphome
