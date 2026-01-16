# Watering System (flower)

Simple Arduino-based watering system project by marci.

## Description
This project contains a minimal automatic watering example for a single plant ("flower"). The firmware configures GPIO pins and toggles an output (pin 4) to drive a pump or valve. Pin function and write-protection are handled in `gpio.cpp` / `gpio.h`, and the main program lives in `watering_system.ino` (with application logic in `app.cpp` / `app.h`).

## Features
- Initializes hardware in `SetupHardware()`
- Example periodic toggle in `FirstFunction()` to simulate watering
- Low-level GPIO helpers:
  - `SetFunction(port, pin, Function::GPIO)`
  - `SetDirection(pin, Direction::OUTPUT|INPUT)`
  - `Write(pin, Level::High|Low)`

## Wiring (example)
- Control pin (digital pin 4) -> transistor base (with base resistor)
- Transistor collector -> pump/valve +
- Pump/valve - -> board GND
- Flyback diode across pump/valve
- Provide proper external power for pump/valve (do not drive from MCU 5V directly)

## Build & Upload
1. Open `watering_system.ino` in the Arduino IDE (or open the project in your preferred environment).
2. Select the correct board and port.
3. Upload.

If you use the provided low-level Renesas FSP register access in `gpio.cpp`, ensure the correct platform headers and FSP libraries are available in your toolchain.

## Notes
- This is a demo template; adapt pin numbers, timings and safety wiring for your hardware.
	- Review `gpio.cpp` locking/unlocking of PFS write-protection if you port to a different MCU.