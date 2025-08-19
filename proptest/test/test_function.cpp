#include "proptest/util/function.hpp"
#include "proptest/std/memory.hpp"
#include "proptest/std/io.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Function, fptr)
{
    auto lambda0 = +[]() { return 1; };
    Function<int()> function0(lambda0);
    EXPECT_EQ(function0(), 1);
    auto lambda = +[](int a, int b) { return a+b; };
    Function<int(int,int)> function(lambda);
    EXPECT_EQ(function(1,2), 3);

    Function<int(int,int)> function2([](int a, int b) { return a+b; });
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, lambda)
{
    auto lambda0 = []() { return 1; };
    Function<int()> function0(lambda0);
    EXPECT_EQ(function0(), 1);
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function(lambda);
    EXPECT_EQ(function(1,2), 3);

    Function<int(int,int)> function2([](int a, int b) { return a+b; });
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, return_type_to_void)
{
    auto lambda = [](int a, int b) { return a + b; };
    Function<void(int,int)> function(lambda);
    function(1,2);
}

TEST(Function, ptr_args_mutable)
{
    auto lambda = [](int* a, int b) { return *a+b; };
    Function<int(int*,int)> function(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ(function(&x,y), 3);
}

TEST(Function, reference_args_mutable)
{
    auto lambda = [](int& a, int b) { a = a + b; return a + b; };
    Function<int(int&,int)> function(lambda);
    int x = 1;
    int y = 2;
    EXPECT_EQ(function(x,y), 5);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(function(x,y), 7);
    EXPECT_EQ(x, 5);
}

/*
TEST(Function, make_function)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = util::make_function(lambda);
    EXPECT_EQ(function(1,2), 3);
}
*/

TEST(Function, complex)
{
    auto complexTupFunc = [](const tuple<int,string,vector<int>>& tup) { return Animal(get<0>(tup), get<1>(tup), get<2>(tup)); };

    Function<Animal(tuple<int,string,vector<int>>)> function(complexTupFunc);
    auto tup = util::make_tuple(1, "hello", vector<int>{1,2,3});
    auto result = function(tup);
    EXPECT_EQ(result.numFeet, 1);
    EXPECT_EQ(result.name, "hello");
    EXPECT_EQ(result.measures.size(), 3ULL);
    EXPECT_EQ(result.measures[0], 1);
    EXPECT_EQ(result.measures[1], 2);
    EXPECT_EQ(result.measures[2], 3);

    auto func2 = function;
    auto result2 = func2(tup);
    EXPECT_EQ(result2.numFeet, 1);
    EXPECT_EQ(result2.name, "hello");
    EXPECT_EQ(result2.measures.size(), 3ULL);
}

TEST(Function, assign_functor)
{
    struct Functor {
        int operator()(int a, int b) { return a+b; }
    };

    Functor functor;
    Function<int(int,int)> function = functor;
    EXPECT_EQ(function(1,2), 3);
}

int func(int a, int b) { return a+b; }

TEST(Function, assign_function)
{
    Function<int(int,int)> function = func;
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, assign_function_pointer)
{
    int (*fptr)(int a, int b) = func;
    Function<int(int,int)> function = fptr;
    EXPECT_EQ(function(1,2), 3);
}

TEST(Function, copy_and_reset_original)
{
    auto lambda = [](int a, int b) { return a+b; };
    Function<int(int,int)> function = lambda;
    Function<int(int,int)> function2(function);
    EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(int,int)>> function3 = util::make_shared<Function<int(int,int)>>(lambda);
    Function<int(int,int)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(1,2), 3);
}

TEST(Function, void_return)
{
    auto lambda = [](int, int) { return; };
    Function<void(int,int)> function = lambda;
    function(1,2);
}

TEST(Function, void_return2)
{
    auto lambda = [](int, int) { return; };
    Function<void(int,int)> function = lambda;
    function(1,2);
}

TEST(Function, noncopyable_args)
{
    struct NonCopyable {
        NonCopyable() = default;
        NonCopyable(const NonCopyable&) = delete;
    };
    auto lambda = [](const NonCopyable&, const NonCopyable&) { return 0; };
    Function<int(const NonCopyable&, const NonCopyable&)> function = lambda;
    function(NonCopyable(), NonCopyable());
}

TEST(Function, Any_as_parameter)
{
    auto lambda = [](Any a, Any b) -> int { return a.getRef<int>() + b.getRef<int>(); };
    Function<int(Any,Any)> function = lambda;
    Function<int(Any,Any)> function2(function);
    EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(Any,Any)>> function3 = util::make_shared<Function<int(Any,Any)>>(lambda);
    Function<int(Any,Any)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(Any(1),Any(2)), 3);
}

TEST(Function, Any_reference_as_parameter)
{
    auto lambda = [](const Any& a, const Any& b) -> int { return a.getRef<int>() + b.getRef<int>(); };
    Function<int(const Any&,const Any&)> function = lambda;
    Function<int(const Any&,const Any&)> function2(function);
    // EXPECT_EQ(function2(1,2), 3);
    shared_ptr<Function<int(const Any&,const Any&)>> function3 = util::make_shared<Function<int(const Any&,const Any&)>>(lambda);
    Function<int(const Any&,const Any&)> function4(*function3);
    function3.reset();
    EXPECT_EQ(function4(Any(1),Any(2)), 3);
}

TEST(Function, in_capture)
{
    auto outerFactory = [](const Function<int(int,int)>& inner) {
        return [inner](int a, int b) -> int { return inner(a, b); };
    };
    auto factory = [](int a) {
        auto lambda = [a](int b) -> int {
            cout << "captured: " << a << ", " << &a << endl;
            return a+b;
        };
        auto lambda2 = lambda;
        EXPECT_EQ(lambda2(a), 2*a);
        cout << "lambda2(a) = " << lambda2(a) << ", sizeof: " << sizeof(lambda) << endl;
        return Function<int(int)>(lambda);
    };

    /*
    auto factory2 = [](int a) {
        auto lambda = [a](int b) -> int { return a+b; };
        return FunctionNHolderConst<decltype(lambda),int,int,int>(util::forward<decltype(lambda)>(lambda));
    };
    auto functionx = factory2(1);
    EXPECT_EQ(functionx(2), 3);
    */
    Function<int(int)> function0 = factory(1);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);
    EXPECT_EQ(function0(2), 3);

    Function<int(int)> function0_2 = function0;
    EXPECT_EQ(function0_2(2), 3);
    Function<int(int)> function0_3 = function0_2;
    EXPECT_EQ(function0_2(2), function0_3(2));
    EXPECT_EQ(function0(2), 3);
    cout << "all ok so far" << endl;
    Function<int(int,int)> function = [function0](int a, int b) { return function0(a) + b; };
    EXPECT_EQ(function(1,2), 4);
    Function<int(int,int)> function2 = outerFactory(function);
    EXPECT_EQ(function2(1,2), 4);
}

TEST(Function, incompatible_const_ref_arg)
{
    auto constLambda = [](const int& n) -> int {
        return n + 1;
    };

    Function<int(int&)> func = constLambda;
    Function<int(const int&)> constFunc = constLambda;
    Function<int(int&)> func2 = constFunc;
}
