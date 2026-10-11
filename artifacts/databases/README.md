# CS 499 Milestone Four — Dmitrii Shostak

This submission contains the original CS 300 Course Planner and its enhanced
C++17/SQLite version. The database is now the persistent source of truth for
course and prerequisite records. The separate Word narrative explains the
artifact, database skills, learning, and course outcome coverage.

## Build, test, and run

Requirements: CMake 3.16 or newer, a C++17 compiler, and SQLite 3 development
files. From this directory:

    cmake -S enhanced -B build
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/course_planner course_catalog.db

For a Visual Studio multi-configuration build, add `--config Debug` to the
build command and `-C Debug` to CTest. Run `build/Debug/course_planner.exe
course_catalog.db`.

Choose option 1 and enter `sample_courses.csv` to import the demonstration
catalog. Options 2 and 3 list and search courses. Options 4, 5, and 6 create,
update, and delete records. The SQLite file remains available after the program
closes and is opened again.

## Database design

The normalized schema has two tables:

- `courses`: unique course number and required title.
- `prerequisites`: course-to-prerequisite relationships with a composite key.

Both relationship fields are foreign keys to `courses`. Deleting a course
removes relationships owned by that course, but deleting a course that is still
required by another course is restricted. An index on `prerequisite_number`
supports reverse-reference checks and deletion rules. Field checks reject blank,
oversized, or malformed values. `schema.sql` presents the schema separately for
review; `Database.cpp` creates the same schema when the application starts.

## Transactions and data integrity

CSV replacement starts an immediate transaction. It deletes the prior catalog,
inserts every course first, and then inserts prerequisite links. This order
supports prerequisites that appear later in the file. Primary keys reject
duplicates, and foreign keys reject missing references. A recursive common table
expression checks the stored graph for cycles before commit. Any failure rolls
back the transaction, so the previous valid catalog remains unchanged and no
partial data is published.

Adding a course and its prerequisite links also uses one transaction. Every
user-provided value is sent through a prepared statement with bound parameters.
The program never builds SQL by joining user text. Tests save titles containing
quotes and SQL-like text to confirm that they remain data. A busy timeout handles
short locks, and foreign-key enforcement is checked when the database opens.

## Database operations

The menu demonstrates create, read, update, and delete operations. Queries return
courses in course-number order and prerequisites in prerequisite-number order.
The service layer normalizes course numbers and validates input before the
repository writes it. The database repeats important rules through constraints.
This gives the application checks at both the program and storage boundaries.

## Tests and remaining scope

The test program checks schema settings, the deliberate prerequisite index,
forward references, missing references, duplicates, cycles, rollback, bound SQL
values, safe deletion, all CRUD actions, persistence after reopening, CSV input,
and the previous AVL behavior. See `VERIFICATION.md` for the recorded checks.

The application is a local single-user catalog. It does not include accounts,
network access, encryption key management, concurrent user roles, or migrations
beyond schema version 1. These features would need more design before using the
application as a shared university service.

## Contents

- `original/`: original CS 300 source and supporting files.
- `enhanced/`: application, schema, SQLite repository, AVL tree, tests, and benchmark.
- `sample_courses.csv`: demonstration catalog.
- `benchmark_results.csv`: evidence retained from the algorithm enhancement.
- `VERIFICATION.md`: build and test record.

## Tested direct compile fallback (macOS/Linux)

From this folder, with a C++17 compiler and SQLite 3 development files:

```sh
mkdir -p build-direct
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Repository.cpp enhanced/src/Database.cpp enhanced/src/Tree.cpp enhanced/src/Service.cpp enhanced/src/Menu.cpp enhanced/src/main.cpp -lsqlite3 -o build-direct/course_planner
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Repository.cpp enhanced/src/Database.cpp enhanced/src/Tree.cpp enhanced/src/Service.cpp enhanced/src/Menu.cpp enhanced/tests/tests.cpp -lsqlite3 -o build-direct/planner_tests
c++ -std=c++17 -Wall -Wextra -Wpedantic -Ienhanced/include enhanced/src/Tree.cpp enhanced/benchmarks/tree_benchmark.cpp -o build-direct/tree_benchmark
./build-direct/planner_tests
./build-direct/tree_benchmark
./build-direct/course_planner course_catalog.db
```

The macOS commands were checked for the final submission. Windows instructions above are documented but were not tested here.
