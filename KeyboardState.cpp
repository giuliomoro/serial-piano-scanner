#include "KeyboardState.h"

#include <limits>
#include <algorithm>

const float KeyboardState::bendOnThreshold;
const float KeyboardState::bendPrimaryDisengageThreshold;
const int KeyboardState::bendMaxDistance;

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

static bool isPressed(int state)
{
	return kPositionTrackerStateDown == state;
}

static bool isPressing(int state)
{
	switch (state)
	{
		case kPositionTrackerStatePartialPressAwaitingMax:
		case kPositionTrackerStatePartialPressFoundMax:
		case kPositionTrackerStatePressInProgress:
			return true;
			break;
		default:
			return false;
	}
}

static bool isReleasing(int state)
{
	return kPositionTrackerStateReleaseInProgress == state;
}

void KeyboardState::render(float* buffer, std::vector<KeyPositionTracker>& keyPositionTrackers, int first, int last)
{
	if(last < 0 || last > numKeys)
	{
		last = numKeys;
	}
	for(unsigned int n = first; n < last; ++n)
	{
		auto state = keyPositionTrackers[n].currentState();
		if(kPositionTrackerStateDown == state
			&& kPositionTrackerStateDown != pastStates[n]) 
		{
			timestamps[n] = timestamp;
		}
		else if(kPositionTrackerStateDown == pastStates[n]
			&& kPositionTrackerStateDown != state) 
		{
			timestamps[n] = 0;
		}
		pastStates[n] = states[n];
		states[n] = state;
	}
	float* foundMax = std::max_element(buffer + first, buffer + last);
	int primaryKey = foundMax - buffer;
	auto* mostRecent = std::max_element(timestamps.data(), timestamps.data() + timestamps.size());
	// if there is at least one key that is in "key down" state,
	// then that will be our primaryKey, instead
	if(*mostRecent != 0)
	{
		primaryKey = mostRecent - timestamps.data();
	}

	// looking for neighbouring keys being pressed down, to detect "bending" gesture
	int secondaryFirst = std::max(first, primaryKey - bendMaxDistance);
	int secondaryLast = std::min(last, primaryKey + bendMaxDistance + 1);
	int secondaryKey = 0;
	float secondaryPos = std::numeric_limits<float>::min();
	for(int n = secondaryFirst; n < secondaryLast; ++n)
	{
		if(n != primaryKey && buffer[n] > secondaryPos)
		{
			if(isPressing(states[n]))
			{
				secondaryPos = buffer[n];
				secondaryKey = n;
			}
		}
	}
	float bendValue = 0;
	int distance = 0;
	if(secondaryPos > bendOnThreshold)
	{
		int secondaryState = states[secondaryKey];
		int primaryState = states[primaryKey];
		// we are actually bending if the primary key is down and the
		// secondary key is going down
		if(isPressed(primaryState) && isPressing(secondaryState))
		{
			// the "bending" gesture is active
			distance = secondaryKey - primaryKey;
			float bendingRange = kPositionTrackerPressPosition + kPositionTrackerPressHysteresis - bendOnThreshold;
			float bendCoeff = (secondaryPos - bendOnThreshold) / bendingRange;
			// clamp
			bendCoeff = std::min(1.f, std::max(-1.f, bendCoeff));
			bendValue = bendCoeff * distance;
		}
		else if (
			isReleasing(primaryState)
			|| (
				isPressed(secondaryState)
				&& timestamps[secondaryKey] > timestamps[primaryKey]
			)
		)
		{
			// the primary key is actually releasing, or they are
			// both pressed but the secondary became pressed most
			// recently,
			// then the secondary is the real primary, and there is
			// no bending going on
			primaryKey = secondaryKey;
		}
	}
	bend = bendValue;
	bendRange = distance;
	monoKey = primaryKey;
	otherPosition = buffer[secondaryKey];
	//position = buffer[primaryKey];
	// gate off position of primaryKey if it's bouncing after release
	position = states[primaryKey] != kPositionTrackerStateReleaseFinished ? buffer[primaryKey] : 0;
	++timestamp;

	float tempPerc = 0;
	for(unsigned int n = secondaryFirst; n < secondaryLast; ++n)
	{
		auto event = keyPositionTrackers[n].getPercussiveness();
		if(!missing_value<float>::isMissing(event.position))
		{
			tempPerc = event.position;
			rt_printf("======= percKey: %d, %f\n", n, tempPerc);
			break;
		}
	}
	percussiveness = tempPerc;
}

int KeyboardState::getKey()
{
	return monoKey;
}

int KeyboardState::getOtherKey()
{
	return otherKey;
}

float KeyboardState::getPosition()
{
	return position;
}

float KeyboardState::getOtherPosition()
{
	return otherPosition;
}

float KeyboardState::getBend()
{
	return bend;
}

float KeyboardState::getBendRange()
{
	return bendRange;
}

float KeyboardState::getPercussiveness()
{
	return percussiveness;
}
