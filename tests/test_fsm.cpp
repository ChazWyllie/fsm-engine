#include "fsm/FSM.hpp"
#include <gtest/gtest.h>

using namespace fsm;

/// Helper: build a fully-wired FSM with all standard effector transitions.
static FSM buildEffectorFSM() {
    FSM fsm(EffectorState::SAFE);

    // Standard transitions.
    fsm.addTransition({ EffectorState::SAFE,     EffectorEvent::ARM,       EffectorState::ARMED,    nullptr, nullptr });
    fsm.addTransition({ EffectorState::ARMED,    EffectorEvent::SAFE,      EffectorState::SAFE,     nullptr, nullptr });
    fsm.addTransition({ EffectorState::ARMED,    EffectorEvent::LOCK,      EffectorState::TRACKING, nullptr, nullptr });
    fsm.addTransition({ EffectorState::TRACKING, EffectorEvent::LOSE_LOCK, EffectorState::ARMED,    nullptr, nullptr });
    fsm.addTransition({ EffectorState::TRACKING, EffectorEvent::FIRE,      EffectorState::ENGAGED,  nullptr, nullptr });
    fsm.addTransition({ EffectorState::ENGAGED,  EffectorEvent::RESET,     EffectorState::SAFE,     nullptr, nullptr });
    fsm.addTransition({ EffectorState::FAULT,    EffectorEvent::RESET,     EffectorState::SAFE,     nullptr, nullptr });

    // ERROR reachable from any non-fault state (REQ-006).
    for (auto s : { EffectorState::SAFE, EffectorState::ARMED,
                    EffectorState::TRACKING, EffectorState::ENGAGED }) {
        fsm.addTransition({ s, EffectorEvent::ERROR, EffectorState::FAULT, nullptr, nullptr });
    }

    return fsm;
}

// REQ-001: exactly one active state at a time.
TEST(FSMTest, SingleActiveState) {
    FSM fsm = buildEffectorFSM();
    EXPECT_EQ(fsm.currentState(), EffectorState::SAFE);
    fsm.dispatch(EffectorEvent::ARM);
    EXPECT_EQ(fsm.currentState(), EffectorState::ARMED);
    // Cannot be in both SAFE and ARMED.
    EXPECT_NE(fsm.currentState(), EffectorState::SAFE);
}

// REQ-002: transition only via registered events.
TEST(FSMTest, OnlyRegisteredTransitions) {
    FSM fsm = buildEffectorFSM();
    // FIRE from SAFE has no registered transition.
    bool result = fsm.dispatch(EffectorEvent::FIRE);
    EXPECT_FALSE(result);
    EXPECT_EQ(fsm.currentState(), EffectorState::SAFE);
}

// REQ-003: invalid event fires error callback, does not crash.
TEST(FSMTest, InvalidEventFiresErrorCallback) {
    FSM fsm = buildEffectorFSM();
    bool errorFired = false;
    fsm.onError([&](EffectorState, EffectorEvent) { errorFired = true; });
    fsm.dispatch(EffectorEvent::FIRE); // invalid from SAFE
    EXPECT_TRUE(errorFired);
}

// REQ-004: entry and exit callbacks fire on valid transition.
TEST(FSMTest, EntryExitCallbacksFire) {
    FSM fsm = buildEffectorFSM();
    bool exitSafe  = false;
    bool enterArmed = false;

    fsm.onExit(EffectorState::SAFE,  [&](EffectorState) { exitSafe   = true; });
    fsm.onEnter(EffectorState::ARMED, [&](EffectorState) { enterArmed = true; });

    fsm.dispatch(EffectorEvent::ARM);

    EXPECT_TRUE(exitSafe);
    EXPECT_TRUE(enterArmed);
}

// REQ-004: callbacks do NOT fire on a failed/blocked transition.
TEST(FSMTest, CallbacksDoNotFireOnFailedTransition) {
    FSM fsm = buildEffectorFSM();
    bool exitFired = false;
    fsm.onExit(EffectorState::SAFE, [&](EffectorState) { exitFired = true; });
    fsm.dispatch(EffectorEvent::FIRE); // invalid
    EXPECT_FALSE(exitFired);
}

// REQ-005: guard blocks transition when it returns false.
TEST(FSMTest, GuardBlocksTransition) {
    FSM fsm(EffectorState::ARMED);
    bool authorized = false;

    fsm.addTransition({
        EffectorState::ARMED, EffectorEvent::LOCK, EffectorState::TRACKING,
        [&] { return authorized; },
        nullptr
    });

    bool result = fsm.dispatch(EffectorEvent::LOCK);
    EXPECT_FALSE(result);
    EXPECT_EQ(fsm.currentState(), EffectorState::ARMED);

    authorized = true;
    result = fsm.dispatch(EffectorEvent::LOCK);
    EXPECT_TRUE(result);
    EXPECT_EQ(fsm.currentState(), EffectorState::TRACKING);
}

// REQ-005: guard allows transition when it returns true.
TEST(FSMTest, GuardAllowsTransitionWhenTrue) {
    FSM fsm(EffectorState::TRACKING);
    fsm.addTransition({
        EffectorState::TRACKING, EffectorEvent::FIRE, EffectorState::ENGAGED,
        [] { return true; },
        nullptr
    });
    EXPECT_TRUE(fsm.dispatch(EffectorEvent::FIRE));
    EXPECT_EQ(fsm.currentState(), EffectorState::ENGAGED);
}

