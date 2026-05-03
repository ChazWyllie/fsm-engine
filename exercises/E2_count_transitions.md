# E2 — Count outgoing transitions

**Difficulty:** ★

## Goal

Add a public method to `FSM` that returns the number of registered
transitions whose `from` state matches a given state:

```cpp
std::size_t outgoingFromCount(EffectorState s) const noexcept;
```

## What you'll touch

- [`include/fsm/FSM.hpp`](../include/fsm/FSM.hpp) — declare the method
- [`src/FSM.cpp`](../src/FSM.cpp) — define it
- [`tests/test_fsm.cpp`](../tests/test_fsm.cpp) — add a test

## Definition of done

```cpp
TEST(FSMTest, OutgoingFromCount) {
    FSM fsm = buildEffectorFSM();
    EXPECT_EQ(fsm.outgoingFromCount(EffectorState::SAFE),     2u);  // ARM, ERROR
    EXPECT_EQ(fsm.outgoingFromCount(EffectorState::ARMED),    3u);  // SAFE, LOCK, ERROR
    EXPECT_EQ(fsm.outgoingFromCount(EffectorState::FAULT),    1u);  // RESET
}
```

## Hints

- The implementation is a 4-line loop that mirrors `findTransition`.
- The trailing `const noexcept` on the declaration is non-negotiable —
  this method must not modify the FSM and must not throw.
- The literal `2u` (with `u` suffix) is an `unsigned` integer literal,
  matching `std::size_t` so `EXPECT_EQ` doesn't warn about signed/unsigned
  comparison.

## Why this matters

You just learned the most common pattern in C++: declare in a header,
define in a source file, test in a separate translation unit. Repeat
this five hundred times and you have a career.
