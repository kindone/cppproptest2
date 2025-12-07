#include "proptest/Generator.hpp"
#include "proptest/shrinker/floating.hpp"
#include "proptest/generator/floating.hpp"
#include "proptest/combinator/combinators.hpp"
#include "proptest/combinator/combinatorimpl.hpp"
#include "proptest/std/math.hpp"
#include "proptest/std/limits.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/vector.hpp"

namespace proptest {

namespace {
    /**
     * @brief Generate a finite float value using bit interpretation with rejection loop
     * @tparam FLOATTYPE float or double
     * @param rand Random number generator
     * @return Shrinkable containing a finite float value
     */
    template <typename FLOATTYPE>
    Shrinkable<FLOATTYPE> generateFiniteFloat(Random& rand)
    {
        while (true) {
            FLOATTYPE value;
            if constexpr (sizeof(FLOATTYPE) == sizeof(float)) {
                auto raw = rand.getRandomUInt32();
                value = *reinterpret_cast<float*>(&raw);
            } else {
                auto raw = rand.getRandomUInt64();
                value = *reinterpret_cast<double*>(&raw);
            }

            // Reject inf/NaN and retry until we get a finite value
            // Use isfinite to ensure we only return finite values
            // Check both isnan/isinf and isfinite for robustness
            if (!isnan(value) && !isinf(value) && isfinite(value)) {
                return shrinkFloat(value);
            }
        }
    }
}  // namespace

Arbi<float>::Arbi(double nanProb, double posInfProb, double negInfProb)
    : nanProb(nanProb), posInfProb(posInfProb), negInfProb(negInfProb)
{
    // Validate probabilities
    if (nanProb < 0.0 || nanProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__, "nanProb must be between 0.0 and 1.0, got " + to_string(nanProb));
    }
    if (posInfProb < 0.0 || posInfProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__, "posInfProb must be between 0.0 and 1.0, got " + to_string(posInfProb));
    }
    if (negInfProb < 0.0 || negInfProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__, "negInfProb must be between 0.0 and 1.0, got " + to_string(negInfProb));
    }

    double totalProb = nanProb + posInfProb + negInfProb;
    if (totalProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__,
            "Sum of probabilities (nanProb + posInfProb + negInfProb) must be <= 1.0, got " + to_string(totalProb));
    }
}

Shrinkable<float> Arbi<float>::operator()(Random& rand) const
{
    // If all probabilities are 0, generate finite values only
    if (nanProb == 0.0 && posInfProb == 0.0 && negInfProb == 0.0) {
        return generateFiniteFloat<float>(rand);
    }

    // Otherwise, use oneOf with weighted generators
    double finiteProb = 1.0 - (nanProb + posInfProb + negInfProb);

    // Build generators list for oneOf using shared_ptr vector
    auto genVecPtr = util::make_shared<vector<util::WeightedBase>>();

    // Add special value generators with their probabilities
    if (nanProb > 0.0) {
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(gen::just<float>(numeric_limits<float>::quiet_NaN()), nanProb)));
    }
    if (posInfProb > 0.0) {
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(gen::just<float>(numeric_limits<float>::infinity()), posInfProb)));
    }
    if (negInfProb > 0.0) {
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(gen::just<float>(-numeric_limits<float>::infinity()), negInfProb)));
    }

    // Add finite generator with remaining probability
    if (finiteProb > 0.0) {
        auto finiteGenFunc = [](Random& rand) -> Shrinkable<float> {
            return generateFiniteFloat<float>(rand);
        };
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(generator(finiteGenFunc), finiteProb)));
    }

    // Use oneOfImpl to create the combined generator
    Generator<float> oneOfGen = util::oneOfImpl(genVecPtr);
    return oneOfGen(rand);
}

Arbi<double>::Arbi(double nanProb, double posInfProb, double negInfProb)
    : nanProb(nanProb), posInfProb(posInfProb), negInfProb(negInfProb)
{
    // Validate probabilities
    if (nanProb < 0.0 || nanProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__, "nanProb must be between 0.0 and 1.0, got " + to_string(nanProb));
    }
    if (posInfProb < 0.0 || posInfProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__, "posInfProb must be between 0.0 and 1.0, got " + to_string(posInfProb));
    }
    if (negInfProb < 0.0 || negInfProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__, "negInfProb must be between 0.0 and 1.0, got " + to_string(negInfProb));
    }

    double totalProb = nanProb + posInfProb + negInfProb;
    if (totalProb > 1.0) {
        throw runtime_error(__FILE__, __LINE__,
            "Sum of probabilities (nanProb + posInfProb + negInfProb) must be <= 1.0, got " + to_string(totalProb));
    }
}

Shrinkable<double> Arbi<double>::operator()(Random& rand) const
{
    // If all probabilities are 0, generate finite values only
    if (nanProb == 0.0 && posInfProb == 0.0 && negInfProb == 0.0) {
        return generateFiniteFloat<double>(rand);
    }

    // Otherwise, use oneOf with weighted generators
    double finiteProb = 1.0 - (nanProb + posInfProb + negInfProb);

    // Build generators list for oneOf using shared_ptr vector
    auto genVecPtr = util::make_shared<vector<util::WeightedBase>>();

    // Add special value generators with their probabilities
    if (nanProb > 0.0) {
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(gen::just<double>(numeric_limits<double>::quiet_NaN()), nanProb)));
    }
    if (posInfProb > 0.0) {
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(gen::just<double>(numeric_limits<double>::infinity()), posInfProb)));
    }
    if (negInfProb > 0.0) {
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(gen::just<double>(-numeric_limits<double>::infinity()), negInfProb)));
    }

    // Add finite generator with remaining probability
    if (finiteProb > 0.0) {
        auto finiteGenFunc = [](Random& rand) -> Shrinkable<double> {
            return generateFiniteFloat<double>(rand);
        };
        genVecPtr->push_back(util::WeightedBase(gen::weightedGen(generator(finiteGenFunc), finiteProb)));
    }

    // Use oneOfImpl to create the combined generator
    Generator<double> oneOfGen = util::oneOfImpl(genVecPtr);
    return oneOfGen(rand);
}

}  // namespace proptest
