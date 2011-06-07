CFLAGS = -Wall
CXXFLAGS = -Wall
LDFLAGS = -Wall -s -ansi -static-libgcc -static-libstdc++
YFLAGS = -d -r all

CC=gcc
LEX=flex
YACC=bison
CXX=g++
LD=g++


all: raytrace.exe

raytrace: raytrace.o scene.o config.o

raytrace.o: raytrace.cpp

scene.o: scene.cpp

config.o: config.cpp


%.exe:
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
.PHONY : clean
clean :
	-del *.exe *.o
	