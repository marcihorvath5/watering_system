// 
// 
// 

#include "gpio.h"
#include <hal_data.h>

namespace gpio
{
	// Setting pin mode starting from bit 16 therefore offset by 16
	//const uint8_t startBit = 16;

	// Register macros to access the hardware registers
	static volatile R_PMISC_Type* const PortWriteProtectRegister = R_PMISC; // PWPR register is located in PMISC, used for write protection of PFS only to set functions
	static volatile R_PORT0_Type* const PORT1 = R_PORT1; // PORT1 register base address
	static volatile R_PFS_Type* const PinFunctionSelect = R_PFS; // PFS register base address

	void SetDirection(uint8_t pinNumber, Direction direction)
	{
		switch (direction)
		{
		case gpio::Direction::INPUT:
			PORT1->PDR &= ~(1u << pinNumber); // Set as input
			break;
		case gpio::Direction::OUTPUT:
			PORT1->PDR |= (1u << pinNumber); // Set as output
			break;
		default:
			break;
		}
	}

	void SetFunction(uint8_t portNumber, uint8_t pinNumber, Function function)
	{
		gpio::UnlockPinFunctionWriteProtection();

		switch (function)
		{
		case gpio::Function::GPIO:
			// Set PmnPFS bit 16 to 0 for GPIO function
			PinFunctionSelect->PORT[portNumber].PIN[pinNumber].PmnPFS &= ~(1u << 16);

			// Set PmnPFS bit 0 to 1 for GPIO function
			PinFunctionSelect->PORT[portNumber].PIN[pinNumber].PmnPFS |= (1u << 0);
			break;
		default:
			break;
		}
		gpio::LockPinFunctionWriteProtection();
	}

	void LockPinFunctionWriteProtection()
	{

		PortWriteProtectRegister->PWPR = 0x00; // Clear bit PFSWE to disable write to Pin Function Select 0x00 = 0000 0000

		PortWriteProtectRegister->PWPR = 0x80; // Set  B0WI bit to disable write PSFSWE 0x80 = 1000 0000
	}

	void Write(uint8_t pinNumber, Level level)
	{
		if (level == gpio::Level::Low)
		{
			// Write low. Changes only the bit corresponding to pinNumber to 0
			PORT1->PODR &= ~(1u << pinNumber);
			return;
		}

		// Write high. Changes only the bit corresponding to pinNumber to 1
		PORT1->PODR |= (1u << pinNumber);
	}

	void UnlockPinFunctionWriteProtection()
	{
		PortWriteProtectRegister->PWPR = 0x00; // Clear  B0WI bit to enable write PSFSWE 0x00 = 0000 0000
		PortWriteProtectRegister->PWPR = 0x40;; // Set bit PFSWE to enable write to Pin Function Select 0x40 = 0100 0000
	}
}
