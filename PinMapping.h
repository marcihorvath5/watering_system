// PinMapping.h

#pragma once
#include <stdint.h>
#include "pins.h"



namespace PinMapping
{
	// TODO: Maybe add flags for PWM, I2C, SPI, UART capabilities
	// struct to describe a pin's port and bit
	typedef struct
	{
		uint8_t port;
		uint8_t bit;
	} PinDesc;
	extern const PinDesc Map(pins::ArduinoPin pin);
}