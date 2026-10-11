# Verification

Environment: macOS, AppleClang 14, C++17, CMake, and SQLite 3.39.5 development library.

- CMake built the SQLite application, automated tests, and retained AVL benchmark.
- All named test cases passed through CTest.
- AddressSanitizer and UndefinedBehaviorSanitizer checks passed for the tested paths.
- Tests confirmed that foreign keys are enabled and the prerequisite target index exists.
- Forward prerequisites imported successfully because courses are inserted before links.
- Missing references, duplicates, self-links, repeated links, and cycles were rejected.
- Failed replacement transactions preserved the previous valid catalog.
- A title containing quotes and SQL-like text was stored unchanged through bound values.
- A referenced prerequisite could not be deleted until its dependent course was removed.
- Data remained available after closing and reopening a file-backed SQLite database.
- The original C++ source in `original/` matches the selected CS 300 artifact.

Run the commands in README to reproduce the normal verification. The tests cover
the documented local application behavior; they do not prove security for an
internet-facing or multiuser deployment.
