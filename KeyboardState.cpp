#include "KeyboardState.h"

#include <limits>
#include <algorithm>

constexpr float KeyboardState::bendOnThreshold;
constexpr float KeyboardState::bendMaxDistance;

KeyboardState::KeyboardState(unsigned int numKeys)
{
	setup(numKeys);
}

bool KeyboardState::setup(unsigned int numKeys)
{
	timestamp = 0;
	this->numKeys = numKeys;
	pastStates.resize(numKeys, kPositionTrackerStateUnknown);
	states.resize(numKeys, kPositionTrackerStateUnknown);
	timestamps.resize(numKeys, timestamp);
	return true;
}

void KeyboardState::render(float* buffer, std::vector<KeyPositionTracker>& trackers, int first, int last)
{
	if(last < 0 || last >= numKeys)
	{
		last = numKeys - 1;
	}
	int maxKey = -1;
	int secondMaxKey = 0;
	float maxPos = std::numeric_limits<float>::min();
	for(unsigned int n = first; n < last + 1; ++n)
	{
		auto state = trackers[n].currentState();
		if(kPositionTrackerStateDown == state
			&& kPositionTrackerStateDown != pastStates[n]) 
		{
			timestamps[n] = timestamp;
		}
		float pos = buffer[n];
		if(pos > maxPos)
		{
			maxPos = pos;
			// this detection is faulty: only works if bending down
			secondMaxKey = maxKey;
			maxKey = n;
		}

		pastStates[n] = states[n];
		states[n] = state;
	}
	float secondMaxPos = buffer[secondMaxKey];
	float distance = secondMaxKey - maxKey;
	if(secondMaxPos >= bendOnThreshold &&
		std::abs(distance) <= bendMaxDistance)
	{
		// we are bending
		bend = distance * secondMaxPos;
		//bend = std::max( -distance, std::min(distance, bend ));
	} else {
		bend = 0;
	}
	/*
	//float* min = std::min_element(buffer, start + gNumKeys);
	if(maxKey != monoKey)
	{
		// hysteresis for the previously active key: it will
		ifdd

	
	}
	*/

	monoKey = maxKey;
	position = buffer[maxKey];
	++timestamp;
}

int KeyboardState::getKey()
{
	return monoKey;
}

float KeyboardState::getPosition()
{
	return position;
}

float KeyboardState::getBend()
{
	return bend;
}

