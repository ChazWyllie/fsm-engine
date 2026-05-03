# E1 — Add a new state

**Difficulty:** ★

## Goal

Add a new state `STANDBY` between `SAFE` and `ARMED`. The mission flow becomes:

```
SAFE → STANDBY → ARMED → TRACKING → ENGAGED → SAFE
```

Triggering events:

- `WAKE` moves `SAFE → STANDBY`
- `ARM` now moves `STANDBY → ARMED` (was `SAFE → ARMED`)
- `SLEEP` moves `STANDBY → SAFE`

## What you'll touch

- [`include/fsm/State.hpp`](../include/fsm/State.hpp) — add `STANDBY` and bump `COUNT`
- [`include/fsm/Event.hpp`](../include/fsm/Event.hpp) — add `WAKE` and `SLEEP`, bump `COUNT`
- The `*ToString` functions in both headers
- [`tests/test_fsm.cpp`](../tests/test_fsm.cpp) — `buildEffectorFSM()` and add a new test

## Definition of done

`ctest --test-dir build --output-on-failure` passes, including a new test
`TEST(FSMTest, StandbyEntersBetweenSafeAndArmed)` that exercises:

```
SAFE --WAKE--> STANDBY --ARM--> ARMED
```

## Hints

- Don't forget to update `*ToString` switch cases — `default: "UNKNOWN"`
  will silently mask a missing case in the log output.
- The callback arrays in `FSM.hpp` are sized by `EffectorState::COUNT`.
  If you bump `COUNT`, the arrays grow automatically.
- `MAX_TRANSITIONS = 32` — you have plenty of headroom.

## Stretch

Make `WAKE` from any state other than `SAFE` route to the error handler
(REQ-003). Add a test for that.
