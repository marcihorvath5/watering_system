// 
// 
// 

#include "I2CCoreFunctions.h"
#include <Arduino.h>
namespace core_functions 
{
	static volatile R_SCI0_Type* SCI = R_SCI1; // Pointer to SCI register
	static volatile R_IIC0_Type* IIC = R_IIC1; // Pointer to IIC

	bool GenerateStart()
	{
		volatile uint8_t& iccr2Register = IIC->ICCR2; // Reference to ICMR2 register
		volatile auto& iccr2Register_b = IIC->ICCR2_b; // Reference to ICMR2 register

		volatile uint8_t& icsr2Register = IIC->ICSR2; // Reference to ICSR2 register
		volatile auto& icsr2Register_b = IIC->ICSR2_b; // Reference to ICSR2 register

		if (!WaitingWithTimeout(iccr2Register, (1u << 7), false)) // Waiting until bus is free
		{
			Serial.println("Bus is busy.");
			return false;
		}

		iccr2Register_b.ST = 1; // If bus is free genarating starting poll
		
		bool isStarted = WaitingWithTimeout(icsr2Register, (1u << 2), true); // Check if start was recognized

		Serial.println(isStarted);

		return isStarted;
	}

	bool WriteIntoSlave(uint8_t address)
	{
		volatile uint8_t& icdrtRegister = IIC->ICDRT; // reference to transmit data register
		volatile uint8_t& icsr2Register = IIC->ICSR2; // reference to ICRS2 data register
		volatile auto& icsr2Register_b = IIC->ICSR2_b; // reference to ICRS2 data register
		Serial.print("Original ICSR2 register:");
		Serial.println(icsr2Register, HEX);
		Serial.print("NACK flag befor send to adress:");
		Serial.println(icsr2Register_b.NACKF);

		bool notNACK = WaitingWithTimeout(icsr2Register, (1u << 4), false,10); // Check NACK flag
		bool isTransmitDataEmpty = WaitingWithTimeout(icsr2Register, (1u << 7), true,10); // Check transmit data register

		if (!notNACK || !isTransmitDataEmpty)
		{
			Serial.print("NACK: ");
			Serial.println(notNACK);
			Serial.print("Transmit DATA: " );
			Serial.println(isTransmitDataEmpty);
			return false;
		}

		icdrtRegister = address;
		Serial.print("Modified ICSR2 register:");
		Serial.println(icsr2Register, HEX);
		Serial.println(icsr2Register_b.NACKF);
		Serial.print("TEND:");
		Serial.println(icsr2Register_b.TEND, BIN);

		if (!WaitingWithTimeout(icsr2Register, (1u << 6),true,10))
		{
			Serial.println("Data is not fully transferred. Issuing STOP");
			return false;
		}

		notNACK = WaitingWithTimeout(icsr2Register, (1u << 4), false);
		isTransmitDataEmpty = WaitingWithTimeout(icsr2Register, (1u << 7), true);

		Serial.print("NOT NACK after send: ");
		Serial.println(icsr2Register_b.NACKF);
		Serial.print("Transmit DATA after send: ");
		Serial.println(icsr2Register_b.TDRE);

		bool result = notNACK && isTransmitDataEmpty;
		Serial.println(result);
		if (!result)
		{
			Serial.println("Unable to transfer data.Issuing STOP.");
			return false;
		}

		return result;
	}

