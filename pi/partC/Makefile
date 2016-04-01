CXX = g++
CXXFLAGS = -I /opt/boost_1_60_0 -O2

default: all

all: partC

partC: main.cpp
	$(CXX) $< $(CXXFLAGS) -o partC

clean:
	rm -rf partC
