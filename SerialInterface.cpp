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

void setPostCallback(void(*postCallback)(void* arg, float* buffer, unsigned int length), void* arg);


static int _handle;

/*
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo - UsbConfigurationDescriptor -
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bLength = 9
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bDescriptorType = 0x02
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .wTotalLength = 32
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bNumInterfaces = 1
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bConfigurationValue = 1
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .iConfiguration = 0
com_silabs_driver_CP210xVCPDriver(<ptr>)::GetCP210xInfo     .bmAttributes = 0x80
com_silabs_driver_CP210xVCPDriver(<ptr>)::start Interface 0 - Sucessfully loaded the driver
*/
int set_interface_attribs(int fd, int speed)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
			return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}


void set_mincount(int fd, int mcount)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN] = mcount ? 1 : 0;
	tty.c_cc[VTIME] = 5;        /* half second timer */

	if (tcsetattr(fd, TCSANOW, &tty) < 0) {
		printf("Error tcsetattr: %s\n", strerror(errno));
	}
}

int initSerial(const char *portname, int speed)
{
	printf ("Attempting to connect to %s\n", portname);
	_handle = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (_handle < 0) {
		printf("Error opening %s: %s\n", portname, strerror(errno));
		return -1;
	} else {
		printf ("Successfully opened %s with file descriptor %d\n", portname, _handle);
	}

	set_interface_attribs(_handle, speed);
	set_mincount(_handle, 0); /* set to pure timed read */
	return 0;
}

int serialRead(char* buf, size_t len, int timeoutMs)
{
	struct pollfd pfd[1];
	pfd[0].fd = _handle;
	pfd[0].events = POLLIN;
	//printf("before poll\n");
	int result = poll(pfd, 1, timeoutMs);
	if(result < 0)
	{
		fprintf(stderr, "Error polling for serial: %d %s\n", errno, strerror(errno));
		return errno;
	} else if (result == 0) {
		printf("Timeout\n");
		// timeout
		return 0;
	} else if (pfd[0].revents & POLLIN){
		//printf("before read\n");
		int rdlen = read(_handle, buf, len);
		//printf("after read\n");
		if(rdlen > 0)
		{
			if(1 || rdlen != 5)
			{
				printf("Serial read %d bytes: ", rdlen);
				for(unsigned int n = 0; n < rdlen; ++n)
				{
					printf("%03d ", buf[n]);
				}
				printf("\n");
			}
		} 
		else if (rdlen < 0) {
			fprintf(stderr, "Error from read: %d: %s\n", rdlen, strerror(errno));
		}
		return rdlen;
	} else {
		fprintf(stderr, "unknown error while reading serial\n");
		return -1;
	}
}

int serialWrite(const char* buf, size_t len)
{
	if(0)
	{
		printf("Writing %d bytes: ", len);
		for(unsigned int n = 0; n < len; ++n)
			printf("%d ", buf[n]);
		printf("\n");
	}
	int ret = write(_handle, buf, len);
	if(ret < 0) {
		fprintf(stderr, "write failed: %d %s\n", errno, strerror(errno));
	}
	return ret;
}

void serialCleanup()
{
	close(_handle);
}
bool serialSetup(const char* device)
{
	return !initSerial(device, B115200);
}

#if 1
// example application
#include <signal.h>
#include <stdint.h>
#include <pthread.h>

int gShouldStop;
int gShouldSendScans;
int frameDataLength = 25;

int sendStatusFrame(int octaves)
{
	char frameBuffer[TOUCHKEY_MAX_FRAME_LENGTH];
	int len = 0;
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameBegin;
	frameBuffer[len++] = kFrameTypeStatus;
	frameBuffer[len++] = 1; // hardware version
	frameBuffer[len++] = 2; // software version major
	frameBuffer[len++]= 0; // software version minor
	frameBuffer[len++] = //flags
		0
		| kStatusFlagHasAnalog
		| kStatusFlagHasI2C
		;
	frameBuffer[len++] = octaves; // octaves 
	frameBuffer[len++] = 0; //lowestHardwareNote
	// two bytes for each octave, saying what keys are connected
	for(unsigned int n = 0; n < octaves; ++n)
	{
		frameBuffer[len++] = 0xf;
		frameBuffer[len++] = 0xff;
	}
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameEnd;
	return serialWrite(frameBuffer, len);
}

