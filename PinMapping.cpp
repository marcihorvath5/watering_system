// 
// 
// 

#include "PinMapping.h"

namespace PinMapping
{
	// Maps ArduinoPin to PinDesc (port and bit)
	const PinDesc Map(pins::ArduinoPin pin)
	{
		switch (pin)
		{
		// Digital Pins
		case pins::ArduinoPin::D0: return PinDesc{ 3, 1 };   // P301
		case pins::ArduinoPin::D1: return PinDesc{ 3, 2 };   // P302
		case pins::ArduinoPin::D2: return PinDesc{ 1, 4 };   // P104
		case pins::ArduinoPin::D3: return PinDesc{ 1, 5 };   // P105
		case pins::ArduinoPin::D4: return PinDesc{ 1, 6 };   // P106
		case pins::ArduinoPin::D5: return PinDesc{ 1, 7 };   // P107
		case pins::ArduinoPin::D6: return PinDesc{ 1, 11 };  // P111
		case pins::ArduinoPin::D7: return PinDesc{ 1, 12 };  // P112
		case pins::ArduinoPin::D8: return PinDesc{ 3, 4 };   // P304
		case pins::ArduinoPin::D9: return PinDesc{ 3, 3 };   // P303
		case pins::ArduinoPin::D10: return PinDesc{ 1, 3 };  // P103
		case pins::ArduinoPin::D11: return PinDesc{ 4, 11 }; // P411
		case pins::ArduinoPin::D12: return PinDesc{ 4, 10 }; // P410
		case pins::ArduinoPin::D13: return PinDesc{ 1, 2 };  // P102
		case pins::ArduinoPin::D14: return PinDesc{ 0, 14 }; // P014
		case pins::ArduinoPin::D15: return PinDesc{ 0, 0 };  // P000
		case pins::ArduinoPin::D16: return PinDesc{ 0, 1 };  // P001
		case pins::ArduinoPin::D17: return PinDesc{ 0, 2 };  // P002
		case pins::ArduinoPin::D18: return PinDesc{ 1, 1 };  // P101
		case pins::ArduinoPin::D19: return PinDesc{ 1, 0 };  // P100
		
		// Analog Pins
		case pins::ArduinoPin::A0: return PinDesc{ 0, 14 };  // P014
		case pins::ArduinoPin::A1: return PinDesc{ 0, 0 };   // P000
		case pins::ArduinoPin::A2: return PinDesc{ 0, 1 };   // P001
		case pins::ArduinoPin::A3: return PinDesc{ 0, 2 };   // P002
		case pins::ArduinoPin::A4: return PinDesc{ 1, 1 };   // P101
		case pins::ArduinoPin::A5: return PinDesc{ 1, 0 };   // P100
		default:
			return PinDesc{ 0xFF,0xFF }; // Invalid pin
		}
	}
}
