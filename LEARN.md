# Learn C++ Through This Repo

This repo is a real, working safety-critical FSM (finite state machine).
It uses ~every C++ feature a working engineer needs — and nothing more.
Read this file top to bottom, opening each cited file as you go.

If you have never compiled C++ before, do this first:

```bash
brew install cmake googletest
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/effector_demo
```

When the demo prints state transitions, you have a working toolchain.

---

## How to read this guide

Every concept points at a real file:line in this repo. Open the file, read
the surrounding code, and come back. Inline `// LEARN:` comments in the
source give one-line nudges. This document gives the deeper why.

The full reading order is:

1. [`include/fsm/State.hpp`](include/fsm/State.hpp) — enums, headers, namespaces
2. [`include/fsm/Event.hpp`](include/fsm/Event.hpp) — same patterns, repeat for fluency
3. [`include/fsm/Transition.hpp`](include/fsm/Transition.hpp) — `struct`, `std::function`, lambdas
4. [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) — class, members, `noexcept`, `std::array`
5. [`src/FSM.cpp`](src/FSM.cpp) — definitions, control flow, `static`
6. [`tests/test_fsm.cpp`](tests/test_fsm.cpp) — GoogleTest, lambda captures, integration
7. [`examples/effector_demo.cpp`](examples/effector_demo.cpp) — putting it all together
8. [`CMakeLists.txt`](CMakeLists.txt) — how the build glues it together

---

## 1. Headers, source files, and `#pragma once`

C++ splits code into **header files** (`.hpp`) that declare what exists and
**source files** (`.cpp`) that define how it works. Other files include
headers but link against the compiled source.

See [`include/fsm/State.hpp`](include/fsm/State.hpp). The first line is:

```cpp
#pragma once
```

This is an **include guard**. If two files both `#include "State.hpp"`, the
preprocessor would otherwise paste its contents twice and you would get
"redefinition" errors. `#pragma once` tells the compiler "include me at
most once per compilation unit". The portable alternative is the older
`#ifndef FOO_HPP / #define FOO_HPP / #endif` triple.

`#include <cstdint>` pulls in fixed-width integer types like `uint8_t`.
Angle brackets `<...>` mean "search system include paths"; double quotes
`"..."` mean "search project include paths first, then system".

## 2. Namespaces

```cpp
namespace fsm {
    // ...
}
```

A **namespace** is a labelled scope. Everything inside `namespace fsm {}` is
named `fsm::SomeName` from the outside. This stops your `State` from
clashing with somebody else's `State` when you both end up in the same
translation unit. Use namespaces for libraries; do not pollute the global
scope.

Inside a `.cpp` file or test you can shorthand it:

```cpp
using namespace fsm;  // tests/test_fsm.cpp:4
```

Avoid `using namespace` in headers — it leaks into every file that
includes them.

## 3. `enum class`

```cpp
enum class EffectorState : uint8_t {
    SAFE = 0, ARMED = 1, /* ... */
};
```

Three things to learn here, all in [`include/fsm/State.hpp`](include/fsm/State.hpp):

- **`enum class`** (a C++11 *scoped enum*) is strongly typed: you cannot
  accidentally pass an `EffectorState` where an `int` is expected, and you
  must write `EffectorState::SAFE`, never bare `SAFE`. Old-style C `enum`s
  leak their names into the surrounding scope and silently convert to int.
- **`: uint8_t`** is the *underlying type*. The enum is stored as one byte
  instead of the default `int` (4 bytes). On embedded hardware this matters.
- **`COUNT = 5`** is a *sentinel* — a fake last enumerator whose only job
  is to give us the array size. We use it on
  [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) line 88 to size the callback
  arrays. If you add a new state, `COUNT` increments automatically and the
  arrays grow with it.

## 4. `constexpr` and `switch`

```cpp
constexpr const char* stateToString(EffectorState s) noexcept { ... }
```

In [`include/fsm/State.hpp`](include/fsm/State.hpp):

- **`constexpr`** means "computable at compile time". For trivial functions
  the compiler can evaluate them during compilation, which is faster and
  lets you use the result in places that demand compile-time constants.
- **`const char*`** is a pointer to immutable characters — this is how
  string *literals* like `"SAFE"` are typed. You cannot modify them.
- **`switch (s)`** on an enum is the idiomatic way to map enum values to
  something else. The `default:` branch handles the `COUNT` sentinel and
  any future enum value you forget to update.

## 5. `struct` vs `class`

[`include/fsm/Transition.hpp`](include/fsm/Transition.hpp) defines a `struct`:

```cpp
struct Transition {
    EffectorState from;
    EffectorEvent event;
    EffectorState to;
    std::function<bool()> guard;
    std::function<void()> action;
};
```

