#include "proptest/Generator.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/generator/floating.hpp"

namespace proptest {

Shrinkable<float> Arbi<float>::operator()(Random& rand) const
{
    auto raw = rand.getRandomUInt32();
    float value = *reinterpret_cast<float*>(&raw);

    return shrinkFloat(value);
}

Shrinkable<double> Arbi<double>::operator()(Random& rand) const
{
    auto raw = rand.getRandomUInt64();
    double value = *reinterpret_cast<double*>(&raw);

    return shrinkFloat(value);
}

}  // namespace proptest
