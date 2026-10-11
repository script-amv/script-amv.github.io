#include "Planner.hpp"
#include <algorithm>
#include <stdexcept>

namespace planner {
namespace {
int nodeHeight(const std::unique_ptr<CourseTree::Node>& node) {
    return node ? node->height : 0;
}
void updateHeight(CourseTree::Node& node) {
    node.height = 1 + std::max(nodeHeight(node.left), nodeHeight(node.right));
}
int balanceFactor(const std::unique_ptr<CourseTree::Node>& node) {
    return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0;
}
std::unique_ptr<CourseTree::Node> rotateRight(std::unique_ptr<CourseTree::Node> parent) {
    auto child = std::move(parent->left);
    parent->left = std::move(child->right);
    child->right = std::move(parent);
    updateHeight(*child->right);
    updateHeight(*child);
    return child;
}
std::unique_ptr<CourseTree::Node> rotateLeft(std::unique_ptr<CourseTree::Node> parent) {
    auto child = std::move(parent->right);
    parent->right = std::move(child->left);
    child->left = std::move(parent);
    updateHeight(*child->left);
    updateHeight(*child);
    return child;
}
std::unique_ptr<CourseTree::Node> insertNode(std::unique_ptr<CourseTree::Node> node,
                                              Course course, std::size_t& comparisons) {
    if (!node) return std::make_unique<CourseTree::Node>(std::move(course));
    const std::string key = course.number;
    ++comparisons;
    if (course.number == node->course.number)
        throw std::runtime_error("Duplicate course: " + course.number);
    if (course.number < node->course.number)
        node->left = insertNode(std::move(node->left), std::move(course), comparisons);
    else
        node->right = insertNode(std::move(node->right), std::move(course), comparisons);

    updateHeight(*node);
    const int balance = balanceFactor(node);
    if (balance > 1 && key < node->left->course.number) return rotateRight(std::move(node));
    if (balance < -1 && key > node->right->course.number) return rotateLeft(std::move(node));
    if (balance > 1) {
        node->left = rotateLeft(std::move(node->left));
        return rotateRight(std::move(node));
    }
    if (balance < -1) {
        node->right = rotateRight(std::move(node->right));
        return rotateLeft(std::move(node));
    }
    return node;
}
}
std::size_t CourseTree::insert(Course course) {
    std::size_t comparisons = 0;
    root = insertNode(std::move(root), std::move(course), comparisons);
    return comparisons;
}
const Course* CourseTree::find(const std::string& number) const {
    return findWithComparisons(number).course;
}
LookupResult CourseTree::findWithComparisons(const std::string& number) const {
    const Node* node = root.get();
    std::size_t comparisons = 0;
    while (node) {
        ++comparisons;
        if (number == node->course.number) return {&node->course, comparisons};
        node = number < node->course.number ? node->left.get() : node->right.get();
    }
    return {nullptr, comparisons};
}
std::vector<Course> CourseTree::sorted() const {
    std::vector<Course> result;
    std::vector<const Node*> stack;
    const Node* node = root.get();
    while (node || !stack.empty()) {
        while (node) { stack.push_back(node); node = node->left.get(); }
        node = stack.back(); stack.pop_back();
        result.push_back(node->course);
        node = node->right.get();
    }
    return result;
}
std::size_t CourseTree::height() const { return static_cast<std::size_t>(nodeHeight(root)); }
void CourseTree::swap(CourseTree& other) noexcept { root.swap(other.root); }
}
