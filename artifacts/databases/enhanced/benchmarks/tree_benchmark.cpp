#include "Planner.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct BaselineNode {
    planner::Course course;
    std::unique_ptr<BaselineNode> left;
    std::unique_ptr<BaselineNode> right;
    explicit BaselineNode(planner::Course item) : course(std::move(item)) {}
};
class BaselineTree {
    std::unique_ptr<BaselineNode> root;
public:
    std::size_t insert(planner::Course course) {
        std::size_t comparisons = 0;
        auto* link = &root;
        while (*link) {
            ++comparisons;
            link = course.number < (*link)->course.number ? &(*link)->left : &(*link)->right;
        }
        *link = std::make_unique<BaselineNode>(std::move(course));
        return comparisons;
    }
    std::size_t height() const {
        if (!root) return 0;
        std::size_t result = 0;
        std::vector<std::pair<const BaselineNode*, std::size_t>> stack{{root.get(), 1}};
        while (!stack.empty()) {
            const auto [node, depth] = stack.back(); stack.pop_back();
            result = std::max(result, depth);
            if (node->left) stack.push_back({node->left.get(), depth + 1});
            if (node->right) stack.push_back({node->right.get(), depth + 1});
        }
        return result;
    }
    std::size_t lookupComparisons(const std::string& key) const {
        std::size_t comparisons = 0;
        const auto* node = root.get();
        while (node) {
            ++comparisons;
            if (key == node->course.number) return comparisons;
            node = key < node->course.number ? node->left.get() : node->right.get();
        }
        return comparisons;
    }
};
std::vector<planner::Course> makeCourses(std::size_t count) {
    std::vector<planner::Course> courses;
    for (std::size_t i = 0; i < count; ++i) {
        std::ostringstream number;
        number << "CS" << std::setw(6) << std::setfill('0') << i;
        courses.push_back({number.str(), "Benchmark Course", {}});
    }
    return courses;
}
void report(const std::string& order, const std::vector<planner::Course>& courses) {
    BaselineTree baseline;
    planner::CourseTree avl;
    std::size_t baselineInsert = 0, avlInsert = 0, baselineLookup = 0, avlLookup = 0;
    for (const auto& course : courses) {
        baselineInsert += baseline.insert(course);
        avlInsert += avl.insert(course);
    }
    for (const auto& course : courses) {
        baselineLookup += baseline.lookupComparisons(course.number);
        avlLookup += avl.findWithComparisons(course.number).comparisons;
    }
    const auto n = static_cast<double>(courses.size());
    std::cout << order << ',' << courses.size() << ',' << baseline.height() << ',' << avl.height()
              << ',' << baselineInsert << ',' << avlInsert << ',' << std::fixed << std::setprecision(2)
              << baselineLookup / n << ',' << avlLookup / n << '\n';
}
}
int main() {
    auto sorted = makeCourses(1000);
    auto random = sorted;
    std::mt19937 generator(499); // Fixed seed makes the comparison reproducible.
    std::shuffle(random.begin(), random.end(), generator);
    std::cout << "order,n,bst_height,avl_height,bst_insert_comparisons,avl_insert_comparisons,"
                 "bst_average_lookup_comparisons,avl_average_lookup_comparisons\n";
    report("sorted", sorted);
    report("random", random);
}
