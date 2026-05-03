#include "fsm/FSM.hpp"
#include <cstdio>
#include <ctime>

namespace fsm {

// LEARN: the bit after `:` is the *member initialiser list*. Use it to
// construct members in place. Code in the body `{}` would assign to already-
// default-constructed members, which is wasteful for non-trivial types.
FSM::FSM(EffectorState initial) noexcept
    : current_(initial)
{
    // LEARN: `std::array::fill` overwrites every element. `{}` is a value-
    // initialised default-constructed instance — for `Transition` that
    // means default fields (`from = SAFE`, empty `std::function`s).
    transitions_.fill({});
    enterCbs_.fill(nullptr);
    exitCbs_.fill(nullptr);
}

bool FSM::addTransition(Transition t) noexcept {
    if (transitionCount_ >= MAX_TRANSITIONS) {
        return false;
    }
    // LEARN: `std::move` is a *cast*, not an action. It tells the compiler
    // "you may treat this argument as a temporary and steal its insides".
    // Important when `Transition` carries `std::function`s with captured
    // state — moving avoids copying that state.
    transitions_[transitionCount_++] = std::move(t);
    return true;
}

bool FSM::addUniversalErrorTransition() noexcept {
    for (std::size_t i = 0; i < static_cast<std::size_t>(EffectorState::COUNT); ++i) {
        const auto state = static_cast<EffectorState>(i);
        if (state == EffectorState::FAULT) continue;
        if (findTransition(state, EffectorEvent::ERROR) != nullptr) continue;
        if (!addTransition({ state, EffectorEvent::ERROR, EffectorState::FAULT, nullptr, nullptr })) {
            return false;
        }
    }
    return true;
}

void FSM::onEnter(EffectorState s, StateCallback cb) noexcept {
    enterCbs_[static_cast<std::size_t>(s)] = std::move(cb);
}

void FSM::onExit(EffectorState s, StateCallback cb) noexcept {
    exitCbs_[static_cast<std::size_t>(s)] = std::move(cb);
}

void FSM::onError(ErrorCallback cb) noexcept {
    errorCb_ = std::move(cb);
}

// LEARN: `dispatch` is the heart of the FSM. Read the body top-to-bottom — the
// ordering (lookup → guard → exit cb → action → state change → log → entry cb)
// is intentional and is what the tests assert.
bool FSM::dispatch(EffectorEvent event) noexcept {
    // LEARN: `const T*` means "pointer to immutable T". Returning a pointer
    // (instead of a reference) lets us return `nullptr` to mean "no match" —
    // references can never be null.
    const Transition* t = findTransition(current_, event);

    if (t == nullptr) {
        // LEARN: testing a `std::function` for truthiness asks "do I hold a
        // callable?". An empty `std::function` is falsy.
        if (errorCb_) {
            errorCb_(current_, event);
        }
        return false;
    }

    // Evaluate guard (REQ-005).
    // LEARN: short-circuit `&&` — if `t->guard` is empty (falsy), we never
    // call `t->guard()`. Order matters here.
    if (t->guard && !t->guard()) {
        if (errorCb_) {
            errorCb_(current_, event);
        }
        return false;
    }

    const EffectorState from = current_;
    const EffectorState to   = t->to;

    // Exit callback for current state (REQ-004).
    // LEARN: `auto&` deduces the type and binds by reference, avoiding a copy
    // of the `std::function`. `auto` (no `&`) would copy.
    auto& exitCb = exitCbs_[static_cast<std::size_t>(from)];
    if (exitCb) {
        exitCb(from);
    }

    // Run transition action.
    if (t->action) {
        t->action();
    }

    // Change state (REQ-001).
    current_ = to;

    // Log transition (REQ-009).
    logTransition(from, event, to);

    // Entry callback for new state (REQ-004).
    auto& enterCb = enterCbs_[static_cast<std::size_t>(to)];
    if (enterCb) {
        enterCb(to);
    }

    return true;
}

EffectorState FSM::currentState() const noexcept {
    return current_;
}

std::size_t FSM::transitionCount() const noexcept {
    return transitionCount_;
}

// LEARN: linear search is fine for `MAX_TRANSITIONS = 32`. If the table grew
// large, you'd switch to a hash map keyed by `(state, event)` — but that
// would heap-allocate, breaking REQ-007. Pick the data structure that fits
// the constraints, not the one that "feels" fastest.
const Transition* FSM::findTransition(EffectorState from, EffectorEvent event) const noexcept {
    for (std::size_t i = 0; i < transitionCount_; ++i) {
        // LEARN: `const auto&` again — read-only reference to the element.
        const auto& t = transitions_[i];
        if (t.from == from && t.event == event) {
            return &t;  // address-of operator yields a pointer
        }
    }
    return nullptr;
}

void FSM::logTransition(EffectorState from, EffectorEvent event, EffectorState to) noexcept {
    // localtime_r is thread-safe; std::localtime returns a shared static buffer.
    std::time_t now = std::time(nullptr);
    std::tm tmbuf{};
    ::localtime_r(&now, &tmbuf);
    char ts[20] = {};
    std::strftime(ts, sizeof(ts), "%H:%M:%S", &tmbuf);
    std::printf("[%s] %s --%s--> %s\n",
        ts,
        stateToString(from),
        eventToString(event),
        stateToString(to));
}

} // namespace fsm
