#pragma once
#include <KeyPositionTracker.h>
#include <vector>

#define DEBEND
//#define PREV_POS
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
	void setPositionCrossFadeDip(float newWeight);
private:
	std::vector<int> pastStates;
	std::vector<int> states;
	std::vector<unsigned int> timestampsDown;
	std::vector<unsigned int> timestampsProgress;
	unsigned int numKeys;
	int monoKey;
	int otherKey;
	float bend;
	float position;
	float otherPosition;
	float percussiveness;
	unsigned int timestamp;
	float bendRange;
	float highestPositionHysteresis = 0;
	unsigned int lastPercussivenessTimestamp = 0;
#ifdef DEBEND
	int lastBentTo = -1;
	int lastBentFrom = -1;
#endif /* DEBEND */
	// tunables
	float positionCrossFadeDip = 0.1;
	static constexpr float bendPrimaryDisengageThreshold = kPositionTrackerPressPosition - kPositionTrackerPressHysteresis;
	static constexpr float bendOnThreshold = 0.1;
	static constexpr int bendMaxDistance = 4;
	static constexpr float highestPositionHysteresisStart = 0.03;
	static constexpr float highestPositionHysteresisDecay = 0.95;
	static constexpr float pressingKeyOnThreshold = 0.4;
};
