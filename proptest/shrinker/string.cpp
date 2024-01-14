#include "proptest/shrinker/string.hpp"
#include "proptest/shrinker/integral.hpp"

namespace proptest {

Shrinkable<string> shrinkString(const string& str, size_t minSize) {
    size_t size = str.size();
    auto shrinkRear =
        shrinkIntegral<uint64_t>(size - minSize).map<string>([str, minSize](const uint64_t& size) {
            return str.substr(0, size + minSize);
        });

    // shrink front
    return shrinkRear.concat([minSize](const Shrinkable<string>& shr) {
        auto& str = shr.getRef();
        size_t maxSizeCopy = str.size();
        if (str.size() <= minSize+1)
            return Stream<Shrinkable<string>>::empty();
        auto newShrinkable = shrinkIntegral<uint64_t>(maxSizeCopy - minSize)
                                 .map<string>([str, minSize, maxSizeCopy](const uint64_t& value) {
                                     return str.substr(minSize + value, maxSizeCopy - (minSize + value));
                                 });
        return newShrinkable.getShrinks();
    });
}

} // namespace proptest
