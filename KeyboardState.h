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
	int getOtherKey();
	float getPosition();
	float getOtherPosition();
	float getBend();
	float getBendRange();
	float getPercussiveness();
private:
	std::vector<int> pastStates;
	std::vector<int> states;
	std::vector<double> timestamps;
	unsigned int numKeys;
	int monoKey;
	int otherKey;
	float bend;
	float position;
	float otherPosition;
	float percussiveness;
	unsigned int timestamp;
	float bendRange;
	static constexpr float bendPrimaryDisengageThreshold = kPositionTrackerPressPosition - kPositionTrackerPressHysteresis;
	static constexpr float bendOnThreshold = 0.2;
	static constexpr int bendMaxDistance = 4;
};
