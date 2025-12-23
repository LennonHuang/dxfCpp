cmake_minimum_required(VERSION 3.16)
project(AutoDxfCpp LANGUAGES CXX)

# Set AUTOUIC, AUTOMOC, AUTORCC BEFORE including qt.cmake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(qt.cmake)
set(CMAKE_AUTOUIC OFF) # ¡û MUST BE OFF when using qt_wrap_ui()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
        OpenGL
        OpenGLWidgets
    REQUIRED
)

# --- libdxfrw configuration ---
set(LIBDXFRW_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(LIBDXFRW_BUILD_DWG2DXF OFF CACHE BOOL "" FORCE)
add_subdirectory(external/libdxfrw)

target_include_directories(dxfrw
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/libdxfrw/src>
        $<INSTALL_INTERFACE:include/libdxfrw>
)

qt_standard_project_setup()

# --- Automatically discover source files ---
file(GLOB_RECURSE  PROJECT_SOURCES
    "src/*.cpp"
    "src/*.cxx"
    "src/*.cc"
)

file(GLOB_RECURSE  PROJECT_HEADERS
    "include/*.h"
    "include/*.hpp"
    "include/*.hxx"
)

file(GLOB PROJECT_UI
    "ui/*.ui"
)

message(STATUS "Found source files: ${PROJECT_SOURCES}")
message(STATUS "Found header files: ${PROJECT_HEADERS}")
message(STATUS "Found UI files: ${PROJECT_UI}")

# This explicitly tells uic where to find .ui files
qt_wrap_ui(UI_GENERATED_HEADERS ${PROJECT_UI})

# Add executable with all source files
qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${UI_GENERATED_HEADERS}  # Generated UI headers
    ${PROJECT_UI}
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

# Include directories for your project
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        dxfrw
)