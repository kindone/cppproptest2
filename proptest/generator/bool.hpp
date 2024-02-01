#pragma once
#include "proptest/Arbitrary.hpp"
#include "proptest/Stream.hpp"

/**
 * @file bool.hpp
 * @brief Arbitrary for bool
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for boolean with configurable true/false probability
 */
template <>
class PROPTEST_API Arbi<bool> final : public ArbiBase<bool> {
public:
    Arbi(double prob = 0.5);

    Shrinkable<bool> operator()(Random& rand) const override;

    Arbi(const Arbi<bool>&) = default;

    shared_ptr<GeneratorBase> clone() const override {
        return util::make_shared<Arbi>(trueProb);
    }

private:
    double trueProb;
};

// template struct Arbi<bool>;

}  // namespace proptest
