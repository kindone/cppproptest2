#pragma once

#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/std/optional.hpp"

/**
 * @file floating.hpp
 * @brief Arbitrary for float and double
 *
 * By default, generates only finite values. Can optionally generate
 * special values (NaN, +inf, -inf) with specified probabilities.
 */

namespace proptest {

namespace util {

/**
 * @brief Configuration for floating-point generators (Arbi<float>, Arbi<double>).
 * All fields are optional; unspecified fields default to 0.0.
 * Use with designated initializers: Arbi<float>({.nanProb = 0.1, .posInfProb = 0.05})
 */
struct FloatGenConfig {
    optional<double> nanProb = nullopt;
    optional<double> posInfProb = nullopt;
    optional<double> negInfProb = nullopt;
};

}  // namespace util

/**
 * @ingroup Generators
 * @brief Arbitrary for float
 *
 * By default, generates only finite values. Can optionally generate
 * NaN, +inf, and -inf with specified probabilities.
 */
template <>
struct PROPTEST_API Arbi<float> : public ArbiBase<float>
{
public:
    /**
     * @brief Constructor with optional probability parameters
     * @param nanProb Probability of generating NaN (0.0 to 1.0, default: 0.0)
     * @param posInfProb Probability of generating +inf (0.0 to 1.0, default: 0.0)
     * @param negInfProb Probability of generating -inf (0.0 to 1.0, default: 0.0)
     *
     * The sum of nanProb, posInfProb, and negInfProb must be <= 1.0.
     * The remaining probability (1.0 - sum) is used for finite values.
     */
    Arbi(double nanProb = 0.0, double posInfProb = 0.0, double negInfProb = 0.0);

    /**
     * @brief Constructor with named parameters (C++20 designated initializers)
     * @param config util::FloatGenConfig with optional .nanProb, .posInfProb, .negInfProb
     */
    Arbi(const util::FloatGenConfig& config);

    Shrinkable<float> operator()(Random& rand) const override;

    static constexpr float boundaryValues[] = {0.0, 1.0, -1.0};

    Arbi(const Arbi<float>&) = default;

private:
    double nanProb;
    double posInfProb;
    double negInfProb;
};

/**
 * @ingroup Generators
 * @brief Arbitrary for double
 *
 * By default, generates only finite values. Can optionally generate
 * NaN, +inf, and -inf with specified probabilities.
 */
template <>
struct PROPTEST_API Arbi<double> : public ArbiBase<double>
{
public:
    /**
     * @brief Constructor with optional probability parameters
     * @param nanProb Probability of generating NaN (0.0 to 1.0, default: 0.0)
     * @param posInfProb Probability of generating +inf (0.0 to 1.0, default: 0.0)
     * @param negInfProb Probability of generating -inf (0.0 to 1.0, default: 0.0)
     *
     * The sum of nanProb, posInfProb, and negInfProb must be <= 1.0.
     * The remaining probability (1.0 - sum) is used for finite values.
     */
    Arbi(double nanProb = 0.0, double posInfProb = 0.0, double negInfProb = 0.0);

    /**
     * @brief Constructor with named parameters (C++20 designated initializers)
     * @param config util::FloatGenConfig with optional .nanProb, .posInfProb, .negInfProb
     */
    Arbi(const util::FloatGenConfig& config);

    Shrinkable<double> operator()(Random& rand) const override;

    static constexpr double boundaryValues[] = {0.0, 1.0, -1.0};

    Arbi(const Arbi<double>&) = default;

private:
    double nanProb;
    double posInfProb;
    double negInfProb;
};

namespace util {

template <typename T>
struct FloatingInRangeFunctor {
    FloatingInRangeFunctor(T _from, T _to) : from(_from), to(_to) {}
    Shrinkable<T> operator()(Random& rand) { return rand.getRandomDouble(from, to); }
    T from;
    T to;
};

} // namespace util

/**
 * @ingroup Generators
 * @brief Generates numeric values in [from, to).
 */
template <proptest::floating_point T>
PROPTEST_API Generator<T> inRange(T from, T to)
{
    return Function1<ShrinkableBase>(util::FloatingInRangeFunctor(from, to));
}

}  // namespace proptest
