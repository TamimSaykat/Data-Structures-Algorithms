#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

const int MAX_CHILD = 26;
const int SUGGESTION_LIMIT = 25;

class RadixTree {
    class Node;
    Node *_root = nullptr;

    Node* insert(Node*, string, Node*);
    Node* remove(Node*, string, Node*);
    bool search(Node*, string);
    void get_entries(Node*, string, vector<string>&);
    void print_tree(Node*, int);

public:
    void insert(string);
    void remove(string);
    bool search(string);
    void display();
    void autocomplete();
    Node* root() { return _root; }
} tree;

class RadixTree::Node {
    Node* next[MAX_CHILD]{};
    int cc = 0;
    string val;
    bool datanode;

public:
    Node(string value, bool dn = false) : val(value), datanode(dn) {}

    Node* split(int ix, Node* parent) {
        if (ix <= 0 || ix >= val.size()) return this;
        Node *prefix = new Node(val.substr(0, ix));
        if (parent) parent->set_child(prefix->value()[0] - 'a', prefix);
        val = val.substr(ix);
        prefix->set_child(val[0] - 'a', this);
        return prefix;
    }

    Node* combine(Node *parent, Node *grandparent) {
        if (parent->child_count() != 1) return this;
        val = parent->value() + val;
        if (grandparent) grandparent->set_child(val[0] - 'a', this);
        delete parent;
        return this;
    }

    void set_child(int ix, Node* child) {
        if (ix < 0 || ix >= MAX_CHILD) return;
        if (!next[ix] && child) cc++;
        else if (next[ix] && !child) cc--;
        next[ix] = child;
    }

    Node* child(int ix) { return (ix >= 0 && ix < MAX_CHILD) ? next[ix] : nullptr; }
    int child_count() const { return cc; }
    void setdatanode(bool b) { datanode = b; }
    bool isdatanode() const { return datanode; }
    string value() const { return val; }
};

void RadixTree::insert(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    _root = insert(_root, str, nullptr);
}

void RadixTree::remove(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    _root = remove(_root, str, nullptr);
}

bool RadixTree::search(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return search(_root, str);
}

RadixTree::Node* RadixTree::insert(Node* root, string str, Node* parent) {
    if (!root) return new Node(str, true);

    string val = root->value();
    int i = 0, lim = min(str.size(), val.size());
    while (i < lim && str[i] == val[i]) i++;

    if (i == val.size()) {
        if (i == str.size()) root->setdatanode(true);
        else {
            Node *child = root->child(str[i] - 'a');
            root->set_child(str[i] - 'a', insert(child, str.substr(i), root));
        }
        return root;
    }

    root = root->split(i, parent);
    if (i == str.size()) root->setdatanode(true);
    else root->set_child(str[i] - 'a', new Node(str.substr(i), true));
    return root;
}

bool RadixTree::search(Node* root, string str) {
    if (!root) return false;

    string val = root->value();
    int i = 0, lim = min(str.size(), val.size());
    while (i < lim && str[i] == val[i]) i++;

    if (i != val.size()) return false;
    if (i == str.size()) return root->isdatanode();

    Node *child = root->child(str[i] - 'a');
    return search(child, str.substr(i));
}

RadixTree::Node* RadixTree::remove(Node* root, string str, Node *parent) {
    if (!root) return nullptr;
    string val = root->value();
    int i = 0, lim = min(str.size(), val.size());
    while (i < lim && str[i] == val[i]) i++;

    if (i != val.size()) return root;

    if (i != str.size()) {
        Node *child = root->child(str[i] - 'a');
        root->set_child(str[i] - 'a', remove(child, str.substr(i), root));
        if (!root->isdatanode() && root->child_count() == 1) {
            for (int e = 0; e < MAX_CHILD; e++) {
                if (root->child(e)) {
                    root = root->child(e)->combine(root, parent);
                    break;
                }
            }
        }
        return root;
    }

    root->setdatanode(false);
    if (root->child_count() == 0) {
        delete root;
        return nullptr;
    }
    if (root->child_count() == 1) {
        for (int e = 0; e < MAX_CHILD; e++) {
            if (root->child(e)) {
                root = root->child(e)->combine(root, parent);
                break;
            }
        }
    }
    return root;
}

void RadixTree::get_entries(Node *root, string value, vector<string> &entries) {
    if (!root || entries.size() >= SUGGESTION_LIMIT) return;
    if (root->isdatanode()) entries.push_back(value);

    for (int i = 0; i < MAX_CHILD; i++) {
        Node *child = root->child(i);
        if (child) get_entries(child, value + child->value(), entries);
    }
}

void RadixTree::autocomplete() {
    string prefix;
    cout << "Enter prefix for autocomplete: ";
    cin >> prefix;

    Node* current = _root;
    int i = 0;
    while (current && i < prefix.size()) {
        string val = current->value();
        int j = 0;
        while (i < prefix.size() && j < val.size() && prefix[i] == val[j]) {
            i++, j++;
        }
        if (j < val.size()) {
            current = nullptr;
            break;
        }
        if (i < prefix.size())
            current = current->child(prefix[i] - 'a');
    }

    if (!current) {
        cout << "No suggestions found." << endl;
        return;
    }

    vector<string> suggestions;
    get_entries(current, prefix, suggestions);
    for (auto& s : suggestions) cout << s << endl;
}

void RadixTree::print_tree(Node* root, int level) {
    if (!root) return;
    string indent(level * 2, ' ');
    cout << indent << (root->isdatanode() ? "*" : "") << root->value() << endl;
    for (int i = 0; i < MAX_CHILD; i++) {
        print_tree(root->child(i), level + 1);
    }
}

void RadixTree::display() {
    cout << "Radix Tree structure:\n";
    print_tree(_root, 0);
}

void menu() {
    cout << "\n1. Insert words\n";
    cout << "2. Remove words\n";
    cout << "3. Search a word\n";
    cout << "4. Display tree\n";
    cout << "5. Autocomplete\n";
    cout << "0. Exit\n";
    cout << "Choice: ";
}

int main() {
    int choice;
    do {
        menu();
        cin >> choice;
        if (choice == 1) {
            int n;
            cout << "How many words to insert? ";
            cin >> n;
            for (int i = 0; i < n; ++i) {
                string word;
                cin >> word;
                tree.insert(word);
            }
        } else if (choice == 2) {
            string word;
            cout << "Word to remove: ";
            cin >> word;
            tree.remove(word);
        } else if (choice == 3) {
            string word;
            cout << "Word to search: ";
            cin >> word;
            cout << (tree.search(word) ? "Found!" : "Not found!") << endl;
        } else if (choice == 4) {
            tree.display();
        } else if (choice == 5) {
            tree.autocomplete();
        }
    } while (choice != 0);

    return 0;
}
