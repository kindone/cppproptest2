// This file MUST fail to compile.
// It tests that gen::elementOf rejects gen::weighted(generator, prob) with a clear static_assert.
// Correct alternatives:
//   - For values: gen::elementOf(gen::weighted(value, prob), ...) or gen::weightedVal(value, prob)
//   - For generators: gen::oneOf(gen::weighted(gen, prob), ...)

#include "proptest/proptest.hpp"

using namespace proptest;

void elementOf_rejects_weighted_generator()
{
    // ERROR: elementOf accepts values only, not generators.
    // Use gen::oneOf for generators.
    auto bad = gen::elementOf<int>(gen::weighted(gen::interval(0, 10), 0.5));
    (void)bad;
}
