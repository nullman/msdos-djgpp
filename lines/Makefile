.RECIPEPREFIX = >

CXX = i686-pc-msdosdjgpp-gcc
#CXXFLAGS = -m16 -Wall
CXXFLAGS = -m32 -Wall

all: lines

lines:
> cp ../bin/CWSDPMI.EXE .
> $(CXX) $(CXXFLAGS) -o lines.exe *.c

clean:
> rm -f *.exe *.EXE
