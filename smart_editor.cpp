#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <memory>
#include <map>

// Forward declarations
class Editor;

// --- Data Structures ---

/**
 * @brief A node in the doubly linked list for the TextBuffer.
 */
struct CharNode {
    char data;
    CharNode* prev;
    CharNode* next;
    CharNode(char c) : data(c), prev(nullptr), next(nullptr) {}
};

/**
 * @brief A doubly linked list to store text efficiently.
 * This is a simplified Rope-like structure, allowing O(1) insertion and deletion at the cursor.
 */
class TextBuffer {
private:
    std::list<char> buffer;
    std::list<char>::iterator cursor;

public:
    TextBuffer() {
        cursor = buffer.begin();
    }

    void insert(char c) {
        cursor = buffer.insert(cursor, c);
        cursor++; // Move cursor after the inserted character
    }

    void backspace() {
        if (cursor != buffer.begin()) {
            cursor = buffer.erase(--cursor);
        }
    }

    void del() {
        if (cursor != buffer.end()) {
            cursor = buffer.erase(cursor);
        }
    }

    void moveCursorLeft() {
        if (cursor != buffer.begin()) {
            --cursor;
        }
    }

    void moveCursorRight() {
        if (cursor != buffer.end()) {
            ++cursor;
        }
    }

    int getCursorPosition() {
        return std::distance(buffer.begin(), cursor);
    }

    std::string getText() const {
        return std::string(buffer.begin(), buffer.end());
    }

    void setText(const std::string& text) {
        buffer.assign(text.begin(), text.end());
        cursor = buffer.end();
    }
};

/**
 * @brief A node in the Trie for the autocomplete system.
 */
struct TrieNode {
    std::map<char, TrieNode*> children;
    bool isEndOfWord;
    TrieNode() : isEndOfWord(false) {}
};

/**
 * @brief Trie data structure for autocomplete suggestions.
 */
class Trie {
private:
    TrieNode* root;

    void getSuggestionsHelper(TrieNode* node, std::string currentPrefix, std::vector<std::string>& suggestions) {
        if (node->isEndOfWord) {
            suggestions.push_back(currentPrefix);
        }
        for (auto const& [key, val] : node->children) {
            getSuggestionsHelper(val, currentPrefix + key, suggestions);
        }
    }

public:
    Trie() {
        root = new TrieNode();
    }

    void insertWord(const std::string& word) {
        TrieNode* current = root;
        for (char ch : word) {
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = new TrieNode();
            }
            current = current->children[ch];
        }
        current->isEndOfWord = true;
    }

    std::vector<std::string> getSuggestions(const std::string& prefix) {
        TrieNode* current = root;
        for (char ch : prefix) {
            if (current->children.find(ch) == current->children.end()) {
                return {}; // No suggestions
            }
            current = current->children[ch];
        }
        std::vector<std::string> suggestions;
        getSuggestionsHelper(current, prefix, suggestions);
        return suggestions;
    }
};

// --- Undo/Redo System ---

/**
 * @brief Represents an action performed by the user for undo/redo.
 */
struct Action {
    enum ActionType { INSERT, DELETE };
    ActionType type;
    char character;
    int position;

    Action(ActionType t, char c, int pos) : type(t), character(c), position(pos) {}
};

/**
 * @brief Manages undo and redo operations using two stacks.
 */
class UndoRedoManager {
private:
    std::stack<Action> undoStack;
    std::stack<Action> redoStack;
    Editor* editor; // Pointer to the editor to apply actions

public:
    UndoRedoManager(Editor* ed) : editor(ed) {}

    void recordAction(Action::ActionType type, char character, int position);

    void undo();
    void redo();
};

// --- Main Editor Class ---

/**
 * @brief The main controller class that integrates all components.
 */
class Editor {
private:
    TextBuffer textBuffer;
    UndoRedoManager undoRedoManager;
    Trie autocompleteTrie;

public:
    Editor() : undoRedoManager(this) {
        // Pre-populate Trie for demo
        autocompleteTrie.insertWord("hello");
        autocompleteTrie.insertWord("world");
        autocompleteTrie.insertWord("word");
        autocompleteTrie.insertWord("work");
        autocompleteTrie.insertWord("working");
    }

