# CS 499 Milestone Two — Dmitrii Shostak

This submission contains the original CS 300 Course Planner and the enhanced
C++17 application for software design and engineering. The separate Word narrative
explains the selection, learning, outcomes, and remaining work.

## Build and run

Requirements: CMake 3.16 or newer and a C++17 compiler (Clang, GCC, or MSVC).
From this directory:

    cmake -S enhanced -B build
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/course_planner

For a Visual Studio multi-configuration build, use `cmake --build build --config Debug`,
`ctest --test-dir build -C Debug --output-on-failure`, and `build/Debug/course_planner.exe`.

Choose 1 and enter `sample_courses.csv`. Choose 2 to list five courses. Choose 3
and enter `csci400` to see its prerequisite CSCI300. Choose 9 to exit.
Paths with spaces are supported; enter the path without surrounding quotes.
The CSV path is relative to the terminal's current directory.

## Design and implemented changes

Menu -> CourseService -> CourseRepository (CsvCourseRepository)

CourseService owns CourseTree. CourseTree owns nodes through unique_ptr.

- Repository: file access and parsing, with explicit errors for unsupported formats.
- Service: normalization, required fields, unique IDs, prerequisite validation,
  and complete replacement only after all records pass checks.
- Tree: sorted listing and searching, with exclusive ownership of nodes.
- Menu: messages and input handling; injected streams allow automated tests.
- main: stack-owned application objects and dependency setup.

The original file mixes storage, validation, console output and application control.
The new boundaries allow a different repository to be tested or added later. A failed
import leaves the previous catalog available. Repeated successful imports replace
the catalog rather than accumulating duplicates. EOF ends the menu cleanly.
Iteration and unique ownership avoid recursive stack growth during traversal and
destruction. The tree remains unbalanced, with O(n) worst-case search/insertion;
AVL balancing and SQLite remain later milestone work.

## Input contract and limitations

Simple CSV: course number,title,optional prerequisite number(s). No header.
Blank lines are skipped; spaces and CRLF endings are trimmed. Quoted CSV is
explicitly rejected. Titles cannot contain commas or double quotes. Course IDs
contain 1–32 ASCII letters/digits and are stored uppercase. Titles contain 1–200
characters. Empty prerequisite fields, missing prerequisite courses, duplicate
IDs or links, and self-prerequisites are rejected. Forward references are accepted.
The importer checks a 4096-byte row limit and a 10000-record limit. The row limit
is checked after getline; it is not a strict pre-allocation memory bound.
Multi-course prerequisite cycle detection is not implemented. This is a local
catalog tool, not a degree scheduler, multiuser service, or database application.

## Review and outcome evidence

Outcome 2: user messages, build instructions, design explanation and narrative.
Outcome 4: separated responsibilities, interfaces, C++17 ownership and tests.
Outcome 5: rejected invalid data, preserved catalog on failure and resource ownership.
Outcome 1: documented interfaces and review instructions support future collaboration;
actual team feedback and stakeholder decision-making are not claimed as completed.
Outcome 3: design trade-offs are documented; measured AVL work remains planned.

Reviewer focus: try valid and malformed files; check that rejected imports preserve
the loaded catalog. Review the separation of validation from file parsing, and
whether the documented simple-CSV contract fits the advising use case.

## Provenance

Original source: CS-300/CS300 Coursework.cpp, the user's CS 300 repository.
The original folder preserves the supplied source and project documentation.
The sample file is demonstration data prepared for the capstone review.
The included verification report describes checks performed on this version.

## Tested direct compile fallback (macOS/Linux)

From this folder, with a C++17 compiler:

```sh
mkdir -p build-direct
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Repository.cpp enhanced/src/Tree.cpp enhanced/src/Service.cpp enhanced/src/Menu.cpp enhanced/src/main.cpp -o build-direct/course_planner
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Repository.cpp enhanced/src/Tree.cpp enhanced/src/Service.cpp enhanced/src/Menu.cpp enhanced/tests/tests.cpp -o build-direct/planner_tests
./build-direct/planner_tests
./build-direct/course_planner
```

The macOS commands were checked for the final submission. Windows instructions above are documented but were not tested here.
