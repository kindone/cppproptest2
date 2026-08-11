# cppproptest Requirement and Component Coverage Matrix

This document is the working harness for broad test-portfolio refreshes. It
starts from what cppproptest supports, maps those capabilities to public
components, and records the component-focused contract properties that should
protect them.

`TEST_PORTFOLIO.md` records portfolio roles, decisions, verification, and
deferrals. This file records the coverage surface that drives those decisions.

## Project Statement

cppproptest is a C++ property-based testing library. It lets users define and
run generated properties, compose generators and combinators, shrink
counterexamples, replay failures, pin deterministic examples and matrices,
collect statistical observations, and test stateful or concurrent workflows.

## Matrix Vocabulary

| Term | Meaning |
|---|---|
| Supported capability | What users can rely on the library to do. |
| Public component / boundary | The API or component family responsible for that capability. |
| Behavioral contract | The stable behavior that should remain true across refactors. |
| Component-focused contract property | A property centered on one public component or boundary, still phrased as user-visible behavior. |
| Broader property it supports | The cross-component or workflow guarantee enabled by that component property. |

Decision labels:

- `covered`: current tests protect this contract clearly.
- `covered differently`: protected through another layer or C++-specific
  mechanism.
- `migrate`: meaningful existing coverage should move into clearer role-named
  tests.
- `subsume`: a stronger property covers weaker old checks.
- `keep`: the current test has a distinct valid role, such as compatibility or
  compile-only API coverage.
- `missing`: no meaningful current coverage.
- `deferred`: valid gap outside the current phase.
- `not applicable`: this port does not expose the behavior.

## Supported Capability Matrix

| Supported capability | Feature area | Public component / boundary | Behavioral contract | Component-focused contract property | Broader property it supports | Current coverage | Decision / gap |
|---|---|---|---|---|---|---|---|
| Users can state generated properties and run them reproducibly. | Property execution | `Property`, `forAll`, `ForAllConfig`, seed handling | Same seed and same generator produce the same generated trace; configured run count controls generated executions. | For a fixed seed and generator, two runs collect the same trace and lifecycle callbacks run once per generated case. | Reproducible execution across runner and generator boundaries. | `test_property_runner_reporting.cpp`, `test_property.cpp` | covered |
| Users can combine deterministic witnesses with generated exploration. | Property API composition | `.example()`, `.matrix()`, `.forAll()` chain | Chain steps execute in order; earlier failure short-circuits later steps. | A chain records example values, matrix values, and generated values in order; failing steps prevent later execution. | Reviewable suites with both witnesses and broad generated coverage. | `test_property_api_composition.cpp` | covered |
| Users can pin finite compatibility cases. | Example and matrix API | `.example()`, `.matrix()` | Deterministic witnesses preserve readable cases and fail predictably. | Finite matrices cover named API cases without pretending to be generated exploration. | Stable witnesses that explain or constrain generated properties. | scattered old tests, new composition tests | migrate during exact-case review |
| Users can generate primitive and structured values. | Generator domains | `gen::interval`, `gen::vector`, `Arbi`, namespace generators | Generated roots remain inside declared domains and expose useful boundary values. | Sampled roots satisfy declared bounds and include named structural subdomains under explicit profiles. | Generated domains are trustworthy inputs for larger properties. | `test_gen.cpp`, `test_generator.cpp`, `test_arbitrary.cpp` | covered but historically mixed |
| Users can compose generators without losing validity. | Combinators | `map`, `filter`, `flatMap`, `oneOf`, `elementOf`, `derive`, `aggregate`, `accumulate` | Combinators preserve preconditions through generation and shrinking. | Roots and bounded shrink candidates satisfy the composed domain predicate after each combinator. | Larger properties can rely on compositional generator validity. | `test_accumulating_combinator_contracts.cpp`, `test_dependent_generator_contracts.cpp`, `test_combinator*.cpp`, `test_flatmap_edge_cases.cpp` | covered for dependent and accumulating combinators; remaining historical tests still mixed |
| Users can model dependent domains. | Dependent generation | `flatMap`, generator size setters | Inner generated domain may depend on an outer generated value, and shrinking must preserve the dependency. | Generated `(size, vector)` values always have `vector.size() == size`, including bounded shrink traversal. | Nested scenarios and stateful tests can build valid dependent worlds. | `test_dependent_generator_contracts.cpp` | covered |
| Users get smaller counterexamples. | Shrinking | `Shrinkable`, shrinker modules, shrink streams | Shrink trees preserve roots, expose simpler candidates, and keep candidates in valid domains. | Integral shrinks move toward zero; filters remove rejected nodes; maps preserve traversal shape. | Failing properties produce useful, valid, diagnosable counterexamples. | `test_shrink_tree_contracts.cpp`, `test_shrinkable.cpp`, `test_shrinker*.cpp` | covered; exact compatibility kept |
| Users can trust exact public output where promised. | Formatting and diagnostics compatibility | `Show`, `serializeShrinkable`, report text | Stable public text remains compatible where intentionally pinned. | Exact shrink tree, tuple/pair/string, and failure report text remains stable. | Docs, diagnostics, and user-visible reports stay compatible. | exact-output tests, reporting tests | keep compatibility inventory |
| Users can diagnose failures. | Reporting | error/output streams, assertion macros, statistical reports | Assertion failures and statistical assertion failures go to configured streams with useful report text. | Configured streams capture assertion-failed examples and statistical assertion summaries. | Failed properties are debuggable and replayable. | `test_property_runner_reporting.cpp` | covered |
| Users can collect statistical observations. | Statistical checks | `PROP_STAT`, `PROP_STAT_ASSERT_*` | Statistical assertions fail when observed distribution violates thresholds; summaries are emitted. | Forced failing distributions emit failed stat name and predicate text. | Confidence checks can detect distribution regressions. | `test_property_runner_reporting.cpp`, `test_floating.cpp` | covered for failure path |
| Users can test stateful workflows. | Stateful property testing | `statefulProperty`, actions, models, callbacks | Generated action sequences preserve model/object agreement and shrink useful failing traces. | Execute generated commands against object and model, asserting observation agreement after each step. | Workflow-level properties can expose state transition bugs. | `test_stateful_model_contracts.cpp`, `test_stateful_function.cpp`, stateful shrink retry tests | covered for passing model workflows; failing-trace shrink coverage remains historically mixed |
| Users can test concurrent workflows. | Concurrent property testing | concurrency stateful APIs, front/rear actions | Concurrent traces preserve expected model behavior or produce replayable counterexamples. | Generated front/rear actions shrink to smaller thread/action structures while preserving failure. | Scheduling-sensitive bugs can be exposed. | `test_concurrency_function.cpp` | covered differently; careful review needed |
| Users can replay and assess flaky failures. | Shrink retry and reproduction | `shrinkMaxRetries`, reproduction stats callbacks | Flaky failures are assessed with configured retry counts and report replay statistics. | Generated seeds exercise flaky failure and assert stats totals and reproduction counts. | Unstable failures still produce useful diagnostics. | `test_shrink_retry.cpp` | keep or split later |
| Users can rely on compile-time API shapes. | Compile/API compatibility | `proptest/test/compile`, `compile_fail` fixtures | Public templates and expected invalid forms compile or fail as intended. | Representative snippets compile; invalid weighted element form fails. | C++ template/API surfaces remain source-compatible. | compile and compile_fail snippets | keep |
| Test execution remains bounded and reproducible. | Harness quality | shared profile constants | Repeated budgets are named and scalable without changing semantic domains. | Repeated seeds, runs, samples, action counts, retries, and timeouts route through profile constants. | Portfolio execution is tunable without changing requirements. | `test_profiles.hpp`, `TEST_PORTFOLIO.md` | covered; keep auditing |

