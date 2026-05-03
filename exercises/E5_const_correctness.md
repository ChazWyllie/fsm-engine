# E5 — Hunting `const` bugs

**Difficulty:** ★★★★

## Goal

This exercise teaches you `const`-correctness — the single biggest
source of silent bugs (and silent breakage of mental models) in C++.

## Setup

In a temporary scratch file (do not commit), write the following code:

```cpp
#include "fsm/FSM.hpp"
using namespace fsm;

void inspect(const FSM& fsm) {
    // Each line below is *intentionally wrong* in some way.
    // Find the bug. Explain it before reaching for the compiler.

    fsm.dispatch(EffectorEvent::ARM);                    // (a)
    auto s = fsm.currentState();                          // (b)
    fsm.addTransition({ /* ... */ });                     // (c)
    EffectorState* leak = (EffectorState*)&fsm;           // (d) compiles, but...
}

int main() {
    FSM fsm(EffectorState::SAFE);
    inspect(fsm);
}
```

## Tasks

For each of (a), (b), (c), (d):

1. State whether it compiles.
2. If it compiles, state whether it should.
3. Identify the contract that's being violated (or not).

Then fix `inspect` so it only does things a const-reference allows.

## Discussion answers

(a) `dispatch` is non-const → won't compile. Good — the compiler is
    saving you. A `const` reference promises to a caller "I will not
    modify your object".

(b) `currentState()` is `const` → compiles, behaves as expected.

(c) `addTransition` mutates the table → won't compile through a `const`
    reference. Good.

(d) **C-style cast strips const**. Compiles. Will then proceed to write
    through what was meant to be a read-only reference and produce
    undefined behaviour the next time anyone reads the FSM.

    Lesson: never use C-style casts in C++ code. Use one of the four
    explicit casts:
    - `static_cast<T>` — well-defined value conversions
    - `dynamic_cast<T>` — runtime-checked downcasts in class hierarchies
    - `const_cast<T>` — strip const (very rarely correct, always a smell)
    - `reinterpret_cast<T>` — bit-level reinterpretation (last resort)

## Stretch

Make `findTransition` overloaded to also have a non-const version that
returns `Transition*` (so callers holding a non-const FSM can mutate
the action without re-adding the whole transition). This is the
canonical "const-overload" pattern. Look up `std::as_const` and how to
share implementation between the two versions.

## What you'll learn

- `const` is a contract enforced by the compiler. Honour it.
- C-style casts subvert the type system silently. Avoid them.
- Const-overload pattern is everywhere in the standard library
  (`std::vector::operator[]`, `std::string::c_str()`, etc).
