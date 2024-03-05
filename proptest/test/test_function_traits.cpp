#include "proptest/util/function_traits.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

template <class... Args>
struct OtherType {
};

TEST(FunctionTraits, basic)
{
    auto lambda = [](int a, int b) { return a+b; };
    using TheOtherType = function_traits<decltype(lambda)>::template_type_with_args<OtherType>;
    [[maybe_unused]] TheOtherType otherType;
}