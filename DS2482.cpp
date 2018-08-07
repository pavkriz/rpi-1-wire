#include "DS2482.h"

extern "C" {
	#include <sys/ioctl.h>
	#include <linux/i2c-dev.h>
	#include <unistd.h>
	#include <time.h>
	#include <stdio.h>
}

#define PTR_STATUS 0xf0
#define PTR_READ 0xe1
#define PTR_CONFIG 0xd2
#define PTR_1WIRE_RESET 0xb4
#define PTR_WRITE_BYTE 0xa5
#define PTR_READ_BYTE 0x96
#define PTR_WRITE_BIT 0x87
#define PTR_TRIPLET 0x78

#define WIRE1_CMD_SKIP_ROM 0xcc
#define WIRE1_CMD_MATCH_ROM 0x55
#define WIRE1_CMD_SEARCH_ROM 0xf0

DS2482::DS2482(int fh, uint8_t addr)
{
	mFh = fh;
	mAddress = 0x18 | addr;
	reset_search();
}

//-------helpers
int DS2482::setReadPtr(uint8_t readPtr)
{
	if (::ioctl(mFh,I2C_SLAVE,mAddress) < 0) {
		return RESULT_ERROR_SET_I2C_ADDR_FAILED;
	}

	uint8_t buf[2] = { PTR_READ, readPtr }; 
	if (::write(mFh,buf,2) != 2) {
		return RESULT_ERROR_SETTING_PTR_READ;
	}

	return RESULT_OK;
}

int DS2482::readByte()
{
	uint8_t buf[1];

	if (::ioctl(mFh,I2C_SLAVE,mAddress) < 0) {
		return RESULT_ERROR_SET_I2C_ADDR_FAILED;
	}

	if (::read(mFh,buf,1) != 1) {
		return RESULT_ERROR_READING_BYTE;
	}

	return buf[0];
}

void DS2482::delayMicroseconds(int us)
{
	struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = us*1000;
    //::nanosleep(&ts, NULL);
}

int DS2482::wireReadStatus(bool setPtr)
{
	if (setPtr) {
		int status = setReadPtr(PTR_STATUS);
		if (status < 0) {
			return status;
		}
	}		
	
	return readByte();
}

int DS2482::busyWait(bool setReadPtr)
{
	int status;
	int loopCount = 1000;
	status = wireReadStatus(setReadPtr);
	while(status >= 0 && (status & DS2482_STATUS_BUSY))
	{
		if (--loopCount <= 0)
		{
			mTimeout = 1;
			break;
		}
		delayMicroseconds(20);
		status = wireReadStatus(setReadPtr);
	}
	//printf("busyWait loops %d\n", loopCount);
	return status;
}

//----------interface
int DS2482::resetMaster()
{
	mTimeout = 0;

	if (::ioctl(mFh,I2C_SLAVE,mAddress) < 0) {
		return RESULT_ERROR_SET_I2C_ADDR_FAILED;
	}

	uint8_t buf[1] = { PTR_STATUS }; 
	if (::write(mFh,buf,1) != 1) {
		return RESULT_ERROR_RESETTING_MASTER;
	}

	return RESULT_OK;
}

int DS2482::configure(uint8_t config)
{
	int status;

	if ((status = busyWait(true)) < 0) {
		return status;
	}	

	uint8_t buf[2] = { PTR_CONFIG, (uint8_t)(config | (~config)<<4) }; 
	if (::write(mFh,buf,2) != 2) {
		return RESULT_ERROR_CONFIGURING_MASTER;
	}

	int byteRead = readByte();
	if (byteRead < 0) {
		return byteRead;
	} else if (byteRead != config) {
		return RESULT_ERROR_CONFIGURING_MASTER;
	} else {
		return RESULT_OK;
	}
}

bool DS2482::selectChannel(uint8_t channel)
{	
	// uint8_t ch, ch_read;

	// switch (channel)
	// {
	// 	case 0:
	// 	default:  
	// 		ch = 0xf0; 
	// 		ch_read = 0xb8; 
	// 		break;
	// 	case 1: 
	// 		ch = 0xe1; 
	// 		ch_read = 0xb1; 
	// 		break;
	// 	case 2: 
	// 		ch = 0xd2; 
	// 		ch_read = 0xaa; 
	// 		break;
	// 	case 3: 
	// 		ch = 0xc3; 
	// 		ch_read = 0xa3; 
	// 		break;
	// 	case 4: 
	// 		ch = 0xb4; 
	// 		ch_read = 0x9c; 
	// 		break;
	// 	case 5: 
	// 		ch = 0xa5; 
	// 		ch_read = 0x95; 
	// 		break;
	// 	case 6: 
	// 		ch = 0x96; 
	// 		ch_read = 0x8e; 
	// 		break;
	// 	case 7: 
	// 		ch = 0x87; 
	// 		ch_read = 0x87; 
	// 		break;
	// };

	// busyWait(true);
	// Wire.beginTransmission(mAddress);
	// Wire.write(0xc3);
	// Wire.write(ch);
	// Wire.endTransmission();
	// busyWait();
	
	// uint8_t check = readByte();
	
	// return check == ch_read;
}

