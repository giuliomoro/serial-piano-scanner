CXXFLAGS=-O2 -g -I/root/spi-pru -std=c++11 -I/root/Bela/include
LDLIBS=-lkeys -lstdc++
LDFLAGS=-L/root/spi-pru

$(shell mkdir -p build)
CPP_SRCS = $(wildcard *.cpp)
OBJS := $(addprefix build/,$(notdir $(CPP_SRCS:.cpp=.o)))

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<


all: SerialInterface

SerialInterface: $(OBJS)