    void insertChar(char c) {
        int pos = textBuffer.getCursorPosition();
        textBuffer.insert(c);
        undoRedoManager.recordAction(Action::INSERT, c, pos);
    }

    void backspace() {
        int pos = textBuffer.getCursorPosition();
        if (pos > 0) {
            std::string text = textBuffer.getText();
            char deletedChar = text[pos - 1];
            textBuffer.backspace();
            undoRedoManager.recordAction(Action::DELETE, deletedChar, pos - 1);
        }
    }
    
    void moveCursorLeft() { textBuffer.moveCursorLeft(); }
    void moveCursorRight() { textBuffer.moveCursorRight(); }
    void undo() { undoRedoManager.undo(); }
    void redo() { undoRedoManager.redo(); }

    void display() {
        std::cout << "Text: \"" << textBuffer.getText() << "\"" << std::endl;
        std::cout << "Cursor at: " << textBuffer.getCursorPosition() << std::endl;
    }

    std::vector<std::string> getAutocompleteSuggestions(const std::string& prefix) {
        return autocompleteTrie.getSuggestions(prefix);
    }

    // KMP Substring Search
    std::vector<int> search(const std::string& pattern) {
        std::string text = textBuffer.getText();
        std::vector<int> positions;
        int n = text.length();
        int m = pattern.length();
        if (m == 0 || n == 0 || m > n) return positions;

        // Preprocess pattern (LPS array)
        std::vector<int> lps(m, 0);
        for (int i = 1, len = 0; i < m;) {
            if (pattern[i] == pattern[len]) {
                lps[i++] = ++len;
            } else {
                if (len != 0) len = lps[len - 1];
                else i++;
            }
        }

        // Search
        int i = 0, j = 0;
        while (i < n) {
            if (pattern[j] == text[i]) {
                i++;
                j++;
            }
            if (j == m) {
                positions.push_back(i - j);
                j = lps[j - 1];
            } else if (i < n && pattern[j] != text[i]) {
                if (j != 0) j = lps[j - 1];
                else i++;
            }
        }
        return positions;
    }
    
    // These methods are public to be accessible by UndoRedoManager
    void applyInsert(char c, int position) {
        std::string currentText = textBuffer.getText();
        currentText.insert(position, 1, c);
        textBuffer.setText(currentText);
    }

    void applyDelete(int position) {
        std::string currentText = textBuffer.getText();
        currentText.erase(position, 1);
        textBuffer.setText(currentText);
    }
};

// --- UndoRedoManager Implementation ---

void UndoRedoManager::recordAction(Action::ActionType type, char character, int position) {
    undoStack.push(Action(type, character, position));
    // Clear redo stack on new action
    while (!redoStack.empty()) {
        redoStack.pop();
    }
}

void UndoRedoManager::undo() {
    if (undoStack.empty()) return;

    Action action = undoStack.top();
    undoStack.pop();

    if (action.type == Action::INSERT) {
        editor->applyDelete(action.position);
        redoStack.push(Action(Action::INSERT, action.character, action.position));
    } else { // DELETE
        editor->applyInsert(action.character, action.position);
        redoStack.push(Action(Action::DELETE, action.character, action.position));
    }
}

void UndoRedoManager::redo() {
    if (redoStack.empty()) return;

    Action action = redoStack.top();
    redoStack.pop();

    if (action.type == Action::INSERT) {
        editor->applyInsert(action.character, action.position);
        undoStack.push(Action(Action::INSERT, action.character, action.position));
    } else { // DELETE
        editor->applyDelete(action.position);
        undoStack.push(Action(Action::DELETE, action.character, action.position));
    }
}


// --- Demo in main() ---

