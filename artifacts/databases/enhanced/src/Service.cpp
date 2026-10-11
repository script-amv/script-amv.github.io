#include "Planner.hpp"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace planner {
namespace {
bool validNumber(const std::string& number) {
    if (number.empty() || number.size() > 32) return false;
    return std::all_of(number.begin(), number.end(), [](char value) {
        return (value >= 'A' && value <= 'Z') || (value >= '0' && value <= '9');
    });
}

void normalizeAndValidate(Course& course) {
    course.number = normalize(course.number);
    course.title = trim(course.title);
    if (!validNumber(course.number))
        throw std::runtime_error("Course number must contain 1-32 letters or digits.");
    if (course.title.empty() || course.title.size() > 200)
        throw std::runtime_error("Course title must contain 1-200 characters.");
    std::unordered_set<std::string> uniquePrerequisites;
    for (auto& prerequisite : course.prerequisites) {
        prerequisite = normalize(prerequisite);
        if (!validNumber(prerequisite))
            throw std::runtime_error("Invalid prerequisite for " + course.number);
        if (prerequisite == course.number)
            throw std::runtime_error("Self-prerequisite: " + course.number);
        if (!uniquePrerequisites.insert(prerequisite).second)
            throw std::runtime_error("Repeated prerequisite: " + prerequisite);
    }
}
}

std::size_t CourseService::load(const std::string& path) {
    auto records = repository.read(path);
    if (records.empty()) throw std::runtime_error("Course file is empty.");
    for (auto& course : records) normalizeAndValidate(course);
    // SQLite validates uniqueness and references inside one atomic transaction.
    catalog.replaceAll(records);
    return records.size();
}

std::optional<Course> CourseService::find(const std::string& number) const {
    return catalog.find(normalize(number));
}

void CourseService::add(Course course) {
    normalizeAndValidate(course);
    catalog.add(course);
}

void CourseService::updateTitle(std::string number, std::string title) {
    number = normalize(number);
    title = trim(title);
    if (!validNumber(number)) throw std::runtime_error("Invalid course number.");
    if (title.empty() || title.size() > 200)
        throw std::runtime_error("Course title must contain 1-200 characters.");
    catalog.updateTitle(number, title);
}

void CourseService::remove(std::string number) {
    number = normalize(number);
    if (!validNumber(number)) throw std::runtime_error("Invalid course number.");
    catalog.remove(number);
}
}
