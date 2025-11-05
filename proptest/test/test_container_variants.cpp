#include "proptest/proptest.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

// Alias for testing - simulates another list-like type
template <typename T>
using my_list = proptest::list<T>;

TEST(ListVariants, default_list)
{
    auto listGen = Arbi<proptest::list<int>>();
    Random rand(getCurrentTime());

    auto shrinkable = listGen(rand);
    [[maybe_unused]] auto& generatedList = shrinkable.getRef();
}

TEST(ListVariants, my_list_alias)
{
    auto mylistGen = Arbi<my_list<int>>();
    Random rand(getCurrentTime());

    auto shrinkable = mylistGen(rand);
    [[maybe_unused]] auto& generatedList = shrinkable.getRef();
}

TEST(ListVariants, list_with_custom_generator)
{
    auto listGen = Arbi<proptest::list<int>>(gen::interval(0, 100));
    Random rand(getCurrentTime());

    auto shrinkable = listGen(rand);
    auto& generatedList = shrinkable.getRef();

    // Verify all elements are in the expected range
    for (const auto& val : generatedList) {
        EXPECT_GE(val, 0);
        EXPECT_LE(val, 100);
    }
}

TEST(ListVariants, my_list_with_custom_generator)
{
    auto mylistGen = Arbi<my_list<int>>(gen::interval(0, 100));
    Random rand(getCurrentTime());

    auto shrinkable = mylistGen(rand);
    auto& generatedList = shrinkable.getRef();

    // Verify all elements are in the expected range
    for (const auto& val : generatedList) {
        EXPECT_GE(val, 0);
        EXPECT_LE(val, 100);
    }
}
