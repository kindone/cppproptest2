#pragma once

#include "proptest/Generator.hpp"
#include "proptest/Random.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/io.hpp"
#include <cstdint>

namespace proptest {

/**
 * @brief Wrapper type for random seeds used in meta-properties.
 * @details This avoids integral boundary-biased generation and prints as Seed(n).
 */
struct Seed
{
    explicit Seed(uint64_t v = 0) : value(v) {}

    uint64_t operator()() const { return value; }
    operator uint64_t() const { return value; }

    uint64_t value = 0;
};

inline bool operator==(const Seed& lhs, const Seed& rhs)
{
    return lhs.value == rhs.value;
}

inline ostream& show(ostream& os, const Seed& seed)
{
    os << "Seed(" << seed.value << ")";
    return os;
}

namespace gen {

/**
 * @ingroup Generators
 * @brief Uniform uint64 seed generator with no shrinking.
 * @details Unlike `gen::uint64()`, this does not inject boundary-value bias.
 */
inline Generator<Seed> seed()
{
    return Function1<ShrinkableBase>([](Random& rand) -> ShrinkableBase {
        return Shrinkable<Seed>(Seed(rand.getRandomUInt64())).clear();
    });
}

} // namespace gen

} // namespace proptest
