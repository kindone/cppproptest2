#include "proptest/combinator/intervals.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/combinator/oneof.hpp"

namespace proptest {

namespace gen {

Generator<int64_t> intervals(initializer_list<Interval> intervals)
{
    using WeightedVec = vector<util::WeightedBase>;

    uint64_t sum = 0;
    for (auto interval : intervals) {
        if (interval.size() == 0)
            throw runtime_error(__FILE__, __LINE__, "invalid empty interval: [" + to_string(interval.min) + ", " +
                                     to_string(interval.max) + "]");
        sum += interval.size();
    }

    auto genVec = util::make_shared<WeightedVec>();
    genVec->reserve(intervals.size());
    for (auto interval : intervals) {
        genVec->push_back(weightedGen(gen::interval<int64_t>(interval.min, interval.max),
                                               static_cast<double>(interval.size()) / sum));
    }

    return util::oneOfImpl(genVec);
}

Generator<uint64_t> uintervals(initializer_list<UInterval> intervals)
{
    using WeightedVec = vector<util::WeightedBase>;

    uint64_t sum = 0;

    for (auto interval : intervals) {
        if (interval.size() == 0)
            throw runtime_error(__FILE__, __LINE__, "invalid empty interval: [" + to_string(interval.min) + ", " +
                                     to_string(interval.max) + "]");
        sum += interval.size();
    }

    auto genVec = util::make_shared<WeightedVec>();

    genVec->reserve(intervals.size());
    for (auto interval : intervals) {
        genVec->push_back(weightedGen(gen::interval<uint64_t>(interval.min, interval.max),
                                                static_cast<double>(interval.size()) / sum));
    }

    return util::oneOfImpl(genVec);
}

}  // namespace gen

}  // namespace proptest