A `struct` and a `class` are *almost* the same thing in C++. The only
language-level difference is the default access level: `struct` defaults
to `public`, `class` defaults to `private`. By convention:

- Use `struct` for plain data with no invariants — anyone can read/write
  any field.
- Use `class` when you have invariants to enforce (the data members in
  [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp) are `private` so callers
  can't bypass `dispatch()`).

You construct a `Transition` with **brace initialisation**:

```cpp
{ EffectorState::SAFE, EffectorEvent::ARM, EffectorState::ARMED, nullptr, nullptr }
```

The braces fill the fields in declaration order. `nullptr` is the typed
"null" — never use the old `NULL` macro in modern C++.

## 6. `std::function` and lambdas

```cpp
std::function<bool()> guard;
std::function<void()> action;
```

`std::function` is a **type-erased callable**. It can hold *anything*
that can be called with the given signature: a free function, a lambda,
a member function bound with `std::bind`, a struct with `operator()`.

A **lambda** is an anonymous function literal. The cleanest example is in
[`tests/test_fsm.cpp`](tests/test_fsm.cpp) line 87:

```cpp
[&] { return authorized; }
```

Read it as:

- `[&]` — *capture by reference*: the lambda body can see and modify
  `authorized` from the enclosing scope. Other capture forms:
  - `[]` capture nothing
  - `[=]` capture everything by value (a copy is taken)
  - `[&authorized]` capture only `authorized`, by reference
  - `[this]` capture the enclosing object's `this` pointer
- `{ return authorized; }` — the body. Returns a `bool`.

**Capture pitfall:** `[&]` captures references. If the lambda outlives
the captured variable (e.g. you store it and the local goes out of scope),
you have a dangling reference and undefined behaviour. The tests are
fine because the lambda's lifetime is bounded by the test scope.

## 7. `std::array` — the heap-free vector

```cpp
std::array<Transition, MAX_TRANSITIONS> transitions_;  // FSM.hpp:84
```

In [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp):

- **`std::array<T, N>`** is a fixed-size array. The size `N` is part of
  the type and known at compile time. Unlike `std::vector`, no heap
  allocation ever happens.
- This is why REQ-007 ("no heap after init") is satisfied just by writing
  `std::array` — there is nothing to allocate later.
- The trailing underscore `transitions_` is a project convention for
  *member variables*. Other codebases use `m_transitions` or no prefix.
  Pick one and be consistent.

## 8. Class anatomy

Look at the layout of `class FSM` in [`include/fsm/FSM.hpp`](include/fsm/FSM.hpp):

```cpp
class FSM {
public:
    using StateCallback = std::function<void(EffectorState)>;  // type alias
    explicit FSM(EffectorState initial) noexcept;              // constructor
    bool addTransition(Transition t) noexcept;                 // mutator
    EffectorState currentState() const noexcept;               // accessor
private:
    EffectorState current_;                                    // state
    void logTransition(...) noexcept;                          // private helper (static)
};
```

Key vocabulary:

- **`public:` / `private:`** — access labels. Public members are part of
  the API; private members are implementation. Switching access is one
  of the cheapest ways to lock down a design.
- **`using Foo = ...;`** — a *type alias*. Same as the older `typedef`,
  but reads left-to-right.
- **`explicit`** on a single-arg constructor stops the compiler from doing
  silent implicit conversions. Without it, `FSM x = EffectorState::SAFE`
  would compile; with it, you must write `FSM x(EffectorState::SAFE)`.
- **`noexcept`** is a promise: this function will not throw an exception.
  The compiler can sometimes generate better code, and code that uses
  the function knows it is safe to call from destructors and signal
  handlers. Lying about `noexcept` (throwing anyway) calls
  `std::terminate`.
- **`const` after a method** (`currentState() const`) means "this method
  does not modify the object". It is the cornerstone of writing safe APIs:
  you can call `const` methods on a `const` object.

## 9. The constructor and member initialisation

In [`src/FSM.cpp`](src/FSM.cpp):

```cpp
FSM::FSM(EffectorState initial) noexcept
    : current_(initial)
{
    transitions_.fill({});
    enterCbs_.fill(nullptr);
    exitCbs_.fill(nullptr);
}
```

The `:` after the signature opens the **member initialiser list**. This
is where you *initialise* members directly. Anything in the body `{}` is
*assignment after default-construction*, which is wasteful. Always
prefer the initialiser list.

`transitions_.fill({})` overwrites every slot with a default-constructed
`Transition`. Defensible defaults for every member are good practice.

## 10. Linear search and `const` pointers

[`src/FSM.cpp`](src/FSM.cpp) `findTransition` returns `const Transition*`.
Two things:

- **Returning a pointer (not a reference) so we can return `nullptr`.**
  References cannot be null. Pointers can. If "no match" is a real
  outcome, you need a pointer or a `std::optional<...>`.
- **`const Transition*`** means the caller cannot modify what it points
  at. The transition table is owned by the FSM; callers only get to peek.

The function itself is a textbook linear scan — fine because
`MAX_TRANSITIONS = 32` is tiny. If it grew, you would switch to a hash
map keyed by `(state, event)`, but that would allocate, breaking REQ-007.

## 11. Dispatch — the heart of the FSM

`FSM::dispatch` in [`src/FSM.cpp`](src/FSM.cpp) is the function that runs
on every event. Read it line by line. The shape:

1. Look up the transition. No match → fire error, return false.
2. Evaluate the guard. False → fire error, return false.
3. Run exit callback for the *current* state.
4. Run the transition action.
5. Update `current_`.
6. Log.
7. Run entry callback for the *new* state.

The order matters and is testable. See `EntryExitCallbacksFire` in
[`tests/test_fsm.cpp`](tests/test_fsm.cpp).

## 12. `std::move` — transfer of ownership

```cpp
transitions_[transitionCount_++] = std::move(t);
```

`std::move` does not actually *move* anything. It is a cast that says
"treat this as an rvalue — feel free to gut it". The receiving side is
allowed to steal internals (e.g. `std::function`'s captured state) and
leave `t` in a valid-but-unspecified state.

Use `std::move` when you are about to throw a value away anyway. It is
how you avoid expensive copies.

## 13. RAII (Resource Acquisition Is Initialisation)

Not directly demonstrated in this codebase because we deliberately avoid
heap allocation, but everywhere you see `std::array`, `std::function`,
`std::string`: those types clean themselves up when their owning object
is destroyed. You never write `delete`. You never write `free`.

This is **the** central idea in modern C++ resource management. The day
you reach for `new` or `malloc`, ask first: can I make this a stack
object whose destructor does the cleanup? Almost always: yes.

## 14. Testing with GoogleTest

[`tests/test_fsm.cpp`](tests/test_fsm.cpp) is the entire test suite.
Patterns to internalise:

```cpp
TEST(FSMTest, SingleActiveState) {
    FSM fsm = buildEffectorFSM();
    EXPECT_EQ(fsm.currentState(), EffectorState::SAFE);
    fsm.dispatch(EffectorEvent::ARM);
    EXPECT_EQ(fsm.currentState(), EffectorState::ARMED);
}
```

- **`TEST(SuiteName, TestName)`** declares one test. GoogleTest finds it
  automatically — no manual registration.
- **`EXPECT_EQ` / `EXPECT_TRUE` / `EXPECT_NE`** assert; on failure they
  log and continue. Use `ASSERT_*` when continuing past a failure would
  cause a crash (e.g. dereferencing a null pointer).
- **Test setup helpers** like `buildEffectorFSM()` keep tests readable.
  When several tests share setup, this is fine; when they share *state*,
  use a `class FSMTest : public ::testing::Test` fixture.

## 15. CMake — what `cmake -B build` actually does

[`CMakeLists.txt`](CMakeLists.txt) is the build description. Read it
top to bottom — it is short:

- `cmake_minimum_required` is a compatibility floor.
- `project(fsm_engine CXX)` names the project and declares it C++.
- `set(CMAKE_CXX_STANDARD 17)` picks the language standard.
- `add_compile_options(...)` sets warning flags. `-Wall -Wextra
  -Wpedantic` should be the default in every C++ project you write.
- `add_library(fsm STATIC src/FSM.cpp)` builds a static library `fsm.a`.
- `target_include_directories(fsm PUBLIC include)` says: anyone linking
  to `fsm` automatically gets `include/` on their header search path.
- `add_executable` + `target_link_libraries` build binaries.
- `enable_testing()` + `add_test(...)` registers a test with CTest.

`cmake -B build` *configures*: it generates Makefiles in `build/`.
`cmake --build build` *compiles*: it runs the generator-specific build.

## 16. Where to go next

You now have enough vocabulary to read the whole codebase. The
[`exercises/`](exercises/) directory contains hands-on tasks of
increasing difficulty. Start with `E1_add_state.md`, then work through
in order.

When something compiles but does not behave as expected, you have one
of three problems:

1. **A type problem** — the compiler will catch most of these for you.
2. **A lifetime problem** — read your captures and your pointers.
3. **A logic problem** — write a test that fails for the right reason,
   then fix the code.

The whole point of TDD (which produced this codebase) is to make the
third category cheap.
