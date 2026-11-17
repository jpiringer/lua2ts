#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "emitter.hpp"

void print_help(const char* prog) {
    std::cout << "Usage: " << prog << " [options] [input.lua]\n"
              << "Options:\n"
              << "  -o, --output <file>       Write output to specified file\n"
              << "  -e, --eval <code>         Transpile a snippet directly from argument\n"
              << "  --no-types                Omit TypeScript type annotations\n"
              << "  --index-mode <mode>       1-based indexing adjustment: 'literals' (default), 'all', or 'none'\n"
              << "  -h, --help                Show this help message\n";
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    std::string eval_code;
    lua2ts::EmitterOptions options;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help(argv[0]);
            return 0;
        }
        else if (arg == "-o" || arg == "--output") {
            if (++i < argc) {
                output_file = argv[i];
            }
            else {
                std::cerr << "Error: " << arg << " requires an argument\n";
                return 1;
            }
        }
        else if (arg == "-e" || arg == "--eval") {
            if (++i < argc) {
                eval_code = argv[i];
            }
            else {
                std::cerr << "Error: " << arg << " requires an argument\n";
                return 1;
            }
        }
        else if (arg == "--no-types") {
            options.emit_types = false;
        }
        else if (arg == "--index-mode") {
            if (++i < argc) {
                std::string mode = argv[i];
                if (mode == "none") {
                    options.index_adjustment = lua2ts::IndexAdjustment::None;
                }
                else if (mode == "literals") {
                    options.index_adjustment = lua2ts::IndexAdjustment::Literals;
                }
                else if (mode == "all") {
                    options.index_adjustment = lua2ts::IndexAdjustment::AllNumeric;
                }
                else {
                    std::cerr << "Unknown index-mode: " << mode << " (expected 'none', 'literals', or 'all')\n";
                    return 1;
                }
            }
            else {
                std::cerr << "Error: --index-mode requires an argument\n";
                return 1;
            }
        }
        else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            print_help(argv[0]);
            return 1;
        }
        else {
            input_file = arg;
        }
    }

    std::string lua_source;
    if (!eval_code.empty()) {
        lua_source = eval_code;
    }
    else if (!input_file.empty()) {
        std::ifstream file(input_file);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file '" << input_file << "'\n";
            return 1;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        lua_source = ss.str();
    }
    else {
        // Read from standard input
        std::stringstream ss;
        ss << std::cin.rdbuf();
        lua_source = ss.str();
    }

    try {
        lua2ts::Lexer lexer(lua_source);
        auto tokens = lexer.tokenize();

        lua2ts::Parser parser(std::move(tokens));
        auto ast = parser.parse();

        lua2ts::TypeScriptEmitter emitter(options);
        std::string ts_code = emitter.emit(*ast);

        if (!output_file.empty()) {
            std::ofstream out(output_file);
            if (!out.is_open()) {
                std::cerr << "Error: Could not open output file '" << output_file << "'\n";
                return 1;
            }
            out << ts_code;
        }
        else {
            std::cout << ts_code;
        }
    }
    catch (const lua2ts::ParseError& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
