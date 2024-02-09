CXX = g++
CXXFLAGS = -std=c++20 -g
SRC_DIR = src
OBJ_DIR = out

all: program

program: $(OBJ_DIR)/LL1_parser.o $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/grammar.o $(OBJ_DIR)/lexer.o
	$(CXX) $(CXXFLAGS) -o parser $^

$(OBJ_DIR)/LL1_parser.o: $(SRC_DIR)/LL1_parser.cpp $(SRC_DIR)/grammar.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/symbol_table.o: $(SRC_DIR)/symbol_table.cpp $(SRC_DIR)/symbol_table.hpp
	 $(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/grammar.o: $(SRC_DIR)/grammar.cpp $(SRC_DIR)/grammar.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/lexer.o: $(SRC_DIR)/lexer.cpp $(SRC_DIR)/lexer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f parser $(OBJ_DIR)/*.o
