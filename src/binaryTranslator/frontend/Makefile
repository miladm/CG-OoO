##
## This is a sample makefile for building Pin tools outside
## of the Pin environment.  This makefile is suitable for
## building with the Pin kit, not a Pin source development tree.
##
## To build the tool, execute the make command:
##
##      make
## or
##      make PIN_HOME=<top-level directory where Pin was installed>
##
## After building your tool, you would invoke Pin like this:
## 
##      $PIN_HOME/pin -t MyPinTool -- /bin/ls
##
##############################################################
#
# User-specific configuration
#
##############################################################

#
# 1. Change PIN_HOME to point to the top-level directory where
#    Pin was installed. This can also be set on the command line,
#    or as an environment variable.
#
PIN_HOME ?= ../../../../pin
#PIN_HOME ?= /home/charting/pinnew/pin-2.9-39599-gcc.3.4.6-ia32_intel64-linux

##############################################################
#
# set up and include *.config files
#
##############################################################

DEBUG=1

PIN_KIT=$(PIN_HOME)
KIT=1
TESTAPP=$(OBJDIR)cp-pin.exe

TARGET_COMPILER?=gnu
ifdef OS
    ifeq (${OS},Windows_NT)
        TARGET_COMPILER=ms
    endif
endif

ifeq ($(TARGET_COMPILER),gnu)
    include $(PIN_HOME)/source/tools/makefile.gnu.config
    CXXFLAGS ?= -g -O3 -lconfig++ -I$(PIN_HOME)/source/tools/InstLib -Wall  -Wno-unknown-pragmas $(DBG) $(OPT)
    PIN=$(PIN_HOME)/pin
    LDFLAGS=  -lconfig++  -lz
endif

ifeq ($(TARGET_COMPILER),ms)
    include $(PIN_HOME)/source/tools/makefile.ms.config
    DBG?=
    PIN=$(PIN_HOME)/pin.bat
endif

CXXFLAGS += -I/home/qtedq/include
LDFLAGS+= -L/home/qtedq/lib
PIN_LDFLAGS += -L/home/qtedq/lib

##############################################################
#
# Tools - you may wish to add your tool name to TOOL_ROOTS
#
##############################################################


TOOL_ROOTS = parser 
#TOOL_ROOTS = unum

TOOL_CXXFLAGS += 
TOOL_LDFLAGS += -lconfig++ -lz
CSFILES= stInstruction utility ../../lib/benchAddrRangeParser ../../lib/message
	
CSDIR=
CSOBJ = $(addsuffix .o, $(addprefix $(OBJDIR), $(CSFILES)))

CSFILES_SUB = stInstruction

CSOBJ_SUB = $(addsuffix .o, $(addprefix $(OBJDIR), $(CSFILES_SUB)))


TOOLS = $(TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))
TOOLS_SUB = $(TOOL_ROOTS_SUB:%=$(OBJDIR)%$(PINTOOL_SUFFIX))


##############################################################
#
# build rules
#
##############################################################

all: tools tools_sub
#all: tools killobj opbTstClient opbTstClientSignal killobj
tools: $(OBJDIR) $(TOOLS) $(OBJDIR)cp-pin.exe
tools_sub: $(OBJDIR) $(TOOLS_SUB) $(OBJDIR)cp-pin.exe

test: $(OBJDIR) $(TOOL_ROOTS:%=%.test)


MyPinTool.test: $(OBJDIR)cp-pin.exe
	$(MAKE) -k -C MyPinTool PIN_HOME=$(PIN_HOME)

.PHONY:opbTstClient
opbTstClient : 
	$(MAKE) -f makefile.client

.PHONY:opbTstClientSignal
opbTstClientSignal :
	$(MAKE) -f makefile.client
killobj:
	rm ./*.o

$(OBJDIR)cp-pin.exe:
	$(CXX) $(PIN_HOME)/source/tools/Tests/cp-pin.cpp $(APP_CXXFLAGS) -o $(OBJDIR)cp-pin.exe

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o : $(CSDIR)/%.cpp $(CSDIR)/%.h
	$(CXX) -c $(CXXFLAGS) $(TOOL_CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $<

$(OBJDIR)%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(TOOL_CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $<

$(TOOLS): $(PIN_LIBNAMES)
$(TOOLS_SUB): $(PIN_LIBNAMES)

$(TOOLS): %$(PINTOOL_SUFFIX) : %.o $(CSOBJ)
	${PIN_LD} $(PIN_LDFLAGS) $(TOOL_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $^ ${PIN_LPATHS} $(PIN_LIBS) $(DBG)

$(TOOLS_SUB): %$(PINTOOL_SUFFIX) : %.o $(CSOBJ_SUB)
	${PIN_LD} $(PIN_LDFLAGS) $(TOOL_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $^ ${PIN_LPATHS} $(PIN_LIBS) $(DBG)


## cleaning
clean:
	-rm -rf $(OBJDIR) *.tested *.failed makefile.copy *.o pin.log *.out
