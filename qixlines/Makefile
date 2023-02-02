.RECIPEPREFIX = >

CXX = i686-pc-msdosdjgpp-gcc
CXXFLAGS = -m32 -Wall

all: qixlines

qixlines:
> cp ../bin/CWSDPMI.EXE .
> $(CXX) $(CXXFLAGS) -o qixlines.exe *.c

clean:
> rm -f *.exe *.EXE
