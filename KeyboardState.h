#pragma once
#include <KeyPositionTracker.h>
#include <vector>

class KeyboardState
{
public:
	KeyboardState() {};
	KeyboardState(unsigned int numKeys);
	bool setup(unsigned int numKeys);
	void render(float* buffer, std::vector<KeyPositionTracker>& trackers, int first = 0, int last = -1);
	int getKey();
	float getPosition();
	float getBend();
private:
	std::vector<int> pastStates;
	std::vector<int> states;
	std::vector<double> timestamps;
	unsigned int numKeys;
	int monoKey;
	float bend;
	float position;
	unsigned int timestamp;
	static constexpr float bendOnThreshold = 0.1;
	static constexpr float bendMaxDistance = 3;
};
