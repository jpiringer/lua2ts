//
//  all_tests.hpp
//  lua2ts
//
//  Created by joerg piringer on 17.11.26.
//

#include <iostream>
#include <cassert>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"
#include "emitter.hpp"

#if __OBJC__

#define MY_ASSERT XCTAssert

#else

#define MY_ASSERT assert

#endif

std::string transpile(const std::string& lua_code, bool emit_types = true) {
    lua2ts::Lexer lexer(lua_code);
    auto tokens = lexer.tokenize();
    lua2ts::Parser parser(std::move(tokens));
    auto ast = parser.parse();
    lua2ts::EmitterOptions opt;
    opt.emit_types = emit_types;
    lua2ts::TypeScriptEmitter emitter(opt);
    return emitter.emit(*ast);
}

bool contains(const std::string& text, const std::string& sub) {
    return text.find(sub) != std::string::npos;
}

void test_variable_declarations() {
    std::cout << "Testing variable declarations...\n";
    std::string lua = "local x = 42\nlocal a, b = 1, 2\nlocal uninit\n";
    std::string ts = transpile(lua);

    MY_ASSERT(contains(ts, "let x: any = 42;"));
    MY_ASSERT(contains(ts, "let a: any = 1, b: any = 2;"));
    MY_ASSERT(contains(ts, "let uninit: any;"));
}

void test_functions() {
    std::cout << "Testing functions...\n";
    std::string lua = "function greet(name)\n  return \"Hello \" .. name\nend\n";
    std::string ts = transpile(lua);

    MY_ASSERT(contains(ts, "function greet(name: any): any"));
    MY_ASSERT(contains(ts, "return (\"Hello \" + name);"));

    // Local function
    std::string lua_local = "local function add(a, b)\n  return a + b\nend\n";
    std::string ts_local = transpile(lua_local);
    assert(contains(ts_local, "function add(a: any, b: any): any"));

    // Anonymous function
    std::string lua_anon = "local fn = function(x) return x * 2 end\n";
    std::string ts_anon = transpile(lua_anon);
    MY_ASSERT(contains(ts_anon, "(x: any): any =>"));
}

void test_tables_and_objects() {
    std::cout << "Testing tables and objects...\n";
    // Array
    std::string lua_arr = "local arr = { 10, 20, 30 }\n";
    std::string ts_arr = transpile(lua_arr);
    MY_ASSERT(contains(ts_arr, "[10, 20, 30]"));

    // Object
    std::string lua_obj = "local user = { name = \"Alice\", age = 30 }\n";
    std::string ts_obj = transpile(lua_obj);
    MY_ASSERT(contains(ts_obj, "name: \"Alice\""));
    MY_ASSERT(contains(ts_obj, "age: 30"));
}

void test_control_flow() {
    std::cout << "Testing control flow...\n";
    // If - elseif - else
    std::string lua_if =
        "if x > 10 then\n"
        "  print(\"high\")\n"
        "elseif x > 5 then\n"
        "  print(\"medium\")\n"
        "else\n"
        "  print(\"low\")\n"
        "end\n";
    std::string ts_if = transpile(lua_if);
    MY_ASSERT(contains(ts_if, "if ((x > 10)) {"));
    MY_ASSERT(contains(ts_if, "} else if ((x > 5)) {"));
    MY_ASSERT(contains(ts_if, "} else {"));
    MY_ASSERT(contains(ts_if, "console.log(\"high\");"));

    // While loop
    std::string lua_while = "while count > 0 do\n  count = count - 1\nend\n";
    std::string ts_while = transpile(lua_while);
    MY_ASSERT(contains(ts_while, "while ((count > 0)) {"));
    MY_ASSERT(contains(ts_while, "count = (count - 1);"));

    // Repeat loop
    std::string lua_repeat = "repeat\n  i = i + 1\nuntil i == 10\n";
    std::string ts_repeat = transpile(lua_repeat);
    MY_ASSERT(contains(ts_repeat, "do {"));
    MY_ASSERT(contains(ts_repeat, "} while (!((i === 10)));"));
}

