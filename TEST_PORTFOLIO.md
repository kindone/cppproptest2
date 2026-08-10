# cppproptest Test Portfolio

## Portfolio Contract

The cppproptest suite protects a C++ property-based testing library. Its stable
contracts are the public API behaviors that let users generate, shrink, replay,
and report counterexamples:

- Generators produce values inside their declared domains.
- Generator combinators preserve constraints through generation and shrinking.
- Shrink trees expose useful and deterministic simplification paths.
- Property execution handles examples, matrices, generated runs, replay,
  assertion macros, shrinking, configuration, and failure reporting.
- Stateful and concurrency properties preserve model/object agreement, callback
  behavior, action generation, trace shrinking, and reproduction output.
- Core utility types preserve public behavior for streams, shrinkables,
  type-erased values, callable wrappers, and tuple/vector bridges.
- Compile-only snippets continue to exercise public template/API forms.

Out of scope for this refresh:

- Renaming or deleting the existing C++ test files.
- Proving exact statistical distributions for every weighted generator.
- Rewriting performance checks into a benchmark harness.
- Forcing compile-only snippets through runtime test fixtures.

## Current Coverage Shape

| Area | Files | Current role |
|---|---|---|
| Generator API contracts | `test_gen.cpp`, `test_generator.cpp`, `test_arbitrary.cpp` | primitive, container, arbitrary, construction, and namespace-level generator behavior |
| Combinator contracts | `test_combinator.cpp`, `test_combinator2.cpp`, `test_container_variants.cpp` | `just`, `oneOf`, `elementOf`, `filter`, transform, derive, chain, aggregate, accumulate, recursive, and container-variant behavior |
| Boundary and shrink-domain coverage | `test_floating.cpp`, `test_flatmap_edge_cases.cpp`, `test_shrinker*.cpp`, `test_shrinkable.cpp` | float frontiers, flatMap replay/regeneration, exact shrink axes, and shrinkable composition |
| Property API and reporting | `test_property.cpp`, `test_shrink_retry.cpp` | `.forAll`, `.example`, `.matrix`, assertion macros, config paths, replay seeds, duration guards, and shrink retry behavior |
| Stateful and concurrency workflows | `test_stateful_function.cpp`, `test_concurrency_function.cpp` | model/object agreement, action generation, callback lifecycle, prefix/last-action shrinking, retry and concurrency traces |
| Core utilities | `test_any*.cpp`, `test_function*.cpp`, `test_stream.cpp`, `test_tupleorvector.cpp`, `test_typelist.cpp`, `test_random.cpp`, `test_printing.cpp`, `test_invoke.cpp`, `test_lazy.cpp` | internal public-support contracts and compatibility surfaces |
| Performance guardrails | `test_performance.cpp` plus performance-named cases in utility files | bounded construction, stream, shrinkable, generator, and type-erasure performance smoke |
| Compile/API snippets | `proptest/test/compile` and `proptest/test/compile_fail` | public syntax smoke and expected compile-failure contracts |

## Refresh Log

- Added `proptest/test/test_profiles.hpp` as the shared profile for replay
  seeds, sample counts, run counts, action counts, shrink retry limits, timeout
  budgets, and performance guardrail sizes.
- Replaced runtime wall-clock seeds with named replay seeds so failures are
  reproducible by default.
- Replaced repeated sample, run-count, retry, timeout, and performance literals
  with named profile constants.
- Named performance-suite budgets separately from correctness profiles, so
  benchmark-like tests keep their own vocabulary.
- Kept file-local constants where the number describes one test's generated
  domain shape rather than an execution budget.
- Left compile snippets as a separate track because their value is partly that
  they compile with minimal context.

## Role Taxonomy

Use role names when adding or splitting tests:

- `contracts`: stable public laws and core API guarantees.
- `regressions`: named bugs, replay seeds, shrink paths, or counterexamples.
- `compatibility`: exact public behavior, including output formatting and
  compile-time API surfaces.
- `confidence`: statistical or sampling checks.
- `illustrations`: readable API witnesses and documentation-like examples.
- `robustness`: invalid options, clean rejection, and failure boundaries.
- `frontier`: bounded exploration of under-sampled generated regions.
- `performance`: smoke/guardrail checks that protect accidental algorithmic
  regressions without replacing a benchmark harness.

Avoid naming new files after migration history. Prefer names that identify the
behavior boundary, such as `property_runner_contracts`, `flatmap_replay`, or
`shrink_tree_compatibility`.

## Constants and Execution Profiles

- Semantic constants name contract boundaries, generated domains, and known
  replay fixtures. They should not scale with environment variables.
- Execution constants name exploration budgets: run counts, sample counts,
  trace/action lengths, retry limits, timeout budgets, and performance smoke
  iteration counts.
- `test_profiles.hpp` is the shared place for repeated runtime execution
  profiles and replay fixtures.
- File-local constants are acceptable when they describe one test's domain
  shape, such as a flatMap search target or a specific compatibility fixture.
- Compile snippets may keep local literal seeds when adding a shared fixture
  would obscure the API form being compiled.

## Subsumption Criteria

A future cleanup can retire or rename existing C++ tests only when the target
portfolio records all of the following:

- The same public contract is named in a portfolio role.
- The replacement generator domain is at least as broad, or the old case is
  preserved as a compatibility/regression fixture.
- Shrink behavior, if relevant, is asserted directly instead of inferred from
  successful execution.
- Stateful/concurrency failures preserve replayable traces.
- Compile-only behavior remains protected by either runtime tests plus compile
  snippets, or by an explicit compile-fail/compile-pass fixture.
- Focused tests, full runtime tests, compile snippets, and diff hygiene pass.

## Cross-Port Comparison Notes

Compared with the JS and Python refreshes:

- JS currently has the clearest role-named file layout and retirement
  inventory.
- Python currently has the broadest API/integration split and the richest
  shared profile helper.
- C++ currently keeps the original file layout but now has runtime profile
  normalization and a portfolio document that records the role decisions.
- C++ has compile-only and compile-fail fixtures that do not map directly to
  JS/Python. Treat those as a separate compatibility layer.
- C++ has explicit concurrency-property coverage that should remain a
  first-class role rather than being folded into generic stateful tests.

## Intentional Deferrals

- Existing C++ test files are not renamed or deleted in this pass. Their current
  names still reflect historical organization, but renaming would be noisy and
  should happen only after a focused compatibility/subsumption review.
- `proptest/test/compile` still uses local literal seeds in minimal snippets.
  Normalize them later only if the compile fixtures grow a small support header
  that does not hide the API surface under test.
- The existing build directory reports no CTest tests; direct execution of
  `build/test_proptest` remains the current verification source.
- Performance tests are still smoke/guardrail tests, not calibrated benchmarks.

## Verification From This Pass

- `cmake --build build --target test_proptest -j2` passed.
- `./build/test_proptest --gtest_brief=1` passed with 512 tests and 5 disabled
  tests.
- Focused performance verification passed for
  `Performance.*:ShrinkerPerformance.*`.
- `git diff --check` passed after the profile normalization.
