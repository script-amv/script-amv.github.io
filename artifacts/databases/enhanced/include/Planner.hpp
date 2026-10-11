// Dmitrii Shostak - CS 499 database enhancement.
#pragma once
#include <cstddef>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

struct sqlite3;

namespace planner {
struct Course {
    std::string number;
    std::string title;
    std::vector<std::string> prerequisites;
};

// A replaceable input boundary; database storage can be added in a later milestone.
class CourseRepository {
public:
    virtual ~CourseRepository() = default;
    virtual std::vector<Course> read(const std::string& path) const = 0;
};
class CsvCourseRepository final : public CourseRepository {
public:
    std::vector<Course> read(const std::string& path) const override;
};

// Owns the persistent SQLite catalog and all SQL operations.
class SqliteCatalog {
    sqlite3* database = nullptr;
public:
    explicit SqliteCatalog(const std::string& path);
    ~SqliteCatalog();
    SqliteCatalog(const SqliteCatalog&) = delete;
    SqliteCatalog& operator=(const SqliteCatalog&) = delete;
    void replaceAll(const std::vector<Course>& courses);
    void add(const Course& course);
    void updateTitle(const std::string& number, const std::string& title);
    void remove(const std::string& number);
    std::optional<Course> find(const std::string& number) const;
    std::vector<Course> all() const;
    std::size_t count() const;
    bool foreignKeysEnabled() const;
    bool hasPrerequisiteIndex() const;
};

struct LookupResult {
    const Course* course = nullptr;
    std::size_t comparisons = 0;
};

// An AVL tree keeps catalog lookups and insertion height bounded.
class CourseTree {
public:
    struct Node {
        Course course;
        std::unique_ptr<Node> left, right;
        int height = 1;
        explicit Node(Course value) : course(std::move(value)) {}
    };
private:
    std::unique_ptr<Node> root;
public:
    CourseTree() = default;
    ~CourseTree() = default;
    CourseTree(const CourseTree&) = delete;
    CourseTree& operator=(const CourseTree&) = delete;
    std::size_t insert(Course course);
    const Course* find(const std::string& number) const;
    LookupResult findWithComparisons(const std::string& number) const;
    std::vector<Course> sorted() const;
    std::size_t height() const;
    void swap(CourseTree& other) noexcept;
};

class CourseService {
    const CourseRepository& repository;
    SqliteCatalog& catalog;
public:
    CourseService(const CourseRepository& source, SqliteCatalog& storage)
        : repository(source), catalog(storage) {}
    // The database transaction replaces the catalog only after every row is valid.
    std::size_t load(const std::string& path);
    std::optional<Course> find(const std::string& number) const;
    std::vector<Course> sorted() const { return catalog.all(); }
    void add(Course course);
    void updateTitle(std::string number, std::string title);
    void remove(std::string number);
    bool loaded() const { return catalog.count() != 0; }
};

class Menu {
    CourseService& service;
    std::istream& input;
    std::ostream& output;
public:
    Menu(CourseService& s, std::istream& in, std::ostream& out)
        : service(s), input(in), output(out) {}
    void run();
};
std::string trim(const std::string& value);
std::string normalize(std::string value);
}
