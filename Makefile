CXX=clang++
CXXFLAGS=-g -DDEBUG
LDFLAGS=`sdl2-config --libs --cflags` -fsanitize=address

TARGET=main.cpp

all:
	${CXX} ${TARGET} ${CXXFLAGS} -o 21p ${LDFLAGS}

run:
	./21p
