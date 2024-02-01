#pragma once

#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"

namespace proptest {

struct PROPTEST_API UnicodeGen
{
    Shrinkable<uint32_t> operator()(Random& rand);
};

}  // namespace proptest
