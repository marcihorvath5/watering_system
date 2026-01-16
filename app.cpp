// 
// 
// 

#include "app.h"
#include <Arduino.h>
#include "gpio.h"
void SetupHardware() {
	// Hardware setup code here
	gpio::SetFunction(1, 16, gpio::Function::GPIO);
	gpio::SetDirection(4, gpio::Direction::OUTPUT);
}

void FirstFunction() {
	// Your function implementation here
	gpio::Write(4, gpio::Level::High);
	delay(1000);
	gpio::Write(4, gpio::Level::Low);
	delay(1000);
}


