#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, aggregate1)
{
    auto gen1 = gen::interval<int>(0, 1000);

    [[maybe_unused]] auto gen = gen::aggregate(
        gen1,
        [](const int& num) {
            return gen::interval(num/2, num*2);
        },
        2, 10);
}

TEST(Compile, aggregate2)
{
    auto gen1 = gen::interval<int>(0, 1).map([](const int& num) {
        list<int> l{num};
        return l;
    });

    [[maybe_unused]] auto gen = gen::aggregate(
        gen1,
        [](const list<int>& nums) -> Generator<list<int>> {
            auto last = nums.back();
            return gen::interval(last, last + 1).map([nums](const int& num) {
                auto newList = list<int>(nums);
                newList.push_back(num);
                return newList;
            });
        },
        2, 4);
}
