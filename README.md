
#  Lua to TypeScript Transpiler (`lua2ts`)

A clean, modular Lua-to-TypeScript transpiler written in modern C++ (C++17).

## Architecture

The transpiler follows standard compiler architecture with three clean phases:

1. **Lexer (`include/lexer.hpp`, `src/lexer.cpp`)**:
   - Tokenizes Lua source code with precise source line/column coordinates.
   - Handles numbers (integers, floats, scientific, hex), strings (single/double-quoted, multiline `[[ ... ]]`, escape sequences), comments (single-line `--` and multiline `--[[ ... ]]`), keywords, and operators.

2. **Parser (`include/parser.hpp`, `src/parser.cpp`)**:
   - Recursive descent parser that models the Lua grammar into an Abstract Syntax Tree (AST).
   - Handles operator precedence and associativity (e.g. right-associative string concatenation `..` and exponentiation `^`).
   - Supports local/global variables, multiple assignment destructuring, if-elseif-else, while, repeat-until, numeric for loops, generic for loops (`pairs`, `ipairs`), function expressions, and OOP colon method syntax.

3. **Code Generator / Emitter (`include/emitter.hpp`, `src/emitter.cpp`)**:
   - AST Visitor that traverses the AST and generates formatted TypeScript code with configurable indentation and type annotations.
   - Translates Lua standard libraries (`print` -> `console.log`, `math.*` -> `Math.*`, `table.insert` -> `.push`, `table.remove` -> `.splice`, `table.concat` -> `.join`, `type` -> `typeof`, etc.).
   - Converts Lua 1-based `ipairs` iterators to TypeScript `.entries()` loops with 1-based index preservation.
   - Translates colon method syntax (`function Obj:method()`) into TypeScript functions with typed `this` and `const self = this;`.

## Supported Conversions

| Lua Feature | TypeScript Output |
|-------------|-------------------|
| `local x = 10` | `let x: any = 10;` |
| `a, b = b, a` | `[a, b] = [b, a];` |
| `a ~= b` | `(a !== b)` |
| `a == b` | `(a === b)` |
| `a and b` | `(a && b)` |
| `a or b` | `(a \|\| b)` |
| `not a` | `!a` |
| `a .. b` | `(a + b)` |
| `a // b` | `Math.floor((a) / (b))` |
| `a ^ b` | `Math.pow(a, b)` |
| `#arr` | `(arr).length` |
| `{ 1, 2, 3 }` | `[1, 2, 3]` |
| `{ a = 1, b = 2 }` | `{ a: 1, b: 2 }` |
| `print("hi")` | `console.log("hi");` |
| `math.floor(x)` | `Math.floor(x)` |
| `table.insert(t, val)` | `t.push(val)` |
| `table.remove(t, i)` | `t.splice(i - 1, 1)[0]` |
| `table.concat(t, ", ")` | `t.join(", ")` |
| `arr[1]` (1-based index) | `arr[0]` (0-based in TS) |
| `arr[2] = 99` | `arr[1] = 99;` |
| `for i = 1, 10 do` | `for (let i = 1; i <= 10; i++)` |
| `for k, v in pairs(t) do` | `for (const [k, v] of Object.entries(t as Record<string, any>))` |
| `for i, v in ipairs(t) do` | `for (const [_i_idx, v] of (t as any[]).entries()) { const i = _i_idx + 1; ... }` |
| `function T:method(arg)` | `T.method = function(this: any, arg: any): any { const self = this; ... }` |
| `obj:method(arg)` | `obj.method(arg)` |

## Building

```bash
# Using Make
make all

# Run unit and end-to-end test suite
make test

# Or using CMake
mkdir build && cd build
cmake ..
cmake --build .
ctest
```

## CLI Usage

```bash
# Transpile a Lua file to TypeScript
./lua2ts input.lua -o output.ts

# Transpile directly from a command-line snippet
./lua2ts -e 'function add(a, b) return a + b end; print(add(3, 4))'

# Transpile from stdin
cat input.lua | ./lua2ts > output.ts

# Configure 1-based index handling:
# - 'literals' (default): adjusts integer literal indices (e.g. arr[1] -> arr[0])
# - 'all': adjusts all non-string indices (e.g. arr[i] -> arr[(i) - 1])
# - 'none': leaves indices untouched as arr[i]
./lua2ts --index-mode all input.lua -o output.ts

# Omit TypeScript type annotations (plain ES)
./lua2ts --no-types input.lua -o output.js
```
