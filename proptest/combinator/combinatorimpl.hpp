#pragma once

#include "proptest/api.hpp"
#include "proptest/util/anyfunction.hpp"

namespace proptest {

struct GeneratorCommon;

namespace util {

/* fwd-declaration of impl. for combinators */
PROPTEST_API GeneratorCommon filterImpl(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon dependencyImpl(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon transformImpl(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon chainImpl1(Function1 gen, Function1 criteria);
PROPTEST_API GeneratorCommon chainImplN(Function1 gen, Function1 criteria);

} // namespace util
} // namespace proptest