## Component Family: Property Runner and Property Object

| Subcomponent / API | User-visible responsibility | Behavioral contract | Component-focused contract property | Deterministic witness | Generated domain / subdomains | Existing coverage | Decision / gap |
|---|---|---|---|---|---|---|---|
| `Property` construction | Wrap a user predicate into an executable property. | Predicate result, assertion failures, and exceptions determine property result consistently. | Predicate returns and assertion failures produce expected pass/fail outcomes. | `.example()` pass/fail cases. | scalar values; assertion vs false-return distinction. | `test_property.cpp`, `test_property_runner_reporting.cpp` | covered |
| `.forAll()` | Run a property over generated values. | Generated run count, seed, and generator determine execution trace. | Same seed and generator gives same trace; configured run count controls trace length. | fixed-seed replay when needed. | scalar generator; seeded replay; lifecycle callbacks. | `test_property_runner_reporting.cpp` | covered |
| `.example()` | Pin readable deterministic witnesses. | Example arguments run immediately and report assertion failures to configured stream. | Assertion-failed example emits report text to configured error stream. | `.example(7)` with failing assertion. | deterministic single case. | `test_property_runner_reporting.cpp` | covered |
| `.matrix()` | Pin finite cross-product witnesses. | Matrix cases execute deterministically in chain order. | Chain recording shows examples, matrix cases, generated values in order. | finite matrix `{1, 2}`. | small finite values. | `test_property_api_composition.cpp` | covered |
| Chain composition | Combine examples, matrices, generated runs, and config setters. | Earlier failure short-circuits later steps; later config setter wins. | Ordered-chain and latest-writer-wins properties. | failing deterministic step before generated run. | examples, matrices, small generated domain. | `test_property_api_composition.cpp` | covered |
| Duration and timeout controls | Bound long-running properties. | Max duration and shrink timeout constrain execution and still report stats. | Timeout property reports bounded stats under generated sleep/failure parameters. | timeout regression examples. | sleep duration, fail denominator, seed. | `test_shrink_retry.cpp` | keep; split later |

