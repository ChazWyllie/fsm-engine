# FSM Engine Requirements

| ID      | Requirement                                                                                     |
|---------|-------------------------------------------------------------------------------------------------|
| REQ-001 | The FSM shall maintain exactly one active state at any time.                                    |
| REQ-002 | State transitions shall only occur via explicitly registered events.                            |
| REQ-003 | Invalid transitions shall invoke an error handler — not silently fail or change state.          |
| REQ-004 | Entry and exit callbacks shall be invoked on every valid state change.                          |
| REQ-005 | Guard conditions shall block a transition and return false if the condition evaluates to false.  |
| REQ-006 | The FSM shall support a FAULT terminal state reachable from any state via the ERROR event.      |
| REQ-007 | No dynamic memory allocation shall occur after FSM initialization.                              |
| REQ-008 | State and transition tables shall be fixed-size arrays — no std::vector or heap containers.     |
| REQ-009 | The FSM shall log every successful transition: from-state, event, to-state, and timestamp.      |
| REQ-010 | All callbacks and guards shall be stored as std::function — no raw function pointers.           |
