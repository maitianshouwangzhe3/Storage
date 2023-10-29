CXX ?= g++

target = storage

DIR_ROOT = $(shell pwd)
DIR_SRC = $(DIR_ROOT)/src/
DIR_OBJ = $(DIR_ROOT)/obj/
DIR_BIN = $(DIR_ROOT)/bin/
FALG = -I$(DIR_ROOT)/include -pthread -std=gnu++14

SRC = $(wildcard $(DIR_SRC)*.cpp)
OBJ = $(patsubst $(DIR_SRC)%.cpp, $(DIR_OBJ)%.o, $(SRC))


$(target) : $(OBJ)
	$(CXX) $(OBJ) -o $(DIR_BIN)$(target) -g $(FALG)

$(DIR_OBJ)%.o : $(DIR_SRC)%.cpp
	$(CXX) -c $< -o $@ -g -I$(DIR_ROOT)/include

.PHONY : clean
clean:
	rm $(DIR_OBJ)*
	rm $(DIR_BIN)*
	