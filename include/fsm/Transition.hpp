#pragma once

#include "State.hpp"
#include "Event.hpp"
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
struct Transition {
    EffectorState from;                  ///< Source state.
    EffectorEvent event;                 ///< Triggering event.
    EffectorState to;                    ///< Destination state.
    std::function<bool()> guard;         ///< Optional. nullptr = always allow.
    std::function<void()> action;        ///< Optional. Runs on successful transition.
};

} // namespace fsm
