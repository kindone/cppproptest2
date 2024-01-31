#include "proptest/generator/bool.hpp"
#include "proptest/shrinker/bool.hpp"

namespace proptest {

Arbi<bool>::Arbi(double prob) : trueProb(prob) {}

Shrinkable<bool> Arbi<bool>::operator()(Random& rand) const
{
    bool value = rand.getRandomBool(trueProb);
    return shrinkBool(value);
}

}  // namespace proptest
