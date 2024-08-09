#pragma once

#include <string>

namespace esphome {
namespace dtu {

/// Parse version string like 1, 1.2, 1.2.3 into uint32_t representation.
/// Max number of part 1 is 9999.
/// Max number of part 2 is 99.
/// Max number of part 3 is 999.
/// Only 3 parts are supported.
/// If the version contains more than 3 parts, all other parts are skipped and the result will be negative.
/// Max version number is 999999999 or its negative version.
/// Part separator is any non number character.
/// Samples:
///  1 -> 100000
///  1.0 -> 100000
///  1.1 -> 101000
///  1.0.0 -> 100000
///  1.1.0 -> 101000
///  1.1.1 -> 101001
///  1.1.1.1 -> -101001
///  1.0.0-beta -> -100000
///  master -> 0
int parse_version(const char *version);

/// @see parse_version(const char*)
inline int parse_version(const std::string &version) { return parse_version(version.c_str()); }

}  // namespace dtu
}  // namespace esphome
