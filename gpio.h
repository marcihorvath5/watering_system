// gpio.h

#pragma once
#include <stdint.h>
#include "pins.h"
#include "PinMapping.h"

// Enum for pin modes
namespace gpio
{
	// Enum for pin functions
	enum class Function: uint8_t
	{
		GPIO,
		I2C,
	};

	// Enum for pin directions
	enum class Direction : uint8_t
	{
		INPUT,
		OUTPUT
	};

	// Enum for pin levels
	enum class Level : uint8_t
	{
		High,
		Low
	};

	// enum for I2C modes
	enum class I2CMode : uint8_t
	{
		Simple
	};

	// This area defines the functions to pins and setting their bits
	void SetDirection(pins::ArduinoPin pin, Direction direction); // Sets the direction of a pin (input/output)
	void SetFunction(pins::ArduinoPin pin, Function function); // Sets the function of a pin (GPIO/Alternate)
	void UnlockPinFunctionWriteProtection(); // Unlocks the pin function write protection
	void LockPinFunctionWriteProtection(); // Locks the pin function write protection
	void Write(pins::ArduinoPin pin, gpio::Level state); // Writes a level (high/low) to a pin
	void InitializeI2C(); // Initialize the I2C
	void StartI2C();
	void ReadI2C();
	void StopI2C();
}

