#include "proptest/combinator/oneof.hpp"

namespace proptest {

namespace util {

GeneratorCommon oneOfImpl(const shared_ptr<vector<util::WeightedBase>>& genVecPtr)
{
    // calculate and assign unassigned weights
    double sum = 0.0;
    int numUnassigned = 0;
    for (size_t i = 0; i < genVecPtr->size(); i++) {
        double weight = (*genVecPtr)[i].weight;
        if (weight < 0.0 || weight > 1.0)
            throw runtime_error(__FILE__, __LINE__, "invalid weight: " + to_string(weight));
        sum += weight;
        if (weight == 0.0)
            numUnassigned++;
    }

    if (sum > 1.0)
        throw runtime_error(__FILE__, __LINE__, "sum of weight exceeds 1.0");

    if (numUnassigned > 0 && sum < 1.0)
        for (size_t i = 0; i < genVecPtr->size(); i++) {
            double& weight = (*genVecPtr)[i].weight;
            if (weight == 0.0)
                weight = (1.0 - sum) / static_cast<double>(numUnassigned);
        }

    return Function1([genVecPtr](Random& rand) {
        while (true) {
            auto dice = rand.getRandomSize(0, genVecPtr->size());
            const util::WeightedBase& weighted = (*genVecPtr)[dice];
            if (rand.getRandomBool(weighted.weight)) {
                // retry the same generator if an exception is thrown
                [[maybe_unused]] uint64_t numRetry = 0;
                while (true) {
                    try {
                        return weighted.func(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>(true);
                    } catch (const Discard&) {
                        // TODO: trace level low
                    }
                    numRetry++;
                    // TODO: trace level low
                }
            }
        };
    });
}

} // namespace util

} // namespace proptest
