CXX = g++
CXXFLAGS = -std=c++20 -O3

all: program

program: LL1_parser.o symbol_table.o grammar.o lexer.o
	$(CXX) $(CXXFLAGS) -o parser LL1_parser.o symbol_table.o grammar.o lexer.o

LL1_parser.o: LL1_parser.cpp grammar.hpp
	$(CXX) $(CXXFLAGS) -c LL1_parser.cpp

symbol_table.o: symbol_table.cpp symbol_table.hpp
	$(CXX) $(CXXFLAGS) -c symbol_table.cpp

grammar.o: grammar.cpp grammar.hpp
	$(CXX) $(CXXFLAGS) -c grammar.cpp

lexer.o: lexer.cpp lexer.hpp
	$(CXX) $(CXXFLAGS) -c lexer.cpp

clean:
	rm -f program *.o
