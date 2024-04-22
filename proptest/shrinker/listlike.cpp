#include "proptest/shrinker/listlike.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/std/math.hpp"

namespace proptest {

namespace util {

struct VectorShrinker
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

    static shrinkable_t shrinkFrontAndThenMid(const shrinkable_t& shrinkableCont, size_t minSize, size_t rearSize);
};


VectorShrinker::stream_t VectorShrinker::shrinkBulk(const VectorShrinker::shrinkable_t& ancestor, size_t power, size_t offset)
{

    // static_assert(is_same_v<elem_stream_element_t, stream_element_t>, "elem_stream_element_t should be stream_element_t");

    static Function<stream_t(const shrinkable_t&, size_t, size_t, const shrinkable_t&, size_t, size_t,
                                shared_ptr<vector<elem_stream_t>>)>
        genStream = +[](const shrinkable_t& _ancestor, size_t _power, size_t _offset, const shrinkable_t& parent,
                        size_t frompos, size_t topos, shared_ptr<vector<elem_stream_t>> elemStreams) -> stream_t {
        const size_t size = topos - frompos;
        if (size == 0)
            return stream_t::empty();

        if (elemStreams->size() != size)
            throw runtime_error(__FILE__, __LINE__, "element streams size error");

        shared_ptr<vector<elem_stream_t>> newElemStreams = util::make_shared<vector<elem_stream_t>>();
        newElemStreams->reserve(size);

        vector_t newVec = parent.getRef();
        const vector_t& ancestorVec = _ancestor.getRef();

        if (newVec.size() != ancestorVec.size())
            throw runtime_error(__FILE__, __LINE__, "list size error: " + to_string(newVec.size()) +
                                " != " + to_string(ancestorVec.size()));

        // shrink each element in frompos~topos, put parent if shrink no longer possible
        bool nothingToDo = true;

        for (size_t i = 0; i < elemStreams->size(); i++) {
            if ((*elemStreams)[i].isEmpty()) {
                newVec[i + frompos] = ancestorVec[i + frompos];
                newElemStreams->push_back(elem_stream_t::empty());  // [1] -> []
            } else {
                newVec[i + frompos] = (*elemStreams)[i].getHeadRef<elem_stream_element_t>();
                newElemStreams->push_back((*elemStreams)[i].getTail());  // [0,4,6,7] -> [4,6,7]
                nothingToDo = false;
            }
        }
        if (nothingToDo)
            return stream_t::empty();

        shrinkable_t newShrinkable = make_shrinkable<vector_t>(newVec);
        newShrinkable = newShrinkable.with(
            [newShrinkable, _power, _offset]() -> stream_t { return shrinkBulk(newShrinkable, _power, _offset); });
        return stream_t(stream_element_t(newShrinkable),
                        [_ancestor, _power, _offset, newShrinkable, frompos, topos, newElemStreams]() -> stream_t {
                            return genStream(_ancestor, _power, _offset, newShrinkable, frompos, topos,
                                                newElemStreams);
                        });
    };

    size_t parentSize = ancestor.getRef().size();
    size_t numSplits = static_cast<size_t>(pow(2, power));
    if (parentSize / numSplits < 1)
        return stream_t::empty();

    if (offset >= numSplits)
        throw runtime_error(__FILE__, __LINE__, "offset should not reach numSplits");

    size_t frompos = parentSize * offset / numSplits;
    size_t topos = parentSize * (offset + 1) / numSplits;

    if (topos < parentSize)
        throw runtime_error(__FILE__, __LINE__, "topos error: " + to_string(topos) + " != " + to_string(parentSize));

    const size_t size = topos - frompos;
    const vector_t& parentVec = ancestor.getRef();
    shared_ptr<vector<elem_stream_t>> elemStreams = util::make_shared<vector<elem_stream_t>>();
    elemStreams->reserve(size);

    bool nothingToDo = true;
    for (size_t i = frompos; i < topos; i++) {
        elem_stream_t shrinks = parentVec[i].getShrinks();
        elemStreams->push_back(shrinks);
        if (!shrinks.isEmpty())
            nothingToDo = false;
    }

    if (nothingToDo)
        return stream_t::empty();

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
    shrinkable_t newShrinkable = shrinkable.concat([power, offset](const stream_element_t& elem) -> stream_t {
        shrinkable_t shr = elem;
        size_t _vecSize = shr.getRef().size();
        size_t _numSplits = static_cast<size_t>(pow(2, power));
        if (_vecSize / _numSplits < 1 || offset >= _numSplits)
            return stream_t::empty();
        // cout << "entire: " << power << ", " << offset << endl;
        return VectorShrinker::shrinkBulk(shr, power, offset);
    });

    return newShrinkable.getShrinks();
}

VectorShrinker::shrinkable_t VectorShrinker::shrinkFrontAndThenMid(const Shrinkable<vector<ShrinkableBase>>& shr, size_t minSize, size_t rearSize) {
    // remove front as much as possible while keeping rear intact
    const auto& shrinkableCont = shr.getRef();
    size_t minFrontSize = minSize >= rearSize ? minSize - rearSize : 0;
    size_t maxFrontSize = shrinkableCont.size() - rearSize; // == rear part size
    // front size within [minFrontSize,maxFrontSize]
    ShrinkableBase rangeShrinkable = shrinkIntegral<size_t>(maxFrontSize - minFrontSize).template map<size_t>([minFrontSize](const size_t& s) { return s + minFrontSize; });
    return rangeShrinkable.flatMap([shr, maxFrontSize](const Any& frontSizeAny) -> ShrinkableBase {
        const size_t& frontSize = frontSizeAny.getRef<size_t>();
        const auto& shrinkableCont = shr.getRef();
        // concat front and rear parts
        auto contPtr = util::make_unique<vector<ShrinkableBase>>(shrinkableCont.begin(), shrinkableCont.begin() + frontSize);
        contPtr->insert(contPtr->end(), shrinkableCont.begin() + maxFrontSize, shrinkableCont.end());
        return ShrinkableBase(util::make_any<vector<ShrinkableBase>>(util::move(contPtr)));
    }).concat([minSize, rearSize](const shrinkable_t& parent) -> stream_t {
        // reduce front [0,size-rearSize-1] as much possible
        size_t parentSize = parent.getRef().size();
        // no further shrinking possible
        if(parentSize <= minSize || parentSize <= rearSize) {
            return stream_t::empty();
        }
        // shrink front further by fixing one more element to rear
        // [1,[2,3,4]]
        // [[1,2,3],4]
        // [[1,2],3,4]
        return shrinkFrontAndThenMid(parent, minSize, rearSize + 1).getShrinks();
    });
}

} // namespace util

