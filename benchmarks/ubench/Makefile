LIB_DIR=.
CC=g++ 
CXX = g++
OPTFLAGS = -O3
LDFLAGS =  
CFLAGS = $(OPTFLAGS) -I$(LIB_DIR)
CPPFLAGS = $(OPTFLAGS) -I$(LIB_DIR)
OBJS = 
BM= ub_simple_cfg_loop

default: $(BM)

ub_simple_cfg_loop: ub_simple_cfg_loop.o  $(OBJS)

clean:
	rm *.o
	rm $(BM)
