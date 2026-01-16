// gpio.h

#pragma once
#include <stdint.h>

// Enum for pin modes
namespace gpio
{
	// Enum for pin functions
	enum class Function: uint8_t
	{
		GPIO
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

	// This area defines the functions to pins and setting their bits
	void SetDirection(uint8_t pinNumber, Direction direction); // Sets the direction of a pin (input/output)
	void SetFunction(uint8_t portNumber, uint8_t pinNumber, Function function); // Sets the function of a pin (GPIO/Alternate)
	void UnlockPinFunctionWriteProtection(); // Unlocks the pin function write protection
	void LockPinFunctionWriteProtection(); // Locks the pin function write protection
	void Write(uint8_t pinNumber, gpio::Level state); // Writes a level (high/low) to a pin
}

