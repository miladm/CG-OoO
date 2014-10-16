TARGET = libyaml.a

SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCES))

all: $(TARGET)

libyaml.a: $(OBJS)
	ar rvs libyaml.a $^

%.o: %.cpp
	g++ -o $@ -O2 -c -I. $<

clean:
	rm -rf *.o libyaml.a
