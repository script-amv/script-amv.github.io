// Dmitrii Shostak - CS 499 Enhancement One.
#pragma once
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <cstddef>
#include <vector>

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
    CourseTree courses;
    std::size_t count = 0;
public:
    explicit CourseService(const CourseRepository& source) : repository(source) {}
    // Replaces the catalog only after the complete input passes validation.
    std::size_t load(const std::string& path);
    const Course* find(const std::string& number) const;
    std::vector<Course> sorted() const { return courses.sorted(); }
    bool loaded() const { return count != 0; }
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
