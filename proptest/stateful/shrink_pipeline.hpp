#pragma once

#include "proptest/stateful/action_gen.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Stream.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/list.hpp"

/**
 * @file shrink_pipeline.hpp
 * @brief Shared shrink pipeline for stateful and concurrency testing.
 *
 * Provides two free function templates consumed by StatefulProperty:
 *
 *   genActionShrinkables   — generate a per-action bookmarked shrinkable vector
 *   applyStatefulShrinkTree — wrap that vector with SequencePruning / InitialStateShrink /
 *                             PrefixParams / LastActionParams and extract list<Action>
 */

namespace proptest {
namespace stateful {

/**
 * Generate a Shrinkable<vector<ShrinkableBase>> where each element is a
 * Shrinkable<pair<Random,Action>> stored as ShrinkableBase.
 *
 * For each slot:
 *   1. Snapshot Random as bookmark before calling the factory.
 *   2. Call factory(obj, model) to get the generator appropriate for the current state.
 *   3. Generate the action (advances rand).
 *   4. Map action → pair<bookmark, action> so every shrunk variant carries the same bookmark.
 *   5. Execute the action to advance (obj, model) for the next factory call.
 *      If execution throws, stop early — the state beyond that point is undefined.
 */
template <typename ObjectType, typename ModelType>
Shrinkable<vector<ShrinkableBase>> genActionShrinkables(
    Random& rand, ObjectType& obj, ModelType& model,
    const ActionGenFactory<ObjectType, ModelType>& factory, size_t numActions)
{
    using ActionType = Action<ObjectType, ModelType>;
    using PairType = pair<Random, ActionType>;

    auto actionShrinkables = make_shrinkable<vector<ShrinkableBase>>();
    auto& vec = actionShrinkables.getMutableRef();
    vec.reserve(numActions);
    for (size_t i = 0; i < numActions; ++i) {
        Random bookmark = rand;                  // snapshot before generation
        auto nextActionGen = factory(obj, model);
        auto actionShr = nextActionGen(rand);    // advances rand
        // Preserve the full shrink tree: every shrunken action keeps the same bookmark.
        auto pairShr = actionShr.template map<PairType>(
            [bookmark](const ActionType& action) -> PairType {
                return PairType(bookmark, action);
            });
        vec.push_back(ShrinkableBase(pairShr));
        try {
            actionShr.getRef()(obj, model);      // execute to advance state
        } catch (...) {
            break;  // state invalid beyond this point; stop generating
        }
    }
    return actionShrinkables;
}

/**
 * Wrap a Shrinkable<vector<ShrinkableBase>> (elements: Shrinkable<pair<Random,Action>>)
 * with four shrink phases, then extract the final Shrinkable<list<Action>>:
 *
 *   SequencePruning   — prefix-length-first (shorter sequences tried before element simplification)
 *   InitialStateShrink — element-wise via stored pair shrink trees (fast, may be stale after SequencePruning)
 *   PrefixParams      — state-aware bookmark shrinking: for each non-last position replay the prefix,
 *                       call the factory with the reconstructed state, regenerate from the bookmark,
 *                       and yield shrinks of that fresh generation
 *   LastActionParams  — last-action parameter shrinking via the last element's stored shrink tree
 *
 * @param initial       Initial object state (used to replay prefix in Phase 2b).
 * @param modelFactory  Factory returning a fresh ModelType from an ObjectType.
 *                      For rear threads in concurrency, pass a lambda returning the
 *                      pre-computed post-front model regardless of obj.
 * @param initialFactory    Nullary factory producing a fresh initial ObjectType on each call.
 *                          Using a factory instead of a stored value ensures that every
 *                          shrink-candidate replay starts from a clean initial state even
 *                          when ObjectType is not copy-constructible (e.g. shared_ptr<T>
 *                          workaround where copies alias the same underlying object).
 * @param actionGenFactory  State-dependent action generator factory.
 */
template <typename ObjectType, typename ModelType>
Shrinkable<list<Action<ObjectType, ModelType>>> applyStatefulShrinkTree(
    Shrinkable<vector<ShrinkableBase>> actionShrinkables,
    size_t minSize,
    const Function<ObjectType()>& initialFactory,
    const Function<ModelType(const ObjectType&)>& modelFactory,
    const ActionGenFactory<ObjectType, ModelType>& actionGenFactory)
{
    using ActionType = Action<ObjectType, ModelType>;
    using PairType = pair<Random, ActionType>;

    // SequencePruning: prefix-length-first shrinking
    auto withPhase1 = shrinkVectorLength(actionShrinkables, minSize);

    // InitialStateShrink: element-wise shrinking via stored pair shrink trees
    auto withPhase2 = shrinkAnyVector(withPhase1, minSize, true, false);

    // PrefixParams: state-aware bookmark-based shrinking for each non-last position.
    // Replays prefix [0,i) to reconstruct live state at position i, regenerates
    // the action from its stored bookmark, yields shrinks of that fresh tree.
    // Each replay calls initialFactory() for a fresh initial state — this is the
    // key fix for non-copyable ObjectType: no aliasing across replays.
    auto withPhase2b = withPhase2.concat(
        [initialFactory, modelFactory, actionGenFactory, minSize](ShrinkableBase& nodeShr) -> ShrinkableBase::StreamType {
            const auto& vec = nodeShr.getRef<vector<ShrinkableBase>>();
            if (vec.size() <= 1)
                return ShrinkableBase::StreamType::empty();

            Stream result = Stream::empty();
            for (size_t i = 0; i + 1 < vec.size(); i++) {
                ObjectType simObj = initialFactory();  // fresh instance per slot replay
                ModelType simModel = modelFactory(simObj);
                bool replayFailed = false;
                for (size_t j = 0; j < i && !replayFailed; j++) {
                    try {
                        vec[j].getAny().getRef<PairType>().second(simObj, simModel);
                    } catch (...) {
                        replayFailed = true;
                    }
                }
                if (replayFailed)
                    break;

                const PairType& pairI = vec[i].getAny().getRef<PairType>();
                Random bookmark = pairI.first;
                Random randForGen = pairI.first;
                auto freshActionShr = actionGenFactory(simObj, simModel)(randForGen);

                auto candidates = freshActionShr.getShrinks()
                    .template transform<ShrinkableBase, ShrinkableBase>(
                        [vec, i, minSize, bookmark](const ShrinkableBase& shrunkAction) -> ShrinkableBase {
                            Shrinkable<ActionType> shrunkActionShr(shrunkAction);
                            auto newPairShr = shrunkActionShr.template map<PairType>(
                                [bookmark](const ActionType& action) -> PairType {
                                    return PairType(bookmark, action);
                                });
                            auto newVec = vec;
                            newVec[i] = ShrinkableBase(newPairShr);
                            auto newVecShr = make_shrinkable<vector<ShrinkableBase>>(newVec);
                            auto newLengthShr = shrinkVectorLength(newVecShr, minSize);
                            return ShrinkableBase(shrinkAnyVector(newLengthShr, minSize, true, false));
                        });

                result = result.concat(candidates);
            }
            return result;
        });

    // LastActionParams: parameter shrinking via the last element's stored shrink tree
    auto withPhase3 = withPhase2b.concat(
        [minSize](ShrinkableBase& nodeShr) -> ShrinkableBase::StreamType {
            const auto& vec = nodeShr.getRef<vector<ShrinkableBase>>();
            if (vec.empty())
                return ShrinkableBase::StreamType::empty();
            return vec.back().getShrinks().template transform<ShrinkableBase, ShrinkableBase>(
                [vec, minSize](const ShrinkableBase& shrunkLast) -> ShrinkableBase {
                    auto newVec = vec;
                    newVec.back() = shrunkLast;
                    auto newVecShr = make_shrinkable<vector<ShrinkableBase>>(newVec);
                    auto newLengthShr = shrinkVectorLength(newVecShr, minSize);
                    return ShrinkableBase(shrinkAnyVector(newLengthShr, minSize, true, false));
                });
        });

    // Final extraction: pull the action out of each pair<Random,Action> element
    return withPhase3.template flatMap<list<ActionType>>(
        +[](const vector<ShrinkableBase>& vec) {
            auto resultPtr = util::make_unique<list<ActionType>>();
            for (const auto& shr : vec)
                resultPtr->push_back(shr.getAny().getRef<PairType>().second);
            return Shrinkable<list<ActionType>>(
                util::make_any<list<ActionType>>(util::move(resultPtr)));
        });
}

/**
 * Backward-compatible overload: wraps a stored initial value in a factory.
 *
 * Used for the concurrent rear-thread path where the "initial" state is
 * the post-front state (a specific, already-advanced ObjectType value).
 * For copyable types this produces the same behaviour as before.
 * For shared_ptr workarounds the alias issue in concurrent tests is a
 * separate concern.
 */
template <typename ObjectType, typename ModelType>
Shrinkable<list<Action<ObjectType, ModelType>>> applyStatefulShrinkTree(
    Shrinkable<vector<ShrinkableBase>> actionShrinkables,
    size_t minSize,
    const ObjectType& initial,
    const Function<ModelType(const ObjectType&)>& modelFactory,
    const ActionGenFactory<ObjectType, ModelType>& actionGenFactory)
{
    return applyStatefulShrinkTree<ObjectType, ModelType>(
        actionShrinkables, minSize,
        [initial]() -> ObjectType { return initial; },
        modelFactory, actionGenFactory);
}

} // namespace stateful
} // namespace proptest
