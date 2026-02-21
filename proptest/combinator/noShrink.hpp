#pragma once

#include "proptest/Generator.hpp"
#include "proptest/GenType.hpp"

/**
 * @file noShrink.hpp
 * @brief Generator combinator that disables shrinking
 */

namespace proptest {

namespace gen {

/**
 * @ingroup Combinators
 * @brief Wraps a generator to produce the same values but with an empty shrink stream.
 * Use when shrinking is meaningless (e.g. seeds, UUIDs, timestamps).
 *
 * @tparam GEN Generator-like type (Generator, Arbi, or GenFunction)
 * @param gen The base generator
 * @return Generator<T> Generator with same values but no shrink candidates
 *
 * @code
 * // Seed that should not be shrunk
 * auto seedGen = gen::noShrink(gen::uint64());
 * // Equivalent to: gen::uint64().noShrink()
 * @endcode
 */
template <GenLike GEN>
decltype(auto) noShrink(GEN&& gen)
{
    return generator(util::forward<GEN>(gen)).noShrink();
}

} // namespace gen

} // namespace proptest
