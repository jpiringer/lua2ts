cmake_minimum_required(VERSION 3.16)
project(lua2ts VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

set(LIB_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/emitter.cpp
)

add_library(lua2ts_lib STATIC ${LIB_SOURCES})

add_executable(lua2ts src/main.cpp)
target_link_libraries(lua2ts PRIVATE lua2ts_lib)

enable_testing()
add_executable(test_runner tests/test_runner.cpp)
target_link_libraries(test_runner PRIVATE lua2ts_lib)
add_test(NAME AllTests COMMAND test_runner)
