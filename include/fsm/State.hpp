// LEARN: `#pragma once` is an include guard — it stops this header from being
// pasted into the same translation unit twice. The portable equivalent is the
// older `#ifndef FSM_STATE_HPP / #define FSM_STATE_HPP / #endif` triple.
#pragma once

// LEARN: angle brackets <> mean "system header search path"; double quotes
// "" mean "project headers first, then system". `<cstdint>` gives us
// `uint8_t` and friends.
#include <cstdint>

// LEARN: a namespace is a labelled scope. Everything inside `namespace fsm {}`
// is referred to as `fsm::Foo` from outside. Namespaces prevent name clashes
// between libraries that happen to pick the same identifiers.
namespace fsm {

/**
 * @brief Effector operating states.
 *
 * Represents the state machine of an effector subsystem as described
 * in REQUIREMENTS.md REQ-001 and REQ-006.
 */
// LEARN: `enum class` is a *scoped* enum (C++11+). You must write
// `EffectorState::SAFE`, never bare `SAFE`, and the values do not implicitly
// convert to int. The `: uint8_t` after the name picks the storage size —
// each value occupies one byte instead of the default `int` (four bytes).
enum class EffectorState : uint8_t {
    SAFE     = 0,  ///< Default safe state. Initial state on construction.
    ARMED    = 1,  ///< Authorization confirmed. Transition from SAFE only.
    TRACKING = 2,  ///< Target lock acquired. Transition from ARMED only.
    ENGAGED  = 3,  ///< Fire authorized and executed. Transition from TRACKING only.
    FAULT    = 4,  ///< Error terminal state. Reachable from any state.
    // LEARN: `COUNT` is a sentinel enumerator. Its job is to give us the
    // number of real states so we can size arrays without hard-coding a
    // magic number — see `enterCbs_` in FSM.hpp.
    COUNT    = 5   ///< Sentinel — do not use as a state.
};

/// Returns a human-readable string for a given state.
// LEARN: `constexpr` means "may be evaluated at compile time" — the compiler
// can fold the call away when the input is known. `const char*` is a pointer
// to immutable characters (string literal storage). `noexcept` is a promise
// that this function will not throw — useful in destructors and signal handlers.
constexpr const char* stateToString(EffectorState s) noexcept {
    // LEARN: `switch` on a scoped enum is the idiomatic mapping. The
    // `default:` branch handles `COUNT` (the sentinel) and any future
    // enumerator you forget to add a case for.
    switch (s) {
        case EffectorState::SAFE:     return "SAFE";
        case EffectorState::ARMED:    return "ARMED";
        case EffectorState::TRACKING: return "TRACKING";
        case EffectorState::ENGAGED:  return "ENGAGED";
        case EffectorState::FAULT:    return "FAULT";
        default:                      return "UNKNOWN";
    }
}

} // namespace fsm
