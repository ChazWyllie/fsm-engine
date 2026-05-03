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

// LEARN: `static constexpr` at namespace scope creates a compile-time constant
// with internal linkage. `std::size_t` is the unsigned integer type used for
// sizes and indices on this platform (`std::array` and `sizeof` both yield it).
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
// LEARN: `class` (vs `struct`) defaults its members to private, so we
// explicitly write `public:` and `private:` to organise the API surface.
class FSM {
public:
    // LEARN: `using Foo = ...;` is a type alias, like `typedef` but reads
    // left-to-right. The aliases below make later signatures shorter.
    using StateCallback = std::function<void(EffectorState)>;
    using ErrorCallback = std::function<void(EffectorState, EffectorEvent)>;

    // LEARN: `explicit` blocks implicit conversions. Without it, the line
    // `FSM x = EffectorState::SAFE;` would compile (silent conversion). With
    // it, you must write `FSM x(EffectorState::SAFE);` — caller must mean it.
    /// @brief Construct FSM with a fixed initial state.
    explicit FSM(EffectorState initial) noexcept;

    /**
     * @brief Register a transition.
     * @return true if added, false if transition table is full.
     */
    bool addTransition(Transition t) noexcept;

    /**
     * @brief Register an ERROR → FAULT transition from every non-FAULT state.
     *
     * Enforces REQ-006: the FAULT terminal state is reachable from any state via
     * the ERROR event. Skips states that already have an ERROR transition
     * registered (so it composes with manual setup). No-op for FAULT itself.
     *
     * @return true if all required entries fit in the transition table.
     */
    bool addUniversalErrorTransition() noexcept;

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

    // LEARN: the trailing `const` after the parameter list means "this method
    // does not modify the object". You can call `const` methods on `const FSM`
    // references — `const`-correctness is one of the cheapest ways to express
    // intent and let the compiler catch mistakes for you.
    /// @brief Returns the current state (REQ-001).
    EffectorState currentState() const noexcept;

    /// @brief Returns number of registered transitions.
    std::size_t transitionCount() const noexcept;

// LEARN: Below `private:` is implementation detail — invisible to callers.
// Members named with a trailing underscore (`current_`) are this project's
// convention for "this is a member variable, not a local or a parameter".
private:
    EffectorState current_;

    // LEARN: `std::array<T, N>` is a fixed-size array stored inline (no heap
    // allocation). Compare with `std::vector<T>` which manages a heap buffer
    // that can grow — banned here because of REQ-007/REQ-008.
    std::array<Transition, MAX_TRANSITIONS> transitions_;
    // LEARN: `{0}` is an in-class default member initialiser. Without it,
    // an `int`-like member would be default-initialised to garbage.
    std::size_t transitionCount_{0};

    // Per-state entry/exit callbacks indexed by EffectorState enum value.
    // LEARN: `static_cast<size_t>(EffectorState::COUNT)` converts the scoped
    // enumerator to its underlying integer. Scoped enums require explicit
    // casts — that explicitness is the safety feature.
    std::array<StateCallback, static_cast<std::size_t>(EffectorState::COUNT)> enterCbs_;
    std::array<StateCallback, static_cast<std::size_t>(EffectorState::COUNT)> exitCbs_;

    ErrorCallback errorCb_;

    /// Find the first matching registered transition. Returns nullptr if none found.
    const Transition* findTransition(EffectorState from, EffectorEvent event) const noexcept;

    /// Log a completed transition to stdout (REQ-009).
    static void logTransition(EffectorState from, EffectorEvent event, EffectorState to) noexcept;
};

} // namespace fsm
