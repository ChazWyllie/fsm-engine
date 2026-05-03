// LEARN: this file is your end-to-end tour. It exercises every public part
// of the FSM API: construction, transitions, guards, actions, callbacks,
// the universal-error helper, and dispatch.
//
// Build (after the main project is configured):
//   cmake --build build --target effector_demo
//   ./build/effector_demo
//
// Output is the timestamped REQ-009 log from each successful transition,
// plus a final state line.

// LEARN: double-quote includes search project headers first. The path
// `fsm/FSM.hpp` resolves because CMake added `include/` to the include
// search path via target_include_directories.
#include "fsm/FSM.hpp"
// LEARN: angle-bracket includes come from system / standard library.
// `<cstdio>` is the C printf family, repackaged into the `std` namespace.
#include <cstdio>

// LEARN: every C++ program has one `main`. Returning 0 means success.
int main() {
    // LEARN: bringing the `fsm` namespace into scope locally. Inside
    // `main` we can now write `FSM` instead of `fsm::FSM`.
    using namespace fsm;

    // LEARN: stack-allocated FSM. No `new`. When `main` returns, the
    // destructor runs automatically and any owned resources are released.
    // This is RAII — Resource Acquisition Is Initialisation.
    FSM fsm(EffectorState::SAFE);

    // Mission transitions.
    // LEARN: the `{ ... }` here is brace initialisation of a `Transition`
    // struct. Fields fill in declaration order: from, event, to, guard, action.
    fsm.addTransition({ EffectorState::SAFE,     EffectorEvent::ARM,       EffectorState::ARMED,    nullptr, nullptr });
    fsm.addTransition({ EffectorState::ARMED,    EffectorEvent::SAFE,      EffectorState::SAFE,     nullptr, nullptr });
    // LEARN: `[] { return true; }` is the smallest possible lambda — a
    // callable taking no arguments, returning `bool`. It is implicitly
    // converted into a `std::function<bool()>` to fill the guard slot.
    fsm.addTransition({ EffectorState::ARMED,    EffectorEvent::LOCK,      EffectorState::TRACKING, [] { return true; }, nullptr });
    fsm.addTransition({ EffectorState::TRACKING, EffectorEvent::LOSE_LOCK, EffectorState::ARMED,    nullptr, nullptr });
    fsm.addTransition({ EffectorState::TRACKING, EffectorEvent::FIRE,      EffectorState::ENGAGED,  [] { return true; }, nullptr });
    fsm.addTransition({ EffectorState::ENGAGED,  EffectorEvent::RESET,     EffectorState::SAFE,     nullptr, nullptr });
    fsm.addTransition({ EffectorState::FAULT,    EffectorEvent::RESET,     EffectorState::SAFE,     nullptr, nullptr });

    // Wire ERROR -> FAULT from every non-FAULT state in one call (REQ-006).
    fsm.addUniversalErrorTransition();

    // Per-state callbacks (REQ-004).
    // LEARN: the lambda parameter `[](EffectorState)` accepts the new state
    // by value. We don't bind it to a name because we don't use it in the
    // body — fine, and silences `-Wno-unused-parameter`.
    fsm.onEnter(EffectorState::ARMED,   [](EffectorState) { std::puts(">> entered ARMED");   });
    fsm.onEnter(EffectorState::ENGAGED, [](EffectorState) { std::puts(">> entered ENGAGED"); });

    // Error handler for invalid events / blocked guards (REQ-003).
    // LEARN: a longer lambda takes both args and uses them. `std::printf`
    // is variadic — same format strings as C printf.
    fsm.onError([](EffectorState s, EffectorEvent e) {
        std::printf("!! invalid event %s in state %s\n", eventToString(e), stateToString(s));
    });

    // Drive a happy-path mission, then fault recovery.
    // LEARN: `dispatch` returns `bool`. We ignore it here for brevity, but
    // production code should usually check it (or wrap it in an assertion).
    fsm.dispatch(EffectorEvent::ARM);
    fsm.dispatch(EffectorEvent::LOCK);
    fsm.dispatch(EffectorEvent::FIRE);
    fsm.dispatch(EffectorEvent::ERROR);   // anything can fault
    fsm.dispatch(EffectorEvent::RESET);

    std::printf("final state: %s\n", stateToString(fsm.currentState()));
    return 0;
}
