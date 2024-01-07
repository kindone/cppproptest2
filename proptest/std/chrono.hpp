#pragma once
#include <chrono>

namespace proptest {

using std::chrono::steady_clock;
using std::chrono::duration_cast;

namespace util {
using std::chrono::milliseconds;
} // namespace util

} // namespace proptest