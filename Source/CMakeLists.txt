cmake_minimum_required(VERSION 3.8)
project(SudokuProject)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(BUILD_GENERATOR "Build the SudokuGenerator module for puzzle generation" ON)
option(BUILD_ADVANCED "Build the SudokuAdvancedChecks module for advanced Sudoku logic" ON)

# Directories for sources and headers
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/inc)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)


set(SOURCES
    ${SRC_DIR}/SudokuBoard.cpp
    ${SRC_DIR}/SudokuSolver.cpp
    ${SRC_DIR}/SudokuGame.cpp     
    ${SRC_DIR}/main.cpp            
)


set(HEADERS
    ${INC_DIR}/SudokuBoard.hpp
    ${INC_DIR}/SudokuSolver.hpp
    ${INC_DIR}/SudokuGame.hpp    
    ${INC_DIR}/IDifficultyRater.hpp
)


if(BUILD_GENERATOR)
    list(APPEND SOURCES ${SRC_DIR}/SudokuGenerator.cpp)
    list(APPEND HEADERS ${INC_DIR}/SudokuGenerator.hpp)
endif()

if(BUILD_ADVANCED)
    list(APPEND SOURCES ${SRC_DIR}/SudokuAdvancedChecks.cpp)
    list(APPEND HEADERS ${INC_DIR}/SudokuAdvancedChecks.hpp)
endif()

add_executable(SudokuProject ${SOURCES} ${HEADERS})
target_include_directories(SudokuProject PRIVATE ${INC_DIR})

if(BUILD_GENERATOR)
    target_compile_definitions(SudokuProject PRIVATE BUILD_GENERATOR)
endif()

if(BUILD_ADVANCED)
    target_compile_definitions(SudokuProject PRIVATE BUILD_ADVANCED)
endif()

set(TEST_SOURCES ${SOURCES})

list(REMOVE_ITEM TEST_SOURCES ${SRC_DIR}/main.cpp)

add_executable(SudokuTests ${TEST_SOURCES} ${TEST_DIR}/test_main.cpp)
target_include_directories(SudokuTests PRIVATE ${INC_DIR})


if(BUILD_GENERATOR)
    target_compile_definitions(SudokuTests PRIVATE BUILD_GENERATOR)
endif()

if(BUILD_ADVANCED)
    target_compile_definitions(SudokuTests PRIVATE BUILD_ADVANCED)
endif()

