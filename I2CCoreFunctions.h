// I2CCoreFunctions.h

#pragma once
#include <hal_data.h>
#include <stdint.h>

namespace core_functions
{
	bool GenerateStart();
	bool WriteIntoSlave(uint8_t addresss);
	bool GenerateStop();
	bool WaitingWithTimeout(volatile uint8_t& reg, uint8_t bitMask, bool expectedState, uint32_t timeout=100000);
	bool ReadByte(uint8_t slaveAddress, uint8_t* readData);
}