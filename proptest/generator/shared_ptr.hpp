#pragma once

#include "proptest/Arbitrary.hpp"
#include "proptest/std/memory.hpp"


/**
 * @file shared_ptr.hpp
 * @brief Arbitrary for shared_ptr<T> and utility function pairOf(gen1, gen2)
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for shared_ptr<T> with configurable element generators
 */
template <typename T>
class Arbi<shared_ptr<T>> final : public ArbiBase<shared_ptr<T>> {
public:
    Arbi() : elemGen(Arbi<T>()) {}

    Arbi(GenFunction<T> _elemGen) : elemGen(_elemGen) {}

    Shrinkable<shared_ptr<T>> operator()(Random& rand) const override
    {
        auto gen = Arbi<T>();
        Shrinkable<T> shrinkable = gen(rand);
        return shrinkable.template map<shared_ptr<T>>(+[](const T& obj) { return util::make_shared<T>(obj); });
    }

private:
    GenFunction<T> elemGen;
};

}  // namespace proptest
