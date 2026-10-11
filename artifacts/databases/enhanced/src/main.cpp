#include "Planner.hpp"
#include <exception>
#include <iostream>
int main(int argc, char* argv[]) {
    try {
        const std::string databasePath = argc > 1 ? argv[1] : "course_catalog.db";
        planner::CsvCourseRepository repository;
        planner::SqliteCatalog catalog(databasePath);
        planner::CourseService service(repository, catalog);
        planner::Menu menu(service, std::cin, std::cout);
        menu.run();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