int main() {
    Editor editor;

    std::cout << "--- Smart Text Editor Demo ---" << std::endl;

    std::cout << "\n1. Typing 'hello worl'" << std::endl;
    editor.insertChar('h');
    editor.insertChar('e');
    editor.insertChar('l');
    editor.insertChar('l');
    editor.insertChar('o');
    editor.insertChar(' ');
    editor.insertChar('w');
    editor.insertChar('o');
    editor.insertChar('r');
    editor.insertChar('l');
    editor.display();

    std::cout << "\n2. Autocomplete for 'worl'" << std::endl;
    std::vector<std::string> suggestions = editor.getAutocompleteSuggestions("worl");
    std::cout << "Suggestions: ";
    for (const auto& s : suggestions) std::cout << s << " ";
    std::cout << std::endl;
    
    std::cout << "\n3. Finishing the word 'world'" << std::endl;
    editor.insertChar('d');
    editor.display();

    std::cout << "\n4. Undo twice" << std::endl;
    editor.undo();
    editor.display();
    editor.undo();
    editor.display();

    std::cout << "\n5. Redo once" << std::endl;
    editor.redo();
    editor.display();

    std::cout << "\n6. Backspace three times" << std::endl;
    editor.backspace();
    editor.backspace();
    editor.backspace();
    editor.display();
    
    std::cout << "\n7. Undo backspaces" << std::endl;
    editor.undo();
    editor.undo();
    editor.undo();
    editor.display();

    std::cout << "\n8. Search for 'l'" << std::endl;
    std::vector<int> positions = editor.search("l");
    std::cout << "Pattern 'l' found at positions: ";
    for (int pos : positions) std::cout << pos << " ";
    std::cout << std::endl;
    
    std::cout << "\n9. Search for 'o'" << std::endl;
    positions = editor.search("o");
    std::cout << "Pattern 'o' found at positions: ";
    for (int pos : positions) std::cout << pos << " ";
    std::cout << std::endl;

    return 0;
}

/*
--- Complexity Explanation ---

1. TextBuffer (Doubly Linked List / std::list):
   - Insert/Delete at cursor: O(1). `std::list` iterators are not invalidated on insertion/deletion, and operations at the iterator position are constant time.
   - Cursor Movement: O(1). Moving to the next/previous node is a pointer change.
   - Get Text: O(N), where N is the number of characters. We must traverse the entire list to construct the string.
   - Why this data structure? A plain `std::string` or `std::vector<char>` would have O(N) complexity for insertion/deletion in the middle, as all subsequent characters need to be shifted. A doubly linked list (or a more complex Rope) provides efficient modifications at the cursor, which is the most common operation in a text editor.

2. UndoRedoManager (Two Stacks):
   - Record Action: O(1). Pushing to a stack is a constant time operation.
   - Undo/Redo: O(1) for the stack operations. The application of the action itself depends on the TextBuffer, but since we are not re-typing, we accept the O(N) for `setText` for simplicity in this model. A more optimized undo would directly manipulate the TextBuffer nodes.
   - Why this data structure? Stacks are a natural fit for undo/redo. The Last-In, First-Out (LIFO) nature perfectly models the behavior: the last action performed is the first one to be undone.

3. Trie (Autocomplete):
   - Insert Word: O(L), where L is the length of the word. We traverse the Trie, creating nodes if they don't exist.
   - Get Suggestions: O(P + K), where P is the length of the prefix and K is the total number of characters in all suggestion words combined. We first traverse P nodes to find the prefix, then perform a DFS-like traversal from there to find all words.
   - Why this data structure? A Trie is highly efficient for prefix-based searches. Compared to iterating through a list of words and checking `startsWith()` for each (which would be O(M*P) for M words), a Trie finds the prefix in O(P) and then efficiently explores only the relevant branches.

4. KMP Search:
   - Time Complexity: O(N + M), where N is the length of the text and M is the length of the pattern. O(M) for building the LPS table and O(N) for searching. This is a significant improvement over the naive O(N*M) approach.
   - Why this algorithm? KMP is a classic, efficient string searching algorithm that avoids re-comparing characters that have already been matched by intelligently shifting the pattern based on its internal structure (the LPS array). It's a perfect example of an optimized algorithm for a common text-based problem.
*/
