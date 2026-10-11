# CS 499 Milestone Three — Dmitrii Shostak

This submission contains the original CS 300 Course Planner and an enhanced
C++17 version focused on algorithms and data structures. The separate Word
narrative explains the artifact, the measured improvement, learning, and course
outcome coverage.

## Build, test, and run

Requirements: CMake 3.16 or newer and a C++17 compiler.

    cmake -S enhanced -B build
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/tree_benchmark
    ./build/course_planner

On a Visual Studio multi-configuration build, add `--config Debug` to the
build command and `-C Debug` to CTest. Use `build/Debug/course_planner.exe`.

To demonstrate the application, choose option 1 and enter `sample_courses.csv`.
Choose option 2 to list courses. Choose option 3 and enter `csci400` to view
CSCI400 and its prerequisite CSCI300. The menu accepts lowercase course numbers.

## Algorithm and data-structure enhancement

The original artifact used a plain binary search tree. Its `addNode` method
does not balance the tree. Inserting sorted course numbers therefore creates a
one-sided tree, so insertion and lookup become O(n) in the worst case.

`CourseTree` is now an AVL tree. Each node stores a height. After an insertion,
the program updates heights and applies a left, right, left-right, or right-left
rotation when the balance factor is outside -1 through 1. `unique_ptr` keeps
exclusive ownership during rotations. In-order traversal remains iterative and
still returns courses in sorted order. AVL insertion and lookup are O(log n)
in the worst case.

`CourseService` now uses `unordered_set` membership for course identifiers and
for duplicate prerequisites. This makes prerequisite membership checks O(1) on
average rather than repeated linear scans through a vector. The importer also
rejects missing references, self-prerequisites, repeated prerequisites, duplicate
course identifiers, and multi-course prerequisite cycles. It builds and validates
a staged catalog before replacing the active catalog, so a failed import does not
leave partial data or erase the previous valid catalog.

## Performance evidence

Run `tree_benchmark` to reproduce the measurements in
`benchmark_results.csv`. It generates 1,000 fixed-format course keys and runs
both a plain baseline BST and the AVL tree. It reports height, total insertion
comparisons, and average successful lookup comparisons. Random order uses fixed
seed 499 so the results are reproducible.

For sorted input, the baseline tree reaches height 1,000 and averages 500.50
comparisons per lookup. The AVL tree reaches height 10 and averages 8.99.
For random input, the baseline height is 25 and averages 12.14 comparisons;
the AVL height is 12 and averages 9.30. Comparison counts show the tree-shape
effect without relying on wall-clock timing, which can vary between computers.

## Tests and scope

The test executable checks sorted output, lowercase search, forward references,
missing references, duplicate courses and prerequisites, direct and multi-course
cycles, failed-import preservation, reloading, menu input, CSV errors, and AVL
height and lookup bounds after 2,000 sorted insertions. See `VERIFICATION.md`.

The simple CSV contract remains: `course number,title,optional prerequisite(s)`.
Quoted CSV fields and titles containing commas are rejected. The program is a
local catalog tool. SQLite persistence, CRUD actions, foreign keys, indexes,
prepared statements, and transactions belong to the later database enhancement.

## Contents

- `original/`: original CS 300 source and supporting files.
- `enhanced/`: CMake project, application, AVL tree, benchmark, and tests.
- `sample_courses.csv`: small interactive demonstration catalog.
- `benchmark_results.csv`: baseline and AVL comparison results.
- `VERIFICATION.md`: environment and validation record.

## Tested direct compile fallback (macOS/Linux)

From this folder, with a C++17 compiler:

```sh
mkdir -p build-direct
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Repository.cpp enhanced/src/Tree.cpp enhanced/src/Service.cpp enhanced/src/Menu.cpp enhanced/src/main.cpp -o build-direct/course_planner
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Repository.cpp enhanced/src/Tree.cpp enhanced/src/Service.cpp enhanced/src/Menu.cpp enhanced/tests/tests.cpp -o build-direct/planner_tests
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Tree.cpp enhanced/benchmarks/tree_benchmark.cpp -o build-direct/tree_benchmark
./build-direct/planner_tests
./build-direct/tree_benchmark
./build-direct/course_planner
```

The macOS commands were checked for the final submission. Windows instructions above are documented but were not tested here.
