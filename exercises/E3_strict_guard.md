# E3 — A guard with state

**Difficulty:** ★★

## Goal

Implement a guard that requires **two consecutive arming attempts within 5
seconds** before allowing `SAFE → ARMED`. Single arming events should be
blocked.

This is a real safety pattern — accidental single-press arming has caused
real-world incidents. Two presses within a window is intentional.

## What you'll touch

- [`tests/test_fsm.cpp`](../tests/test_fsm.cpp) — add a new test

You should not need to modify the production code.

## Hints

- Use a lambda that captures by reference (`[&]`) a counter and a
  timestamp. Maintain them inside the lambda body across calls.
- `std::chrono::steady_clock::now()` is the right clock — it never goes
  backwards, even if the system time changes.
- Consider what the guard returns on the first call (false), the second
  call within the window (true), and the second call after the window
  expires (false, reset counter).

## Skeleton

```cpp
TEST(FSMTest, DoubleArmGuard) {
    FSM fsm(EffectorState::SAFE);

    int presses = 0;
    auto last = std::chrono::steady_clock::now();
    auto guard = [&]() {
        // your logic here
    };

    fsm.addTransition({
        EffectorState::SAFE, EffectorEvent::ARM, EffectorState::ARMED,
        guard,
        nullptr
    });

    EXPECT_FALSE(fsm.dispatch(EffectorEvent::ARM));   // first press blocked
    EXPECT_EQ(fsm.currentState(), EffectorState::SAFE);

    EXPECT_TRUE(fsm.dispatch(EffectorEvent::ARM));    // second within window
    EXPECT_EQ(fsm.currentState(), EffectorState::ARMED);
}
```

## What you'll learn

- Capturing local state in a lambda — and the lifetime trap if the locals
  go out of scope before the lambda is called.
- Why `std::chrono` is used instead of `time(nullptr)` for elapsed-time
  measurements.
- How `std::function`'s "I can hold any callable" property lets you
  smuggle stateful behaviour into a `Transition` slot.
