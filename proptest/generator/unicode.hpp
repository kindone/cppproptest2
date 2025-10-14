#pragma once

#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"

namespace proptest {

namespace gen {

PROPTEST_API Generator<uint32_t> unicode();

} // namespace gen

}  // namespace proptest
