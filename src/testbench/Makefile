INCDIR = ../global 
BACKEND = ../backend/
INO_DIR = ../backend/ino
LIB_DIR = ../lib
GLOB_DIR = ../global
OBJDIR = ../obj-intel64/testbench
BINDIR = ../obj-intel64/testbench

HEADERS = $(wildcard $(INCDIR)/*.h)
HEADERS += $(wildcard *.h)

OBJNAMES := port_test $(INO_DIR)/port $(LIB_DIR)/timer $(LIB_DIR)/utility
OBJS = $(patsubst %, $(OBJDIR)/%.o, $(OBJNAMES))
DEBUGOBJS = $(patsubst %, $(OBJDIR)/%.o, $(OBJNAMES))

TARGETNAMES := testbench
TARGETS := $(patsubst %, $(BINDIR)/%, $(TARGETNAMES))
DEBUGTARGETS := $(patsubst %, $(BINDIR)/%.debug, $(TARGETNAMES))

CXXFLAGS += -I/home/subhasis/include
LDFLAGS += -L/home/subhasis/lib -static-libstdc++

.PHONY: all

all: $(TARGETS) $(DEBUGTARGETS)

$(TARGETS) : $(BINDIR)/%: $(OBJDIR)/%.o $(OBJS)
		/home/subhasis/gcc-install/bin/g++ -std=c++0x -O3 $(LDFLAGS) -g -o $@ $^ $(LIBS)

$(DEBUGTARGETS): $(BINDIR)/%.debug: $(OBJDIR)/%.debug.o $(DEBUGOBJS)
		/home/subhasis/gcc-install/bin/g++ -std=c++0x $(LDFLAGS) -g -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp $(HEADERS)
		/home/subhasis/gcc-install/bin/g++ -std=c++0x -c -O3 $(CXXFLAGS) -g -I $(INCDIR) -I ./ -o $@ $<

$(OBJDIR)/%.debug.o: %.cpp $(HEADERS)
		/home/subhasis/gcc-install/bin/g++ -std=c++0x -c $(CXXFLAGS) -g -I $(INCDIR) -I ./ -o $@ $<

clean:
	rm -f $(OBJDIR)/*
