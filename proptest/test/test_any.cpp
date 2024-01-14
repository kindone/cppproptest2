#include "proptest/util/any.hpp"
#include "proptest/std/any.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/vector.hpp"
#include "gtest/gtest.h"

using namespace proptest;

struct NonCopyable {
    NonCopyable(int val) : value(val) {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    int value;
};

struct Movable {
    Movable(int val) : value(val) {}
    Movable(const Movable&) = delete;
    Movable(Movable&& other) { value = other.value; other.value = 0; };
    Movable& operator=(const Movable&) = delete;

    int value;
};

struct Copyable {
    Copyable(int val) : value(val), numCopied(0) {}
    Copyable(const Copyable& other) : value(other.value), numCopied(other.numCopied + 1) {
    };

    Copyable& operator=(const Copyable& other) {
        value = other.value;
        numCopied = other.numCopied + 1;
        return *this;
    };

    int value;
    int numCopied;
};


TEST(AssumptionStdAny, int_performance)
{
    for(int i = 0; i < 1000000; i++)
    {
        util::any any = 1;
        ASSERT_EQ(any.type(), typeid(int));
        ASSERT_EQ(util::any_cast<int>(any), 1);
    }
}

TEST(AssumptionStdAny, string_performance)
{
    for(int i = 0; i < 1000000; i++)
    {
        util::any any = string("hello");
        ASSERT_EQ(any.type(), typeid(string));
        ASSERT_EQ(util::any_cast<string>(any), "hello");
    }
}

TEST(AssumptionStdAny, copyable)
{
    // std::any copies one more time
    util::any any = Copyable(100);
    ASSERT_EQ(any.type(), typeid(Copyable));
    ASSERT_EQ(util::any_cast<Copyable>(any).value, 100);
    ASSERT_EQ(util::any_cast<Copyable>(any).numCopied, 2);

    // reassignment copies one more time
    util::any any2 = any;
    ASSERT_EQ(any2.type(), typeid(Copyable));
    ASSERT_EQ(util::any_cast<Copyable>(any2).value, 100);
    ASSERT_EQ(util::any_cast<Copyable>(any2).numCopied, 3);

    // any does not share reference
    ASSERT_EQ(util::any_cast<Copyable>(any).numCopied, 2);
}

TEST(AssumptionStdAny, ptr)
{
    int a = 100;
    util::any any = &a;
    ASSERT_EQ(any.type(), typeid(int*));
    ASSERT_EQ(*util::any_cast<int*>(any), 100);
}

TEST(AssumptionStdAny, reference)
{
    int a = 100;
    int& b = a;
    util::any any = b;
    ASSERT_EQ(any.type(), typeid(int&));
    ASSERT_EQ(util::any_cast<int&>(any), 100);
}

TEST(AssumptionContainerInit, Copyable)
{
    struct Container {
        Copyable copyable;
    };
    Copyable copyable(100);
    ASSERT_EQ(copyable.value, 100);
    ASSERT_EQ(copyable.numCopied, 0);

    // copy elision
    Container container{Copyable(100)};
    ASSERT_EQ(container.copyable.numCopied, 0);

    // lvalue reference does copy
    struct Container2 {
        Container2(const Copyable& c) : copyable(c) {}
        Copyable copyable;
    };

    Container2 container2{Copyable(100)};
    ASSERT_EQ(container2.copyable.numCopied, 1);

    // rvalue reference does copy
    struct Container3 {
        Container3(Copyable&& c) : copyable(util::move(c)) {}
        Copyable copyable;
    };

    Container3 container3{util::move(Copyable(100))};
    ASSERT_EQ(container3.copyable.numCopied, 1);

    // value does copy
    struct Container4 {
        // copy elision for parameter, but field initializaion needs copying
        Container4(Copyable c) : copyable{c} {}
        Copyable copyable;
    };

    Container4 container4{Copyable(100)};
    ASSERT_EQ(container4.copyable.numCopied, 1);
}

TEST(AssumptionContainerInit, CopyableVirtual)
{
    struct Base {
        virtual ~Base() {}
        int otherValue;
    };

    struct Copyable : public Base {
        Copyable(int val) : value(val), numCopied(0) {}
        Copyable(const Copyable& other) : value(other.value), numCopied(other.numCopied + 1) {
        };

        Copyable& operator=(const Copyable& other) {
            value = other.value;
            numCopied = other.numCopied + 1;
            return *this;
        };

        int value;
        int numCopied;
    };

    struct Container {
        Copyable copyable;
    };

    Copyable copyable(100);
    ASSERT_EQ(copyable.value, 100);
    ASSERT_EQ(copyable.numCopied, 0);

    // copy elision
    Container container{Copyable(100)};
    ASSERT_EQ(container.copyable.numCopied, 0);

    // lvalue reference does copy
    struct Container2 {
        Container2(const Copyable& c) : copyable(c) {}
        Copyable copyable;
    };

    Container2 container2{Copyable(100)};
    ASSERT_EQ(container2.copyable.numCopied, 1);

    // rvalue reference does copy
    struct Container3 {
        Container3(Copyable&& c) : copyable(util::move(c)) {}
        Copyable copyable;
    };

    Container3 container3{util::move(Copyable(100))};
    ASSERT_EQ(container3.copyable.numCopied, 1);

    // value does copy
    struct Container4 {
        // copy elision for parameter, but field initializaion needs copying
        Container4(Copyable c) : copyable{c} {}
        Copyable copyable;
    };

    Container4 container4{Copyable(100)};
    ASSERT_EQ(container4.copyable.numCopied, 1);
}


TEST(AssumptionContainerInit, InitializeWithCopyElision)
{
    struct Container {
        NonCopyable nonCopyable;
    };
    NonCopyable nonCopyable(100);
    ASSERT_EQ(nonCopyable.value, 100);

    // copy elision
    Container container{NonCopyable(100)};

    // further construction methods are not allowed (copy constructor is deleted + no move constructor)
}

TEST(AssumptionContainerInit, Movable)
{
    struct Container {
        Movable movable;
    };
    Movable movable(100);
    ASSERT_EQ(movable.value, 100);

    // copy elision
    Container container{Movable(100)};
    ASSERT_EQ(container.movable.value, 100);

    // lvalue reference does copy
    struct Container2 {
        Container2(Movable&& m) : movable(util::move(m)) {}
        Movable movable;
    };

    Container2 container2{util::move(Movable(100))};
    ASSERT_EQ(container2.movable.value, 100);
}

TEST(AnyVal, copied_when_assigned)
{
    AnyVal<Copyable> val = util::make_anyval<Copyable>(Copyable(100));
    ASSERT_EQ(val.type(), typeid(Copyable));
    ASSERT_EQ(val.getRef<Copyable>().value, 100);
    ASSERT_EQ(val.getRef<Copyable>().numCopied, 1);

    AnyVal<Copyable> val2 = val;
    ASSERT_EQ(val2.type(), typeid(Copyable));
    ASSERT_EQ(val2.getRef<Copyable>().value, 100);
    ASSERT_EQ(val2.getRef<Copyable>().numCopied, 2);

    ASSERT_EQ(val.getRef<Copyable>().numCopied, 1);
}

TEST(AnyVal, ptr_mutation)
{
    Copyable copyable(100);
    Copyable* copyablePtr = &copyable;
    AnyVal<Copyable*> val = util::make_anyval<Copyable*>(copyablePtr);
    ASSERT_EQ(val.type(), typeid(Copyable*));
    ASSERT_EQ(val.getRef<Copyable*>()->value, 100);
    ASSERT_EQ(val.getRef<Copyable*>()->numCopied, 0);

    AnyVal<Copyable*> val2 = val;
    ASSERT_EQ(val2.type(), typeid(Copyable*));
    ASSERT_EQ(val2.getRef<Copyable*>()->value, 100);
    ASSERT_EQ(val2.getRef<Copyable*>()->numCopied, 0);

    ASSERT_EQ(val.getRef<Copyable*>()->numCopied, 0);

    copyable.value = 200;
    ASSERT_EQ(val.getRef<Copyable*>()->value, 200);
    ASSERT_EQ(val2.getRef<Copyable*>()->value, 200);
}

TEST(AnyVal, reference_mutation)
{
    Copyable copyable(100);
    Copyable& copyableRef = copyable;
    AnyVal<Copyable&> val = util::make_anyval<Copyable&>(copyableRef);
    ASSERT_EQ(val.type(), typeid(Copyable&));
    ASSERT_EQ(val.getRef<Copyable&>().value, 100);
    ASSERT_EQ(val.getRef<Copyable&>().numCopied, 0);

    AnyVal<Copyable&> val2 = val;
    ASSERT_EQ(val2.type(), typeid(Copyable&));
    ASSERT_EQ(val2.getRef<Copyable&>().value, 100);
    ASSERT_EQ(val2.getRef<Copyable&>().numCopied, 0);
    ASSERT_EQ(val.getRef<Copyable&>().numCopied, 0);

    copyable.value = 200;

    ASSERT_EQ(val.getRef<Copyable&>().value, 200);
    ASSERT_EQ(val2.getRef<Copyable&>().value, 200);
}

TEST(AnyRef, not_copied_when_assigned)
{
    AnyRef<NonCopyable> ref(util::make_shared<NonCopyable>(100));
    ASSERT_EQ(ref.type(), typeid(NonCopyable));
    ASSERT_EQ(ref.getRef<NonCopyable>().value, 100);

    AnyRef<NonCopyable> ref2 = ref;
    ASSERT_EQ(ref2.type(), typeid(NonCopyable));
    ASSERT_EQ(ref2.getRef<NonCopyable>().value, 100);
}

TEST(AnyHolder, value)
{
    unique_ptr<AnyHolder> holder = util::make_unique<AnyVal<int>>(100);
    ASSERT_EQ(holder->type(), typeid(int));
    ASSERT_EQ(holder->getRef<int>(), 100);
}

TEST(AnyHolder, object_value)
{
    struct Object {
        Object(int val) : value(val) {}
        int value;
    };
    unique_ptr<AnyHolder> holder = util::make_unique<AnyVal<Object>>(Object(100));
    ASSERT_EQ(holder->type(), typeid(Object));
    ASSERT_EQ(holder->getRef<Object>().value, 100);
}

TEST(AnyHolder, object_reference)
{
    struct Object {
        Object(int val) : value(val) {}
        int value;
    };
    unique_ptr<AnyHolder> holder = util::make_unique<AnyRef<Object>>(Object(100));
    ASSERT_EQ(holder->type(), typeid(Object));
    ASSERT_EQ(holder->getRef<Object>().value, 100);
}

TEST(AnyHolder, value_reference)
{
    unique_ptr<AnyHolder> holder = util::make_unique<AnyRef<int>>(100);
    ASSERT_EQ(holder->type(), typeid(int));
    ASSERT_EQ(holder->getRef<int>(), 100);
}

TEST(AnyHolder, value_reference_with_shared_ptr)
{
    unique_ptr<AnyHolder> holder = util::make_unique<AnyRef<int>>(util::make_shared<int>(100));
    ASSERT_EQ(holder->type(), typeid(int));
    ASSERT_EQ(holder->getRef<int>(), 100);
}

TEST(AnyHolder, noncopyable)
{
    unique_ptr<AnyHolder> holder = util::make_unique<AnyRef<NonCopyable>>(util::make_shared<NonCopyable>(100));
    ASSERT_EQ(holder->type(), typeid(NonCopyable));
    ASSERT_EQ(holder->getRef<NonCopyable>().value, 100);
}

TEST(AnyHolder, int_performance)
{
    for(int i = 0; i < 1000000; i++)
    {
        unique_ptr<AnyHolder> holder = util::make_unique<AnyVal<int>>(100);
        ASSERT_EQ(holder->type(), typeid(int));
        ASSERT_EQ(holder->getRef<int>(), 100);
    }
}

TEST(AnyHolder, string_performance)
{
    for(int i = 0; i < 1000000; i++)
    {
        unique_ptr<AnyHolder> holder = util::make_unique<AnyRef<string>>("hello");
        ASSERT_EQ(holder->type(), typeid(string));
        ASSERT_EQ(holder->getRef<string>(), "hello");
    }
}

TEST(Any, empty)
{
    Any any;
    ASSERT_EQ(any.isEmpty(), true);
    Any any2 = any;
    ASSERT_EQ(any2.isEmpty(), true);
    auto lambda = [any]() { return any; };
    ASSERT_EQ(lambda().isEmpty(), true);
    const Any& any3 = any;
    auto lambda2 = [any3]() -> const Any& { return any3; };
    ASSERT_EQ(lambda2().isEmpty(), true);
}

TEST(Any, primitive)
{
    Any any = 1;
    ASSERT_EQ(any.type(), typeid(int));
    ASSERT_EQ(any.getRef<int>(), 1);
}

TEST(Any, ptr)
{
    int a = 1;
    Any any = &a;
    ASSERT_EQ(any.type(), typeid(int*));
    ASSERT_EQ(*any.getRef<int*>(), 1);
    // mutate a
    a = 2;
    ASSERT_EQ(*any.getRef<int*>(), 2);
}

TEST(Any, reference)
{
    int a = 1;
    Any any = util::make_any<int&>(a);
    ASSERT_EQ(any.type(), typeid(int&));
    ASSERT_EQ(any.getRef<int&>(), 1);
    // mutate a
    a = 2;
    ASSERT_EQ(any.getRef<int&>(), 2);
}

TEST(Any, class)
{
    Any any = string("hello");
    ASSERT_EQ(any.type(), typeid(string));
    ASSERT_EQ(any.getRef<string>(), "hello");
}

TEST(Any, complex)
{
    vector<string> vec = {"hello", "world"};

    Any any = vec;
    ASSERT_EQ(any.type(), typeid(vector<string>));

    const vector<string>& ref = any.getRef<vector<string>>();
    ASSERT_EQ(ref[0], "hello");
}

TEST(Any, Any)
{
    vector<string> vec = {"hello", "world"};
    Any any0 = vec;
    ASSERT_EQ(any0.type(), typeid(vector<string>));
    Any any(Any{any0});
    ASSERT_EQ(any.type(), typeid(vector<string>));
}

TEST(Any, reassignment_empty)
{
    Any any, empty;
    ASSERT_EQ(any.isEmpty(), true);
    any = 1;
    ASSERT_EQ(any.isEmpty(), false);
    ASSERT_EQ(any.type(), typeid(int));
    any = empty;
    ASSERT_EQ(any.isEmpty(), true);
}

TEST(Any, getMutableRef)
{
    Any any = 1;
    ASSERT_EQ(any.type(), typeid(int));
    ASSERT_EQ(any.getRef<int>(), 1);
    any.getMutableRef<int>() = 2;
    ASSERT_EQ(any.getRef<int>(), 2);
}

TEST(Any, reassignment_with_other_type_throws_error)
{
    Any any = string("hello");
    ASSERT_EQ(any.type(), typeid(string));
    ASSERT_EQ(any.getRef<string>(), "hello");
    EXPECT_THROW(any = 1, invalid_cast_error);
}

TEST(Any, noncopyable_via_shared_ptr)
{
    auto nc = util::make_shared<NonCopyable>(100);
    Any any(nc);
    ASSERT_EQ(any.type(), typeid(shared_ptr<NonCopyable>));
    ASSERT_EQ(any.getRef<shared_ptr<NonCopyable>>()->value, 100);
}

TEST(Any, noncopyable_via_anyref)
{
    shared_ptr<AnyRef<NonCopyable>> ref = util::make_shared<AnyRef<NonCopyable>>(util::make_shared<NonCopyable>(100));
    Any any(ref);
    ASSERT_EQ(any.type(), typeid(NonCopyable));
    ASSERT_EQ(any.getRef<NonCopyable>().value, 100);
}

TEST(Any, primitive_via_make_any)
{
    Any any = util::make_any<int>(100);
    ASSERT_EQ(any.type(), typeid(int));
    ASSERT_EQ(any.getRef<int>(), 100);

    Any copiedAny = any;
    ASSERT_EQ(copiedAny.type(), typeid(int));
    ASSERT_EQ(copiedAny.getRef<int>(), 100);
}

TEST(Any, noncopyable_via_make_any)
{
    Any any = util::make_any<NonCopyable>(100);
    ASSERT_EQ(any.type(), typeid(NonCopyable));
    ASSERT_EQ(any.getRef<NonCopyable>().value, 100);

    Any copiedAny = any;
    ASSERT_EQ(copiedAny.type(), typeid(NonCopyable));
    ASSERT_EQ(copiedAny.getRef<NonCopyable>().value, 100);
}

TEST(Any, int_performance)
{
    for(int i = 0; i < 1000000; i++)
    {
        Any any = 1;
        ASSERT_EQ(any.type(), typeid(int));
        ASSERT_EQ(any.getRef<int>(), 1);
    }
}

TEST(Any, string_performance)
{
    for(int i = 0; i < 1000000; i++)
    {
        Any any = string("hello");
        ASSERT_EQ(any.type(), typeid(string));
        ASSERT_EQ(any.getRef<string>(), "hello");
    }
}