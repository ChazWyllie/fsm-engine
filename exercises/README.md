# Exercises

Hands-on tasks that get harder as you go. Work through them in order.

For each one:

1. Read the task.
2. Open the cited files. Make the changes.
3. Build (`cmake --build build`) and run tests (`ctest --test-dir build`).
4. Compare your work to `solutions/EN_*.md`.

Try not to peek at the solution until you've spent at least 15 minutes.
The struggle is where you actually learn C++.

| #  | File | Topic | Difficulty |
|----|------|-------|------------|
| E1 | [E1_add_state.md](E1_add_state.md) | Adding a new state to the enum | ★ |
| E2 | [E2_count_transitions.md](E2_count_transitions.md) | Reading the public API | ★ |
| E3 | [E3_strict_guard.md](E3_strict_guard.md) | Using guards and lambda captures | ★★ |
| E4 | [E4_history_buffer.md](E4_history_buffer.md) | Fixed-size circular buffer | ★★★ |
| E5 | [E5_const_correctness.md](E5_const_correctness.md) | Hunting `const` bugs | ★★★★ |

After E5 you will have touched: enums, headers, classes, `std::array`,
`std::function`, lambdas with captures, `const`-correctness, and
GoogleTest. That covers the working-engineer 80%.
