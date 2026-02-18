# Simplifying Failed Inputs (Shrinking)

> **New to property-based testing?** Start with the [Walkthrough](Walkthrough.md) for a step-by-step guide. Shrinking happens automatically when a property fails—you don't need to configure it. This page explains how it works and when it helps.

When a property fails, `cppproptest` finds a *counterexample*—an input that disproves the property. *Shrinking* then simplifies that counterexample to make debugging easier. All [built-in generators](Generators.md) and [combinators](Combinators.md) support shrinking out of the box.

&nbsp;

## Overview

| Concept | Description |
|---------|-------------|
| **Counterexample** | An input that causes the property to fail |
| **Shrinking** | The process of finding a simpler counterexample that still fails |
| **Shrinkable** | A wrapper that carries a value and its shrink candidates (created internally by generators) |

**Example:** Assume a property fails with `a = -13680124, b = 7524.68454e-14, c = true`. After shrinking, you might get `a = 0, b = 0, c = true`—if `a` and `b` have no relation to the failure condition and only `c` matters.

&nbsp;

## How Shrinking Works

When `forAll` detects a failure, it runs a shrinking phase. It repeatedly tries simpler variants of the failing input (**while keeping the constraint given when it was generated**). If a simpler variant still fails the test, the framework keeps it and continues until there is no more that fails. The result is a minimal counterexample that still reproduces the failure.

This automates the manual debugging step of finding which arguments matter and simplifying them to isolate the root cause as much as possible.

&nbsp;

## Shrinkability by Type

`cppproptest` uses type-specific heuristics for what counts as "simpler". For example, simplifying some basic types can have following approaches:

| Type | Simpler means |
|------|---------------|
| **Booleans** | `false` is simpler than `true` |
| **Integers** | Smaller absolute value; remove sign (e.g., `-34` → `34`, `16384` → `1024`) |
| **Floats** | Smaller exponent; fewer digits (e.g., `12e55` → `12e20`, `-29.5134` → `-29`) |
| **Strings** | Fewer characters, simpler characters. (e.g., `"Hello world!"` → `"Hello"`) |
| **Containers** | Fewer elements, simpler elements. (e.g., `[0,1,2,3,4,5]` → `[0,0,0]`) |

&nbsp;

## When Shrinking Helps

- **Debugging** — A minimal counterexample is easier to reason about than a large random one. It's often a single parameter that's causing the failure while the rest are irrelevant.
- **Test design**
- **Reproducibility** — The shrunk counterexample is reported with the failure; you can use it to reproduce the bug locally.

&nbsp;

## Custom Generators and Shrinking

When building [custom generators](CustomGenerator.md), you wrap values with `make_shrinkable<T>(value)`. The default `make_shrinkable` provides no shrink candidates (the value is already minimal). For custom shrinking behavior, you can provide alternative shrink candidates—see the [CustomGenerator](CustomGenerator.md) and shrinker APIs for advanced use.

&nbsp;

---

## Related Topics

- [Walkthrough](Walkthrough.md) - Step-by-step guide for creating property tests
- [Property API](PropertyAPI.md) - Using `forAll` and property configuration
- [Generators](Generators.md) - Built-in generators (all support shrinking)
- [Combinators](Combinators.md) - Generator combinators (all support shrinking)
- [CustomGenerator](CustomGenerator.md) - Building custom generators with `make_shrinkable`
