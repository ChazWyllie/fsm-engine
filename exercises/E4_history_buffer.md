# E4 — Fixed-size history buffer

**Difficulty:** ★★★

## Goal

Add a circular history buffer to `FSM` that records the last N
transitions. Caller can ask "what were the last 8 transitions?". No heap
allocation — it's a `std::array`.

## API to add

```cpp
struct HistoryEntry {
    EffectorState from;
    EffectorEvent event;
    EffectorState to;
};

static constexpr std::size_t HISTORY_SIZE = 16;

std::size_t historyCount() const noexcept;          // up to HISTORY_SIZE
HistoryEntry historyAt(std::size_t i) const noexcept; // 0 = oldest
```

## What you'll touch

- [`include/fsm/FSM.hpp`](../include/fsm/FSM.hpp) — declare `HistoryEntry`,
  the buffer member, and the two methods
- [`src/FSM.cpp`](../src/FSM.cpp) — record on each successful transition
  in `dispatch`, implement `historyAt` to return entries oldest-first
- [`tests/test_fsm.cpp`](../tests/test_fsm.cpp) — write tests

## Definition of done

```cpp
TEST(FSMTest, HistoryRecordsLastTransitions) {
    FSM fsm = buildEffectorFSM();
    fsm.dispatch(EffectorEvent::ARM);
    fsm.dispatch(EffectorEvent::LOCK);
    fsm.dispatch(EffectorEvent::FIRE);

    EXPECT_EQ(fsm.historyCount(), 3u);

    EXPECT_EQ(fsm.historyAt(0).from,  EffectorState::SAFE);
    EXPECT_EQ(fsm.historyAt(0).event, EffectorEvent::ARM);
    EXPECT_EQ(fsm.historyAt(0).to,    EffectorState::ARMED);

    EXPECT_EQ(fsm.historyAt(2).to, EffectorState::ENGAGED);
}

TEST(FSMTest, HistoryWrapsAroundAtCapacity) {
    FSM fsm(EffectorState::SAFE);
    fsm.addTransition({ EffectorState::SAFE, EffectorEvent::ARM,  EffectorState::ARMED, nullptr, nullptr });
    fsm.addTransition({ EffectorState::ARMED, EffectorEvent::SAFE, EffectorState::SAFE, nullptr, nullptr });

    // 20 transitions, but we only keep the last HISTORY_SIZE (16).
    for (int i = 0; i < 10; ++i) {
        fsm.dispatch(EffectorEvent::ARM);
        fsm.dispatch(EffectorEvent::SAFE);
    }
    EXPECT_EQ(fsm.historyCount(), 16u);
}
```

## Hints

- A circular buffer needs two indices: `head_` (next write slot) and
  `count_` (how many valid entries, capped at `HISTORY_SIZE`).
- Modulo arithmetic: `head_ = (head_ + 1) % HISTORY_SIZE`.
- `historyAt(i)` returns the i-th oldest entry. If the buffer has wrapped,
  the oldest is at `(head_ - count_ + HISTORY_SIZE) % HISTORY_SIZE`.
- Watch out for unsigned underflow when subtracting in `size_t`. Add
  `HISTORY_SIZE` before the modulo.

## What you'll learn

- Designing a fixed-memory data structure that "feels like" a queue.
- Why production-grade embedded code lives and dies by careful index
  arithmetic.
- How `static_assert` lets you bake in invariants like
  `static_assert(HISTORY_SIZE > 0)` at compile time.
