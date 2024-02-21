CXX = g++
CXXFLAGS = -std=c++20 -O3
SRC_DIR = src
OBJ_DIR = out

all: program

program: $(OBJ_DIR)/main.o $(OBJ_DIR)/ll1_parser.o  $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/lexer.o $(OBJ_DIR)/grammar.o
	$(CXX) $(CXXFLAGS) -o parser $^

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/grammar.hpp  $(OBJ_DIR)/ll1_parser.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/grammar.o: $(SRC_DIR)/grammar.cpp $(SRC_DIR)/grammar.hpp $(OBJ_DIR)/symbol_table.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/symbol_table.o: $(SRC_DIR)/symbol_table.cpp $(SRC_DIR)/symbol_table.hpp
	 $(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/lexer.o: $(SRC_DIR)/lexer.cpp $(SRC_DIR)/lexer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/ll1_parser.o: $(SRC_DIR)/ll1_parser.cpp $(SRC_DIR)/ll1_parser.hpp $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/lexer.o $(OBJ_DIR)/grammar.o
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
	rm -f parser $(OBJ_DIR)/*.o