// REQ-006: ERROR transitions to FAULT from any non-fault state.
TEST(FSMTest, ErrorFromAnyState) {
    for (auto startState : { EffectorState::SAFE, EffectorState::ARMED,
                              EffectorState::TRACKING, EffectorState::ENGAGED }) {
        FSM fsm = buildEffectorFSM();
        // Advance to start state.
        if (startState == EffectorState::ARMED || startState == EffectorState::TRACKING || startState == EffectorState::ENGAGED) {
            fsm.dispatch(EffectorEvent::ARM);
        }
        if (startState == EffectorState::TRACKING || startState == EffectorState::ENGAGED) {
            fsm.dispatch(EffectorEvent::LOCK);
        }
        if (startState == EffectorState::ENGAGED) {
            fsm.dispatch(EffectorEvent::FIRE);
        }

        EXPECT_EQ(fsm.currentState(), startState);
        fsm.dispatch(EffectorEvent::ERROR);
        EXPECT_EQ(fsm.currentState(), EffectorState::FAULT)
            << "Expected FAULT from " << stateToString(startState);
    }
}

// REQ-007 / REQ-008: transition table is fixed size, excess transitions rejected.
TEST(FSMTest, TransitionTableFixedSize) {
    FSM fsm(EffectorState::SAFE);

    // Fill the table to MAX_TRANSITIONS.
    for (std::size_t i = 0; i < MAX_TRANSITIONS; ++i) {
        fsm.addTransition({ EffectorState::SAFE, EffectorEvent::ARM, EffectorState::ARMED, nullptr, nullptr });
    }
    // One more should be rejected.
    bool result = fsm.addTransition({ EffectorState::SAFE, EffectorEvent::ARM, EffectorState::ARMED, nullptr, nullptr });
    EXPECT_FALSE(result);
}

// REQ-010: transition action runs on successful transition.
TEST(FSMTest, TransitionActionRuns) {
    FSM fsm(EffectorState::SAFE);
    bool actionRan = false;

    fsm.addTransition({
        EffectorState::SAFE, EffectorEvent::ARM, EffectorState::ARMED,
        nullptr,
        [&] { actionRan = true; }
    });

    fsm.dispatch(EffectorEvent::ARM);
    EXPECT_TRUE(actionRan);
}

// REQ-009: every successful transition emits a log line containing
// from-state, event, to-state, and a timestamp.
TEST(FSMTest, SuccessfulTransitionEmitsLogLine) {
    FSM fsm(EffectorState::SAFE);
    fsm.addTransition({ EffectorState::SAFE, EffectorEvent::ARM, EffectorState::ARMED, nullptr, nullptr });

    testing::internal::CaptureStdout();
    EXPECT_TRUE(fsm.dispatch(EffectorEvent::ARM));
    const std::string out = testing::internal::GetCapturedStdout();

    EXPECT_NE(out.find("SAFE"), std::string::npos)  << out;
    EXPECT_NE(out.find("ARM"),  std::string::npos)  << out;
    EXPECT_NE(out.find("ARMED"), std::string::npos) << out;
    // Timestamp prefix `[HH:MM:SS]` — at minimum two colons inside brackets.
    EXPECT_NE(out.find('['), std::string::npos) << out;
    EXPECT_NE(out.find(']'), std::string::npos) << out;
}

// REQ-009: failed transitions must NOT emit a log line.
TEST(FSMTest, FailedTransitionEmitsNoLog) {
    FSM fsm = buildEffectorFSM();

    testing::internal::CaptureStdout();
    EXPECT_FALSE(fsm.dispatch(EffectorEvent::FIRE));  // invalid from SAFE
    const std::string out = testing::internal::GetCapturedStdout();

    EXPECT_EQ(out.find("-->"), std::string::npos) << out;
}

// Integration: full mission sequence SAFE -> ARMED -> TRACKING -> ENGAGED -> SAFE.
TEST(FSMTest, FullMissionSequence) {
    FSM fsm = buildEffectorFSM();

    EXPECT_TRUE(fsm.dispatch(EffectorEvent::ARM));
    EXPECT_EQ(fsm.currentState(), EffectorState::ARMED);

    EXPECT_TRUE(fsm.dispatch(EffectorEvent::LOCK));
    EXPECT_EQ(fsm.currentState(), EffectorState::TRACKING);

    EXPECT_TRUE(fsm.dispatch(EffectorEvent::FIRE));
    EXPECT_EQ(fsm.currentState(), EffectorState::ENGAGED);

    EXPECT_TRUE(fsm.dispatch(EffectorEvent::RESET));
    EXPECT_EQ(fsm.currentState(), EffectorState::SAFE);
}

// Integration: fault recovery SAFE -> ARMED -> ERROR -> FAULT -> RESET -> SAFE.
TEST(FSMTest, FaultRecoverySequence) {
    FSM fsm = buildEffectorFSM();

    fsm.dispatch(EffectorEvent::ARM);
    EXPECT_EQ(fsm.currentState(), EffectorState::ARMED);

    fsm.dispatch(EffectorEvent::ERROR);
    EXPECT_EQ(fsm.currentState(), EffectorState::FAULT);

    fsm.dispatch(EffectorEvent::RESET);
    EXPECT_EQ(fsm.currentState(), EffectorState::SAFE);
}
