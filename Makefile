CXX = g++
CXXFLAGS = -std=c++20 -O3 
BOOST_LIB_DIR ?= .
LIBS = -L$(BOOST_LIB_DIR) -lboost_system -lboost_filesystem
SRC_DIR = src
HPP_DIR = include
OBJ_DIR = out

all: program

program: $(OBJ_DIR)/main.o $(OBJ_DIR)/ll1_parser.o  $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/lexer.o $(OBJ_DIR)/grammar.o
	$(CXX) $(CXXFLAGS) -o ll1 $^ $(LIBS)

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(HPP_DIR)/grammar.hpp  $(OBJ_DIR)/ll1_parser.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/grammar.o: $(SRC_DIR)/grammar.cpp $(HPP_DIR)/grammar.hpp $(OBJ_DIR)/symbol_table.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/symbol_table.o: $(SRC_DIR)/symbol_table.cpp $(HPP_DIR)/symbol_table.hpp
	 $(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/lexer.o: $(SRC_DIR)/lexer.cpp $(HPP_DIR)/lexer.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/ll1_parser.o: $(SRC_DIR)/ll1_parser.cpp $(HPP_DIR)/ll1_parser.hpp $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/lexer.o $(OBJ_DIR)/grammar.o
	$(CXX) $(CXXFLAGS) -c $< -o $@

format:
	@find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

clean:
	rm -f ll1 $(OBJ_DIR)/*.o
