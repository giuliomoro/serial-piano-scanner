CXXFLAGS=-O0 -g -I/root/spi-pru -std=c++14 -I/root/Bela/include
LDLIBS=-lkeys
LDFLAGS=-L/root/spi-pru

$(shell mkdir -p build)
CPP_SRCS = $(wildcard *.cpp)
OBJS := $(addprefix build/,$(notdir $(CPP_SRCS:.cpp=.o)))
ALL_DEPS += $(addprefix build/,$(notdir $(CPP_SRCS:.c=.d)))
-include $(ALL_DEPS)

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< -MMD -MP -MF"$(@:%.o=%.d)" 

all: tracker

build/KeyPositionTracker.o: KeyPositionTracker.h
build/TrackerTest.o: KeyPositionTracker.h


SerialPianoScanner: build/SerialInterface.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

tracker: build/TrackerTest.o build/KeyPositionTracker.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(OBJS) SerialPianoScanner
