#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/integral.hpp"

namespace proptest {

template <typename StringLike>
Shrinkable<StringLike> shrinkStringLike(const StringLike& str, size_t minSize, size_t charsize, const vector<int>& bytePositions) {
    auto shrinkRear =
        shrinkIntegral<uint64_t>(charsize - minSize)
            .template map<StringLike,uint64_t>([str, minSize, bytePositions](const uint64_t& _size) -> StringLike {
                if (bytePositions.empty())
                    return StringLike();
                else
                    return StringLike(str.substr(0, bytePositions[_size + minSize]));
            });

    return shrinkRear.concat([minSize = minSize + 1, bytePositions](const Shrinkable<StringLike>& shr) {
        auto& str = shr.template getRef<StringLike>();
        size_t maxSizeCopy = str.charsize();
        if (maxSizeCopy <= minSize)
            return Stream<Shrinkable<StringLike>>::empty();
        auto newShrinkable =
            shrinkIntegral<uint64_t>(maxSizeCopy - minSize)
                .map<StringLike,uint64_t>([str, minSize, maxSizeCopy, bytePositions](const uint64_t& value) {
                    if (bytePositions.empty())
                        return StringLike();
                    else
                        return StringLike(str.substr(bytePositions[minSize + value],
                                                      bytePositions[maxSizeCopy] - bytePositions[minSize + value]));
                });
        return newShrinkable.getShrinks();
    });
}

}
