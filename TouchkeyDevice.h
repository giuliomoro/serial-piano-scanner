/*
  TouchKeys: multi-touch musical keyboard control software
  Copyright (c) 2013 Andrew McPherson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
  =====================================================================
 
  TouchkeyDevice.h: handles communication with the TouchKeys hardware
*/

#ifndef TOUCHKEY_DEVICE_H
#define TOUCHKEY_DEVICE_H

#define TOUCHKEY_MAX_FRAME_LENGTH 256	// Maximum data length in a single frame
#define ESCAPE_CHARACTER 0xFE			// Indicates control sequence

//#define TRANSMISSION_LENGTH_WHITE 9
//#define TRANSMISSION_LENGTH_BLACK 8
//#define TRANSMISSION_LENGTH_TOTAL (8*TRANSMISSION_LENGTH_WHITE + 5*TRANSMISSION_LENGTH_BLACK)

const int kTransmissionLengthWhiteOldHardware = 9;
const int kTransmissionLengthBlackOldHardware = 8;
const int kTransmissionLengthWhiteNewHardware = 9;
const int kTransmissionLengthBlackNewHardware = 9;
const int kTransmissionLengthTotalOldHardware = (8 * kTransmissionLengthWhiteOldHardware + 5 * kTransmissionLengthBlackOldHardware);
const int kTransmissionLengthTotalNewHardware = (8 * kTransmissionLengthWhiteNewHardware + 5 * kTransmissionLengthBlackNewHardware);

// Maximum integer values for different types of sliders

//#define WHITE_MAX_VALUE 1280.0		// White keys, vertical	(64 * 20)
//#define WHITE_MAX_H_VALUE 255.0		// Whtie keys, horizontal
//#define BLACK_MAX_VALUE 1024.0		// Black keys, vertical (64 * 16)
//#define SIZE_MAX_VALUE 255.0		// Max touch size for either key type

const float kWhiteMaxYValueOldHardware = 1280.0;    // White keys, vertical	(64 * 20)
const float kWhiteMaxXValueOldHardware = 255.0;     // White keys, horizontal (1 byte)
const float kBlackMaxYValueOldHardware = 1024.0;    // Black keys, vertical (64 * 16)
const float kWhiteMaxYValueNewHardware = 2432.0;    // White keys, vertical (128 * 19)
const float kWhiteMaxXValueNewHardware = 256.0;     // White keys, horizontal (1 byte + 1 bit)
const float kBlackMaxYValueNewHardware = 1536.0;    // Black keys, vertical (128 * 12)
const float kBlackMaxXValueNewHardware = 256.0;     // Black keys, horizontal (1 byte + 1 bit)

const float kSizeMaxValue = 255.0;

enum {
	kControlCharacterFrameBegin = 0x00,
	kControlCharacterAck = 0x01,
	kControlCharacterNak = 0x02,
	kControlCharacterFrameError = 0xFD,
	kControlCharacterFrameEnd = 0xFF
};

// Frame types for data sent over USB.  The first byte following a frame start control sequence gives the type.

enum {
	kFrameTypeStatus = 0,		// Status info: connected keys, current operating modes
	kFrameTypeCentroid = 16,	// Centroid data (default mode of operation)
	kFrameTypeI2CResponse = 17,	// Response from a specific I2C command
	kFrameTypeRawKeyData = 18,	// Raw data from the selected key	
    kFrameTypeAnalog = 19,		// Analog data from Z-axis optical sensors
	
    kFrameTypeErrorMessage = 127, // Error message from controller
	// These types are for incoming (computer -> us) data
	kFrameTypeStartScanning = 128,	// Start auto-scan
	kFrameTypeStopScanning = 129,	// Stop auto-scan
	kFrameTypeSendI2CCommand = 130,	// Send a specific I2C command
	kFrameTypeResetDevices = 131,	// Physically reset the system
	kFrameTypeScanRate = 132,		// Set the scan rate (in milliseconds)	
	kFrameTypeNoiseThreshold = 133,
	kFrameTypeSensitivity = 134,
	kFrameTypeSizeScaler = 135,
	kFrameTypeMinimumSize = 136,
	kFrameTypeSetEnabledKeys = 137,
	kFrameTypeMonitorRawFromKey = 138,
	kFrameTypeUpdateBaselines = 139,	// Reinitialize baseline values
	kFrameTypeRescanKeyboard = 140,	// Rescan what keys are connected
    kFrameTypeEncapsulatedMIDI = 167, // MIDI messages to pass to MIDI standalone firmware
    kFrameTypeRGBLEDSetColors = 168, // Set RGBLEDs of given index to specific values
	kFrameTypeRGBLEDAllOff = 169,    // All LEDs off
	kFrameTypeEnterISPMode = 192,
    kFrameTypeEnterSelfProgramMode = 193
};

enum {
	kKeyColorWhite = 0,
	kKeyColorBlack
};

enum {
	kStatusFlagRunning = 0x01,
	kStatusFlagRawMode = 0x02,
	kStatusFlagHasI2C = 0x04,
	kStatusFlagHasAnalog = 0x08,
	kStatusFlagHasRGBLED = 0x10,
	kStatusFlagComError = 0x80
};


const int kKeyColor[13] = { kKeyColorWhite, kKeyColorBlack, kKeyColorWhite,
	kKeyColorBlack, kKeyColorWhite, kKeyColorWhite, kKeyColorBlack,
	kKeyColorWhite, kKeyColorBlack, kKeyColorWhite, kKeyColorBlack,
	kKeyColorWhite, kKeyColorWhite };

const int kWhiteKeyIndices[13] = { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6, 7};

const unsigned char kCommandStatus[] = { ESCAPE_CHARACTER, kControlCharacterFrameBegin, kFrameTypeStatus,
	ESCAPE_CHARACTER, kControlCharacterFrameEnd };
const unsigned char kCommandStartScanning[] = { ESCAPE_CHARACTER, kControlCharacterFrameBegin, kFrameTypeStartScanning,
	ESCAPE_CHARACTER, kControlCharacterFrameEnd };
const unsigned char kCommandStopScanning[] = { ESCAPE_CHARACTER, kControlCharacterFrameBegin, kFrameTypeStopScanning,
	ESCAPE_CHARACTER, kControlCharacterFrameEnd };

#define octaveNoteToIndex(octave, note) (100*octave + note)	// Generate indices for containers
#define indexToOctave(index) (int)(index / 100)
#define indexToNote(index) (index % 100)

const float kTouchkeyAnalogValueMax = 4095.0; // Maximum value any analog sample can take

#endif /* TOUCHKEY_DEVICE_H */
