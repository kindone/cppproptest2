#pragma once
#include "proptest/std/io.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/util/unicode.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/util/utf16string.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/std/tuple.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/list.hpp"
#include "proptest/std/set.hpp"
#include "proptest/std/map.hpp"
#include "proptest/std/optional.hpp"

namespace proptest {

PROPTEST_API ostream& show(ostream& os, const char*);
PROPTEST_API ostream& show(ostream& os, const char*, size_t len);
PROPTEST_API ostream& show(ostream& os, const string&);
PROPTEST_API ostream& show(ostream& os, const UTF8String&);
PROPTEST_API ostream& show(ostream& os, const CESU8String&);
PROPTEST_API ostream& show(ostream& os, const UTF16BEString&);
PROPTEST_API ostream& show(ostream& os, const UTF16LEString&);
PROPTEST_API ostream& show(ostream& os, const bool&);
PROPTEST_API ostream& show(ostream& os, const char&);
PROPTEST_API ostream& show(ostream& os, const int8_t&);
PROPTEST_API ostream& show(ostream& os, const uint8_t&);
// ostream& show(ostream& os, const int16_t&);
// ostream& show(ostream& os, const uint16_t&);
// ostream& show(ostream& os, const int32_t&);
// ostream& show(ostream& os, const uint32_t&);
// ostream& show(ostream& os, const int64_t&);
// ostream& show(ostream& os, const uint64_t&);
// ostream& show(ostream& os, const long&);
// ostream& show(ostream& os, const unsigned long&);
PROPTEST_API ostream& show(ostream& os, const float&);
PROPTEST_API ostream& show(ostream& os, const double&);

// forward declaration is needed to be available at call sites
template <typename T>
ostream& show(ostream& os, const T& obj);
template <integral T>
ostream& show(ostream& os, const T& obj) {
    os << obj;
    return os;
}
template <typename T>
ostream& show(ostream& os, const Shrinkable<T>& shrinkable);
template <typename... ARGS>
ostream& show(ostream& os, const tuple<ARGS...>& tuple);
template <typename ARG1, typename ARG2>
ostream& show(ostream& os, const pair<ARG1, ARG2>& pair);
template <typename T, typename Allocator>
ostream& show(ostream& os, const vector<T, Allocator>& vec);
template <typename T, typename Allocator>
ostream& show(ostream& os, const list<T, Allocator>& list);
template <typename T, typename Compare, typename Allocator>
ostream& show(ostream& os, const set<T, Compare, Allocator>& input);
template <class Key, class T, class Compare, class Allocator>
ostream& show(ostream& os, const map<Key, T, Compare, Allocator>& input);
template <typename T>
ostream& show(ostream& os, const shared_ptr<T>& ptr);
template <typename T>
ostream& show(ostream& os, const optional<T>& opt);
template <typename T>
ostream& show(ostream& os, const Any& anyVal);

namespace stateful {
template <typename ObjectType, typename ModelType>
ostream& show(ostream& os, const Action<ObjectType,ModelType>& action);
}

namespace util {

struct PROPTEST_API HasShowImpl
{
    template <typename T, same_as<decltype(show(cout, declval<T>()))> CRITERIA>
    static true_type test(const T&);

