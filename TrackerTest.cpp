// based on https://github.com/dr-offig/BelaArduinoComms/blob/master/render.cpp
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "TouchkeyDevice.h"
#include <poll.h>

// example application
#include <signal.h>
#include <stdint.h>
#include <pthread.h>

#include <Keys.h>
#include "KeyPositionTracker.h"
int gShouldStop = 0;
int gXenomaiInited = 0; // required by libbelaextra
unsigned int gAuxiliaryTaskStackSize  = 1 << 17; // required by libbelaextra
BoardsTopology bt;
Keys* keys;
unsigned int octaves;

void interrupt_handler(int)
{
	gShouldStop = 1;
}

extern "C" int rt_printf(const char *format, ...);
KeyBuffers keyBuffers;
std::vector<KeyBuffer> keyBuffer;
std::vector<KeyPositionTracker> keyPositionTrackers;
void postCallback(void* arg, float* buffer, unsigned int length)
{
	Keys* keys = (Keys*)arg;
	static int count = 0;
	keyBuffers.postCallback(buffer, length);
	for(unsigned int n = 0; n < length; ++n)
	{
		if(n >= 45 && n < 82)
		{
			keyPositionTrackers[n].triggerReceived(count);
		}
	}
	count++;
}

int main()
{
	int dummy = 0;
	auto path = "/root/out.calib";
	keys = new Keys;
	bt.setLowestNote(0);
	bt.setBoard(0, 0, 24);
	bt.setBoard(1, 0, 23);
	bt.setBoard(2, 0, 23);

	int ret = keys->start(&bt, NULL);
	int bottomKey = bt.getLowestNote();
	int topKey = bt.getHighestNote();
	int bottomOctave = bottomKey / 12;
	int topOctave = topKey / 12;
	int numKeys = topKey - bottomKey + 1;
	keyBuffers.setup(numKeys, 1000);
	keyBuffer.reserve(numKeys); // avoid reallocation in the loop below
	for(unsigned int n = 0; n < numKeys; ++n)
	{
		keyBuffer.emplace_back(
			keyBuffers.positionBuffer[n],
			keyBuffers.timestamps[n],
			keyBuffers.firstSampleIndex,
			keyBuffers.writeIdx
		);
		keyPositionTrackers.emplace_back(
				10, keyBuffer[n]
				);
		keyPositionTrackers.back().engage();
	}
	keys->setPostCallback(postCallback, keys);
	keys->startTopCalibration();
	keys->loadInverseSquareCalibrationFile(path, 0);
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);
	while(!gShouldStop)
	{
		usleep(100000);
	}
	keys->stopAndWait();
	delete keys;
}
