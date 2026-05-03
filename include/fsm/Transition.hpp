#pragma once

#include "State.hpp"
#include "Event.hpp"
// LEARN: `<functional>` brings in `std::function`, a type-erased holder for
// anything callable: free functions, lambdas, member-fn binds, struct with
// `operator()`. The price for that flexibility is a small amount of indirection
// (and possibly heap storage for large captures — see REQ-007 discussion).
#include <functional>

namespace fsm {

/**
 * @brief Defines a single state transition.
 *
 * A transition fires when the FSM is in `from`, receives `event`,
 * and the `guard` (if set) returns true.
 *
 * See REQUIREMENTS.md REQ-002, REQ-005, REQ-010.
 */
// LEARN: `struct` and `class` are nearly identical in C++. The only language
// difference is default access: `struct` defaults to public, `class` to
// private. Convention: `struct` for plain data with no invariants, `class`
// when you have invariants that the type must enforce.
struct Transition {
    EffectorState from;                  ///< Source state.
    EffectorEvent event;                 ///< Triggering event.
    EffectorState to;                    ///< Destination state.
    // LEARN: `std::function<bool()>` is "any callable taking no arguments and
    // returning bool". `nullptr` is the typed null pointer literal — always
    // prefer it over the old `NULL` macro.
    std::function<bool()> guard;         ///< Optional. nullptr = always allow.
    std::function<void()> action;        ///< Optional. Runs on successful transition.
};

} // namespace fsm
