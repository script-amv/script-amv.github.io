#include "Planner.hpp"
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <unordered_set>

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
    std::unordered_set<std::string> courseNumbers;
    std::unordered_map<std::string, const Course*> coursesByNumber;
    for (auto& course : records) {
        course.number = normalize(course.number);
        course.title = trim(course.title);
        if (!validNumber(course.number)) throw std::runtime_error("Course number must contain 1-32 letters or digits.");
        if (course.title.empty() || course.title.size() > 200)
            throw std::runtime_error("Course title must contain 1-200 characters.");
        std::unordered_set<std::string> uniquePrerequisites;
        for (auto& prerequisite : course.prerequisites) {
            prerequisite = normalize(prerequisite);
            if (!validNumber(prerequisite)) throw std::runtime_error("Empty or invalid prerequisite for " + course.number);
            if (prerequisite == course.number) throw std::runtime_error("Self-prerequisite: " + course.number);
            if (!uniquePrerequisites.insert(prerequisite).second)
                throw std::runtime_error("Repeated prerequisite: " + prerequisite);
        }
        if (!courseNumbers.insert(course.number).second) throw std::runtime_error("Duplicate course: " + course.number);
        staged.insert(course);
        coursesByNumber.emplace(course.number, &course);
    }
    // Set membership keeps prerequisite validation O(1) average per reference.
    for (const auto& course : records)
        for (const auto& prerequisite : course.prerequisites)
            if (!courseNumbers.count(prerequisite)) throw std::runtime_error("Missing prerequisite: " + prerequisite);
    // A cycle cannot form a usable advising path, so reject it before publishing the catalog.
    enum class Mark { unseen, visiting, done };
    std::unordered_map<std::string, Mark> marks;
    std::function<void(const Course&)> visit = [&](const Course& course) {
        const auto mark = marks[course.number];
        if (mark == Mark::visiting) throw std::runtime_error("Prerequisite cycle includes: " + course.number);
        if (mark == Mark::done) return;
        marks[course.number] = Mark::visiting;
        for (const auto& prerequisite : course.prerequisites) visit(*coursesByNumber.at(prerequisite));
        marks[course.number] = Mark::done;
    };
    for (const auto& course : records) visit(course);
    courses.swap(staged);
    count = records.size();
    return count;
}
const Course* CourseService::find(const std::string& number) const { return courses.find(normalize(number)); }
}
