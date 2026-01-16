// 
// 
// 

#include "app.h"
#include <Arduino.h>
#include "gpio.h"
void SetupHardware() {
	// Hardware setup code here
	gpio::SetFunction(pins::ArduinoPin::D2, gpio::Function::GPIO);
	gpio::SetDirection(pins::ArduinoPin::D2, gpio::Direction::OUTPUT);
}

void FirstFunction() {
	// Your function implementation here
	gpio::Write(pins::ArduinoPin::D2, gpio::Level::High);
	delay(1000);
	gpio::Write(pins::ArduinoPin::D2, gpio::Level::Low);
	delay(1000);
}


