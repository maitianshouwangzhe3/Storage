CXX ?= g++

DIR_ROOT = $(shell pwd)

SRC = $(wildcard $(DIR_SRC)*.cpp)
OBJ = $(patsubst $(DIR_SRC)%.cpp, $(DIR_OBJ)%.o, $(SRC))

libhistorage.a : $(OBJ)
	ar -rc libhistorage.a $<

$(DIR_ROOT)%.o : $(DIR_ROOT)%.cpp
	$(CXX) -c $< -o $@ -g $(FALG)


.PHONY : clean
clean:
	rm *.o
