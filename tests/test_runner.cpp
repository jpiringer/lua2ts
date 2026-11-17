#include <iostream>
#include <cassert>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"
#include "emitter.hpp"

#include "all_tests.hpp"

int main() {
    std::cout << "Running Lua to TypeScript Transpiler Test Suite\n";
    std::cout << "===============================================\n";

    test_variable_declarations();
    test_functions();
    test_tables_and_objects();
    test_control_flow();
    test_for_loops();
    test_operators_and_math();
    test_standard_library_and_methods();
    test_oop_method_definition();
    test_array_indexing();

    std::cout << "===============================================\n";
    std::cout << "All 9 test suites passed successfully!\n";
    return 0;
}
