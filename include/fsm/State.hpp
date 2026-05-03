#pragma once

#include <cstdint>

namespace fsm {

/**
 * @brief Effector operating states.
 *
 * Represents the state machine of an effector subsystem as described
 * in REQUIREMENTS.md REQ-001 and REQ-006.
 */
enum class EffectorState : uint8_t {
    SAFE     = 0,  ///< Default safe state. Initial state on construction.
    ARMED    = 1,  ///< Authorization confirmed. Transition from SAFE only.
    TRACKING = 2,  ///< Target lock acquired. Transition from ARMED only.
    ENGAGED  = 3,  ///< Fire authorized and executed. Transition from TRACKING only.
    FAULT    = 4,  ///< Error terminal state. Reachable from any state.
    COUNT    = 5   ///< Sentinel — do not use as a state.
};

/// Returns a human-readable string for a given state.
constexpr const char* stateToString(EffectorState s) noexcept {
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
