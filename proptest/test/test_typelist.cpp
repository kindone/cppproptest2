#include "proptest/util/function_traits.hpp"
#include "proptest/std/type.hpp"
#include "gtest/gtest.h"

using namespace proptest;


TEST(TypeList, basic)
{
    using namespace proptest::util;
    using TypeList0 = TypeList<>;
    using TypeList1 = TypeList<int, char, double>;
    using TypeList2 = TypeList<int, char, double>;
    using TypeList3 = TypeList<int, char, double, int>;
    using TypeList4 = TypeList<int, char, double, int, char>;
    using TypeList5 = TypeList<int, char, double, int, char, double>;

    EXPECT_TRUE((is_same<TypeList0, TypeList<>>::value));
    EXPECT_TRUE((is_same<TypeList1, TypeList2>::value));
    EXPECT_FALSE((is_same<TypeList1, TypeList3>::value));
    EXPECT_FALSE((is_same<TypeList1, TypeList4>::value));
    EXPECT_FALSE((is_same<TypeList1, TypeList5>::value));

    EXPECT_FALSE((is_same<TypeList2, TypeList3>::value));
    EXPECT_FALSE((is_same<TypeList2, TypeList4>::value));
    EXPECT_FALSE((is_same<TypeList2, TypeList5>::value));

    EXPECT_FALSE((is_same<TypeList3, TypeList4>::value));
    EXPECT_FALSE((is_same<TypeList3, TypeList5>::value));

    EXPECT_FALSE((is_same<TypeList4, TypeList5>::value));
}

TEST(TypeList, append_prepend)
{
    using namespace proptest::util;
    using TypeList1 = TypeList<int, char, double>;
    using TypeList2 = TypeList<int, char, double, int>;
    using TypeList3 = TypeList<int, char, double, int, char>;
    using TypeList4 = TypeList<char, int, char, double, int, char, double>;

    EXPECT_TRUE((is_same_v<TypeList1::append<int>, TypeList2>));
    EXPECT_TRUE((is_same_v<TypeList1::append<int, char>, TypeList3>));
    EXPECT_TRUE((is_same_v<TypeList1::prepend<char>::append<int, char>::append<double>, TypeList4>));
}