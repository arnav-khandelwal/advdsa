# Smart Text Editor

This is a mini-project for an Advanced Data Structures course, demonstrating the practical application of several advanced data structures in a full-stack application. The project is a smart text editor with a C++ backend handling the core logic and a React frontend for the user interface.

## Features

*   **Efficient Text Editing**: Core text operations (insertion, deletion, cursor movement) are handled efficiently.
*   **Advanced Text Storage**: The backend uses a `std::list` to simulate a Rope-like data structure, allowing for O(1) insertion and deletion at the cursor.
*   **Undo/Redo System**: A robust undo/redo functionality is implemented using two stacks to track user actions.
*   **Autocomplete System**: A Trie data structure provides fast, prefix-based autocomplete suggestions.
*   **Fast Substring Search**: The backend uses the Knuth-Morris-Pratt (KMP) algorithm for O(N+M) substring searching, with matches highlighted in real-time on the frontend.
*   **Client-Server Architecture**: The core logic is decoupled from the UI, with the C++ backend serving a RESTful API consumed by the React frontend.

## Architecture

The application is divided into two main parts:

1.  **Backend (C++)**:
    *   Located in `smart-editor-fullstack/backend/`.
    *   Written in C++ and built with CMake.
    *   Implements all the core data structures and algorithms (`TextBuffer`, `Trie`, `UndoRedoManager`, KMP search).
    *   Exposes the editor's functionality through a simple, multi-threaded HTTP server listening on port `8080`.

2.  **Frontend (React)**:
    *   Located in `smart-editor-fullstack/frontend/`.
    *   A standard Create React App project.
    *   Provides the user interface, including the text area, search bar, and control buttons.
    *   Communicates with the C++ backend via HTTP requests to update the editor state and perform actions.

## Technology Stack

*   **Backend**: C++, POSIX Sockets
*   **Frontend**: React.js, JavaScript (ES6+), HTML5, CSS3
*   **Build Tools**: CMake (for C++), Node.js/npm (for React)

## How to Build and Run

You will need two separate terminal windows to run the backend and frontend servers simultaneously.

### Prerequisites

*   A C++ compiler (like g++ or Clang)
*   CMake
*   Node.js and npm

### 1. Build and Run the Backend

```bash
# Navigate to the backend directory
cd smart-editor-fullstack/backend

# Create a build directory if it doesn't exist
mkdir -p build
cd build

# Configure the project with CMake
cmake ..

# Compile the project
make

# Run the backend server
./editor_backend
```
The backend server will start and listen on `http://localhost:8080`.

### 2. Build and Run the Frontend

```bash
# In a new terminal, navigate to the frontend directory
cd smart-editor-fullstack/frontend

# Install dependencies
npm install

# Start the development server
npm start
```
Your default web browser will open to `http://localhost:3000`, where you can use the application.
