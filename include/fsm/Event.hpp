#pragma once

#include <cstdint>

namespace fsm {

/**
 * @brief Events that drive state transitions.
 *
 * Each event maps to one or more valid transitions depending on
 * the current state. See REQUIREMENTS.md REQ-002.
 */
enum class EffectorEvent : uint8_t {
    ARM       = 0,  ///< Request transition from SAFE to ARMED.
    SAFE      = 1,  ///< Request transition from ARMED back to SAFE.
    LOCK      = 2,  ///< Request transition from ARMED to TRACKING (requires guard).
    LOSE_LOCK = 3,  ///< Request transition from TRACKING back to ARMED.
    FIRE      = 4,  ///< Request transition from TRACKING to ENGAGED (requires guard).
    RESET     = 5,  ///< Request transition from ENGAGED or FAULT back to SAFE.
    ERROR     = 6,  ///< Drive FSM into FAULT from any state.
    COUNT     = 7   ///< Sentinel — do not use as an event.
};

/// Returns a human-readable string for a given event.
constexpr const char* eventToString(EffectorEvent e) noexcept {
    switch (e) {
        case EffectorEvent::ARM:       return "ARM";
        case EffectorEvent::SAFE:      return "SAFE";
        case EffectorEvent::LOCK:      return "LOCK";
        case EffectorEvent::LOSE_LOCK: return "LOSE_LOCK";
        case EffectorEvent::FIRE:      return "FIRE";
        case EffectorEvent::RESET:     return "RESET";
        case EffectorEvent::ERROR:     return "ERROR";
        default:                       return "UNKNOWN";
    }
}

} // namespace fsm
