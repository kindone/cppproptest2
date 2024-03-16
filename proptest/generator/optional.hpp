#pragma once

#include "proptest/Arbitrary.hpp"
#include "proptest/std/optional.hpp"

/**
 * @file optional.hpp
 * @brief Arbitrary for optional<T>
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for optional<T> with configurable element generator
 */
template <typename T>
class Arbi<optional<T>> final : public ArbiBase<optional<T>> {
public:
    Arbi(double _nonEmptyProb = 0.95) : elemGen(Arbi<T>()), nonEmptyProb(_nonEmptyProb) {}

    Arbi(GenFunction<T> _elemGen, double _nonEmptyProb = 0.95) : elemGen(_elemGen), nonEmptyProb(_nonEmptyProb) {}

    Shrinkable<optional<T>> operator()(Random& rand) const override
    {
        if (rand.getRandomBool(nonEmptyProb)) {
            Shrinkable<T> shrinkable = elemGen(rand);
            return shrinkable.template map<optional<T>>(
                +[](const T& t) { return optional<T>(t); });
        } else {
            // null
            return make_shrinkable<optional<T>>();
        }
    }

    shared_ptr<GeneratorBase<optional<T>>> clone() const override {
        return util::make_shared<Arbi>(*this);
    }

    GenFunction<T> elemGen;
    double nonEmptyProb; // probability of providing a nonEmpty value
};

}  // namespace proptest
