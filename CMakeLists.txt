cmake_minimum_required(VERSION 4.0)

project(hammer
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(HAMMER_BUILD_EXAMPLES "Build the hammer example programs" ON)
option(HAMMER_BUILD_TESTS "Build the hammer test suite" ON)
option(HAMMER_ENABLE_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

# Treat every compiler warning as an error across the whole project.
add_library(hammer_warnings INTERFACE)

if(MSVC)
    target_compile_options(hammer_warnings INTERFACE /W4 /WX)
    target_compile_definitions(hammer_warnings INTERFACE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(hammer_warnings INTERFACE -Wall -Wextra -Wpedantic -Werror)
endif()

add_library(hammer_sanitizers INTERFACE)

if(HAMMER_ENABLE_SANITIZERS)
    if(MSVC)
        message(WARNING "HAMMER_ENABLE_SANITIZERS has no effect with MSVC; ignoring.")
    else()
        target_compile_options(hammer_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
        target_link_options(hammer_sanitizers INTERFACE -fsanitize=address,undefined)
    endif()
endif()


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig QUIET)
endif()

find_package(Vulkan 1.3 REQUIRED)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

add_subdirectory(src)

if(HAMMER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(HAMMER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
