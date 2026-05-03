#include "fsm/FSM.hpp"
#include <cstdio>
#include <ctime>

namespace fsm {

FSM::FSM(EffectorState initial) noexcept
    : current_(initial)
{
    transitions_.fill({});
    enterCbs_.fill(nullptr);
    exitCbs_.fill(nullptr);
}

bool FSM::addTransition(Transition t) noexcept {
    if (transitionCount_ >= MAX_TRANSITIONS) {
        return false;
    }
    transitions_[transitionCount_++] = std::move(t);
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

bool FSM::dispatch(EffectorEvent event) noexcept {
    const Transition* t = findTransition(current_, event);

    if (t == nullptr) {
        if (errorCb_) {
            errorCb_(current_, event);
        }
        return false;
    }

    // Evaluate guard (REQ-005).
    if (t->guard && !t->guard()) {
        if (errorCb_) {
            errorCb_(current_, event);
        }
        return false;
    }

    const EffectorState from = current_;
    const EffectorState to   = t->to;

    // Exit callback for current state (REQ-004).
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

const Transition* FSM::findTransition(EffectorState from, EffectorEvent event) const noexcept {
    for (std::size_t i = 0; i < transitionCount_; ++i) {
        const auto& t = transitions_[i];
        if (t.from == from && t.event == event) {
            return &t;
        }
    }
    return nullptr;
}

void FSM::logTransition(EffectorState from, EffectorEvent event, EffectorState to) noexcept {
    std::time_t now = std::time(nullptr);
    char ts[20] = {};
    std::strftime(ts, sizeof(ts), "%H:%M:%S", std::localtime(&now));
    std::printf("[%s] %s --%s--> %s\n",
        ts,
        stateToString(from),
        eventToString(event),
        stateToString(to));
}

} // namespace fsm
