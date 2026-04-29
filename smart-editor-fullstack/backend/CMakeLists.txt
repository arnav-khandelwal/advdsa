cmake_minimum_required(VERSION 3.10)
project(SmartEditorBackend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(editor_backend main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(editor_backend PRIVATE ${CMAKE_THREAD_LIBS_INIT})