    static false_type test(...);
};

template <typename T>
struct HasShow
{
    static constexpr bool value = decltype(HasShowImpl::test(declval<T>()))::value;
};

// default printer
template <typename T>
struct ShowDefault;
template <typename T>
struct ShowDefault
{
    static ostream& show(ostream& os, const T&)
    {
        os << "<\?\?\?>";
        return os;
    }
};

}  // namespace util

template <typename T>
ostream& show(ostream& os, const T& obj)
{
    util::ShowDefault<T>::show(os, obj);
    return os;
}

template <typename T>
ostream& show(ostream& os, const Shrinkable<T>& shrinkable)
{
    show(os, shrinkable.getRef());
    return os;
}

template <typename T, typename U = void>
struct Show
{
    Show(const T& _value) : value(_value) {}
    friend ostream& operator<<(ostream& os, const Show<T>& sh)
    {
        show(os, sh.value);
        return os;
    }
    const T& value;
};

template <>
struct Show<char*>
{
    Show(const char* _value, size_t _n) : value(_value), n(_n) {}
    friend ostream& operator<<(ostream& os, const Show<char*>& sh)
    {
        show(os, sh.value, sh.n);
        return os;
    }
    const char* value;
    size_t n;
};

template <typename U>
struct Show<ShrinkableBase, U>
{
    Show(const ShrinkableBase& _value) : value(_value) {}
    friend ostream& operator<<(ostream& os, const Show<ShrinkableBase, U>& sh)
    {
        if constexpr(is_same_v<U, void>)
            show(os, "<Any:\?\?\?>");
        else
            show(os, sh.value.getAny().template getRef<U>());
        return os;
    }
    const ShrinkableBase& value;
};

template <typename U>
struct Show<Any, U>
{
    Show(const Any& _value) : value(_value) {}
    friend ostream& operator<<(ostream& os, const Show<Any, U>& sh)
    {
        if constexpr(is_same_v<U, void>)
            show(os, "<Any:\?\?\?>");
        else
            show(os, sh.value.template getRef<U>());
        return os;
    }
    const Any& value;
};

namespace util {

template <typename T>
bool toStreamLast(ostream& os, const T& t)
{
    show(os, t);
    return true;
}

template <typename T>
bool toStreamFrontHelper(ostream& os, const T& t)
{
    os << Show<T>(t) << ", ";
    return true;
}

template <typename Tuple, size_t... index>
decltype(auto) toStreamFront(ostream& os, const Tuple& tuple, index_sequence<index...>)
{
    [[maybe_unused]] auto dummy = {toStreamFrontHelper(os, get<index>(tuple))...};
}

template <size_t Size, typename Tuple>
struct ToStreamEach
{
    void get(ostream& os, const Tuple& tuple) { toStreamFront(os, tuple, make_index_sequence<Size>{}); }
};

template <typename Tuple>
struct ToStreamEach<0, Tuple>
{
    void get(ostream&, const Tuple&) {}
};

}  // namespace util

template <typename ARG1, typename ARG2>
ostream& show(ostream& os, const pair<ARG1, ARG2>& pair)
{
    os << "(" << Show<ARG1>(pair.first) << ", " << Show<ARG2>(pair.second) << ")";
    return os;
}

template <typename... ARGS>
ostream& show(ostream& os, const tuple<ARGS...>& tup)
{
    constexpr auto Size = sizeof...(ARGS);
    os << "{ ";
    util::ToStreamEach<Size - 1, tuple<ARGS...>> toStreamEach;
    toStreamEach.get(os, tup);
    util::toStreamLast(os, get<Size - 1>(tup));
    os << " }";
    return os;
}

template <typename T, typename Allocator>
ostream& show(ostream& os, const vector<T, Allocator>& seq)
{
    os << "[ ";
    auto begin = seq.begin();
    if (begin != seq.end()) {
        os << Show<T>(*begin);
        for (auto itr = ++begin; itr != seq.end(); itr++) {
            os << ", " << Show<T>(*itr);
        }
    }
    os << " ]";
    return os;
}

template <typename T, typename Allocator>
ostream& show(ostream& os, const list<T, Allocator>& seq)
{
    os << "[ ";
    auto begin = seq.begin();
    if (begin != seq.end()) {
        os << Show<T>(*begin);
        for (auto itr = ++begin; itr != seq.end(); itr++) {
            os << ", " << Show<T>(*itr);
        }
    }
    os << " ]";
    return os;
}

template <typename T, typename Compare, typename Allocator>
ostream& show(ostream& os, const set<T, Compare, Allocator>& input)
{
    os << "{ ";
    if (input.size() == 1) {
        os << *input.begin();
    } else if (input.size() > 0) {
        os << *input.begin();
        auto second = input.begin();
        second++;
        for (auto itr = second; itr != input.end(); ++itr) {
            os << ", " << *itr;
        }
    }

    os << " }";
    return os;
}

template <typename Key, typename T, typename Compare, typename Allocator>
ostream& show(ostream& os, const map<Key, T, Compare, Allocator>& input)
{
    os << "{ ";
    if (input.size() == 1) {
        os << Show<pair<Key, T>>(*input.begin());
    } else if (input.size() > 0) {
        os << Show<pair<Key, T>>(*input.begin());
        auto second = input.begin();
        second++;
        for (auto itr = second; itr != input.end(); ++itr) {
            os << ", " << Show<pair<Key, T>>(*itr);
        }
    }

    os << " }";
    return os;
}

template <typename T>
ostream& show(ostream& os, const shared_ptr<T>& ptr)
{
    if (static_cast<bool>(ptr))
        os << Show<T>(*ptr);
    else
        os << "(null)";
    return os;
}

template <typename T>
ostream& show(ostream& os, const optional<T>& opt)
{
    if (opt)
        os << Show<T>(*opt);
    else
        os << "(empty)";
    return os;
}

namespace stateful {

template <typename ObjectType, typename ModelType>
ostream& show(ostream& os, const Action<ObjectType,ModelType>& action)
{
    os << action.name;
    return os;
}

}


}  // namespace proptest
