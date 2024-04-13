#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/algorithm.hpp"

namespace proptest {

namespace util {

struct PROPTEST_API VectorShrinker
{
    using elem_t = ShrinkableBase;
    using vector_t = vector<elem_t>;

    using shrinkable_t = Shrinkable<vector_t>;
    using stream_t = shrinkable_t::StreamType;
    using stream_element_t = shrinkable_t::StreamElementType;

    using elem_stream_t = elem_t::StreamType;
    using elem_stream_element_t = elem_t::StreamElementType;

    static stream_t shrinkBulk(const shrinkable_t& ancestor, size_t power, size_t offset);

    static stream_t shrinkElementwise(const shrinkable_t& shrinkable, size_t power, size_t offset);

    static shrinkable_t shrinkMid(const shrinkable_t& shrinkableCont, size_t minSize, size_t frontSize, size_t rearSize);

    static shrinkable_t shrinkFrontAndThenMid(const shrinkable_t& shrinkableCont, size_t minSize, size_t rearSize);
};

}  // namespace util

//extern template struct PROPTEST_API Shrinkable<vector<Shrinkable<Any>>>;

PROPTEST_API Shrinkable<vector<ShrinkableBase>> shrinkMembershipwise(const Shrinkable<vector<ShrinkableBase>>& shr, size_t minSize);

PROPTEST_API Shrinkable<vector<ShrinkableBase>> shrinkAnyVector(const Shrinkable<vector<ShrinkableBase>>& shrinkAnyVecShr, size_t minSize, bool elementwise, bool membershipwise);

template <template <typename...> class Container, typename T>
Shrinkable<vector<ShrinkableBase>> toVectorShrinkableBase(const Container<Shrinkable<T>>& shrinkableCont)
{
    Shrinkable<vector<ShrinkableBase>> shrinkBaseVecShr = make_shrinkable<vector<ShrinkableBase>>();
    vector<ShrinkableBase>& shrinkBaseVec = shrinkBaseVecShr.getMutableRef();
    shrinkBaseVec.reserve(shrinkableCont.size());
    util::transform(shrinkableCont.begin(), shrinkableCont.end(), util::inserter(shrinkBaseVec, shrinkBaseVec.begin()), +[](const Shrinkable<T>& shr) -> ShrinkableBase {
        return shr;
    });
    return shrinkBaseVecShr;
}

template <template <typename...> class Container, typename T>
Shrinkable<Container<T>> toContainerTShrinkable(const Shrinkable<vector<ShrinkableBase>>& shrinkableBaseVecShr)
{
    return shrinkableBaseVecShr.template flatMap<Container<T>>(
        +[](const vector<ShrinkableBase>& _shrinkableVector) {
            auto valueContPtr = util::make_shared<Container<T>>();
            util::transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), util::inserter(*valueContPtr, valueContPtr->begin()),
                +[](const ShrinkableBase& shr) -> T { return shr.getAny().getRef<T>(); });
            return Shrinkable<Container<T>>(util::make_any<Container<T>>(valueContPtr));
        });
}

template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<T>> toListLikeTShrinkable(const Shrinkable<vector<ShrinkableBase>>& shrinkableBaseVecShr)
{
    return shrinkableBaseVecShr.template flatMap<ListLike<T>>(
        +[](const vector<ShrinkableBase>& _shrinkableVector) {
            auto valueContPtr = util::make_shared<ListLike<T>>();
            util::transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), util::back_inserter(*valueContPtr),
                +[](const ShrinkableBase& shr) -> T { return shr.getAny().getRef<T>(); });
            return Shrinkable<ListLike<T>>(util::make_any<ListLike<T>>(valueContPtr));
        });
}

/**
 * @brief Shrinking of a container (such as a set) using membership-wise shrinking
 *
 *  * Membership-wise shrinking searches through inclusion or exclusion of elements in the container
 * @tparam Container A container such as vector or set
 * @tparam T Contained type
 * @param shrinkableCont container of Shrinkable<T>
 * @param minSize minimum size a shrunk list can be
 * @param elementwise whether to enable element-wise shrinking. If false, only membership-wise shrinking is performed
 * @return Shrinkable<ListLike<T>>
 */
template <template <typename...> class Container, typename T>
Shrinkable<Container<T>> shrinkContainer(const Shrinkable<Container<Shrinkable<T>>>& shr, size_t minSize, bool elementwise = true, bool membershipwise = true)
{
    auto shrinkableCont = shr.getRef();
    // change type to any
    Shrinkable<vector<ShrinkableBase>> shrinkVecShrBase = toVectorShrinkableBase(shrinkableCont);
    // membershipwise shrinking
    shrinkVecShrBase = shrinkAnyVector(shrinkVecShrBase, minSize, elementwise, membershipwise);

    // transform to proper output type
    return toContainerTShrinkable<Container, T>(shrinkVecShrBase);
}


/**
 * @brief Shrinking of list-like container using membership-wise and element-wise shrinking
 *
 *  * Membership-wise shrinking searches through inclusion or exclusion of elements in the container
 *  * Element-wise shrinking tries to shrink the elements themselves (e.g. shrink integer elements in vector<int>)
 * @tparam ListLike A list-like container such as vector or list
 * @tparam T Contained type
 * @param shrinkableVector vector of Shrinkable<T>
 * @param minSize minimum size a shrunk list can be
 * @param elementwise whether to enable element-wise shrinking.
 * @param membershipwise whether to enable membership-wise shrinking.
 * @return Shrinkable<ListLike<T>>
 */
template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<T>> shrinkListLike(const Shrinkable<vector<ShrinkableBase>>& shrinkAnyVecShr, size_t minSize, bool elementwise = true, bool membershipwise = true)
{
    // membershipwise shrinking
    Shrinkable<vector<ShrinkableBase>> shrinkableElemsShr = shrinkAnyVector(shrinkAnyVecShr, minSize, elementwise, membershipwise);

    // transform to proper output type
    return toListLikeTShrinkable<ListLike, T>(shrinkableElemsShr);
}

/**
 * @brief Simple shrinking of list-like containers into sublists within minSize and the given list size
 *
 * @param shrinkableElems Shrinkable<T>
 * @param minSize minimum size the shrunk container can be
 * @return Shrinkable<ListLike<T>>
 */
template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<Shrinkable<T>>> shrinkListLikeLength(const Shrinkable<ListLike<Shrinkable<T>>>& shr,
                                                         size_t minSize)
{
    auto shrinkableElems = shr.getRef();
    auto size = shrinkableElems.size();
    auto rangeShrinkable =
        shrinkIntegral<size_t>(size - minSize).template map<size_t>([minSize](const size_t& s) { return s + minSize; });
    return rangeShrinkable.template flatMap<ListLike<Shrinkable<T>>>([shr](const size_t& newSize) {
        if (newSize == 0)
            return Shrinkable<ListLike<Shrinkable<T>>>(ListLike<Shrinkable<T>>());
        else {
            const auto& shrinkableElems = shr.getRef();
            return Shrinkable<ListLike<Shrinkable<T>>>(
                util::make_any<ListLike<Shrinkable<T>>>(shrinkableElems.begin(), shrinkableElems.begin() + newSize));
        }
    });
}

PROPTEST_API Shrinkable<vector<ShrinkableBase>> shrinkVectorLength(const Shrinkable<vector<ShrinkableBase>>& shr,
size_t minSize);

}  // namespace proptest