Shrinkable<vector<ShrinkableBase>> shrinkMembershipwise(const Shrinkable<vector<ShrinkableBase>>& shr, size_t minSize) {
    return util::VectorShrinker::shrinkFrontAndThenMid(shr, minSize, 0);
}

Shrinkable<vector<ShrinkableBase>> shrinkAnyVector(const Shrinkable<vector<ShrinkableBase>>& shrinkAnyVecShr, size_t minSize, bool elementwise, bool membershipwise) {
    // membershipwise shrinking
    Shrinkable<vector<ShrinkableBase>> shrinkableElemsShr = (membershipwise ? shrinkMembershipwise(shrinkAnyVecShr, minSize) : shrinkAnyVecShr);

    // elementwise shrinking
    if(elementwise)
        shrinkableElemsShr = shrinkableElemsShr.andThen(+[](const ShrinkableBase& parent) -> Shrinkable<vector<ShrinkableBase>>::StreamType {
            Shrinkable<vector<ShrinkableBase>> shr = parent;
            return util::VectorShrinker::shrinkElementwise(shr, 0, 0);
        });

    return shrinkableElemsShr;
}

Shrinkable<vector<ShrinkableBase>> shrinkVectorLength(const Shrinkable<vector<ShrinkableBase>>& shr,
                                                         size_t minSize)
{
    auto shrinkableElems = shr.getRef();
    auto size = shrinkableElems.size();
    auto rangeShrinkable =
        shrinkIntegral<size_t>(size - minSize).template map<size_t>([minSize](const size_t& s) { return s + minSize; });
    return rangeShrinkable.template flatMap<vector<ShrinkableBase>>([shr](const size_t& newSize) {
        if (newSize == 0)
            return Shrinkable<vector<ShrinkableBase>>(vector<ShrinkableBase>());
        else {
            const auto& shrinkableElems = shr.getRef();
            return Shrinkable<vector<ShrinkableBase>>(
                util::make_any<vector<ShrinkableBase>>(shrinkableElems.begin(), shrinkableElems.begin() + newSize));
        }
    });
}

} // namespace proptest
