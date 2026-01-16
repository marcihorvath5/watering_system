// PinMapping.h

#pragma once
#include <stdint.h>
#include "pins.h"



namespace PinMapping
{
	typedef struct
	{
		uint8_t port;
		uint8_t bit;
	} PinDesc;
	extern const PinDesc Map(pins::ArduinoPin pin);
}