int sendScanFrame(unsigned char octave, uint32_t timestamp, float* data, int offset)
{
	char frameBuffer[TOUCHKEY_MAX_FRAME_LENGTH];
	int len = 0;
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameBegin;
	frameBuffer[len++] = kFrameTypeAnalog;
	// Format: [Octave] [TS0] [TS1] [TS2] [TS3] [Key0L] [Key0H] [Key1L] [Key1H] ... [Key24L] [Key24H]
	//                   ... (more frames)
	//                  [TS0] [TS1] [TS2] [TS3] [Key0L] [Key0H] [Key1L] [Key1H] ... [Key24L] [Key24H]
	frameBuffer[len++] = octave; //octave
	// timestamp (i.e. frame ID generated by the device). 32-bit little-endian.
	memcpy(&frameBuffer[len], &timestamp, sizeof(timestamp));
	len += sizeof(timestamp);
	for(unsigned int n = 0; n < offset; ++n)
	{
		int16_t value = 0;
		memcpy(&frameBuffer[len], &value, sizeof(value));
		len += sizeof(value);
	}
	for(unsigned int n = offset; n < frameDataLength; ++n)
	{
		int16_t value = (1-data[n]) * 4096.f;
		memcpy(&frameBuffer[len], &value, sizeof(value));
		len += sizeof(value);
	}
	frameBuffer[len++] = ESCAPE_CHARACTER;
	frameBuffer[len++] = kControlCharacterFrameEnd;
	int ret = serialWrite(frameBuffer, len);
	if(0 > ret)
	{
		gShouldSendScans = 0;
		printf("Failed writing, breaking\n");
		return -1;
	}
	return len;
}

//#define DUMMY
#ifdef DUMMY
void* writeThreadLoop(void* arg)
{
	while(!gShouldStop)
	{
		while(gShouldSendScans && !gShouldStop)
		{
			float data[frameDataLength];
			static int count = 0;
			// fake some channels
			for(unsigned int n = 0; n < frameDataLength; ++n)
			{
				float value = n;
				if(10 == n)
				{
					value = (count * 3) % 8192;
					if(value >= 4096)
						value = 8192 - value;
				}
				value /= 4096.f;
				data[n] = value;
			}
			sendScanFrame(0, count, data, 0);
			count++;
			usleep(2000);
		}
		usleep(50000);
	}
	return NULL;
}

pthread_t writeThread;
int start_write_thread()
{
	return pthread_create(&writeThread, NULL, writeThreadLoop, NULL);
}
#else /* DUMMY */
#include <Keys.h>
int gXenomaiInited = 0; // required by libbelaextra
unsigned int gAuxiliaryTaskStackSize  = 1 << 17; // required by libbelaextra
BoardsTopology bt;
Keys* keys;
unsigned int octaves;
void postCallback(void* arg, float* buffer, unsigned int length)
{
	if(!gShouldSendScans)
		return;
	Keys* keys = (Keys*)arg;
	unsigned int numKeys = bt.getHighestNote() - bt.getLowestNote() + 1;
	float values[numKeys];
	for(int n = bt.getLowestNote(); n <= bt.getHighestNote(); ++n)
	{
		values[n-bt.getLowestNote()] = keys->getNoteValue(n);
	}
	static int count = 0;
	for(unsigned int octave = 0; octave < octaves; octave += 2)
	{
		int offset;
		if(octave == 0)
		{
			offset = bt.getLowestNote() % 12;
		} else {
			offset = 0;
		}
		float* data = values + octave * 12 + offset;
		sendScanFrame(octave, count, data, offset);
	}
	++count;
}
#endif /* DUMMY */

#define SERIAL_BUFFER_SIZE 1024
static char serialBuffer[SERIAL_BUFFER_SIZE];
void interrupt_handler(int)
{
	gShouldStop = 1;
}
int main()
{
	serialSetup("/dev/ttyGS0");
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);
	int dummy = 0;
	auto path = "/root/serial-calibration.txt";
#ifdef DUMMY
	start_write_thread();
	int octaves = 2;
#else /* DUMMY */
	keys = new Keys;
	bt.setLowestNote(0);
	bt.setBoard(0, 0, 24);
	bt.setBoard(1, 0, 23);
	bt.setBoard(2, 10, 23);
	keys->setPostCallback(postCallback, keys);
	int ret = keys->start(&bt, NULL);
	keys->startTopCalibration();
	keys->loadLinearCalibrationFile(path);
	int bottomKey = bt.getLowestNote();
	int topKey = bt.getHighestNote();
	int bottomOctave = bottomKey / 12;
	int topOctave = topKey / 12;
	octaves = topOctave - bottomOctave + 1;
	printf("Using %d real octaves (notes %d to %d)\n", octaves, bottomKey, topKey);
#endif /* DUMMY */
	while(!gShouldStop)
	{
		int ret = serialRead(serialBuffer, SERIAL_BUFFER_SIZE, -1);
		if(ret > 0)
		{
			if(ESCAPE_CHARACTER == serialBuffer[0])
			{
				if(kControlCharacterFrameBegin == serialBuffer[1])
				{
					if(kFrameTypeStatus == serialBuffer[2])
					{
						sendStatusFrame(octaves);
						printf(">> kFrameTypeStatus\n");
					} else if (kFrameTypeStartScanning == serialBuffer[2]) {
						printf(">> StartScanning\n");
						gShouldSendScans = 1;
					} else if (kFrameTypeStopScanning == serialBuffer[2]) {
						printf(">> Stop scanning\n");
						gShouldSendScans = 0;
					}
				}
			} else {
				printf("Received raw byte: %d\n", serialBuffer[0]);
			}
		}
		usleep(50000);
	}
#ifdef DUMMY
	printf("pthread_join\n");
	pthread_join(writeThread, NULL);
	printf("pthread_joined\n");
#else /* DUMMY */
	keys->stopAndWait();
	delete keys;
#endif /* DUMMY */
	serialCleanup();
}
#endif
