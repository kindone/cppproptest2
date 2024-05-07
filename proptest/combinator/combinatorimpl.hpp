#pragma once

#include "proptest/api.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

struct GeneratorCommon;

namespace util {
struct WeightedBase;

/* fwd-declaration of impl. for combinators */
PROPTEST_API GeneratorCommon filterImpl(Function1<ShrinkableBase> gen, Function1<bool> criteria);
PROPTEST_API GeneratorCommon dependencyImpl(Function1<ShrinkableBase> gen, Function1<Function1<ShrinkableBase>> gen2gen);
PROPTEST_API GeneratorCommon transformImpl(Function1<ShrinkableBase> gen, Function1<Any> transformer);
PROPTEST_API GeneratorCommon deriveImpl(Function1<ShrinkableBase> gen1, Function1<Function1<ShrinkableBase>> gen2gen);
PROPTEST_API GeneratorCommon chainImpl1(Function1<ShrinkableBase> gen, Function1<Function1<ShrinkableBase>> gen2gen);
PROPTEST_API GeneratorCommon chainImplN(Function1<ShrinkableBase> gen, Function1<Function1<ShrinkableBase>> gen2gen);
PROPTEST_API GeneratorCommon aggregateImpl(Function1<ShrinkableBase> gen1, Function1<Function1<ShrinkableBase>> gen2gen, size_t minSize, size_t maxSize);
PROPTEST_API GeneratorCommon oneOfImpl(const shared_ptr<vector<util::WeightedBase>>& genVecPtr);

} // namespace util
} // namespace proptest

