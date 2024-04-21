#pragma once
#include "proptest/api.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/algorithm.hpp"

namespace proptest {

PROPTEST_API Shrinkable<vector<ShrinkableBase>> shrinkAnyVector(const Shrinkable<vector<ShrinkableBase>>& shrinkAnyVecShr, size_t minSize, bool elementwise, bool membershipwise);

template <template <typename...> class Container, typename T>
Shrinkable<vector<ShrinkableBase>> toVectorShrinkableBase(const Container<Shrinkable<T>>& shrinkableCont)
{
    Shrinkable<vector<ShrinkableBase>> shrinkBaseVecShr = make_shrinkable<vector<ShrinkableBase>>();
    vector<ShrinkableBase>& shrinkBaseVec = shrinkBaseVecShr.getMutableRef();
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
            auto valueContPtr = util::make_unique<Container<T>>();
            util::transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), util::inserter(*valueContPtr, valueContPtr->begin()),
                +[](const ShrinkableBase& shr) -> T { return shr.getAny().getRef<T>(); });
            return Shrinkable<Container<T>>(util::make_any<Container<T>>(util::move(valueContPtr)));
        });
}

template <template <typename...> class ListLike, typename T>
Shrinkable<ListLike<T>> toListLikeTShrinkable(const Shrinkable<vector<ShrinkableBase>>& shrinkableBaseVecShr)
{
    return shrinkableBaseVecShr.template flatMap<ListLike<T>>(
        +[](const vector<ShrinkableBase>& _shrinkableVector) {
            auto valueContPtr = util::make_unique<ListLike<T>>();
            util::transform(
                _shrinkableVector.begin(), _shrinkableVector.end(), util::back_inserter(*valueContPtr),
                +[](const ShrinkableBase& shr) -> T { return shr.getAny().getRef<T>(); });
            return Shrinkable<ListLike<T>>(util::make_any<ListLike<T>>(util::move(valueContPtr)));
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
Shrinkable<Container<T>> shrinkContainer(const shared_ptr<Container<Shrinkable<T>>>& shr, size_t minSize, bool elementwise = true, bool membershipwise = true)
{
    auto& shrinkableCont = *shr;
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