## Component Family: Generator Combinators

| Subcomponent / API | User-visible responsibility | Behavioral contract | Component-focused contract property | Deterministic witness | Generated domain / subdomains | Existing coverage | Decision / gap |
|---|---|---|---|---|---|---|---|
| `map` | Transform generated values. | Mapped roots and shrink candidates are transformed consistently. | Mapped shrink traversal equals original traversal transformed pointwise. | `value + 10` witness. | simple integral shrink tree. | `test_shrink_tree_contracts.cpp`, combinator tests | covered |
| `filter` | Restrict generated domain. | Accepted roots and shrink candidates satisfy predicate. | Bounded shrink traversal contains no rejected values. | predicate `value > 10`. | accepted/rejected boundary. | `test_shrink_tree_contracts.cpp`, combinator tests | covered |
| `flatMap` | Generate dependent inner domains. | Inner domain may depend on outer value; shrinking preserves dependency. | `(size, vector)` root and shrink candidates satisfy `vector.size() == size`. | compact size/vector example if useful. | size 0..5; inner vector length equals outer size. | `test_dependent_generator_contracts.cpp` | covered |
| `oneOf` / weighted variants | Select among branches. | Generated roots come from declared branches; weights are intentional when used as profiles. | Branch-tagged generator produces only declared branch values and preserves tags when applicable. | branch matrix for finite alternatives. | branch tags, common/risky/frontier profiles. | `test_combinator*.cpp` | likely covered; role clarity deferred |
| `elementOf` | Select from finite set. | Generated values are members of the input set/list. | Membership property over generated roots and shrink candidates. | finite matrix of all elements. | duplicate values, empty invalid case if defined. | `test_combinator*.cpp`, compile_fail weighted case | covered differently |
| `derive` | Derive structured values from component generators. | Derived structure fields satisfy their generator domains. | Generated object decomposes into valid field domains. | simple derived object example. | product fields, nested field domains. | `test_combinator*.cpp`, compile snippets | covered but mixed |
| `aggregate` | Build a vector of dependent generated values. | Generated transitions preserve builder invariants; shrink remains diagnosable. | Generated vectors and bounded shrink candidates satisfy adjacent dependent-step bounds. | deterministic vector `{1, 2, 3}`. | step count, previous-value bounds, shrink candidates. | `test_accumulating_combinator_contracts.cpp` | covered |
| `accumulate` | Return the final accumulated value after dependent generation steps. | Every intermediate append preserves the accumulated-value invariant, and the returned final value remains valid through shrinking. | Generated final lists and bounded shrink candidates satisfy append-step bounds. | deterministic list `{1, 2, 3}`. | initial list, appended values, step count, shrink candidates. | `test_accumulating_combinator_contracts.cpp` | covered |

## Component Family: Shrinkable and Shrinkers

| Subcomponent / API | User-visible responsibility | Behavioral contract | Component-focused contract property | Deterministic witness | Generated domain / subdomains | Existing coverage | Decision / gap |
|---|---|---|---|---|---|---|---|
| `Shrinkable<T>` root | Represent a generated value and its shrink tree. | Root value is preserved; shrink tree can be traversed safely. | Preorder traversal starts with root and stays bounded under traversal limit. | root-only shrinkable. | root-only, one child, nested child. | `test_shrinkable.cpp`, `test_shrink_tree_contracts.cpp` | covered |
| `with`, `concatStatic`, `andThenStatic` | Attach shrink candidates. | Horizontal and vertical candidate placement are distinct. | Direct children differ for concat vs andThen, while full traversal includes fallback. | root 2, fallback 3. | direct children, leaf candidates. | `test_shrink_tree_contracts.cpp`, `test_shrinkable.cpp` | covered |
| `map` on shrinkable | Transform a shrink tree. | Traversal shape is preserved while values transform. | Mapped values equal original values transformed pointwise. | `+10` transform. | integral shrink tree. | `test_shrink_tree_contracts.cpp` | covered |
| `filter` on shrinkable | Prune invalid shrink candidates. | Every traversed value satisfies predicate. | Filtered shrink tree contains no rejected nodes. | `value > 10`. | accepted branch, rejected branch, boundary. | `test_shrink_tree_contracts.cpp`, `test_shrinkable.cpp` | covered |
| Integral shrinker | Shrink integers toward zero. | Candidates do not cross sign and absolute value does not increase. | Matrix over negative, zero, positive roots. | roots `-20,-8,-1,0,1,8,20`. | sign, zero, magnitude. | `test_shrink_tree_contracts.cpp`, exact serialized tests | covered |
| List-like shrinker | Shrink membership and/or elements. | Minimum size is respected; duplicate vectors are not exposed. | Membership shrink traversal has unique values with size in bounds. | root `[1,2,3,4,5]`, min size 2. | empty allowed, min size, full size, element shrink. | `test_shrink_tree_contracts.cpp`, `test_shrinker2.cpp` | covered |

