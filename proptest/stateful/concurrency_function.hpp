#pragma once

#include "proptest/stateful/stateful_function.hpp"

/**
 * @file concurrency_function.hpp
 * @brief Compatibility include for concurrent stateful testing.
 *
 * The standalone concurrency() API has been removed.  Use:
 *
 *     statefulProperty<T>(initialGen, actionGen).setMaxConcurrency(n)
 *
 * This header remains so older includes continue to resolve while callers move
 * to the unified StatefulProperty API.
 */
