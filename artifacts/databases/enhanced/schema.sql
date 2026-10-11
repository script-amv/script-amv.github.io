PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS courses (
    course_number TEXT PRIMARY KEY
        CHECK(length(course_number) BETWEEN 1 AND 32)
        CHECK(course_number NOT GLOB '*[^A-Z0-9]*'),
    title TEXT NOT NULL
        CHECK(length(trim(title)) BETWEEN 1 AND 200)
);

CREATE TABLE IF NOT EXISTS prerequisites (
    course_number TEXT NOT NULL,
    prerequisite_number TEXT NOT NULL,
    PRIMARY KEY (course_number, prerequisite_number),
    CHECK (course_number <> prerequisite_number),
    FOREIGN KEY (course_number) REFERENCES courses(course_number)
        ON UPDATE CASCADE ON DELETE CASCADE,
    FOREIGN KEY (prerequisite_number) REFERENCES courses(course_number)
        ON UPDATE CASCADE ON DELETE RESTRICT
);

CREATE INDEX IF NOT EXISTS idx_prerequisites_target
    ON prerequisites(prerequisite_number);
