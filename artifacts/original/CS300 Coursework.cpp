#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};



struct Node {
    Course course;
    Node* left;
    Node* right;

    Node() {
        left = nullptr;
        right = nullptr;
    }

    Node(Course aCourse) {
        course = aCourse;
        left = nullptr;
        right = nullptr;
    }
};


class BinarySearchTree {
private:
    Node* root;

    void addNode(Node* node, Course course) {
        if (course.courseNumber < node->course.courseNumber) {
            if (node->left == nullptr) {
                node->left = new Node(course);
            } else {
                addNode(node->left, course);
            }
        } else {
            if (node->right == nullptr) {
                node->right = new Node(course);
            } else {
                addNode(node->right, course);
            }
        }
    }



    void inOrder(Node* node) {
        if (node != nullptr) {
            inOrder(node->left);
            cout << node->course.courseNumber << ", " << node->course.title << endl;
            inOrder(node->right);
        }
    }
    void destroyTree(Node* node) {
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }

public:
    BinarySearchTree() {
        root = nullptr;
    }

    virtual ~BinarySearchTree() {
        destroyTree(root);
    }

    void Insert(Course course) {
        if (root == nullptr) {
            root = new Node(course);
        } else {
            addNode(root, course);
        }
    }

    void InOrder() {
        inOrder(root);
    }



    Course Search(string courseNumber) {
        Node* current = root;
        while (current != nullptr) {
            if (current->course.courseNumber == courseNumber) {
                return current->course;
            }
            if (courseNumber < current->course.courseNumber) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        Course emptyCourse;
        return emptyCourse;
    }
};
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    stringstream ss(str);
    while (getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}


bool loadCourses(string filename, BinarySearchTree* bst) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file" << endl;
        return false;
    }

    vector<string> courseNumbers;
    string line;

    while (getline(file, line)) {
        if (trim(line).empty()) continue;
        vector<string> tokens = split(line, ',');
        if (tokens.size() >= 1) {
            courseNumbers.push_back(tokens[0]);
        }
    }

    file.clear();
    file.seekg(0, ios::beg);

    int count = 0;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;
        vector<string> tokens = split(line, ',');
        if (tokens.size() < 2) {
            cout << "Error: Invalid file line format" << endl;
            continue;
        }

        Course course;
        course.courseNumber = tokens[0];
        course.title = tokens[1];

        bool validCourse = true;
        for (size_t i = 2; i < tokens.size(); ++i) {
            string prereq = tokens[i];
            if (prereq.empty()) continue;

            if (find(courseNumbers.begin(), courseNumbers.end(), prereq) == courseNumbers.end()) {
                cout << "Prerequisite " << prereq << " is missing from the curriculum file" << endl;
                validCourse = false;
            } else {
                course.prerequisites.push_back(prereq);
            }
        }

        if (validCourse) {
            bst->Insert(course);
            count++;
        }
    }

    file.close();
    cout << count << " courses loaded successfully." << endl;
    return true;
}



void printCourseInfo(BinarySearchTree* bst, string courseNumber) {
    Course course = bst->Search(courseNumber);
    if (course.courseNumber.empty()) {
        cout << "Course " << courseNumber << " not found in the tree" << endl;
        return;
    }

    cout << course.courseNumber << ", " << course.title << endl;
    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
    } else {
        cout << "Prerequisites: ";
        for (size_t i = 0; i < course.prerequisites.size(); ++i) {
            string prereqNum = course.prerequisites[i];
            Course prereqCourse = bst->Search(prereqNum);
            cout << prereqNum;
            if (!prereqCourse.title.empty()) {
                cout << " (" << prereqCourse.title << ")";
            }
            if (i < course.prerequisites.size() - 1) {
                cout << ", ";
            }
        }
        cout << endl;
    }
}

int main() {
    BinarySearchTree* bst = new BinarySearchTree();
    bool isLoaded = false;
    string filename;
    string courseKey;
    int choice = 0;

    cout << "Menu:" << endl;

    while (choice != 9) {
        cout << endl;
        cout << "  1. Load Course Data" << endl;
        cout << "  2. Print Course List" << endl;
        cout << "  3. Print Course" << endl;
        cout << "  9. Exit" << endl;
        cout << "Enter choice: ";
        
        if (!(cin >> choice)) {
            cout << "Please enter a valid menu number." << endl;
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                cout << "What is the file name containing course data?";
                cin >> filename;
                isLoaded = loadCourses(filename, bst);
                break;

            case 2:
                if (!isLoaded) {
                    cout << "Error: No data loaded." << endl;
                    break;
                }
                cout << "Here is a sample schedule:" << endl << endl;
                bst->InOrder();
                break;

            case 3:
                if (!isLoaded) {
                    cout << "Error: No data loaded." << endl;
                    break;
                }
                cout << "Enter course number to search: ";
                cin >> courseKey;
                transform(courseKey.begin(), courseKey.end(), courseKey.begin(), ::toupper);
                printCourseInfo(bst, courseKey);
                break;

            case 9:
                cout << "Good bye." << endl;
                break;

            default:
                cout << "Invalid choice" << endl;
                break;
        }
    }

    delete bst;
    return 0;
}