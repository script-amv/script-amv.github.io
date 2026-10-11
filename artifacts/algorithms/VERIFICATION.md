# Verification

Environment: macOS, AppleClang 14, C++17, and CMake.

- CMake built the enhanced application, automated test executable, and tree benchmark.
- The automated test executable passed. It reports 16 named checks, including
  multi-course prerequisite-cycle rejection and AVL bounds after sorted input.
- The interactive application loaded five sample courses, printed them in order,
  found CSCI400 through lowercase input, and retained that catalog after a failed
  import attempt.
- `tree_benchmark` generated the checked-in comparison results using 1,000 keys.
- The original C++ source in `original/` matches the source selected from the
  CS 300 repository.

Run the commands in README to reproduce the normal build, test, and benchmark.
The results describe this compiler and input size. They do not measure database
performance or establish complete security coverage.
