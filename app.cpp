// 
// 
// 

#include "app.h"
#include <Arduino.h>
#include <Wire.h>
#include <hal_data.h>
#include "gpio.h"
#include "I2CCoreFunctions.h"

void SetupHardware() {
	// Hardware setup code here
	Serial.begin(115200);
	Wire.begin();

	gpio::SetFunction(pins::ArduinoPin::D2, gpio::Function::GPIO);
	gpio::SetFunction(pins::ArduinoPin::D3, gpio::Function::GPIO);
	gpio::SetDirection(pins::ArduinoPin::D2, gpio::Direction::OUTPUT);
	gpio::SetDirection(pins::ArduinoPin::D3, gpio::Direction::OUTPUT);
	gpio::Write(pins::ArduinoPin::D2, gpio::Level::Low);
	gpio::Write(pins::ArduinoPin::D3, gpio::Level::Low);
	//gpio::SetFunction(pins::ArduinoPin::D18, gpio::Function::I2C);
	//gpio::SetFunction(pins::ArduinoPin::D19, gpio::Function::I2C);
	//gpio::SetFunction(pins::ArduinoPin::D4, gpio::Function::GPIO);
	//gpio::SetDirection(pins::ArduinoPin::D4, gpio::Direction::OUTPUT);
	
	//gpio::InitializeI2C();
	
}

double CalculateAverageMoisture(int sampleCount)
{
    long moistureSum = 0;
    int validMeasurements = 0;

    for (int i = 0; i < sampleCount; i++)
    {
        int measurement = GetMoisture();

        if (measurement > 0)
        {
            moistureSum += measurement;
            validMeasurements++;
		}
		else return 0.0; 
        delay(50);
    }

    

    return (double)moistureSum / validMeasurements;
}

int GetMoisture() 
{
	

	/*Serial.println("GENERATE START:");
	if (core_functions::GenerateStart()) 
	{
		Serial.println("WRITE TO SLAVE:");
		core_functions::WriteIntoSlave(((0x36 << 1) | 0));
		Serial.println();
		core_functions::WriteIntoSlave(0x0F);
		core_functions::WriteIntoSlave(0x10);
		delay(1000);
		
	}
	
	Serial.println("GENERATE STOP:");
	core_functions::GenerateStop();

	Serial.println();*/
	Wire.beginTransmission(0x36);
	Wire.write(0x0F);
	Wire.write(0x10);
	Wire.endTransmission(false);
	Wire.requestFrom(0x36, (uint8_t)2);
	uint16_t v = 0;
	if (Wire.available() == 2)
	{
		v = (uint16_t)Wire.read() << 8;
		v |= (uint16_t)Wire.read();
	}
	Serial.print("Moisture: ");
	Serial.println(v);
	delay(1000);

	return v;
}

void ControlWatering()
{
    double currentMoisture = CalculateAverageMoisture(5);
    Serial.print("Start Moisture: ");
    Serial.println(currentMoisture);

    if (currentMoisture == 0.0 || currentMoisture >= 950)
    {
        if (currentMoisture == 0.0) Serial.println("Error: Sensor error (0) detected at start!");
        return;
    }

    if (currentMoisture < 940)
    {
        Serial.println("--- Starting Watering Cycle ---");

        int safetyCounter = 0;
        const int MAX_CYCLES = 5;

        while (currentMoisture < 990 && safetyCounter < MAX_CYCLES)
        {
            Serial.print("Cycle "); Serial.println(safetyCounter + 1);
            gpio::Write(pins::ArduinoPin::D2, gpio::Level::High);
            gpio::Write(pins::ArduinoPin::D3, gpio::Level::Low);
            delay(2000);

			gpio::Write(pins::ArduinoPin::D2, gpio::Level::Low);
            Serial.println("Soaking...");
            delay(5000);

            double newAvg = CalculateAverageMoisture(5);

            if (newAvg == 0.0)
            {
                Serial.println("CRITICAL ERROR: Sensor returned 0 inside loop! Stopping pump.");
                break;
            }

            currentMoisture = newAvg;
            Serial.print("New Moisture: ");
            Serial.println(currentMoisture);

            safetyCounter++;
        }

        Serial.println("--- Watering Finished ---");
		
		gpio::Write(pins::ArduinoPin::D3,gpio::Level::High);
    }
}
void MainLoop()
{
	ControlWatering();
}
