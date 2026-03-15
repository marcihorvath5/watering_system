
# Automata Növényöntöző Rendszer – Projekt Dokumentáció

## Bevezetés és Motiváció

Az automatikus növényöntözés ötlete abból adódott, hogy a tanulmányok és munka közepette szinte lehetetlen naprakészen tartani az otthoni szobanövényeim ellátását.

Az automata öntöző rendszer kifejlesztésének célja egy olyan autonóm, beágyazott rendszert létrehozni, amely:

- **Folyamatosan monitorozza** a talaj nedvességtartalmát egy kapacitív szenzor segítségével
- **Intelligens döntéseket hoz** az öntözés időpontjáról és időtartamáról
- **Autonóm módon végzi** az öntözési ciklus fizikai megvalósítását
- **Tanulságait** jegyzi fel a fejlesztési folyamatban, különösen az alacsony szintű hardverkommunikációban

A projekt során lehetőségem nyílt megismerkedni az embedded szoftver alacsony szintű aspektusaival – szemben az Arduino-s absztrakciós rétegek gyakori használatával, a baremetalos hardverregiszter-kezelésre helyeztem a hangsúlyt. Ez az Arduino R4 WiFi mikrokontroller RA4M1 magja révén lehetséges, amely egyenes 
hozzáférést biztosít a mikrokontroller **periféria regisztereinek** (pl. GPIO, I2C, 
UART regiszterek) az ARM Cortex-M4 memória-leképezés révén.

---

## Hardveres Felépítés és Áramköri Tervezés

### Rendszer Komponensei

Az öntöző rendszer alapvetően az alábbi hardverkomponentekből áll:

- **Arduino R4 WiFi**: RA4M1 Arm Cortex-M4 mikrokontroller, amely a központi vezérlő
- **Adafruit Seesaw Kapacitív Talajnedvesség-Érzékelő**: I2C protokollon kommunikáló szenzor (0x36-os címmel)
- **Vízpumpa és MOSFET**: Elektromechanikus rendszer az öntözéshez
- **Flyback dióda**: Az induktív terhelés lökésimpulzusainak elvezetésére
- **LED-ek (D2, D3)**: Vizuális visszajelzés a rendszer állapotáról

### Tápellátási Koncepció

Egy fontos tervezési döntés volt, hogy az Arduino és a magas teljesítményt igénylő pumpáját **két külön tápforrás** biztosítja. Az Arduino belső feszültségszabályozója elég az Arduino logikájához és az I2C szenzorhoz. A pumpát egy dedikált 12V-os tápegység hajtja meg MOSFET-es vezérléssel. 

---

## Baremetalos Hardver Inicializálás – Regiszter Szintű Vezérlés

A rendszer tervezésének egyik legfontosabb része az, hogy a Pin Mapping és GPIO vezérlés **nem az Arduino API-n keresztül**, hanem közvetlenül a mikrokontroller hardverregisztereinek manipulálásán keresztül valósul meg.

### Pin Inicializálás a Regiszter Szinten

Az `app.cpp`-ben a `SetupHardware()` függvény hívja meg a `gpio` névtér függvényeit, amelyek a Hardware Abstraction Layer-en (hal_data.h) keresztül közvetlenül hozzáférnek az RA4M1 regiszterekhez:

```cpp
void SetupHardware() {
	Serial.begin(115200);
	Wire.begin();

	gpio::SetFunction(pins::ArduinoPin::D2, gpio::Function::GPIO);
	gpio::SetFunction(pins::ArduinoPin::D3, gpio::Function::GPIO);
	gpio::SetDirection(pins::ArduinoPin::D2, gpio::Direction::OUTPUT);
	gpio::SetDirection(pins::ArduinoPin::D3, gpio::Direction::OUTPUT);
	gpio::Write(pins::ArduinoPin::D2, gpio::Level::Low);
	gpio::Write(pins::ArduinoPin::D3, gpio::Level::Low);
}
```

Az `SetFunction` és `SetDirection` függvények a `gpio.cpp`-ben a PFS (Pin Function Select) regiszterekhez közvetlenül hozzáférnek:

