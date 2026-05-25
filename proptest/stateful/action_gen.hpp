#pragma once

#include "proptest/stateful/action.hpp"
#include "proptest/Generator.hpp"

/**
 * @file action_gen.hpp
 * @brief Generator type aliases for stateful and concurrency action pipelines.
 *
 * Kept separate from stateful_function.hpp so that shrink_pipeline.hpp can
 * include these aliases without creating a circular dependency.
 */

namespace proptest {
namespace stateful {

template <typename ObjectType>
using SimpleActionGen = Generator<SimpleAction<ObjectType>>;

template <typename ObjectType, typename ModelType>
using ActionGen = Generator<Action<ObjectType, ModelType>>;

template <typename ObjectType>
using SimpleActionGenFactory = Function<SimpleActionGen<ObjectType>(ObjectType&)>;

template <typename ObjectType, typename ModelType>
using ActionGenFactory = Function<ActionGen<ObjectType, ModelType>(ObjectType&, ModelType&)>;

} // namespace stateful
} // namespace proptest
