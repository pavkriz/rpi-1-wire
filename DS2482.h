/*
  DS2482 library for Arduino
  Copyright (C) 2009 Paeae Technologies

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
*/

#ifndef __DS2482_H__
#define __DS2482_H__

#include <inttypes.h>

// you can exclude onewire_search by defining that to 0
#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif

// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif


#define DS2482_CONFIG_APU (1<<0)
#define DS2482_CONFIG_PPM (1<<1)
#define DS2482_CONFIG_SPU (1<<2)
#define DS2484_CONFIG_WS  (1<<3)

#define DS2482_STATUS_BUSY 	(1<<0)
#define DS2482_STATUS_PPD 	(1<<1)
#define DS2482_STATUS_SD	(1<<2)
#define DS2482_STATUS_LL	(1<<3)
#define DS2482_STATUS_RST	(1<<4)
#define DS2482_STATUS_SBR	(1<<5)
#define DS2482_STATUS_TSB	(1<<6)
#define DS2482_STATUS_DIR	(1<<7)

class DS2482
{
public:
	//Address is 0-3
	DS2482(int fh, uint8_t address);
	
	int configure(uint8_t config);
	int resetMaster();
	
	//DS2482-800 only
	bool selectChannel(uint8_t channel);
	
	bool reset(); // return true if presence pulse is detected
	int wireReadStatus(bool setPtr=false);
	
	int write(uint8_t b, uint8_t power = 0);
	int read();
	
	int write_bit(uint8_t bit);
	int read_bit();
    // Issue a 1-Wire rom select command, you do the reset first.
    int select( uint8_t rom[8]);
	// Issue skip rom
	int skip();
	
	uint8_t hasTimeout() { return mTimeout; }
#if ONEWIRE_SEARCH
    // Clear the search state so that if will start from the beginning again.
    void reset_search();

    // Look for the next device. Returns 1 if a new address has been
    // returned. A zero might mean that the bus is shorted, there are
    // no devices, or you have already retrieved all of them.  It
    // might be a good idea to check the CRC to make sure you didn't
    // get garbage.  The order is deterministic. You will always get
    // the same devices in the same order.
    int search(uint8_t *newAddr);
#endif
#if ONEWIRE_CRC
    // Compute a Dallas Semiconductor 8 bit CRC, these are used in the
    // ROM and scratchpad registers.
    static uint8_t crc8( uint8_t *addr, uint8_t len);
#endif

    static constexpr int RESULT_OK = 0;
    static constexpr int RESULT_SLAVE_DETECTED = 1;
    static constexpr int RESULT_ERROR_SET_I2C_ADDR_FAILED = -1;
    static constexpr int RESULT_ERROR_SETTING_PTR_READ = -2;
    static constexpr int RESULT_ERROR_READING_BYTE = -3;
    static constexpr int RESULT_ERROR_RESETTING_MASTER = -4;
    static constexpr int RESULT_ERROR_CONFIGURING_MASTER = -5;
    static constexpr int RESULT_ERROR_RESETTING_1WIRE = -6;
    static constexpr int RESULT_ERROR_WRITING_BYTE = -7;
    static constexpr int RESULT_ERROR_ISSUING_READ_BYTE = -8;
    static constexpr int RESULT_ERROR_WRITING_BIT = -9;
    static constexpr int RESULT_ERROR_WRITING_TRIPLET = -10;

private:
	
	int mFh; // I2C bus sysfs filehandle
    uint8_t mAddress;
	uint8_t mTimeout = 0;
	int readByte();
	int setReadPtr(uint8_t readPtr);
    void delayMicroseconds(int us);
	
	int busyWait(bool setReadPtr=false); //blocks until
	
#if ONEWIRE_SEARCH
	uint8_t searchAddress[8];
	int8_t searchLastDisrepancy;
	uint8_t searchExhausted;
#endif
	
};



#endif