```cpp
void SetFunction(pins::ArduinoPin p, Function function) {
	PinMapping::PinDesc pin = PinMapping::Map(p);
	auto& pinFunctionRegister = PinFunctionSelect->PORT[pin.port].PIN[pin.bit].PmnPFS;
	
	switch (function) {
	case gpio::Function::GPIO:
		pinFunctionRegister &= ~(1u << 16);
		pinFunctionRegister |= (1u << 0);
		break;
	case gpio::Function::I2C:
		SCI->SCR = 0x00;
		pinFunctionRegister |= (1u << 16);
		pinFunctionRegister &= ~(I2CFunctionMask << I2CFunctionShift);
		pinFunctionRegister &= ~(1u << 1);
		break;
	}
}
```

A `gpio::Write` függvény közvetlenül a PORT Data Register-ét (PDR) manipulálja:

```cpp
void SetDirection(pins::ArduinoPin p, Direction direction) {
	PinMapping::PinDesc pin = PinMapping::Map(p);
	switch (direction) {
	case gpio::Direction::INPUT:
		PORT1->PDR &= ~(1u << pin.bit);
		break;
	case gpio::Direction::OUTPUT:
		PORT1->PDR |= (1u << pin.bit);
		break;
	}
}
```

Ez a megközelítés biztosítja, hogy az Arduino absztrakciós rétegei **nem keverednek** az alacsony szintű, teljes szabályozást igénylő I2C és GPIO műveletek közé.

---

## Az I2C Kommunikáció – Két Szintű Implementáció

### Jelenlegi Működés: Wire Könyvtár (Arduino API)

A projekt jelen működésében az `GetMoisture()` függvény a Wire könyvtáron keresztül kommunikál az Adafruit Seesaw szenzorral:

```cpp
int GetMoisture() {
	Wire.beginTransmission(0x36);
	Wire.write(0x0F);
	Wire.write(0x10);
	Wire.endTransmission(false);
	Wire.requestFrom(0x36, (uint8_t)2);
	
	uint16_t v = 0;
	if (Wire.available() == 2) {
		v = (uint16_t)Wire.read() << 8;
		v |= (uint16_t)Wire.read();
	}
	
	Serial.print("Moisture: ");
	Serial.println(v);
	delay(1000);
	
	return v;
}
```

Ez a megközelítés megbízható és helytálló, mivel az Arduino Wire könyvtára jól tesztelt és széles körben használt. Az adatok megfelelően jutnak a szenzorról a mikrovezérlőhöz.

### Fejlesztés Alatt: Alacsony Szintű I2C Kommunikáció

Az `I2CCoreFunctions.cpp` és `.h` fájlok az alacsony szintű I2C kommunikáció jelenlegi fejlesztési szakaszát mutatják. Ezek a függvények közvetlenül az IIC (Inter-Integrated Circuit) periféria regisztereivel kommunikálnak:

```cpp
bool GenerateStart() {
	volatile uint8_t& iccr2Register = IIC->ICCR2;
	volatile auto& iccr2Register_b = IIC->ICCR2_b;
	volatile uint8_t& icsr2Register = IIC->ICSR2;
	volatile auto& icsr2Register_b = IIC->ICSR2_b;

	if (!WaitingWithTimeout(iccr2Register, (1u << 7), false)) {
		Serial.println("Bus is busy.");
		return false;
	}

	iccr2Register_b.ST = 1; // Start condition
	bool isStarted = WaitingWithTimeout(icsr2Register, (1u << 2), true);

	return isStarted;
}
```

A `WaitingWithTimeout` függvény bitenkénti ellenőrzéseket végez a regiszter státuszán, biztosítva, hogy az I2C protokoll megfelelően haladjon előre. Ez az implementáció lehetővé tenné a teljes kontrollt az I2C kommunikáció felett, azonban a jelenlegi verzió az API-s megközelítést használja a stabilitás érdekében.

---

## Az Öntözési Logika és Vezérlés – Fejlesztési Kihívások és Megoldások

