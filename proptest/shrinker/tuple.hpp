#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/util/tuple.hpp"
#include "proptest/util/tupleorvector.hpp"

namespace proptest {


namespace util {

template <typename... ARGS>
class TupleShrinker {
    using out_tuple_t = tuple<ARGS...>;
    using tuple_t = tuple<Shrinkable<ARGS>...>;
    using shrinkable_t = Shrinkable<tuple_t>;
    using stream_t = Stream<shrinkable_t>;

    static constexpr auto Size = sizeof...(ARGS);

private:
    template <size_t N>
    static shrinkable_t ConcatHelper(const shrinkable_t& aggr)
    {
        if constexpr (N >= sizeof...(ARGS))
            return aggr;
        else
            return ConcatHelper<N + 1>(aggr.concat(genStream<N>()));
    }

    template <size_t N>
    static Function<stream_t(const shrinkable_t&)> genStream()
    {
        using e_shrinkable_t = typename tuple_element<N, tuple_t>::type;
        using element_t = typename e_shrinkable_t::type;

        return +[](const shrinkable_t& parent) -> stream_t {
            if (Size == 0 || N > Size - 1)
                return stream_t::empty();

            shared_ptr<tuple_t> parentRef = util::make_shared<tuple_t>(parent.getRef());

            e_shrinkable_t& elem = get<N>(*parentRef);
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrinked from 2 is also transformed to [x,x,x,1]
            shrinkable_t tupWithElems = elem.template flatMap<tuple_t>([parentRef](const element_t& val) {
                get<N>(*parentRef) = make_shrinkable<element_t>(val);
                return make_shrinkable<tuple_t>(*parentRef);
            });
            return tupWithElems.shrinks();
        };
    }

public:
    template <typename T>
    struct GetValueFromShrinkable
    {
        static decltype(auto) transform(T&& shr) { return shr.get(); }
    };

    static Shrinkable<out_tuple_t> shrink(const shrinkable_t& shrinkable)
    {
        return ConcatHelper<0>(shrinkable).template flatMap<out_tuple_t>(+[](const tuple_t& tuple) {
            return make_shrinkable<out_tuple_t>(transformHeteroTuple<GetValueFromShrinkable>(util::move(tuple)));
        });
    }
};

} // namespace util

template <typename... ARGS>
Shrinkable<tuple<ARGS...>> shrinkTuple(const Shrinkable<tuple<Shrinkable<ARGS>...>>& shrinkable)
{
    Shrinkable<vector<ShrinkableAny>> vectorAnyShr = shrinkable.map<vector<ShrinkableAny>>(+[](const tuple<Shrinkable<ARGS>...>& tuple) {
        vector<ShrinkableAny> anyVector;
        util::For([&] (auto index_sequence) {
            anyVector.push_back(ShrinkableAny(get<index_sequence.value>(tuple)));
        }, make_index_sequence<sizeof...(ARGS)>{});
        return anyVector;
    });

    constexpr size_t Size = sizeof...(ARGS);

    for(size_t N = 0; N < Size; N++) {
        vectorAnyShr = vectorAnyShr.concat([N](const Shrinkable<vector<ShrinkableAny>>& parent) -> Stream<Shrinkable<vector<ShrinkableAny>>> {
            const ShrinkableAny& elem = parent.getRef()[N];
            // need a mutable clone
            const auto& parentVec = parent.getRef();
            shared_ptr<vector<ShrinkableAny>> parentCopy = util::make_shared<vector<ShrinkableAny>>();
            parentCopy->reserve(parent.getRef().size());
            for(auto& shrAny : parentVec)
                parentCopy->push_back(shrAny.clone());

            // rebuild full vector from an element
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrunk from 2 is also transformed to [x,x,x,1]
            Shrinkable<vector<ShrinkableAny>> vecWithElems = elem.template flatMap<vector<ShrinkableAny>>([N,parentCopy](const Any& val) {
                // create a copy
                (*parentCopy)[N] = make_shrinkable<Any>(val); // replace parent copy with val at tuple position N
                return make_shrinkable<vector<ShrinkableAny>>(*parentCopy);
            });
            return vecWithElems.getShrinks();
        });
    }

    return vectorAnyShr.map<tuple<ARGS...>>([](const vector<ShrinkableAny>& shrAnyVec) {
        vector<Any> anyVec;
        anyVec.reserve(shrAnyVec.size());
        for(auto& shrAny : shrAnyVec)
            anyVec.push_back(shrAny.getAny());
        return util::vectorToTuple<ARGS...>(anyVec);
    });

    // tupleOrVectorShr.
    // return util::TupleShrinker<ARGS...>::shrink(shrinkable);
}

} // namespace proptest
