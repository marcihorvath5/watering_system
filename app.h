// app.h

#ifndef _APP_h
#define _APP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
void SetupHardware();
void FirstFunction();

#endif

