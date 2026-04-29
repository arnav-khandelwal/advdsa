#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <memory>
#include <map>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// --- Data Structures (from previous implementation) ---

struct CharNode {
    char data;
    CharNode* prev;
    CharNode* next;
    CharNode(char c) : data(c), prev(nullptr), next(nullptr) {}
};

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
        cursor++;
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

struct TrieNode {
    std::map<char, TrieNode*> children;
    bool isEndOfWord;
    TrieNode() : isEndOfWord(false) {}
};

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
                return {};
            }
            current = current->children[ch];
        }
        std::vector<std::string> suggestions;
        getSuggestionsHelper(current, prefix, suggestions);
        return suggestions;
    }
};

struct Action {
    enum ActionType { INSERT, DELETE };
    ActionType type;
    char character;
    int position;

    Action(ActionType t, char c, int pos) : type(t), character(c), position(pos) {}
};

class Editor; // Forward declaration

class UndoRedoManager {
private:
    std::stack<Action> undoStack;
    std::stack<Action> redoStack;
    Editor* editor;

public:
    UndoRedoManager(Editor* ed) : editor(ed) {}
    void recordAction(Action::ActionType type, char character, int position);
    void undo();
    void redo();
};

class Editor {
private:
    TextBuffer textBuffer;
    UndoRedoManager undoRedoManager;
    Trie autocompleteTrie;

public:
    Editor() : undoRedoManager(this) {
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

    std::string getText() { return textBuffer.getText(); }
    int getCursorPosition() { return textBuffer.getCursorPosition(); }

    std::vector<std::string> getAutocompleteSuggestions(const std::string& prefix) {
        return autocompleteTrie.getSuggestions(prefix);
    }

    std::vector<int> search(const std::string& pattern) {
        std::string text = textBuffer.getText();
        std::vector<int> positions;
        int n = text.length();
        int m = pattern.length();
        if (m == 0 || n == 0 || m > n) return positions;

        std::vector<int> lps(m, 0);
        for (int i = 1, len = 0; i < m;) {
            if (pattern[i] == pattern[len]) {
                lps[i++] = ++len;
            } else {
                if (len != 0) len = lps[len - 1];
                else i++;
            }
        }

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

void UndoRedoManager::recordAction(Action::ActionType type, char character, int position) {
    undoStack.push(Action(type, character, position));
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
    } else {
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
    } else {
        editor->applyDelete(action.position);
        undoStack.push(Action(Action::DELETE, action.character, action.position));
    }
}

// --- Simple HTTP Server ---

Editor globalEditor;

std::string create_json_response(const std::string& text, int cursor) {
    std::stringstream ss;
    ss << "{\"text\": \"" << text << "\", \"cursor\": " << cursor << "}";
    return ss.str();
}

void handle_connection(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);
    std::string request(buffer);

    std::string response_body;
    std::string http_response;

    if (request.find("POST /insert") != std::string::npos) {
        std::string body = request.substr(request.find("\r\n\r\n") + 4);
        size_t char_pos = body.find("\"char\":\"");
        if (char_pos != std::string::npos) {
            char c = body[char_pos + 8];
            globalEditor.insertChar(c);
        }
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("POST /backspace") != std::string::npos) {
        globalEditor.backspace();
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("POST /undo") != std::string::npos) {
        globalEditor.undo();
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("POST /redo") != std::string::npos) {
        globalEditor.redo();
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("POST /move-left") != std::string::npos) {
        globalEditor.moveCursorLeft();
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("POST /move-right") != std::string::npos) {
        globalEditor.moveCursorRight();
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("GET /state") != std::string::npos) {
        response_body = create_json_response(globalEditor.getText(), globalEditor.getCursorPosition());
    } else if (request.find("GET /") != std::string::npos) {
        response_body = "{\"message\": \"Hi\", \"Server\": \"Running\"}";
    } 
    else {
        response_body = "{\"error\": \"Invalid endpoint\"}";
    }

    std::stringstream http_stream;
    http_stream << "HTTP/1.1 200 OK\r\n";
    http_stream << "Content-Type: application/json\r\n";
    http_stream << "Content-Length: " << response_body.length() << "\r\n";
    http_stream << "Access-Control-Allow-Origin: *\r\n"; // For development
    http_stream << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    http_stream << "Access-Control-Allow-Headers: Content-Type\r\n";
    http_stream << "\r\n";
    http_stream << response_body;

    http_response = http_stream.str();
    
    write(client_socket, http_response.c_str(), http_response.length());
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Backend server listening on port 8080" << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::thread(handle_connection, new_socket).detach();
    }

    return 0;
}