### Probléma: Az "Instabil Küszöbérték"

Amikor a fejlesztésem kezdetén egyszerű, egy-egy mérésre alapozott öntözési logikát implementáltam, gyorsan felismertem egy kritikus problémát. A szenzor adatok természetüknél fogva zajosak: egy mérés alatt a nedvesség érték 938-940 között ingadozhat, attól függően, hogy éppen hol mérünk a virágcserépben, vagy éppen mit csinál az elektronika. 

Ezt az ingadozást természetesnek nevezem – a szenzor kapacitív elven működik, és az olyan környezeti tényezők, mint a hőmérséklet vagy a levegő páratartalma is hatnak a mérésre. Az eredmény az volt, hogy ha a küszöbértéket 940-re állítottam, a pumpa  be-ki kapcsolt, mivel az érték folyton átlépte a határt.

### A Megoldás: Átlagszámítás és Kettős Küszöbérték

Ezt a problémát többszintű megközelítéssel oldottam meg:

#### 1. Többmintás Átlagszámítás

A `CalculateAverageMoisture()` függvény 5 mért érték átlagát számítja:

```cpp
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
```

Az 50ms-os delay az egyes mérések között biztosítja, hogy ne ugyanazt az értéket mérjük többször. Ez az átlag jellemzően sokkal stabilabb, és a rövid időtartamú ingadozások kiátlagolódnak.

#### 2. Kettős Küszöbérték (Hisztérézis)

Az öntözési logika nem egy, hanem **két határértéket** használ:

```cpp
void ControlWatering()
{
    double currentMoisture = CalculateAverageMoisture(5);
    Serial.print("Start Moisture: ");
    Serial.println(currentMoisture);

    // Szenzor hiba vagy rendszerhiba detektálása
    if (currentMoisture == 0.0 || currentMoisture >= 950)
    {
        if (currentMoisture == 0.0) Serial.println("Error: Sensor error (0) detected!");
        return;
    }

    // Öntözési küszöb: ha a talaj 940 alá megy, kezdj el öntözni
    if (currentMoisture < 940)
    {
        Serial.println("--- Starting Watering Cycle ---");
        int safetyCounter = 0;
        const int MAX_CYCLES = 5;

        // Öntöz, amíg 990-es nedvességi szintet nem érünk el
        // (vagy biztonsági korlát nem lép fel)
        while (currentMoisture < 990 && safetyCounter < MAX_CYCLES)
        {
            Serial.print("Cycle "); Serial.println(safetyCounter + 1);
            gpio::Write(pins::ArduinoPin::D2, gpio::Level::High);
            gpio::Write(pins::ArduinoPin::D3, gpio::Level::Low);
            delay(2000);
            
            gpio::Write(pins::ArduinoPin::D2, gpio::Level::Low);
            currentMoisture = CalculateAverageMoisture(5);
            safetyCounter++;
        }
        
        Serial.println("--- Watering Cycle Complete ---");
    }
}
```

**A logika működése:**
- **940**: Az "öntözés szükséges" küszöb. Ha alá esik, az öntözés megkezdődik.
- **990**: Az "elég nedves" küszöb. Az öntözés addig tart, amíg el nem éri ezt az értéket.

Ez a kettős küszöbérték (más néven hisztérézis) biztosítja, hogy a pumpa nem kapcsol ki és be másodpercenként, hanem valóban megvárja, amíg a talaj megfelelően felszívódik. Ez természetesebbnek és hatékonyabbnak tűnik.

### Küszöbértékek Meghatározása és Kalibrálása

A 940 és 990-es értékek **nem véletlenül választódtak**. Az Adafruit Seesaw szenzor kapacitív adatokat ad vissza tipikusan 350-1540 közötti tartományban. Az én konkrét virágcserepemre, talajára és a szobakörülményeimre vonatkozóan ezek az értékek empirikusan voltak meghatározva:

