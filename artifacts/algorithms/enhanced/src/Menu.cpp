#include "Planner.hpp"
#include <exception>

namespace planner {
void Menu::run() {
    std::string choice;
    for (;;) {
        output << "\n1. Load Course Data\n2. Print Course List\n3. Print Course\n9. Exit\nEnter choice: ";
        if (!std::getline(input, choice)) return;
        choice = trim(choice);
        if (choice == "9") { output << "Good bye.\n"; return; }
        try {
            if (choice == "1") {
                output << "Course file path: ";
                std::string path;
                if (!std::getline(input, path)) return;
                const auto count = service.load(trim(path));
                output << count << " courses loaded successfully.\n";
            } else if (choice == "2" || choice == "3") {
                if (!service.loaded()) { output << "No data loaded.\n"; continue; }
                if (choice == "2") {
                    output << "Course catalog (course-number order):\n";
                    for (const auto& course : service.sorted()) output << course.number << ", " << course.title << '\n';
                } else {
                    output << "Course number: ";
                    std::string number;
                    if (!std::getline(input, number)) return;
                    const auto* course = service.find(number);
                    if (!course) { output << "Course not found.\n"; continue; }
                    output << course->number << ", " << course->title << "\nPrerequisites:";
                    if (course->prerequisites.empty()) output << " None";
                    for (const auto& key : course->prerequisites)
                        output << ' ' << key << " (" << service.find(key)->title << ")";
                    output << '\n';
                }
            } else output << "Please enter 1, 2, 3, or 9.\n";
        } catch (const std::exception& error) {
            output << "Error: " << error.what() << "\nExisting catalog is unchanged.\n";
        }
    }
}
}
