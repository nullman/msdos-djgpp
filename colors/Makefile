.RECIPEPREFIX = >

CXX = i686-pc-msdosdjgpp-gcc
CXXFLAGS = -m32 -Wall

all: colors

colors:
> cp ../bin/CWSDPMI.EXE .
> $(CXX) $(CXXFLAGS) -o colors.exe *.c

clean:
> rm -f *.exe *.EXE
