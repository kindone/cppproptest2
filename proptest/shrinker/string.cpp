#include "proptest/shrinker/string.hpp"
#include "proptest/shrinker/integral.hpp"

namespace proptest {

Shrinkable<string> shrinkString(const string& str, size_t minSize) {
    size_t size = str.size();

    // "abc" -> ["", "a", "ab"]
    auto shrinkRear =
        shrinkIntegral<uint64_t>(size - minSize).map<string>([str, minSize](const uint64_t& theSize) {
            return str.substr(0, theSize + minSize);
        });

    // shrink front
    return shrinkRear.concat([minSize = minSize + 1](const Shrinkable<string>& shr) {
        auto str = shr.getRef();
        size_t maxSizeCopy = str.size();
        if (str.size() <= minSize)
            return Shrinkable<string>::StreamType::empty();
        auto newShrinkable = shrinkIntegral<uint64_t>(maxSizeCopy - minSize)
                                 .map<string>([str, minSize, maxSizeCopy](const uint64_t& value) {
                                     return str.substr(minSize + value, maxSizeCopy - (minSize + value));
                                 });
        return newShrinkable.getShrinks();
    });
}

} // namespace proptest
