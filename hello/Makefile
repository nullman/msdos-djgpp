.RECIPEPREFIX = >

CXX = i686-pc-msdosdjgpp-gcc
#CXXFLAGS = -m16 -Wall
CXXFLAGS = -m32 -Wall

all: hello

hello:
> cp ../bin/CWSDPMI.EXE .
> $(CXX) $(CXXFLAGS) -o hello.exe *.c

clean:
> rm -f *.exe *.EXE
