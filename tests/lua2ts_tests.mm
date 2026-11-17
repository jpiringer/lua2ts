//
//  lua2ts_tests.m
//  lua2ts-tests
//
//  Created by joerg piringer on 17.11.26.
//

#import <XCTest/XCTest.h>

#include "all_tests.hpp"

using namespace lua2ts;

@interface lua2ts_tests : XCTestCase

@end

@implementation lua2ts_tests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)test_all {
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
}

@end
