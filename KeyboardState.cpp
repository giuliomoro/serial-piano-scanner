#include "KeyboardState.h"

#include <limits>
#include <algorithm>

const float KeyboardState::bendOnThreshold;
const float KeyboardState::bendPrimaryDisengageThreshold;
const int KeyboardState::bendMaxDistance;
const float KeyboardState::highestPositionHysteresisStart;
const float KeyboardState::highestPositionHysteresisDecay;
const float KeyboardState::bendSmoothEnd;
const float KeyboardState::pressingKeyOnThreshold;

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
	timestampsDown.resize(numKeys, timestamp);
	timestampsProgress.resize(numKeys, timestamp);
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
			timestampsDown[n] = timestamp;
		}
		else if(kPositionTrackerStateDown == pastStates[n]
			&& kPositionTrackerStateDown != state) 
		{
			timestampsDown[n] = 0;
		}

		if(buffer[n] > pressingKeyOnThreshold && isPressing(state) && 0 == timestampsProgress[n])
		{
			timestampsProgress[n] = timestamp;
		} else if(buffer[n] <= pressingKeyOnThreshold - 0.05 && 0 != timestampsProgress[n])
		{
			timestampsProgress[n] = 0;
		}
		pastStates[n] = states[n];
		states[n] = state;
	}
	float* foundMax = std::max_element(buffer + first, buffer + last);
	int primaryKey = foundMax - buffer;
	auto* mostRecentDown = std::max_element(timestampsDown.data(), timestampsDown.data() + timestampsDown.size());

	auto* mostRecentProgress = std::max_element(timestampsProgress.data(), timestampsProgress.data() + timestampsProgress.size());
	// if there is at least one key that is in "key down" state,
	// then that will be our primaryKey, instead, unless there is a key
	// that most recently entered the "press in progress" state that is
	// outside the bending range
	if(*mostRecentDown != 0)
	{
		int mostRecentProgressKey = mostRecentProgress - timestampsProgress.data();
		int mostRecentDownKey = mostRecentDown - timestampsDown.data();
		if(0 != *mostRecentProgress && *mostRecentProgress > *mostRecentDown
			&& std::abs(mostRecentProgressKey - mostRecentDownKey) > bendMaxDistance)
		{
			primaryKey = mostRecentProgressKey;
		} else {
			primaryKey = mostRecentDownKey;
		}
	}
	if(primaryKey != monoKey)
	{
		if(buffer[monoKey] > 0.1 && buffer[primaryKey] > 0.1)
		{
			// adding hysteresis to make sure we don't switch too often because of noise
			if(pressingKeyOnThreshold + highestPositionHysteresis > buffer[primaryKey])
			{
				highestPositionHysteresis = highestPositionHysteresisStart;
				primaryKey = monoKey;
			}
		}
	}
	highestPositionHysteresis *= highestPositionHysteresisDecay;

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
				&& timestampsDown[secondaryKey] > timestampsDown[primaryKey]
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
	otherKey = secondaryKey;
	// as the bend gesture is approaching the end (full bend), crossfade
	// the position values of the two keys
	float bendIdx = bend / bendRange;
	float bendSmoothIdx = bendIdx > 1.f - bendSmoothEnd ?
		(bendIdx - (1.f - bendSmoothEnd)) / bendSmoothEnd : 0;
	otherPosition = buffer[secondaryKey];
	//position = buffer[primaryKey];
	// gate off position of primaryKey if it's bouncing after release
	position = states[primaryKey] != kPositionTrackerStateReleaseFinished ? buffer[primaryKey] : 0;
	position = position * (1.f - bendSmoothIdx) + otherPosition * bendSmoothIdx;

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
