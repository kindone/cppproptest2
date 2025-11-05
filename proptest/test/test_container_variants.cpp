#include "proptest/proptest.hpp"
#include "proptest/generator/list.hpp"
#include "proptest/generator/integral.hpp"
#include <gtest/gtest.h>

using namespace proptest;

// Alias for testing - simulates another list-like type
template <typename T>
using my_list = std::list<T>;

TEST(ListVariants, std_list)
{
    auto listGen = Arbi<std::list<int>>();
    Random rand(getCurrentTime());

    auto shrinkable = listGen(rand);
    auto& generatedList = shrinkable.getRef();

    // Just verify we can generate a list
    EXPECT_TRUE(generatedList.size() >= 0);
}

TEST(ListVariants, my_list_alias)
{
    auto mylistGen = Arbi<my_list<int>>();
    Random rand(getCurrentTime());

    auto shrinkable = mylistGen(rand);
    auto& generatedList = shrinkable.getRef();

    // Verify we can generate a list using the alias
    EXPECT_TRUE(generatedList.size() >= 0);
}

TEST(ListVariants, list_with_custom_generator)
{
    auto listGen = Arbi<std::list<int>>(interval(0, 100));
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
    auto mylistGen = Arbi<my_list<int>>(interval(0, 100));
    Random rand(getCurrentTime());

    auto shrinkable = mylistGen(rand);
    auto& generatedList = shrinkable.getRef();

    // Verify all elements are in the expected range
    for (const auto& val : generatedList) {
        EXPECT_GE(val, 0);
        EXPECT_LE(val, 100);
    }
}
