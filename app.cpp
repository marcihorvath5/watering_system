// 
// 
// 

#include "app.h"
#include <Arduino.h>
#include "gpio.h"
void SetupHardware() {
	// Hardware setup code here
	Serial.print("Peripheral clock frequency: ");
	
	//gpio::SetDirection(pins::ArduinoPin::D18, gpio::Direction::OUTPUT);
	gpio::SetFunction(pins::ArduinoPin::D18, gpio::Function::I2C);
	gpio::SetFunction(pins::ArduinoPin::D19, gpio::Function::I2C);

	gpio::InitializeI2C();
	
}

void FirstFunction() {
	// Your function implementation here
	/*gpio::Write(pins::ArduinoPin::D18, gpio::Level::High);
	delay(1000);
	gpio::Write(pins::ArduinoPin::D18, gpio::Level::Low);
	delay(1000);*/
}


