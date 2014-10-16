TARGET = driver_simple

SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCES))

all: $(TARGET)

driver_simple: $(OBJS) ../../lib/yaml/libyaml.a
	/home/subhasis/gcc-install/bin/g++ -o $@ $^

%.o: %.cpp
	/home/subhasis/gcc-install/bin/g++ -std=c++0x -o $@ -O2 -c -I. -I ../../lib/ $<

clean:
	rm -f *.o *~ $(MAIN)