1. **Kezdeti méréseink**: A száraz talajban a szenzor ~700-900 között mért.
2. **Frissen öntözött talajban**: Az érték ~990-1000 közötti volt.
3. **Optimális zóna**: Tapasztalat alapján a 940-990 tartomány jelent egy "komfortos" helyzetet a növénynek – nem túlságosan száraz, de nem is vízzel telített.

Érdekes megfigyelés volt, hogy ez az optimális tartomány gyökérnövényekre (például paradicsomra) más lenne, mint egy szobanövényre. Az értékek a talaj típusától, a cserépméretétől és még a növény típusától is függnek. Saját növényem számára ezt az iterációval, napi monitoring és némi próba-szerencse alapján határoztam meg.

#### Jövőbeli Finomítás

Egy lehetséges fejlesztés egy automatikus kalibrációs fázis lenne az induláskor, amely meghatározná a konkrét talaj "száraz" és "nedves" értékeit. Ez azonban nem volt a jelenlegi projekt közvetlen célja.

---

---

## Takeaway: Szoftveres Megközelítések és Tanulságok

A projekt során több különböző szintű megközelítést tapasztaltam:

1. **Arduino API szint**: Gyors prototípizálás, széles körű könyvtár-támogatás, de az algoritmusok felelőssége az alkalmazáson van
2. **Baremetalos szint**: Teljes hardverkontroll, de nagyobb komplexitás és tesztelési igény
3. **Szenzor hisztérézis**: Az egyik legfontosabb lecke az volt, hogy a valós szenzorbemenettel egy robotika vagy beágyazott rendszer nem működhet naiv küszöbértékekkel – szükség van szűrésre, átlagolásra, és logikai folyamatvezérlésre.

Az `I2CCoreFunctions` fejlesztése azt mutatja, hogy még akkor is, ha az Arduino Wire könyvtára teljesen megfelelő, érdekes és értékes megismerni az alatta működő regiszter-szintű logikát. Ez mélyebb megértést ad az embedded rendszerekről.

---

## Továbbfejlesztési Irányok

### IoT és Távolról Történő Monitorozás

Az Arduino R4 WiFi beépített Wi-Fi képessége lehetővé tenné az MQTT vagy REST-alapú adatküldést egy felhő platformra (ThingSpeak, saját szerver, stb.), ahonnan a felhasználó monitorozhatná a növényi helyzetet valós időben, és akár kézzel is elindíthatna öntözési ciklust.

### Több Növény Kezelése

A jelenlegi architektúra könnyen skálázható. Több I2C szenzor (eltérő címekkel: 0x36, 0x37, stb.) és több MOSFET-vezérelt pumpaciklus párhuzamosan üzemeltethető ugyanazon az Arduinón. Elég minden csatornához saját kalibrációs paramétereket definiálni.

### UV-fényforrás kontrollja UV-igényes növényekhez

Olyan növények számára, amelyeknek UV-fényre van szükségük  a rendszer kiterjeszthető egy **LED-es UV-fényforrás vezérlésére**. Egy fotótranzisztor vagy LDR szenzor detektálhatná a rendelkezésre álló természetes fényt:

### Adatgyűjtés és Tanulás

Az összes öntözési esemény, nedvesség érték és fényadat naplózható lenne, létrehozva egy  adatbázist. Ez később felhasználható lenne gépi tanuláshoz – például egy neurális hálózat megtanulhatná az optimális öntözési mintákat az adott növényre és környezetre.

---

## Összefoglalás

Ez a projekt egy teljes körű embedded rendszer, amely jól szemlélteti az Arduino-fejlesztés különböző szintjeit: az Arduino API-s kényelmes magas szintű programozástól a HAL-on keresztüli baremetalos regiszter-kezeléséig. 

A legfontosabb tanulság talán az volt, hogy a valós szenzor-alapú vezérlésben az egyszerű küszöbértékek nem elegendőek – a rendszer robusztusságához szűrés, átlagolás és hisztérézis szükséges. A rendszer stabil működésben van, az alkalmazás-szintű öntözési logika megbízhatóan működik, és a kutatás folyamatos az I2C alacsony szintű implementációban is.
