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
	static volatile auto* const SCI = R_SCI1; // SCI1 register base address
	static volatile auto* const MSTP = R_MSTP; // MSTP register base address
	static volatile R_SYSTEM_Type* const systemClockDivision= R_SYSTEM;
	
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
		auto& pinFunctionRegister = PinFunctionSelect->PORT[pin.port].PIN[pin.bit].PmnPFS;
		Serial.print("Original PmnPFS register:");
		Serial.print(pinFunctionRegister, BIN);
		Serial.println();
		constexpr uint32_t I2CFunctionBits = 0b00111u; // I2C function selection bits
		constexpr uint32_t I2CFunctionMask = 0b11111u; // Mask for bits 24-28
		constexpr uint8_t I2CFunctionShift = 24; // Shift amount for bits 24-28


		switch (function)
		{
		case gpio::Function::GPIO:
			// Set PmnPFS bit 16 to 0 for GPIO function
			pinFunctionRegister &= ~(1u << 16);

			// Set PmnPFS bit 0 to 1 for GPIO function
			pinFunctionRegister |= (1u << 0);
			break;
		case gpio::Function::I2C:

			// SCR to 0x00 to disable transmitter and receiver
			SCI->SCR = 0x00;

			// Configure for I2C function
			pinFunctionRegister |= (1u << 16); // Set PmnPFS bit 16 to 1 for peripheral function
			pinFunctionRegister &= ~(I2CFunctionMask << I2CFunctionShift); // Clear bits 24-28
			pinFunctionRegister &= ~(1u << 1); // Clear bits 24-28
			pinFunctionRegister |= (I2CFunctionBits << I2CFunctionShift); // Set bits 24-28 for I2C function
			pinFunctionRegister |= (1u << 6); // Set bit 6 to enable open-drain output
			
			break;
		default:
			break;
		}

		Serial.print("PmnPFS regiszter: ");
		Serial.print(pinFunctionRegister,BIN);
		Serial.println();
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

	void InitializeI2C()
	{
		uint8_t highImpedanceBits = 0b11; // Bits to set SCL/SDA in high impedance since it is needed to IIC init
		volatile auto& portProtectRegister = systemClockDivision->PRCR;
		Serial.print("Original PRCR register: ");
		Serial.print(portProtectRegister, BIN);
		Serial.println();

		volatile auto& moduleStopControlRegister = MSTP->MSTPCRB; // Reference to the MSTPCRB register
		Serial.print("Original MSTPCRB register: ");
		Serial.print(moduleStopControlRegister, BIN);
		Serial.println();

		volatile auto& simr3Register = SCI->SIMR3; // Reference to the SIMR3 register, here are the SCL/SDA high impedance bits
		Serial.print("Original SCI SIMR3 register: ");
		Serial.print(simr3Register, HEX);
		Serial.println();

		volatile auto& smrRegister = SCI->SMR; // Reference to the SMR register Serial Mode Register
		Serial.print("Original SCI SMR register: ");
		Serial.print(smrRegister, BIN);
		Serial.println();

		volatile auto& scmrRegister = SCI->SCMR; // Reference to the SCMR register Smart Card Mode Register
		Serial.print("Original SCI SCMR register: ");
		Serial.print(scmrRegister, BIN);
		Serial.println();

		volatile uint8_t& brrRegister = SCI->BRR; // Reference to the BRR register Bit Rate Register
		Serial.print("Original SCI BRR register: ");
		Serial.print(brrRegister, HEX);

		// Cancel module stop for IIC1 and SCI1
		moduleStopControlRegister &= ~(1u << 8); // Clear MSTPB 8 to cancel the module stop IIC1
		moduleStopControlRegister &= ~(1u << 30); // Clear MSTPB 30 to cancel the module stop SCI1
		Serial.print("Modified MSTPCRB register: ");
		Serial.print(moduleStopControlRegister, BIN);
		Serial.println();

		// 2. step: Configure SCI registers for I2C operation
		// Set SCL and SDA to high impedance
		simr3Register = 0xF0; // Set SIMR3 register to 0xF0 1111 0000 to set SCL/SDA to high impedance
		Serial.print("Modified SCI SIMR3 register: ");
		Serial.print(simr3Register, HEX);
		Serial.println();

		// 3. step: Configure SMR and SCMR registers
		smrRegister = 0x00; // Set SMR register to 0x00 0000 0000 to select clock source PCLK
		Serial.print("Modified SCI SMR register: ");
		Serial.print(smrRegister, BIN);
		Serial.println();

		scmrRegister |= (1u << 3); // Set SCMR bit 3 to 1 to set Transfer with MSB first
		scmrRegister &= ~(1u << 2); // Clear SCMR bit 2 to 0 to disable data reverse
		scmrRegister &= ~(1u << 0); // Clear SCMR bit 0 to 0 to disable smart card mode
		Serial.print("Modified SCI SCMR register: ");
		Serial.print(scmrRegister, BIN);
		Serial.println();

		// Step 4: Set the baud rate for I2C communication
		
		brrRegister = 0x0E; // Set BRR register to 0x0E(14) for 100 kHz I2C clock assuming PCLK is 48 MHz
		Serial.print("Modified SCI BRR register(dec): ");
		Serial.println(brrRegister, DEC);

		
		// Assuming PCLK is 48 MHz and desired I2C clock is 100 kHz
		/*uint32_t pclka_hz = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_PCLKA);
		Serial.print("PCLKA frequency: ");
		Serial.println(pclka_hz);

		uint32_t iclk_hz = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_ICLK);
		Serial.print("ICLK frequency: ");
		Serial.println(iclk_hz);*/
	}
}
