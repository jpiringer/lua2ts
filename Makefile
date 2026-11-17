CXX ?= clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude

SRCS = src/lexer.cpp src/parser.cpp src/emitter.cpp
OBJS = $(SRCS:.cpp=.o)

all: lua2ts test_runner

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

lua2ts: $(OBJS) src/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test_runner: $(OBJS) tests/test_runner.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test: test_runner
	./test_runner

clean:
	rm -f src/*.o tests/*.o lua2ts test_runner

.PHONY: all clean test