	bool GenerateStop()
	{
		volatile uint8_t& iccr2Register= IIC->ICCR2; // reference to transmit data register
		volatile auto& iccr2Register_b= IIC->ICCR2_b; // reference to transmit data register
		volatile uint8_t& icsr2Register = IIC->ICSR2; // reference to ICRS2 data register
		volatile auto& icsr2Register_b = IIC->ICSR2_b; // reference to ICRS2 data register

		if (!WaitingWithTimeout(IIC->ICSR2, (1u << 6), true)) {
			Serial.println("Error: Timeout before Stop (TEND)!");
			return false;
		}

		Serial.println("Data sent, initiating Stop...");
		iccr2Register_b.SP = 1;

		bool stopSuccess = WaitingWithTimeout(IIC->ICSR2, (1u << 3), true);

		if (!stopSuccess) {
			Serial.println("Error: Hardware failed to generate Stop!");
			iccr2Register_b.SP = 0;
			return false;
		}

		bool isSlaveNow = WaitingWithTimeout(IIC->ICCR2, (1u << 5), false); 

		iccr2Register_b.SP = 0;     
		icsr2Register_b.STOP = 0;   
		icsr2Register_b.NACKF = 0;  

		// DEBUG INFO
		Serial.print("Stop Done. MST Bit: ");
		Serial.println(iccr2Register_b.MST, BIN); 

		return stopSuccess;
	}
	
	// Ezt másold a namespace core_functions-be
	bool ReadByte(uint8_t slaveAddress, uint8_t* readData)
	{
		volatile auto& iccr2Register_b = IIC->ICCR2_b; // Control Reg 2 (Restart, Stop)
		volatile auto& icsr2Register_b = IIC->ICSR2_b; // Status Reg 2 (Start flag, RDRF)
		volatile auto& icmr3Register_b = IIC->ICMR3_b; // Mode Reg 3 (ACK/NACK control)

		Serial.println("--- RESTART & READ SEQUENCE ---");

		// 1. RESTART Generálása (Repeated Start)
		// A Renesas hardverben az RS bit beállításával kérünk új Start jelet a busz elengedése nélkül.
		IIC->ICSR2 &= ~(1u << 2); // START flag törlése biztonságból
		iccr2Register_b.RS = 1;   // Request Restart

		// Megvárjuk, amíg a hardver ténylegesen generálja a Start jelet
		if (!WaitingWithTimeout(IIC->ICSR2, (1u << 2), true, 100)) {
			Serial.println("Error: Restart Timeout!");
			return false;
		}
		Serial.println("Restart Generated.");

		// 2. Cím küldése OLVASÁS (Read = 1) módban
		// slaveAddress << 1 | 1 -> Ez jelzi a szenzornak, hogy most ő küldjön adatot
		IIC->ICDRT = (slaveAddress << 1) | 1;

		// 3. Várakozás az adatra (RDRF - Receive Data Register Full)
		// Amint a cím kimegy és kapunk ACK-t, a szenzor elkezdi küldeni a byte-ot.
		// A 5. bit (1u << 5) az RDRF (Receive Data Full).
		if (!WaitingWithTimeout(IIC->ICSR2, (1u << 5), true, 100)) {
			Serial.println("Error: Receive Timeout (No Data)!");
			return false;
		}

		// 4. NACK beállítása (FONTOS!)
		// Mivel csak 1 byte-ot olvasunk, jeleznünk kell a szenzornak, hogy "Elég volt, kösz".
		// Ezt NACK küldésével tesszük. A Renesas-nál ezt az adat kiolvasása ELŐTT kell beállítani/ellenőrizni.
		icmr3Register_b.ACKWP = 1; // ACKBT írásvédelmének feloldása
		icmr3Register_b.ACKBT = 1; // 1 = NACK küldése az ACK ciklusban

		// 5. Az adat tényleges kiolvasása a bufferből
		*readData = IIC->ICDRR; // Ez olvassa ki az adatot a regiszterből

		Serial.print("Data Read: 0x");
		Serial.println(*readData, HEX);

		// 6. STOP Generálása
		GenerateStop();

		return true;
	}

	bool WaitingWithTimeout(volatile uint8_t& reg, uint8_t bitMask, bool expectedState,uint32_t timeout)
	{
		while (timeout > 0)
		{
			bool currentState = (reg & bitMask) != 0;
			Serial.print("Current State: ");
			Serial.println(currentState ? "true" : "false");
			if (currentState == expectedState) return true;

			timeout--;
		}

		return false;
	}
}