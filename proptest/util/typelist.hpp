#pragma once

namespace proptest {

namespace util {

template <typename... Ts>
struct TypeList {
    template <typename... Prefix>
    using prepend = TypeList<Prefix..., Ts...>;

    template <typename... Suffix>
    using append = TypeList<Ts..., Suffix...>;
};

template <class First, class... Ts>
struct TypeList<First, Ts...>
{
    using tail = TypeList<Ts...>;

    template <typename... Prefix>
    using prepend = TypeList<Prefix..., First, Ts...>;

    template <typename... Suffix>
    using append = TypeList<First, Ts..., Suffix...>;
};

} // namespace util

} // namespace proptest