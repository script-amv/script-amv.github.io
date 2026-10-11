#include "Planner.hpp"
#include <fstream>
#include <stdexcept>

namespace planner {
std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \r\n\t");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \r\n\t") - first + 1);
}
std::string normalize(std::string value) {
    value = trim(value);
    for (char& c : value) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    return value;
}
std::vector<Course> CsvCourseRepository::read(const std::string& path) const {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open course file: " + path);
    std::vector<Course> result;
    std::string line;
    std::size_t row = 0;
    // Explicit bounds keep malformed input from consuming unlimited resources.
    while (std::getline(file, line)) {
        ++row;
        if (line.size() > 4096) throw std::runtime_error("Row exceeds 4096 bytes.");
        if (trim(line).empty()) continue;
        if (result.size() >= 10000) throw std::runtime_error("Catalog exceeds 10000 courses.");
        // The coursework format is simple comma-separated text, not quoted CSV.
        if (line.find('"') != std::string::npos)
            throw std::runtime_error("Quoted CSV is unsupported at row " + std::to_string(row));
        std::vector<std::string> fields;
        std::size_t start = 0;
        for (;;) {
            const auto end = line.find(',', start);
            fields.push_back(trim(line.substr(start, end - start)));
            if (end == std::string::npos) break;
            start = end + 1;
        }
        if (fields.size() < 2) throw std::runtime_error("Missing title at row " + std::to_string(row));
        Course course{fields[0], fields[1], {}};
        for (std::size_t i = 2; i < fields.size(); ++i) course.prerequisites.push_back(fields[i]);
        result.push_back(std::move(course));
    }
    if (file.bad()) throw std::runtime_error("Could not finish reading course file.");
    return result;
}
}
