#include "Planner.hpp"
#include <exception>
#include <sstream>

namespace planner {
namespace {
std::vector<std::string> commaSeparated(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream stream(input);
    std::string value;
    while (std::getline(stream, value, ',')) {
        value = trim(value);
        if (!value.empty()) result.push_back(value);
    }
    return result;
}
}

void Menu::run() {
    std::string choice;
    for (;;) {
        output << "\n1. Import CSV into database\n"
                  "2. Print course list\n"
                  "3. Print one course\n"
                  "4. Add course\n"
                  "5. Update course title\n"
                  "6. Delete course\n"
                  "9. Exit\nEnter choice: ";
        if (!std::getline(input, choice)) return;
        choice = trim(choice);
        if (choice == "9") { output << "Good bye.\n"; return; }
        try {
            if (choice == "1") {
                output << "Course file path: ";
                std::string path;
                if (!std::getline(input, path)) return;
                output << service.load(trim(path)) << " courses saved to the database.\n";
            } else if (choice == "2") {
                const auto courses = service.sorted();
                if (courses.empty()) { output << "No courses in the database.\n"; continue; }
                output << "Course catalog (course-number order):\n";
                for (const auto& course : courses) output << course.number << ", " << course.title << '\n';
            } else if (choice == "3") {
                output << "Course number: ";
                std::string number;
                if (!std::getline(input, number)) return;
                const auto course = service.find(number);
                if (!course) { output << "Course not found.\n"; continue; }
                output << course->number << ", " << course->title << "\nPrerequisites:";
                if (course->prerequisites.empty()) output << " None";
                for (const auto& key : course->prerequisites) {
                    const auto prerequisite = service.find(key);
                    output << ' ' << key;
                    if (prerequisite) output << " (" << prerequisite->title << ')';
                }
                output << '\n';
            } else if (choice == "4") {
                Course course;
                output << "Course number: ";
                if (!std::getline(input, course.number)) return;
                output << "Course title: ";
                if (!std::getline(input, course.title)) return;
                output << "Prerequisite numbers separated by commas (or blank): ";
                std::string prerequisites;
                if (!std::getline(input, prerequisites)) return;
                course.prerequisites = commaSeparated(prerequisites);
                service.add(std::move(course));
                output << "Course saved.\n";
            } else if (choice == "5") {
                std::string number, title;
                output << "Course number: ";
                if (!std::getline(input, number)) return;
                output << "New title: ";
                if (!std::getline(input, title)) return;
                service.updateTitle(number, title);
                output << "Course title updated.\n";
            } else if (choice == "6") {
                std::string number;
                output << "Course number: ";
                if (!std::getline(input, number)) return;
                service.remove(number);
                output << "Course deleted.\n";
            } else {
                output << "Please enter 1, 2, 3, 4, 5, 6, or 9.\n";
            }
        } catch (const std::exception& error) {
            output << "Error: " << error.what() << "\nNo changes were saved.\n";
        }
    }
}
}
