#pragma once

#include "proptest/util/any.hpp"

namespace proptest {

template <typename T> struct Shrinkable;
template <typename T> struct Iterator;
template <typename T> struct Stream;

struct ShrinkableLike {
private:
};

template <typename T>
struct ShrinkableLikeImpl {

};

} // namespace proptest