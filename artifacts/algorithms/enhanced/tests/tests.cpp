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
}
int main() {
    int passed = 0;
    auto test = [&](const char* name, const std::function<void()>& run) {
        try { run(); ++passed; std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& e) { std::cerr << "FAIL " << name << ": " << e.what() << '\n'; throw; }
    };
    test("sorted list, search, normalization and forward reference", [] {
        MemoryRepository r; r.records = {{" cs200 "," Advanced ",{"cs100"}}, {"CS100","Intro",{}}};
        CourseService s(r); require(s.load("") == 2);
        require(s.sorted()[0].number == "CS100"); require(s.find("cs200")->title == "Advanced");
        require(!s.find("MISSING")); require(s.find("CS200")->prerequisites[0] == "CS100");
    });
    const std::vector<std::vector<Course>> invalid = {
        {}, {{"","Title",{}}}, {{"CS1","",{}}}, {{"CS-1","Title",{}}},
        {{"CS1","Title",{}},{"cs1","Duplicate",{}}},
        {{"CS1","Title",{"CS2"}}}, {{"CS1","Title",{"CS1"}}},
        {{"CS1","Title",{""}}},
        {{"CS1","Title",{}},{"CS2","Title",{"CS1","cs1"}}},
        {{"CS1","Title",{"CS2"}},{"CS2","Title",{"CS1"}}}
    };
    for (const auto& records : invalid) test("invalid import preserves existing catalog", [&] {
        MemoryRepository r; r.records = {{"OLD1","Existing",{}}};
        CourseService s(r); s.load(""); r.records = records;
        rejected([&] { s.load(""); }); require(s.loaded());
        require(s.sorted().size() == 1 && s.find("OLD1"));
    });
    test("reloading replaces catalog without accumulating duplicates", [] {
        MemoryRepository r; r.records = {{"CS1","Title",{}}}; CourseService s(r);
        s.load(""); s.load(""); require(s.sorted().size() == 1);
        r.records = {{"CS2","Other",{}}}; s.load(""); require(!s.find("CS1") && s.find("CS2"));
    });
    test("menu rejects mixed input and handles end of input", [] {
        MemoryRepository r; CourseService s(r);
        std::istringstream in("1abc\n2\n3\n"); std::ostringstream out;
        Menu(s,in,out).run(); require(out.str().find("Please enter") != std::string::npos);
        require(out.str().find("No data loaded") != std::string::npos);
    });
    test("menu load, lowercase search, not found and exit", [] {
        MemoryRepository r; r.records={{"CS1","Intro",{}},{"CS2","Advanced",{"CS1"}}}; CourseService s(r);
        std::istringstream in("1\na file with spaces.csv\n3\ncs2\n3\nmissing\n2\n9\n"); std::ostringstream out;
        Menu(s,in,out).run(); require(out.str().find("CS1 (Intro)") != std::string::npos);
        require(out.str().find("Course not found") != std::string::npos);
        require(out.str().find("Good bye") != std::string::npos);
    });
    test("AVL tree stays logarithmic for sorted insertion", [] {
        CourseTree tree;
        for (int i=10000;i<12000;++i) tree.insert({std::to_string(i),"Title",{}});
        require(tree.sorted().size()==2000); require(tree.find("11999"));
        require(tree.height() <= 11);
        require(tree.findWithComparisons("11999").comparisons <= 11);
    });
    test("CSV parsing, blank lines, CRLF, missing file and bad formats", [] {
        // Runs in the build directory; fixtures never replace a user's input file.
        const auto path=std::filesystem::current_path()/"planner_test_fixture.csv";
        require(!std::filesystem::exists(path));
        struct Cleanup { std::filesystem::path path; ~Cleanup(){std::filesystem::remove(path);} } cleanup{path};
        CsvCourseRepository r;
        auto write=[&](const std::string& data){ std::ofstream f(path); f << data; };
        write("\r\nCS1, Intro\r\nCS2, Advanced, CS1\r\n");
        CourseService s(r); require(s.load(path.string())==2);
        rejected([&]{s.load(path.string()+".missing");}); require(s.find("CS1"));
        for(const auto& bad:std::vector<std::string>{"CS1", "CS1,Title,", "CS1,\"Title\"", std::string(4097,'x')}) {
            write(bad); rejected([&]{s.load(path.string());}); require(s.find("CS2"));
        }
    });
    std::cout << passed << " test cases passed.\n";
}
