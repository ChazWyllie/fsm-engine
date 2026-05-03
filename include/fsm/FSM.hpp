#pragma once

#include "State.hpp"
#include "Event.hpp"
#include "Transition.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ctime>

namespace fsm {

/// Maximum number of transitions that can be registered.
/// Fixed at compile time — no heap allocation after init (REQ-007, REQ-008).
static constexpr std::size_t MAX_TRANSITIONS = 32;

/**
 * @brief Finite state machine for an effector subsystem.
 *
 * Usage:
 * @code
 *   FSM fsm(EffectorState::SAFE);
 *
 *   fsm.addTransition({ EffectorState::SAFE, EffectorEvent::ARM,
 *                        EffectorState::ARMED, nullptr, nullptr });
 *
 *   fsm.onEnter(EffectorState::ARMED, [](EffectorState s) {
 *       printf("Entered ARMED\n");
 *   });
 *
 *   fsm.dispatch(EffectorEvent::ARM);
 * @endcode
 */
class FSM {
public:
    using StateCallback = std::function<void(EffectorState)>;
    using ErrorCallback = std::function<void(EffectorState, EffectorEvent)>;

    /// @brief Construct FSM with a fixed initial state.
    explicit FSM(EffectorState initial) noexcept;

    /**
     * @brief Register a transition.
     * @return true if added, false if transition table is full.
     */
    bool addTransition(Transition t) noexcept;

    /// @brief Register a callback invoked on entry to state `s`.
    void onEnter(EffectorState s, StateCallback cb) noexcept;

    /// @brief Register a callback invoked on exit from state `s`.
    void onExit(EffectorState s, StateCallback cb) noexcept;

    /**
     * @brief Register a callback invoked when an invalid or guard-blocked
     *        event is dispatched. See REQ-003.
     */
    void onError(ErrorCallback cb) noexcept;

    /**
     * @brief Dispatch an event to the FSM.
     *
     * Looks up a matching transition from the current state.
     * Evaluates the guard if present.
     * Fires exit callback, runs action, changes state, fires entry callback.
     * Logs the transition (REQ-009).
     *
     * @return true if the transition completed successfully.
     *         false if no transition matched or the guard returned false.
     */
    bool dispatch(EffectorEvent event) noexcept;

    /// @brief Returns the current state (REQ-001).
    EffectorState currentState() const noexcept;

    /// @brief Returns number of registered transitions.
    std::size_t transitionCount() const noexcept;

private:
    EffectorState current_;

    std::array<Transition, MAX_TRANSITIONS> transitions_;
    std::size_t transitionCount_{0};

    // Per-state entry/exit callbacks indexed by EffectorState enum value.
    std::array<StateCallback, static_cast<std::size_t>(EffectorState::COUNT)> enterCbs_;
    std::array<StateCallback, static_cast<std::size_t>(EffectorState::COUNT)> exitCbs_;

    ErrorCallback errorCb_;

    /// Find the first matching registered transition. Returns nullptr if none found.
    const Transition* findTransition(EffectorState from, EffectorEvent event) const noexcept;

    /// Log a completed transition to stdout (REQ-009).
    static void logTransition(EffectorState from, EffectorEvent event, EffectorState to) noexcept;
};

} // namespace fsm
