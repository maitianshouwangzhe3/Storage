CXX ?= g++
TARGET = $(filter-out $@,$(MAKECMDGOALS))

all: $(TARGET)

static : historage.cpp 
	ar -rc libhistorage.a $<
shared : historage.cpp
	g++ -shared -o libhistorage.so $<

shared_lua : historage_lua.c
	gcc -shared -fPIC -o libhistorage_lua.so $<

%:
	@echo "make [xxx]" 

.PHONY : clean
clean:
	rm *.a
	rm *.so
