# FSM Engine

A C++17 finite state machine framework targeting safety-critical embedded systems.
Designed around defense/effector system requirements: fixed memory, deterministic transitions,
full guard and callback support.

## Learning C++? Start here.

This repo doubles as a hands-on C++ tutorial. Read in this order:

1. **[LEARN.md](LEARN.md)** — guided tour of every C++ concept used here,
   each citation pointing at a real file:line in the codebase.
2. **Source files**, in this order — every important construct has an
   inline `// LEARN:` comment:
   - [`include/fsm/State.hpp`](include/fsm/State.hpp)
   - [`include/fsm/Event.hpp`](include/fsm/Event.hpp)
   - [`include/fsm/Transition.hpp`](include/fsm/Transition.hpp)
   - [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp)
   - [`src/FSM.cpp`](src/FSM.cpp)
   - [`tests/test_fsm.cpp`](tests/test_fsm.cpp)
   - [`examples/effector_demo.cpp`](examples/effector_demo.cpp)
3. **[exercises/](exercises/)** — five hands-on tasks, easy → hard.

## Requirements

See [REQUIREMENTS.md](REQUIREMENTS.md).

## Build

```bash
brew install cmake googletest
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### macOS Command Line Tools workaround

Some macOS installs ship Command Line Tools where Apple Clang searches a
non-existent C++ stdlib path. If the build fails with `'cstdint' file not found`,
prefix the configure and build commands with:

```bash
export CPLUS_INCLUDE_PATH=$(xcrun --show-sdk-path)/usr/include/c++/v1
```

### Sanitizers

```bash
cmake -B build-san -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined" \
                   -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-san --parallel
ctest --test-dir build-san --output-on-failure
```

## Usage

A runnable end-to-end example lives in [`examples/effector_demo.cpp`](examples/effector_demo.cpp):

```bash
cmake --build build --target effector_demo
./build/effector_demo
```

Minimal API in code:

```cpp
#include "fsm/FSM.hpp"
using namespace fsm;

FSM fsm(EffectorState::SAFE);

fsm.addTransition({ EffectorState::SAFE,  EffectorEvent::ARM, EffectorState::ARMED, nullptr, nullptr });
fsm.addTransition({ EffectorState::ARMED, EffectorEvent::LOCK, EffectorState::TRACKING,
                    [] { return authorized(); },                  // guard (REQ-005)
                    [] { /* side effect on success */ } });        // action

fsm.addUniversalErrorTransition();                                  // REQ-006 in one call

fsm.onEnter(EffectorState::ARMED, [](EffectorState) { /* ... */ }); // REQ-004
fsm.onError([](EffectorState s, EffectorEvent e) { /* log */ });    // REQ-003

fsm.dispatch(EffectorEvent::ARM);                                   // returns bool
```

## Design principles

- No heap allocation after init (REQ-007)
- Fixed-size transition table (REQ-008)
- Every transition is logged with timestamp (REQ-009)
- Guards block transitions without side effects (REQ-005)
- Entry/exit callbacks fire on every valid state change (REQ-004)

## States

| State    | Description                        |
|----------|------------------------------------|
| SAFE     | Default. No authorization.         |
| ARMED    | Authorization confirmed.           |
| TRACKING | Target lock acquired.              |
| ENGAGED  | Fire authorized and executed.      |
| FAULT    | Error terminal. Reset to recover.  |

## Requirements traceability

Each requirement in [REQUIREMENTS.md](REQUIREMENTS.md) is enforced in code and
covered by at least one GoogleTest case.

<!-- AUTO-GENERATED: REQ-TRACE -->
| REQ | Enforced in | Verified by |
|-----|-------------|-------------|
| REQ-001 | [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) (`current_` member) | [`SingleActiveState`](tests/test_fsm.cpp#L34) |
| REQ-002 | [`src/FSM.cpp`](src/FSM.cpp) `findTransition` | [`OnlyRegisteredTransitions`](tests/test_fsm.cpp#L44) |
| REQ-003 | [`src/FSM.cpp`](src/FSM.cpp) `dispatch` error path | [`InvalidEventFiresErrorCallback`](tests/test_fsm.cpp#L53) |
| REQ-004 | [`src/FSM.cpp`](src/FSM.cpp) `dispatch` exit/entry calls | [`EntryExitCallbacksFire`](tests/test_fsm.cpp#L62), [`CallbacksDoNotFireOnFailedTransition`](tests/test_fsm.cpp#L77) |
| REQ-005 | [`src/FSM.cpp`](src/FSM.cpp) guard short-circuit | [`GuardBlocksTransition`](tests/test_fsm.cpp#L86), [`GuardAllowsTransitionWhenTrue`](tests/test_fsm.cpp#L111) |
| REQ-006 | [`src/FSM.cpp`](src/FSM.cpp) `addUniversalErrorTransition` | [`ErrorFromAnyState`](tests/test_fsm.cpp#L123), [`UniversalErrorTransitionReachesFaultFromEveryState`](tests/test_fsm.cpp#L174), [`UniversalErrorTransitionSkipsExistingEntries`](tests/test_fsm.cpp#L187) |
| REQ-007 | [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) `std::array` storage | [`TransitionTableFixedSize`](tests/test_fsm.cpp#L146) + ASan build |
| REQ-008 | [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) `std::array<Transition, MAX_TRANSITIONS>` | [`TransitionTableFixedSize`](tests/test_fsm.cpp#L146) |
| REQ-009 | [`src/FSM.cpp`](src/FSM.cpp) `logTransition` (uses `localtime_r`) | [`SuccessfulTransitionEmitsLogLine`](tests/test_fsm.cpp#L210), [`FailedTransitionEmitsNoLog`](tests/test_fsm.cpp#L227) |
| REQ-010 | [`include/fsm/Transition.hpp`](include/fsm/Transition.hpp), [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) `std::function` aliases | All guard / callback tests |
<!-- /AUTO-GENERATED -->

## License

MIT
