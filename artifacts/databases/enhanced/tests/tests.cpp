#include "Planner.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace planner;

namespace {
void require(bool value) { if (!value) throw std::runtime_error("Check failed"); }
struct MemoryRepository : CourseRepository {
    std::vector<Course> records;
    std::vector<Course> read(const std::string&) const override { return records; }
};
void rejected(const std::function<void()>& action) {
    bool failed = false;
    try { action(); } catch (const std::runtime_error&) { failed = true; }
    require(failed);
}
struct DatabaseFiles {
    std::filesystem::path path;
    explicit DatabaseFiles(std::filesystem::path value) : path(std::move(value)) { clear(); }
    ~DatabaseFiles() { clear(); }
    void clear() const {
        std::error_code error;
        std::filesystem::remove(path, error);
        std::filesystem::remove(path.string() + "-journal", error);
        std::filesystem::remove(path.string() + "-wal", error);
        std::filesystem::remove(path.string() + "-shm", error);
    }
};
}

int main() {
    int passed = 0;
    auto test = [&](const char* name, const std::function<void()>& run) {
        try { run(); ++passed; std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& error) {
            std::cerr << "FAIL " << name << ": " << error.what() << '\n';
            throw;
        }
    };

    test("schema enables foreign keys and prerequisite index", [] {
        SqliteCatalog catalog(":memory:");
        require(catalog.foreignKeysEnabled());
        require(catalog.hasPrerequisiteIndex());
    });

    test("transactional import accepts forward references and normalizes input", [] {
        MemoryRepository source;
        source.records = {{" cs200 ", " Advanced ", {"cs100"}}, {"CS100", "Intro", {}}};
        SqliteCatalog catalog(":memory:");
        CourseService service(source, catalog);
        require(service.load("") == 2);
        require(service.sorted()[0].number == "CS100");
        require(service.find("cs200")->title == "Advanced");
        require(service.find("CS200")->prerequisites[0] == "CS100");
    });

    const std::vector<std::vector<Course>> invalidImports = {
        {}, {{"", "Title", {}}}, {{"CS1", "", {}}}, {{"CS-1", "Title", {}}},
        {{"CS1", "Title", {}}, {"cs1", "Duplicate", {}}},
        {{"CS1", "Title", {"CS2"}}}, {{"CS1", "Title", {"CS1"}}},
        {{"CS1", "Title", {""}}},
        {{"CS1", "Title", {}}, {"CS2", "Title", {"CS1", "cs1"}}},
        {{"CS1", "Title", {"CS2"}}, {"CS2", "Title", {"CS1"}}}
    };
    for (const auto& records : invalidImports) {
        test("invalid import rolls back and preserves previous database", [records] {
            MemoryRepository source;
            source.records = {{"OLD1", "Existing", {}}};
            SqliteCatalog catalog(":memory:");
            CourseService service(source, catalog);
            service.load("");
            source.records = records;
            rejected([&] { service.load(""); });
            require(catalog.count() == 1);
            require(service.find("OLD1")->title == "Existing");
        });
    }

    test("database constraints roll back missing references duplicates and cycles", [] {
        SqliteCatalog catalog(":memory:");
        catalog.replaceAll({{"OLD1", "Existing", {}}});
        rejected([&] { catalog.replaceAll({{"CS1", "Missing", {"CS2"}}}); });
        require(catalog.find("OLD1").has_value());
        rejected([&] { catalog.replaceAll({{"CS1", "One", {}}, {"CS1", "Two", {}}}); });
        require(catalog.find("OLD1").has_value());
        rejected([&] { catalog.replaceAll({{"CS1", "One", {"CS2"}}, {"CS2", "Two", {"CS1"}}}); });
        require(catalog.find("OLD1").has_value());
    });

    test("CRUD uses prepared values and restricts deletion of referenced courses", [] {
        MemoryRepository source;
        SqliteCatalog catalog(":memory:");
        CourseService service(source, catalog);
        service.add({"CS1", "Intro'); DROP TABLE courses; --", {}});
        service.add({"CS2", "Advanced", {"CS1"}});
        require(service.find("CS1")->title == "Intro'); DROP TABLE courses; --");
        service.updateTitle("cs2", "Updated ' title");
        require(service.find("CS2")->title == "Updated ' title");
        rejected([&] { service.remove("CS1"); });
        require(catalog.count() == 2);
        service.remove("CS2");
        service.remove("CS1");
        require(catalog.count() == 0);
    });

    test("database persists after reopening", [] {
        DatabaseFiles files(std::filesystem::temp_directory_path() / "cs499_m4_catalog_test.db");
        {
            SqliteCatalog catalog(files.path.string());
            catalog.replaceAll({{"CS1", "Persistent Course", {}}});
        }
        {
            SqliteCatalog catalog(files.path.string());
            require(catalog.find("CS1")->title == "Persistent Course");
        }
    });

    test("menu provides import read create update and delete operations", [] {
        MemoryRepository source;
        source.records = {{"CS1", "Intro", {}}};
        SqliteCatalog catalog(":memory:");
        CourseService service(source, catalog);
        std::istringstream input(
            "1\nfile.csv\n4\nCS2\nAdvanced\nCS1\n5\nCS2\nUpdated\n3\ncs2\n"
            "6\nCS2\n2\n9\n");
        std::ostringstream output;
        Menu(service, input, output).run();
        require(output.str().find("1 courses saved") != std::string::npos);
        require(output.str().find("Course saved") != std::string::npos);
        require(output.str().find("CS2, Updated") != std::string::npos);
        require(output.str().find("CS1 (Intro)") != std::string::npos);
        require(output.str().find("Course deleted") != std::string::npos);
    });

    test("AVL behavior remains available after database enhancement", [] {
        CourseTree tree;
        for (int value = 10000; value < 12000; ++value)
            tree.insert({std::to_string(value), "Title", {}});
        require(tree.sorted().size() == 2000);
        require(tree.height() <= 11);
        require(tree.findWithComparisons("11999").comparisons <= 11);
    });

    test("CSV parser handles valid data and rejects unsupported formats", [] {
        const auto path = std::filesystem::temp_directory_path() / "cs499_m4_csv_test.csv";
        std::error_code error;
        std::filesystem::remove(path, error);
        struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code e; std::filesystem::remove(path, e); } } cleanup{path};
        CsvCourseRepository source;
        auto write = [&](const std::string& data) { std::ofstream file(path); file << data; };
        write("\r\nCS1, Intro\r\nCS2, Advanced, CS1\r\n");
        require(source.read(path.string()).size() == 2);
        for (const auto& bad : std::vector<std::string>{"CS1", "CS1,\"Title\"", std::string(4097, 'x')}) {
            write(bad);
            rejected([&] { source.read(path.string()); });
        }
    });

    std::cout << passed << " test cases passed.\n";
}
