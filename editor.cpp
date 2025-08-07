// Cross-platform compatible includes
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <cstdlib>

#ifdef _WIN32
#include <conio.h>
#define CLEAR "cls"
char getKeyPress() { return _getch(); }
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR "clear"

char getKeyPress() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

std::string text;
std::stack<std::string> undoStack;
std::stack<std::string> redoStack;
std::string clipboard;
int cursorPos = 0;

void openFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file) {
        std::string line;
        while (std::getline(file, line)) {
            text += line + "\n";
        }
        file.close();
    } else {
        std::cout << "File not found. Creating a new file.\n";
    }
}

void saveFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file) {
        file << text;
        file.close();
        std::cout << "File saved successfully.\n";
    } else {
        std::cerr << "Failed to save the file.\n";
    }
}

void undo() {
    if (!undoStack.empty()) {
        redoStack.push(text);
        text = undoStack.top();
        undoStack.pop();
    }
}

void redo() {
    if (!redoStack.empty()) {
        undoStack.push(text);
        text = redoStack.top();
        redoStack.pop();
    }
}

void searchText(const std::string& searchTerm) {
    size_t pos = text.find(searchTerm);
    if (pos != std::string::npos) {
        std::cout << "Found at position: " << pos << "\n";
    } else {
        std::cout << "Not found.\n";
    }
}

void copy(int len) {
    if (cursorPos + len <= text.length()) {
        clipboard = text.substr(cursorPos, len);
        std::cout << "Copied: " << clipboard << std::endl;
    } else {
        std::cout << "Invalid copy length.\n";
    }
}

void paste() {
    undoStack.push(text);
    text.insert(cursorPos, clipboard);
    cursorPos += clipboard.length();
    std::cout << "Pasted: " << clipboard << std::endl;
}

void cut(int len) {
    if (cursorPos + len <= text.length()) {
        undoStack.push(text);
        clipboard = text.substr(cursorPos, len);
        text.erase(cursorPos, len);
        std::cout << "Cut: " << clipboard << std::endl;
    } else {
        std::cout << "Invalid cut length.\n";
    }
}

void insertText(const std::string& inputText) {
    undoStack.push(text);
    text.insert(cursorPos, inputText);
    cursorPos += inputText.length();
}

void editorLoop(const std::string& filename) {
    openFile(filename);

    while (true) {
        system(CLEAR);
        std::cout << text.substr(0, cursorPos) << "|" << text.substr(cursorPos) << std::endl;
        std::cout << "\nCursor at position: " << cursorPos << "\n";
        std::cout << "Commands: i-insert, d-delete, s-save, q-quit, l/r-move, u-undo, y-redo, c-copy, x-cut, v-paste, f-find\n";

        char key = getKeyPress();
        if (key == 'q') break;

        switch (key) {
            case 's': saveFile(filename); break;
            case 'i': {
                std::cout << "\nEnter text: ";
                std::string input;
                std::getline(std::cin, input);
                insertText(input);
                break;
            }
            case 'd':
                if (cursorPos > 0) {
                    undoStack.push(text);
                    text.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
                break;
            case 'l': if (cursorPos > 0) cursorPos--; break;
            case 'r': if (cursorPos < text.length()) cursorPos++; break;
            case 'u': undo(); break;
            case 'y': redo(); break;
            case 'c': {
                std::cout << "\nEnter length to copy: ";
                int len; std::cin >> len; std::cin.ignore();
                copy(len); break;
            }
            case 'x': {
                std::cout << "\nEnter length to cut: ";
                int len; std::cin >> len; std::cin.ignore();
                cut(len); break;
            }
            case 'v': paste(); break;
            case 'f': {
                std::cout << "\nEnter word to search: ";
                std::string term;
                std::getline(std::cin, term);
                searchText(term);
                std::cout << "Press any key to continue...";
                getKeyPress();
                break;
            }
        }
    }
}

int main() {
    std::string filename;
    std::cout << "Enter filename to open/create: ";
    std::getline(std::cin, filename);
    editorLoop(filename);
    std::cout << "Exiting editor.\n";
    return 0;
}
