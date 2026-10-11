#include "Planner.hpp"
#include <stdexcept>

namespace planner {
CourseTree::~CourseTree() {
    // Rotate ownership before deleting: even a skewed tree has no recursive teardown.
    while (root) {
        if (root->left) {
            auto child = std::move(root->left);
            root->left = std::move(child->right);
            child->right = std::move(root);
            root = std::move(child);
        } else {
            auto next = std::move(root->right);
            root = std::move(next);
        }
    }
}
void CourseTree::insert(Course course) {
    auto* link = &root;
    while (*link) {
        if (course.number == (*link)->course.number) throw std::runtime_error("Duplicate course: " + course.number);
        link = course.number < (*link)->course.number ? &(*link)->left : &(*link)->right;
    }
    *link = std::make_unique<Node>(std::move(course));
}
const Course* CourseTree::find(const std::string& number) const {
    const Node* node = root.get();
    while (node) {
        if (number == node->course.number) return &node->course;
        node = number < node->course.number ? node->left.get() : node->right.get();
    }
    return nullptr;
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
void CourseTree::swap(CourseTree& other) noexcept { root.swap(other.root); }
}
