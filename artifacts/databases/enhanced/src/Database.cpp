#include "Planner.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace planner {
namespace {
[[noreturn]] void fail(sqlite3* database, const std::string& operation) {
    throw std::runtime_error(operation + ": " + sqlite3_errmsg(database));
}

void execute(sqlite3* database, const char* sql) {
    char* message = nullptr;
    if (sqlite3_exec(database, sql, nullptr, nullptr, &message) != SQLITE_OK) {
        const std::string detail = message ? message : sqlite3_errmsg(database);
        sqlite3_free(message);
        throw std::runtime_error(detail);
    }
}

class Statement {
    sqlite3* database;
    sqlite3_stmt* statement = nullptr;
public:
    Statement(sqlite3* db, const char* sql) : database(db) {
        if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK)
            fail(database, "Could not prepare SQL statement");
    }
    ~Statement() { sqlite3_finalize(statement); }
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    void bind(int position, const std::string& value) {
        if (sqlite3_bind_text(statement, position, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
            fail(database, "Could not bind SQL value");
    }
    bool row() {
        const int result = sqlite3_step(statement);
        if (result == SQLITE_ROW) return true;
        if (result == SQLITE_DONE) return false;
        fail(database, "Database operation failed");
    }
    void done() {
        const int result = sqlite3_step(statement);
        if (result != SQLITE_DONE) fail(database, "Database operation failed");
    }
    void reuse() {
        if (sqlite3_reset(statement) != SQLITE_OK) fail(database, "Could not reset SQL statement");
        sqlite3_clear_bindings(statement);
    }
    std::string text(int column) const {
        const auto* value = sqlite3_column_text(statement, column);
        return value ? reinterpret_cast<const char*>(value) : std::string{};
    }
    bool isNull(int column) const { return sqlite3_column_type(statement, column) == SQLITE_NULL; }
    std::size_t integer(int column) const {
        return static_cast<std::size_t>(sqlite3_column_int64(statement, column));
    }
};

class Transaction {
    sqlite3* database;
    bool active = true;
public:
    explicit Transaction(sqlite3* db) : database(db) { execute(database, "BEGIN IMMEDIATE;"); }
    ~Transaction() {
        if (active) sqlite3_exec(database, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    void commit() {
        execute(database, "COMMIT;");
        active = false;
    }
};

void rejectCycle(sqlite3* database) {
    Statement cycle(database,
        "WITH RECURSIVE reach(start, node) AS ("
        " SELECT course_number, prerequisite_number FROM prerequisites"
        " UNION"
        " SELECT reach.start, p.prerequisite_number"
        " FROM reach JOIN prerequisites p ON p.course_number = reach.node"
        ") SELECT 1 FROM reach WHERE start = node LIMIT 1;");
    if (cycle.row()) throw std::runtime_error("Prerequisite cycle detected.");
}
}

SqliteCatalog::SqliteCatalog(const std::string& path) {
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    if (sqlite3_open_v2(path.c_str(), &database, flags, nullptr) != SQLITE_OK) {
        const std::string detail = database ? sqlite3_errmsg(database) : "unknown SQLite error";
        if (database) sqlite3_close(database);
        database = nullptr;
        throw std::runtime_error("Could not open database: " + detail);
    }
    sqlite3_busy_timeout(database, 3000);
    try {
        execute(database, "PRAGMA foreign_keys = ON;");
        execute(database,
            "CREATE TABLE IF NOT EXISTS courses ("
            " course_number TEXT PRIMARY KEY"
            "   CHECK(length(course_number) BETWEEN 1 AND 32)"
            "   CHECK(course_number NOT GLOB '*[^A-Z0-9]*'),"
            " title TEXT NOT NULL CHECK(length(trim(title)) BETWEEN 1 AND 200)"
            ");"
            "CREATE TABLE IF NOT EXISTS prerequisites ("
            " course_number TEXT NOT NULL,"
            " prerequisite_number TEXT NOT NULL,"
            " PRIMARY KEY(course_number, prerequisite_number),"
            " CHECK(course_number <> prerequisite_number),"
            " FOREIGN KEY(course_number) REFERENCES courses(course_number)"
            "   ON UPDATE CASCADE ON DELETE CASCADE,"
            " FOREIGN KEY(prerequisite_number) REFERENCES courses(course_number)"
            "   ON UPDATE CASCADE ON DELETE RESTRICT"
            ");"
            "CREATE INDEX IF NOT EXISTS idx_prerequisites_target"
            " ON prerequisites(prerequisite_number);"
            "PRAGMA user_version = 1;");
        if (!foreignKeysEnabled()) throw std::runtime_error("SQLite foreign keys could not be enabled.");
    } catch (...) {
        sqlite3_close(database);
        database = nullptr;
        throw;
    }
}

SqliteCatalog::~SqliteCatalog() {
    if (database) sqlite3_close(database);
}

void SqliteCatalog::replaceAll(const std::vector<Course>& courses) {
    Transaction transaction(database);
    execute(database, "DELETE FROM prerequisites; DELETE FROM courses;");
    Statement insertCourse(database, "INSERT INTO courses(course_number, title) VALUES(?, ?);");
    for (const auto& course : courses) {
        insertCourse.bind(1, course.number);
        insertCourse.bind(2, course.title);
        insertCourse.done();
        insertCourse.reuse();
    }
    Statement insertPrerequisite(database,
        "INSERT INTO prerequisites(course_number, prerequisite_number) VALUES(?, ?);");
    for (const auto& course : courses) {
        for (const auto& prerequisite : course.prerequisites) {
            insertPrerequisite.bind(1, course.number);
            insertPrerequisite.bind(2, prerequisite);
            insertPrerequisite.done();
            insertPrerequisite.reuse();
        }
    }
    rejectCycle(database);
    transaction.commit();
}

void SqliteCatalog::add(const Course& course) {
    Transaction transaction(database);
    Statement insertCourse(database, "INSERT INTO courses(course_number, title) VALUES(?, ?);");
    insertCourse.bind(1, course.number);
    insertCourse.bind(2, course.title);
    insertCourse.done();
    Statement insertPrerequisite(database,
        "INSERT INTO prerequisites(course_number, prerequisite_number) VALUES(?, ?);");
    for (const auto& prerequisite : course.prerequisites) {
        insertPrerequisite.bind(1, course.number);
        insertPrerequisite.bind(2, prerequisite);
        insertPrerequisite.done();
        insertPrerequisite.reuse();
    }
    rejectCycle(database);
    transaction.commit();
}

void SqliteCatalog::updateTitle(const std::string& number, const std::string& title) {
    Statement update(database, "UPDATE courses SET title = ? WHERE course_number = ?;");
    update.bind(1, title);
    update.bind(2, number);
    update.done();
    if (sqlite3_changes(database) == 0) throw std::runtime_error("Course not found: " + number);
}

void SqliteCatalog::remove(const std::string& number) {
    Statement removeCourse(database, "DELETE FROM courses WHERE course_number = ?;");
    removeCourse.bind(1, number);
    removeCourse.done();
    if (sqlite3_changes(database) == 0) throw std::runtime_error("Course not found: " + number);
}

std::optional<Course> SqliteCatalog::find(const std::string& number) const {
    Statement selectCourse(database, "SELECT course_number, title FROM courses WHERE course_number = ?;");
    selectCourse.bind(1, number);
    if (!selectCourse.row()) return std::nullopt;
    Course course{selectCourse.text(0), selectCourse.text(1), {}};
    Statement selectPrerequisites(database,
        "SELECT prerequisite_number FROM prerequisites"
        " WHERE course_number = ? ORDER BY prerequisite_number;");
    selectPrerequisites.bind(1, number);
    while (selectPrerequisites.row()) course.prerequisites.push_back(selectPrerequisites.text(0));
    return course;
}

std::vector<Course> SqliteCatalog::all() const {
    Statement select(database,
        "SELECT c.course_number, c.title, p.prerequisite_number"
        " FROM courses c LEFT JOIN prerequisites p ON p.course_number = c.course_number"
        " ORDER BY c.course_number, p.prerequisite_number;");
    std::vector<Course> result;
    while (select.row()) {
        const std::string number = select.text(0);
        if (result.empty() || result.back().number != number)
            result.push_back({number, select.text(1), {}});
        if (!select.isNull(2)) result.back().prerequisites.push_back(select.text(2));
    }
    return result;
}

std::size_t SqliteCatalog::count() const {
    Statement select(database, "SELECT COUNT(*) FROM courses;");
    if (!select.row()) return 0;
    return select.integer(0);
}

bool SqliteCatalog::foreignKeysEnabled() const {
    Statement setting(database, "PRAGMA foreign_keys;");
    return setting.row() && setting.integer(0) == 1;
}

bool SqliteCatalog::hasPrerequisiteIndex() const {
    Statement index(database,
        "SELECT 1 FROM sqlite_master WHERE type = 'index' AND name = ?;");
    index.bind(1, "idx_prerequisites_target");
    return index.row();
}
}
