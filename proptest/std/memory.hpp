#pragma once
#include <memory>

namespace proptest {

using std::shared_ptr;
using std::static_pointer_cast;
using std::unique_ptr;

namespace util {
using std::make_shared;
using std::make_unique;
}  // namespace util

} // namespace proptest