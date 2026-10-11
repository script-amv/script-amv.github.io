#include "Planner.hpp"
#include <algorithm>
#include <stdexcept>

namespace planner {
namespace {
bool validNumber(const std::string& number) {
    if (number.empty() || number.size() > 32) return false;
    return std::all_of(number.begin(), number.end(), [](char c) {
        return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    });
}
}
std::size_t CourseService::load(const std::string& path) {
    auto records = repository.read(path);
    if (records.empty()) throw std::runtime_error("Course file is empty.");
    CourseTree staged;
    for (auto& course : records) {
        course.number = normalize(course.number);
        course.title = trim(course.title);
        if (!validNumber(course.number)) throw std::runtime_error("Course number must contain 1-32 letters or digits.");
        if (course.title.empty() || course.title.size() > 200)
            throw std::runtime_error("Course title must contain 1-200 characters.");
        for (std::size_t i = 0; i < course.prerequisites.size(); ++i) {
            auto& prerequisite = course.prerequisites[i];
            prerequisite = normalize(prerequisite);
            if (!validNumber(prerequisite)) throw std::runtime_error("Empty or invalid prerequisite for " + course.number);
            if (prerequisite == course.number) throw std::runtime_error("Self-prerequisite: " + course.number);
            if (std::find(course.prerequisites.begin(), course.prerequisites.begin() + i, prerequisite)
                != course.prerequisites.begin() + i) throw std::runtime_error("Repeated prerequisite: " + prerequisite);
        }
        staged.insert(course);
    }
    // Validate references after all records exist, including forward references.
    for (const auto& course : records)
        for (const auto& prerequisite : course.prerequisites)
            if (!staged.find(prerequisite)) throw std::runtime_error("Missing prerequisite: " + prerequisite);
    courses.swap(staged);
    count = records.size();
    return count;
}
const Course* CourseService::find(const std::string& number) const { return courses.find(normalize(number)); }
}
