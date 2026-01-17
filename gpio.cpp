// 
// 
// 

#include "gpio.h"
#include <hal_data.h>
#include <Arduino.h>


namespace gpio
{
	// Setting pin mode starting from bit 16 therefore offset by 16
	//const uint8_t startBit = 16;

	// Register macros to access the hardware registers
	static volatile R_PMISC_Type* const PortWriteProtectRegister = R_PMISC; // PWPR register is located in PMISC, used for write protection of PFS only to set functions
	static volatile R_PORT0_Type* const PORT1 = R_PORT1; // PORT1 register base address
	static volatile R_PFS_Type* const PinFunctionSelect = R_PFS; // PFS register base address

	// Set the direction of the specified pin
	void SetDirection(pins::ArduinoPin p, Direction direction)
	{
		PinMapping::PinDesc pin = PinMapping::Map(p); // Get port and bit number from pin mapping

		switch (direction)
		{
		case gpio::Direction::INPUT:
			PORT1->PDR &= ~(1u << pin.bit); // Set as input
			break;
		case gpio::Direction::OUTPUT:
			PORT1->PDR |= (1u << pin.bit); // Set as output
			break;
		default:
			break;
		}
	}

	// Set the function of the specified pin
	void SetFunction(pins::ArduinoPin p, Function function)
	{
		PinMapping::PinDesc pin = PinMapping::Map(p); // Get port and bit number from pin mapping

		// Reference to the Pin Function Select register for the specified pin
		auto& reg = PinFunctionSelect->PORT[pin.port].PIN[pin.bit].PmnPFS;
		Serial.print("Original PmnPFS register:");
		Serial.print(reg, BIN);
		Serial.println();
		constexpr uint32_t I2CFunctionBits = 0b00111u; // I2C function selection bits
		constexpr uint32_t I2CFunctionMask = 0b11111u; // Mask for bits 24-28
		constexpr uint8_t I2CFunctionShift = 24; // Shift amount for bits 24-28


		gpio::UnlockPinFunctionWriteProtection();

		switch (function)
		{
		case gpio::Function::GPIO:
			// Set PmnPFS bit 16 to 0 for GPIO function
			reg &= ~(1u << 16);

			// Set PmnPFS bit 0 to 1 for GPIO function
			reg |= (1u << 0);
			break;
		case gpio::Function::I2C:
			// Configure for I2C function
			reg |= (1u << 16); // Set PmnPFS bit 16 to 1 for peripheral function
			reg &= ~(I2CFunctionMask << I2CFunctionShift); // Clear bits 24-28
			reg |= (I2CFunctionBits << I2CFunctionShift); // Set bits 24-28 for I2C function
			break;
		default:
			break;
		}
		gpio::LockPinFunctionWriteProtection();

		Serial.print("PmnPFS regiszter: ");
		Serial.print(reg,BIN);
		Serial.println();
	}

	// Lock the pin function write protection
	void LockPinFunctionWriteProtection()
	{

		PortWriteProtectRegister->PWPR = 0x00; // Clear bit PFSWE to disable write to Pin Function Select 0x00 = 0000 0000

		PortWriteProtectRegister->PWPR = 0x80; // Set  B0WI bit to disable write PSFSWE 0x80 = 1000 0000
	}

	// Write a level to the specified pin
	void Write(pins::ArduinoPin p, Level level)
	{
		PinMapping::PinDesc pin = PinMapping::Map(p); // Get port and bit number from pin mapping

		if (level == gpio::Level::Low)
		{
			// Write low. Changes only the bit corresponding to pinNumber to 0
			PORT1->PODR &= ~(1u << pin.bit);
			return;
		}

		// Write high. Changes only the bit corresponding to pinNumber to 1
		PORT1->PODR |= (1u << pin.bit);
	}

	// Unlock the pin function write protection
	void UnlockPinFunctionWriteProtection()
	{
		PortWriteProtectRegister->PWPR = 0x00; // Clear  B0WI bit to enable write PSFSWE 0x00 = 0000 0000
		PortWriteProtectRegister->PWPR = 0x40;; // Set bit PFSWE to enable write to Pin Function Select 0x40 = 0100 0000
	}
}
