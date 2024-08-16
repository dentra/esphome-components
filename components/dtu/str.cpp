#include "str.h"

namespace esphome {
namespace dtu {

template<typename T> std::vector<std::string> str_split_(const std::string &s, T delimiter) {
  std::vector<std::string> res;
  size_t start = 0, end;
  while ((end = s.find(delimiter, start)) != std::string::npos) {
    res.emplace_back(s.substr(start, end - start));
    start = end + 1;
  }
  res.push_back(s.substr(start));
  return res;
}

std::vector<std::string> str_split(const std::string &s, char delimiter) { return str_split_(s, delimiter); }

std::vector<std::string> str_split(const std::string &s, const char *delimiter) { return str_split_(s, delimiter); }

}  // namespace dtu
}  // namespace esphome