bool DS2482::reset()
{
	int status;

	if ((status = busyWait(true)) < 0) {
		return status;
	}	

	uint8_t buf[1] = { PTR_1WIRE_RESET }; 
	if (::write(mFh,buf,1) != 1) {
		return RESULT_ERROR_RESETTING_1WIRE;
	}
	
	status = busyWait();
	if (status < 0) {
		return status;
	}

	return (status & DS2482_STATUS_PPD) ? RESULT_SLAVE_DETECTED : RESULT_OK;
}


int DS2482::write(uint8_t b, uint8_t power)
{
	int status;

	if ((status = busyWait(true)) < 0) {
		return status;
	}
	
	uint8_t buf[2] = { PTR_WRITE_BYTE, b }; 
	if (::write(mFh,buf,2) != 2) {
		return RESULT_ERROR_WRITING_BYTE;
	}

	return RESULT_OK;
}

int DS2482::read()
{
	int status;

	if ((status = busyWait(true)) < 0) {
		return status;
	}

	uint8_t buf[1] = { PTR_READ_BYTE }; 
	if (::write(mFh,buf,1) != 1) {
		return RESULT_ERROR_ISSUING_READ_BYTE;
	}

	if ((status = busyWait()) < 0) {
		return status;
	}

	if ((status = setReadPtr(PTR_READ)) < 0) {
		return status;
	}

	return readByte();
}

int DS2482::write_bit(uint8_t bit)
{
	int status;

	if ((status = busyWait(true)) < 0) {
		return status;
	}

	uint8_t buf[2] = { PTR_WRITE_BIT, (uint8_t)(bit ? 0x80 : 0) }; 
	if (::write(mFh,buf,2) != 2) {
		return RESULT_ERROR_WRITING_BIT;
	}
	
	return RESULT_OK;
}

int DS2482::read_bit()
{
	int status;

	if ((status = write_bit(1)) < 0) {
		return status;
	}

	if ((status = busyWait(true)) < 0) {
		return status;
	}

	return (status & DS2482_STATUS_SBR) ? 1 : 0;
}

int DS2482::skip()
{
	return write(WIRE1_CMD_SKIP_ROM);
}

int DS2482::select(uint8_t rom[8])
{
	int status;

	if ((status = write(WIRE1_CMD_MATCH_ROM)) < 0) {
		return status;
	}
	for (int i=0;i<8;i++) {
		if ((status = write(rom[i])) < 0) {
			return status;
		}
	}

	return RESULT_OK;
}


#if ONEWIRE_SEARCH
void DS2482::reset_search()
{
	searchExhausted = 0;
	// Initialize to negative value
	searchLastDisrepancy = -1;
	
	for(uint8_t i = 0; i<8; i++) 
		searchAddress[i] = 0;
}

int DS2482::search(uint8_t *newAddr)
{
	int status;
	uint8_t i;
	uint8_t direction;
	uint8_t last_zero=0;
	
	if (searchExhausted) 
		return 0;
	
	status = reset();
	if (status < 0) {
		return status;
	}

	if (status != RESULT_SLAVE_DETECTED) {
		return 0;
	}

	if ((status = busyWait(true)) < 0) {
		return status;
	}

	if ((status = write(WIRE1_CMD_SEARCH_ROM)) < 0) {
		return status;
	}
	
	for(i=0;i<64;i++) 
	{
		int romByte = i/8;
		int romBit = 1<<(i&7);
		
		if (i < searchLastDisrepancy)
			direction = searchAddress[romByte] & romBit;
		else
			direction = i == searchLastDisrepancy;
		
		if ((status = busyWait()) < 0) {
			return status;
		}

		uint8_t buf[2] = { PTR_TRIPLET, (uint8_t)(direction ? 0x80 : 0) }; 
		if (::write(mFh,buf,2) != 2) {
			return RESULT_ERROR_WRITING_TRIPLET;
		}

		if ((status = busyWait()) < 0) {
			return status;
		}
		
		uint8_t id = status & DS2482_STATUS_SBR;
		uint8_t comp_id = status & DS2482_STATUS_TSB;
		direction = status & DS2482_STATUS_DIR;
		
		if (id && comp_id)
			return 0;
		else
		{ 
			if (!id && !comp_id && !direction)
				last_zero = i;
		}
		
		if (direction)
			searchAddress[romByte] |= romBit;
		else
			searchAddress[romByte] &= (uint8_t)~romBit;
	}

	searchLastDisrepancy = last_zero;

	if (last_zero == 0) 
		searchExhausted = 1;
	
	for (i=0;i<8;i++) 
		newAddr[i] = searchAddress[i];
	
	return 1;  
}
#endif

#if ONEWIRE_CRC
// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
//

uint8_t DS2482::crc8( uint8_t *addr, uint8_t len)
{
	uint8_t crc=0;
	
	for (uint8_t i=0; i<len;i++) 
	{
		uint8_t inbyte = addr[i];
		for (uint8_t j=0;j<8;j++) 
		{
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) 
				crc ^= 0x8C;
			
			inbyte >>= 1;
		}
	}
	return crc;
}

#endif
