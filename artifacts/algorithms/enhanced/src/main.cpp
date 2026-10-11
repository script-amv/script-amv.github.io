#include "Planner.hpp"
#include <iostream>
int main() {
    planner::CsvCourseRepository repository;
    planner::CourseService service(repository);
    planner::Menu menu(service, std::cin, std::cout);
    menu.run();
}
