#include "listlike.hpp"
#include "proptest/shrinker/integral.hpp"

namespace proptest {

namespace util {

Stream<Shrinkable<vector<ShrinkableAny>>> VectorShrinker::shrinkBulk(const Shrinkable<vector<ShrinkableAny>>& ancestor, size_t power, size_t offset)
{
    static Function<Stream<Shrinkable<vector<ShrinkableAny>>>(const Shrinkable<vector<ShrinkableAny>>&, size_t, size_t, const Shrinkable<vector<ShrinkableAny>>&, size_t, size_t,
                                shared_ptr<vector<Stream<ShrinkableAny>>>)>
        genStream = +[](const Shrinkable<vector<ShrinkableAny>>& _ancestor, size_t _power, size_t _offset, const Shrinkable<vector<ShrinkableAny>>& parent,
                        size_t frompos, size_t topos, shared_ptr<vector<e_stream_t>> elemStreams) -> Stream<Shrinkable<vector<ShrinkableAny>>> {
        const size_t size = topos - frompos;
        if (size == 0)
            return Stream<Shrinkable<vector<ShrinkableAny>>>::empty();

        if (elemStreams->size() != size)
            throw runtime_error(__FILE__, __LINE__, "element streams size error");

        shared_ptr<vector<e_stream_t>> newElemStreams = util::make_shared<vector<e_stream_t>>();
        newElemStreams->reserve(size);

        vector<ShrinkableAny> newVec = parent.getRef();
        const vector<ShrinkableAny>& ancestorVec = _ancestor.getRef();

        if (newVec.size() != ancestorVec.size())
            throw runtime_error(__FILE__, __LINE__, "list size error: " + to_string(newVec.size()) +
                                " != " + to_string(ancestorVec.size()));

        // shrink each element in frompos~topos, put parent if shrink no longer possible
        bool nothingToDo = true;

        for (size_t i = 0; i < elemStreams->size(); i++) {
            if ((*elemStreams)[i].isEmpty()) {
                newVec[i + frompos] = ancestorVec[i + frompos];
                newElemStreams->push_back(e_stream_t::empty());  // [1] -> []
            } else {
                newVec[i + frompos] = (*elemStreams)[i].getHeadRef<ShrinkableAny>();
                newElemStreams->push_back((*elemStreams)[i].getTail());  // [0,4,6,7] -> [4,6,7]
                nothingToDo = false;
            }
        }
        if (nothingToDo)
            return Stream<Shrinkable<vector<ShrinkableAny>>>::empty();

        auto newShrinkable = make_shrinkable<vector<ShrinkableAny>>(newVec);
        newShrinkable = newShrinkable.with(
            [newShrinkable, _power, _offset]() -> Stream<Shrinkable<vector<ShrinkableAny>>> { return shrinkBulk(newShrinkable, _power, _offset); });
        return Stream<Shrinkable<vector<ShrinkableAny>>>(newShrinkable,
                        [_ancestor, _power, _offset, newShrinkable, frompos, topos, newElemStreams]() -> Stream<Shrinkable<vector<ShrinkableAny>>> {
                            return genStream(_ancestor, _power, _offset, newShrinkable, frompos, topos,
                                                newElemStreams);
                        });
    };

    size_t parentSize = ancestor.getRef().size();
    size_t numSplits = static_cast<size_t>(pow(2, power));
    if (parentSize / numSplits < 1)
        return Stream<Shrinkable<vector<ShrinkableAny>>>::empty();

    if (offset >= numSplits)
        throw runtime_error(__FILE__, __LINE__, "offset should not reach numSplits");

    size_t frompos = parentSize * offset / numSplits;
    size_t topos = parentSize * (offset + 1) / numSplits;

    if (topos < parentSize)
        throw runtime_error(__FILE__, __LINE__, "topos error: " + to_string(topos) + " != " + to_string(parentSize));

    const size_t size = topos - frompos;
    const vector<ShrinkableAny>& parentVec = ancestor.getRef();
    shared_ptr<vector<e_stream_t>> elemStreams = util::make_shared<vector<e_stream_t>>();
    elemStreams->reserve(size);

    bool nothingToDo = true;
    for (size_t i = frompos; i < topos; i++) {
        auto shrinks = parentVec[i].getShrinks();
        elemStreams->push_back(shrinks);
        if (!shrinks.isEmpty())
            nothingToDo = false;
    }

    if (nothingToDo)
        return Stream<Shrinkable<vector<ShrinkableAny>>>::empty();

    return genStream(ancestor, power, offset, ancestor, frompos, topos, elemStreams);
}

VectorShrinker::stream_t VectorShrinker::shrinkElementwise(const VectorShrinker::shrinkable_t& shrinkable, size_t power, size_t offset)
{
    if (shrinkable.getRef().empty())
        return stream_t::empty();

    size_t vecSize = shrinkable.getRef().size();
    size_t numSplits = static_cast<size_t>(pow(2, power));
    if (vecSize / numSplits < 1 || offset >= numSplits)
        return stream_t::empty();
    // entirety
    shrinkable_t newShrinkable = shrinkable.concat([power, offset](const shrinkable_t& shr) -> stream_t {
        size_t _vecSize = shr.getRef().size();
        size_t _numSplits = static_cast<size_t>(pow(2, power));
        if (_vecSize / _numSplits < 1 || offset >= _numSplits)
            return stream_t::empty();
        // cout << "entire: " << power << ", " << offset << endl;
        return VectorShrinker::shrinkBulk(shr, power, offset);
    });

    return newShrinkable.getShrinks();
}

VectorShrinker::shrinkable_t VectorShrinker::shrinkMid(const Shrinkable<vector<ShrinkableAny>>& shr, size_t minSize, size_t frontSize, size_t rearSize) {
    auto shrinkableCont = shr.getRef();
    // remove mid as much as possible
    size_t minRearSize = minSize >= frontSize ? minSize - frontSize : 0;
    size_t maxRearSize = shrinkableCont.size() - frontSize;
    // rear size within [minRearSize, minRearSize]
    auto rangeShrinkable = shrinkIntegral<size_t>(maxRearSize - minRearSize).template map<size_t>([minRearSize](const size_t& s) { return s + minRearSize; });
    return rangeShrinkable.template flatMap<vector<ShrinkableAny>>([shr, frontSize](const size_t& rearSize) {
        // concat front and rear
        auto shrinkableCont = shr.getRef();
        Any cont = Any(vector<ShrinkableAny>(shrinkableCont.begin(), shrinkableCont.begin() + frontSize));
        auto& contRef = cont.getMutableRef<vector<ShrinkableAny>>();
        contRef.insert(contRef.end(), shrinkableCont.begin() + (contRef.size()-rearSize), shrinkableCont.end());
        return Shrinkable<vector<ShrinkableAny>>(cont);
    }).concat([minSize, frontSize, rearSize](const shrinkable_t& parent) -> stream_t {
        size_t parentSize = parent.getRef().size();
        // no further shrinking possible
        if(parentSize <= minSize || parentSize <= frontSize)
            return stream_t::empty();
        return shrinkMid(parent, minSize, frontSize + 1, rearSize).getShrinks();
    });
}

VectorShrinker::shrinkable_t VectorShrinker::shrinkFrontAndThenMid(const Shrinkable<vector<ShrinkableAny>>& shr, size_t minSize, size_t rearSize) {
    const auto& shrinkableCont = shr.getRef();
    // remove front as much as possible
    size_t minFrontSize = minSize >= rearSize ? minSize - rearSize : 0;
    size_t maxFrontSize = shrinkableCont.size() - rearSize;
    // front size within [min,max]
    auto rangeShrinkable = shrinkIntegral<size_t>(maxFrontSize - minFrontSize).template map<size_t>([minFrontSize](const size_t& s) { return s + minFrontSize; });
    return rangeShrinkable.template flatMap<vector<ShrinkableAny>>([shr, maxFrontSize](const size_t& frontSize) {
        const auto& shrinkableCont = shr.getRef();
        // concat front and rear
        Any cont = Any(vector<ShrinkableAny>(shrinkableCont.begin(), shrinkableCont.begin() + frontSize));
        auto& contRef = cont.getMutableRef<vector<ShrinkableAny>>();
        contRef.insert(contRef.end(), shrinkableCont.begin() + maxFrontSize, shrinkableCont.end());
        return Shrinkable<vector<ShrinkableAny>>(cont);
    }).concat([minSize, rearSize](const shrinkable_t& parent) -> stream_t {
        // reduce front [0,size-rearSize-1] as much possible
        size_t parentSize = parent.getRef().size();
        // no further shrinking possible
        if(parentSize <= minSize || parentSize <= rearSize) {
            // try shrinking mid
            if(minSize < parentSize && rearSize + 1 < parentSize)
                return shrinkMid(parent, minSize, 1, rearSize + 1).getShrinks();
            else
                return stream_t::empty();
        }
        // shrink front further by fixing last element in front to rear
        // [1,[2,3,4]]
        // [[1,2,3],4]
        // [[1,2],3,4]
        return shrinkFrontAndThenMid(parent, minSize, rearSize + 1).getShrinks();
    });
}

} // namespace util

Shrinkable<vector<ShrinkableAny>> shrinkMembershipwise(const Shrinkable<vector<ShrinkableAny>>& shr, size_t minSize) {
    return util::VectorShrinker::shrinkFrontAndThenMid(shr, minSize, 0);
}

Shrinkable<vector<ShrinkableAny>> shrinkAnyVector(const Shrinkable<vector<ShrinkableAny>>& shrinkAnyVecShr, size_t minSize, bool elementwise, bool membershipwise) {
    const vector<ShrinkableAny>& shrinkAnyVec = shrinkAnyVecShr.getRef();
    // membershipwise shrinking
    Shrinkable<vector<ShrinkableAny>> shrinkableElemsShr = (membershipwise ? shrinkMembershipwise(shrinkAnyVec, minSize) : shrinkAnyVecShr);

    // elementwise shrinking
    if(elementwise)
        shrinkableElemsShr = shrinkableElemsShr.andThen(+[](const Shrinkable<vector<ShrinkableAny>>& parent) {
            return util::VectorShrinker::shrinkElementwise(parent, 0, 0);
        });

    return shrinkableElemsShr;
}

#ifndef PROPTEST_UNTYPED_SHRINKABLE
namespace typed {
template struct Shrinkable<vector<ShrinkableAny>>;
}
#endif // PROPTEST_UNTYPED_SHRINKABLE

#ifndef PROPTEST_UNTYPED_STREAM
namespace typed {
template struct Stream<Shrinkable<vector<ShrinkableAny>>>;
template struct Stream<ShrinkableAny>;
}
#endif

} // namespace proptest
