# FSM Engine

A C++17 finite state machine framework targeting safety-critical embedded systems.
Designed around defense/effector system requirements: fixed memory, deterministic transitions,
full guard and callback support.

## Requirements

See [REQUIREMENTS.md](REQUIREMENTS.md).

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
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

## License

MIT
