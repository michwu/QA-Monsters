CXX = g++
CXXFLAGS = -I /opt/boost_1_60_0 -g -O2

default: all

all: partA

partA: main.cpp
	$(CXX) $< $(CXXFLAGS) -o partA

clean:
	rm -rf partA