void test_for_loops() {
    std::cout << "Testing for loops...\n";
    // Numeric for: step 1
    std::string lua_for1 = "for i = 1, 10 do\n  print(i)\nend\n";
    std::string ts_for1 = transpile(lua_for1);
    MY_ASSERT(contains(ts_for1, "for (let i = 1; i <= 10; i++) {"));

    // Numeric for: step -1
    std::string lua_for2 = "for i = 10, 1, -1 do\n  print(i)\nend\n";
    std::string ts_for2 = transpile(lua_for2);
    MY_ASSERT(contains(ts_for2, "for (let i = 10; i >= 1; i--) {"));

    // Generic for: pairs
    std::string lua_pairs = "for k, v in pairs(tbl) do\n  print(k, v)\nend\n";
    std::string ts_pairs = transpile(lua_pairs);
    MY_ASSERT(contains(ts_pairs, "for (const [k, v] of Object.entries(tbl as Record<string, any>)) {"));

    // Generic for: ipairs
    std::string lua_ipairs = "for i, val in ipairs(list) do\n  print(i, val)\nend\n";
    std::string ts_ipairs = transpile(lua_ipairs);
    MY_ASSERT(contains(ts_ipairs, "for (const [_i_idx, val] of (list as any[]).entries()) {"));
    MY_ASSERT(contains(ts_ipairs, "const i = _i_idx + 1;"));
}

void test_operators_and_math() {
    std::cout << "Testing operators and math mapping...\n";
    std::string lua =
        "local pow = a ^ b\n"
        "local idiv = a // b\n"
        "local neq = a ~= b\n"
        "local flr = math.floor(3.7)\n"
        "local len = #arr\n";
    std::string ts = transpile(lua);

    MY_ASSERT(contains(ts, "Math.pow(a, b)"));
    MY_ASSERT(contains(ts, "Math.floor((a) / (b))"));
    MY_ASSERT(contains(ts, "(a !== b)"));
    MY_ASSERT(contains(ts, "Math.floor(3.7)"));
    MY_ASSERT(contains(ts, "(arr).length"));
}

void test_standard_library_and_methods() {
    std::cout << "Testing standard library and methods...\n";
    std::string lua =
        "table.insert(arr, 99)\n"
        "local s = table.concat(arr, \", \")\n"
        "local t = type(x)\n"
        "hero:takeDamage(25)\n";
    std::string ts = transpile(lua);

    MY_ASSERT(contains(ts, "arr.push(99);"));
    MY_ASSERT(contains(ts, "arr.join(\", \")"));
    MY_ASSERT(contains(ts, "typeof (x)"));
    MY_ASSERT(contains(ts, "hero.takeDamage(25);"));
}

void test_oop_method_definition() {
    std::cout << "Testing OOP colon method definition...\n";
    std::string lua =
        "function Player:levelUp(amount)\n"
        "  self.level = self.level + amount\n"
        "end\n";
    std::string ts = transpile(lua);

    MY_ASSERT(contains(ts, "Player.levelUp = function(this: any, amount: any): any {"));
    MY_ASSERT(contains(ts, "self.level = (self.level + amount);"));
}

void test_array_indexing() {
    std::cout << "Testing 1-based array indexing conversion...\n";

    // 1. Array index access and assignment with integer literals (default mode: Literals)
    std::string lua1 = "local first = arr[1]\narr[2] = 99\nlocal key = dict[\"name\"]\n";
    std::string ts1 = transpile(lua1);
    MY_ASSERT(contains(ts1, "let first: any = arr[0];"));
    MY_ASSERT(contains(ts1, "arr[1] = 99;"));
    MY_ASSERT(contains(ts1, "let key: any = dict[\"name\"];"));

    // 2. AllNumeric index adjustment mode
    std::string lua2 = "local val = arr[idx]\n";
    lua2ts::Lexer lexer(lua2);
    lua2ts::Parser parser(lexer.tokenize());
    auto ast = parser.parse();
    lua2ts::EmitterOptions opt;
    opt.index_adjustment = lua2ts::IndexAdjustment::AllNumeric;
    lua2ts::TypeScriptEmitter emitter(opt);
    std::string ts2 = emitter.emit(*ast);
    assert(contains(ts2, "arr[(idx) - 1]"));
}
