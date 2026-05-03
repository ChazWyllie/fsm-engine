# FSM Engine

A C++17 finite state machine framework targeting safety-critical embedded systems.
Designed around defense/effector system requirements: fixed memory, deterministic transitions,
full guard and callback support.

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
| REQ-001 | [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) (`current_` member) | [`SingleActiveState`](tests/test_fsm.cpp#L29) |
| REQ-002 | [`src/FSM.cpp`](src/FSM.cpp) `findTransition` | [`OnlyRegisteredTransitions`](tests/test_fsm.cpp#L39) |
| REQ-003 | [`src/FSM.cpp`](src/FSM.cpp) `dispatch` error path | [`InvalidEventFiresErrorCallback`](tests/test_fsm.cpp#L48) |
| REQ-004 | [`src/FSM.cpp`](src/FSM.cpp) `dispatch` exit/entry calls | [`EntryExitCallbacksFire`](tests/test_fsm.cpp#L57), [`CallbacksDoNotFireOnFailedTransition`](tests/test_fsm.cpp#L72) |
| REQ-005 | [`src/FSM.cpp`](src/FSM.cpp) guard short-circuit | [`GuardBlocksTransition`](tests/test_fsm.cpp#L81), [`GuardAllowsTransitionWhenTrue`](tests/test_fsm.cpp#L102) |
| REQ-006 | [`include/fsm/Event.hpp`](include/fsm/Event.hpp) `ERROR` event + integrator pattern | [`ErrorFromAnyState`](tests/test_fsm.cpp#L114) |
| REQ-007 | [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) `std::array` storage | [`TransitionTableFixedSize`](tests/test_fsm.cpp#L137) + ASan build |
| REQ-008 | [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) `std::array<Transition, MAX_TRANSITIONS>` | [`TransitionTableFixedSize`](tests/test_fsm.cpp#L137) |
| REQ-009 | [`src/FSM.cpp`](src/FSM.cpp) `logTransition` | [`SuccessfulTransitionEmitsLogLine`](tests/test_fsm.cpp#L166), [`FailedTransitionEmitsNoLog`](tests/test_fsm.cpp#L183) |
| REQ-010 | [`include/fsm/Transition.hpp`](include/fsm/Transition.hpp), [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) `std::function` aliases | All guard / callback tests |
<!-- /AUTO-GENERATED -->

## License

MIT
