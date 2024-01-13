#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

PROPTEST_API Shrinkable<bool> shrinkBool(bool value);

} // namespace proptest
