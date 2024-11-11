cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

# Your option to toggle production build
option(PRODUCTION_BUILD "Make this a production build" OFF)

# Set runtime libraries for MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(FallingSandSim)

set(SDL_STATIC ON)

# Add subdirectories for libraries
add_subdirectory(libs/SDL-release-2.30.5)
add_subdirectory(libs/glad)
add_subdirectory(libs/glm)
add_subdirectory(libs/imgui-docking)

# Get all source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create the executable
add_executable("${CMAKE_PROJECT_NAME}")

# Set the C++ standard for the project
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)

# Define target source files
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

# Set up includes
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Define production build settings
if(PRODUCTION_BUILD)
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)
else()
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)
endif()

# Link libraries
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glad SDL2-static imgui)
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SDL_MAIN_HANDLED)
