cmake_minimum_required(VERSION 3.10)
project(RefactoringWorkshop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -pedantic -fno-rtti)

# coverage (GCC)
option(BUILD_COVERAGE_UNIT_TESTS "Decide whether generate coverage report for unit tests" OFF)
if (BUILD_COVERAGE_UNIT_TESTS)
    set(COVERAGE_REPORT_LOCATION "CoverageReport" CACHE STRING "Location for HTML coverage report." FORCE)

    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
    include(CodeCoverage)
endif()

# common libs
include(cmake/GTest.cmake)

enable_testing()
add_subdirectory(DynamicEvents)

add_subdirectory(SnakeController)

add_custom_target(SnakeTests
                  COMMAND "./SnakeController/SnakeController_UT"
                  DEPENDS SnakeController_UT)
