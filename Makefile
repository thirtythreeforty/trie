CXX      ?= g++
CXXFLAGS = -O4 -Wall -march=native -std=c++11
SRCS     = benchmark.cpp
OBJS     = $(SRCS:.cpp=.o)
EXEC     = benchmark

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

.depend: *.cpp
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^ > ./.depend

clean:
	$(RM) $(OBJS) $(EXEC)

-include .depend