## Component Family: Stateful and Concurrent Testing

| Subcomponent / API | User-visible responsibility | Behavioral contract | Component-focused contract property | Deterministic witness | Generated domain / subdomains | Existing coverage | Decision / gap |
|---|---|---|---|---|---|---|---|
| `statefulProperty` | Generate initial state and action sequence. | Generated actions execute against object and preserve model agreement. | After each action, object observation equals model observation. | short fixed action sequence. | empty/nonempty, valid/invalid action, terminal states if relevant. | `test_stateful_model_contracts.cpp`, `test_stateful_function.cpp` | covered for passing model agreement |
| Stateful action generator | Produce meaningful action sequences. | Generated actions are valid for the intended model state or explicitly test invalid transitions. | Action generator reaches state-changing and observation-only actions with named weights. | matrix of action types. | action mix, sequence length, state-dependent validity. | `test_stateful_model_contracts.cpp`, `test_stateful_function.cpp` | covered for state-dependent current-model selection; weight/profile clarity deferred |
| Stateful callbacks | Report lifecycle and per-action execution boundaries. | Successful runs balance startup/cleanup, action start/end, and post-check callbacks. | Fixed-size successful action sequence yields exact callback counts. | fixed action list size with deterministic increment action. | run count, action count, successful post-check. | `test_stateful_model_contracts.cpp`, `test_stateful_function.cpp` | covered |
| Stateful shrinking | Minimize failing action sequences. | Shrunk traces preserve failure and remain replayable. | Known failing workflow shrinks to a simpler action sequence. | replay seed or fixed trace. | prefix shrink, last-action shrink, command simplification. | `test_stateful_function.cpp`, `test_shrink_retry.cpp` | covered but mixed |
| Concurrent front/rear actions | Model schedule-sensitive workflows. | Concurrent trace failures shrink by thread/action structure while preserving failure. | Generated front/rear trace produces replayable minimal counterexample. | known seed examples. | thread count, front actions, rear actions, interleavings. | `test_concurrency_function.cpp` | keep; careful review before migration |

## Component Family: Harness and Portfolio Quality

| Subcomponent / API | User-visible responsibility | Behavioral contract | Component-focused contract property | Deterministic witness | Generated domain / subdomains | Existing coverage | Decision / gap |
|---|---|---|---|---|---|---|---|
| Shared test profiles | Name repeated execution budgets. | Reused run/sample/action/retry/timeout budgets have semantic names. | Audit repeated literals and route reusable budgets through profile constants. | none. | fast, focused, smoke, shrink retry, performance. | `test_profiles.hpp` | covered |
| Semantic constants | Name domain facts separately from execution budgets. | Domain bounds do not scale with environment multipliers. | Review generator sizes/ranges and classify semantic vs execution. | finite matrix for semantic boundaries. | size bounds, root values, run counts. | portfolio notes | partially covered |
| Isolation guardrails | Avoid destructive global effects. | Tests mutate only scoped resources and unique env names. | Destructive setup uses scoped temp paths, unique env names, and guard assertions before deletion. | temp resource example. | env var collision, stale globals, filesystem deletion. | writing_test lesson | missing automated guard |
| Portfolio docs | Record decisions and asymmetries. | Every major component has matrix coverage and every decision has a reason. | Decision log rows connect component contracts to tests and verification. | none. | covered, keep, migrate, retire, deferred. | `TEST_PORTFOLIO.md`, this file | active work |

## Current Strong Next Candidates

| Candidate | Target role | Reason |
|---|---|---|
| Concurrent workflow contract review | contracts/regressions | High-risk behavior; preserve replay semantics before any migration. |
| Exact formatting compatibility inventory | compatibility | Exact-output tests should be kept deliberately, not mixed with structural laws. |

## Component-Family Completion Gate

A component-family pass is complete only when:

- every public subcomponent/API appears in the matrix or is marked not
  applicable
- every row has a behavioral contract
- every row has a coverage decision
- `migrate` and `missing` rows have target files or explicit deferrals
- compatibility rows are not mistaken for structural contract coverage
- focused verification and full verification are recorded
- lessons learned are recorded outside active test headers
