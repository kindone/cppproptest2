#pragma once
#include "proptest/Shrinkable.hpp"

namespace proptest {

PROPTEST_API Shrinkable<string> shrinkString(const string& str, size_t minSize = 0);

}
