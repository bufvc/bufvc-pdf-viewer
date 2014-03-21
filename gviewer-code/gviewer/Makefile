SHELL = /bin/sh

SRCDIR = ./
TOPDIR = ../
XPDFSRCDIR = ../xpdf
XPDFLIBDIR = ../xpdf
GOOSRCDIR = ../goo
GOOLIBDIR = ../goo
FOFISRCDIR = ../fofi
FOFILIBDIR = ../fofi
SPLASHSRCDIR = ../splash
SPLASHLIBDIR = ../splash

## Path to Magick installation currently looking at my local installation... replace for environment... 
MAGICKSRC = /usr/include/ImageMagick/
MAGICKLIB = /usr/local/lib

CXXFLAGS = -g -O2 -D_THREAD_SAFE -pthread -I$(GOOSRCDIR) -I$(XPDFSRCDIR) -I$(FOFISRCDIR) -I$(SPLASHSRCDIR) -I$(MAGICKSRC) -I$(TOPDIR) -I$(srcdir)           -I/usr/X11R6/include
CFLAGS = -g -O2 -Wall -D_THREAD_SAFE -pthread

LDFLAGS =
LIB_DL =
OTHERLIBS = 

# CC ?= cc
# CXX ?= c++
CXX =
CXX = g++
CC = gcc -std=gnu99 -std=gnu99
AR = ar rc

RANLIB = ranlib
#RANLIB = ranlib
#RTAPELIB =

#YACC = bison -y

LIBPREFIX = lib

EXE = 

#------------------------------------------------------------------------

CXX_SRC = \
	$(SRCDIR)/gviewer.cc $(XPDFSRCDIR)/TextOutputDev.cc

#------------------------------------------------------------------------

all: gviewer$(EXE)

#-------------------------------------------------------------------------

OBJECTS = gviewer.o TextOutputDev.o GlobalParams.o GfxFont.o
LIBS = -L$(GOOLIBDIR) -L$(FOFILIBDIR) -L$(SPLASHLIBDIR) -L$(XPDFLIBDIR) -L$(MAGICKLIB) $(OTHERLIBS) -lMagick++ -lGoo -lfofi -lsplash -lXpdf

gviewer$(EXE): $(OBJECTS) $(GOOLIBDIR)/$(LIBPREFIX)Goo.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o gviewer$(EXE) $(OBJECTS) \
		$(LIBS)
		@cp -f $(SRCDIR)gviewer$(EXE) /usr/local/bin/gviewer$(EXE)
	
#------------------------------------------------------------------------

.SUFFIXES: .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

TextOutputDev.o:
	$(CXX) $(CXXFLAGS) -c $(XPDFSRCDIR)/TextOutputDev.cc
GlobalParams.o:
	$(CXX) $(CXXFLAGS) -c $(XPDFSRCDIR)/GlobalParams.cc
GfxFont.o:
	$(CXX) $(CXXFLAGS) -c $(XPDFSRCDIR)/GfxFont.cc

#-------------------------------------------------------------------------
clean:
	rm -f $(OBJECTS) gviewer$(EXE)

#------------------------------------------------------------------------

distdepend:
	cp Makefile.in Makefile.in.bak
	sed '/^#----- dependences -----/q' Makefile.in.bak >Makefile.in
	$(CXX) $(CXXFLAGS) -MM $(CXX_SRC) >>Makefile.in

