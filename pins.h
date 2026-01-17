#pragma once
#include <stdint.h>
namespace pins
{
	// Enum for pins 
	typedef enum
	{
		D0,		// Digital Pin 0/Rx/P301
		D1,		// Digital Pin 1/Tx/P302
		D2,		// Digital Pin 2/P104
		D3,		// Digital Pin 3/P105/PWM
		D4,		// Digital Pin 4/P106			
		D5,		// Digital Pin 5/P107/PWM
		D6,		// Digital Pin 6/P111/PWM
		D7,		// Digital Pin 7/P112
		D8,		// Digital Pin 8/P304
		D9,		// Digital Pin 9/P303/PWM
		D10,	// Digital Pin 10/P103/PWM/SSLA0
		D11,	// Digital Pin 11/P411/PWM/COPIA
		D12,	// Digital Pin 12/P410/CIPOA
		D13,	// Digital Pin 13/P102/RSPCKA
		D14,	// Digital Pin 14/P014
		D15,	// Digital Pin 15/P000
		D16,	// Digital Pin 16/P001
		D17,	// Digital Pin 17/P002
		D18,	// Digital Pin 18/P101/I2CSDA1
		D19,	// Digital Pin 19/P100/I2CSCL1
		Vin,	// External Power Supply  
		GND1,	// Ground
		GND2,	// Ground
		GND3,	// Ground
		AREF,	// Analog Reference
		V5,		// 5V OUT
		V3V3,	// 3.3V OUT
		RESET,	// Reset
		IOREF,	// IO Reference
		A0,		// Analog Pin 0/P014/AN009/DA0
		A1,		// Analog Pin 1/P000/AN000/DA1
		A2,		// Analog Pin 2/P001/AN001
		A3,		// Analog Pin 3/P002/AN002
		A4,		// Analog Pin 4/P101/AN021
		A5		// Analog Pin 5/P100/AN022
	}ArduinoPin;
}