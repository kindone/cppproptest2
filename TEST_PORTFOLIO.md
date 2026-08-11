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
| Generator API contracts | `test_gen.cpp`, `test_generator.cpp`, `test_arbitrary.cpp`, `test_dependent_generator_contracts.cpp` | primitive, container, arbitrary, construction, replay, dependent-domain preservation, and namespace-level generator behavior |
| Combinator contracts | `test_accumulating_combinator_contracts.cpp`, `test_combinator.cpp`, `test_combinator2.cpp`, `test_container_variants.cpp`, `test_dependent_generator_contracts.cpp` | `just`, `oneOf`, `elementOf`, `filter`, transform, derive, chain, aggregate, accumulate, recursive, dependent combinators, and container-variant behavior |
| Boundary and shrink-domain coverage | `test_floating.cpp`, `test_flatmap_edge_cases.cpp`, `test_shrink_tree_contracts.cpp`, `test_shrinker*.cpp`, `test_shrinkable.cpp` | float frontiers, flatMap replay/regeneration, structural shrink-tree invariants, exact shrink axes, and shrinkable composition |
| Property API and reporting | `test_property.cpp`, `test_property_api_composition.cpp`, `test_property_runner_reporting.cpp`, `test_shrink_retry.cpp` | `.forAll`, `.example`, `.matrix`, assertion macros, chain composition, config paths, replay seeds, configured streams, statistical assertion reporting, duration guards, and shrink retry behavior |
| Stateful and concurrency workflows | `test_stateful_model_contracts.cpp`, `test_stateful_function.cpp`, `test_concurrency_function.cpp` | model/object agreement, action generation, callback lifecycle, prefix/last-action shrinking, retry and concurrency traces |
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
- Added `test_property_api_composition.cpp` as the first additive role-named
  split from the historical `test_property.cpp` mix. It protects ordered
  `.example()` / `.matrix()` / `.forAll()` composition, config precedence, and
  chain short-circuiting without deleting the original coverage yet.
- Added `test_dependent_generator_contracts.cpp` as an additive role-named
  split for dependent generator behavior. It protects deterministic replay for
  flatMap traces and validates shrink-tree preservation of dependent size and
  filter contracts.
- Added `test_property_runner_reporting.cpp` as a role-named split for runner
  configuration and reporting. It protects config-driven replay, lifecycle
  callbacks, configured output/error streams, example failure reports, and
  statistical assertion failure reports.
- Added `test_shrink_tree_contracts.cpp` as a role-named split for structural
  shrink-tree behavior. It protects integral movement toward zero, filter
  pruning, map traversal preservation, list-like size/uniqueness bounds, and
  concat/andThen placement without depending on full serialized tree text.
- Added `TEST_COVERAGE_MATRIX.md` as the repo-local harness for mapping
  supported capabilities to component families, behavioral contracts,
  component-focused properties, current coverage, and gaps.
- Added `test_accumulating_combinator_contracts.cpp` as a role-named split for
  aggregate/accumulate behavior. It protects deterministic dependent witnesses
  and bounded shrink-domain preservation for aggregate vectors and accumulated
  lists.
- Reworked the new dependent/accumulating contract tests to self-host through
  `property(..., gen::seed())`, `.example(Seed(...))`, and `.forAll()` instead
  of using top-level `Random` loops. Direct `Random` construction remains only
  inside property bodies where the generator itself is the component under test.
- The self-hosted accumulate witness exposed a real type-erasure bug:
  `accumulateImpl` hard-coded `flatMap<int>`, which made non-`int` accumulated
  values unsafe. The implementation now derives from the generated step count
  without naming a concrete result type.
- Added `test_stateful_model_contracts.cpp` as a role-named split for passing
  stateful model workflows. It protects object/model agreement across generated
  actions, state-dependent action selection from the current model, and balanced
  lifecycle/action callbacks for successful runs.
- Reduced `test_profiles.hpp` to cross-suite seed fixtures. Seeds used by one
  component test now live beside that test, where the replay scenario can be
  named concretely.

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
- Shared seed fixtures must serve more than one suite or name a portfolio-level
  replay role. Single-suite seeds belong in the test file that explains the
  scenario they replay.
- File-local constants are acceptable when they describe one test's domain
  shape, such as a flatMap search target or a specific compatibility fixture.
- Compile snippets may keep local literal seeds when adding a shared fixture
  would obscure the API form being compiled.
- Seed-replay properties should attach `gen::seed()` at property construction
  time, not only at `.forAll(...)`, so deterministic `.example(Seed(...))`
  witnesses do not require a global `Arbitrary<Seed>`.
- Use `PROP_ASSERT(value == expected)` instead of typed equality macros when
  the compared value lacks reliable stream formatting. Typed equality macros
  are better for scalar or formatter-backed values because they print operands
  on failure.

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
  first-class role rather than being folded into passing stateful model tests.

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
- `./build/test_proptest --gtest_brief=1` passed with 534 tests and 5 disabled
  tests.
- Focused portfolio verification passed for
  `AccumulatingCombinatorContracts.*:DependentGeneratorContracts.*:PropertyApiComposition.*:PropertyRunnerReporting.*:StatefulModelContracts.*`.
- Focused shrink-tree verification passed for `ShrinkTreeContracts.*`.
- Focused performance verification passed for
  `Performance.*:ShrinkerPerformance.*`.
- Focused seed-profile verification passed for
  `Random.*:Property.*:PropertyRunnerReporting.*:concurrency_function.shrink_uses_saved_rng_for_later_failure:Performance.ArbiBool:Performance.ArbiInt`.
- `git diff --check` passed after the profile normalization.
