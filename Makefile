CXXFLAGS=-g -Wall -W 
LDLIBS=-lpthread

all: counter

counter: counter.cc csp.o

clean:
	rm -f counter *.o
