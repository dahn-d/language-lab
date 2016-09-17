CC=gcc
OBJ_DIR=../obj

all: sparseness generation modelling detection spelling

sparseness:
	$(CC) -o $(OBJ_DIR)/$@ sparseness.cpp fileRead.cpp -I. -std=c++14 -lstdc++

generation:
	$(CC) -o $(OBJ_DIR)/$@ generation.cpp fileRead.cpp -I. -std=c++14 -lstdc++

modelling:
	$(CC) -o $(OBJ_DIR)/$@ modelling.cpp fileRead.cpp -I. -std=c++14 -lstdc++ -lm

detection:
	$(CC) -o $(OBJ_DIR)/$@ detection.cpp fileRead.cpp -I. -std=c++14 -lstdc++ -lm

spelling:
	$(CC) -o $(OBJ_DIR)/$@ spelling.cpp fileRead.cpp -I. -std=c++14 -lstdc++ -lm
