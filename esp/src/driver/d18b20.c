// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// Version 3.7.2 modified on Dec 6, 2011 to support Arduino 1.0
// See Includes...
// Modified by Jordan Hochenbaum
#include "ets_sys.h"
#include "osapi.h"
#include "driver/d18b20.h"


typedef uint8_t ScratchPad[9];

// parasite power on or off
bool parasite;

// used to determine the delay amount needed to allow for the
// temperature conversion to take place
uint8_t bitResolution;

// used to requestTemperature with or without delay
bool waitForConversion;

// used to requestTemperature to dynamically check if a conversion is complete
bool checkForConversion;

// count of devices on the bus
uint8_t devices;

// Take a pointer to one wire instance
// reads scratchpad and returns the raw temperature
int16_t d18b20_calculateTemperature(const uint8_t*, uint8_t*);

int16_t d18b20_millisToWaitForConversion(uint8_t);

void	d18b20_blockTillConversionComplete(uint8_t, const uint8_t*);

#if REQUIRESALARMS

    // required for alarmSearch
    uint8_t alarmSearchAddress[8];
    char alarmSearchJunction;
    uint8_t alarmSearchExhausted;

    // the alarm handler function pointer
    AlarmHandler *_AlarmHandler;

#endif



ICACHE_FLASH_ATTR
void d18b20_delay(int ms){
    os_delay_us(ms * 1000);
}

ICACHE_FLASH_ATTR
void d18b20_init(){
    devices = 0;
    parasite = false;
    bitResolution = 9;
    waitForConversion = true;
    checkForConversion = true;

}

ICACHE_FLASH_ATTR
uint8_t d18b20_max(uint8_t v1, uint8_t v2){
	if(v1 >= v2)
		return v1;

	return v2;
}

ICACHE_FLASH_ATTR
uint32_t d18b20_millis(void){
	return system_get_time();
}

// initialise the bus
ICACHE_FLASH_ATTR
void d18b20_begin(void){

    DeviceAddress deviceAddress;

    oneWire_reset_search();
    devices = 0; // Reset the number of devices when we enumerate wire devices

    while (oneWire_search(deviceAddress)){
//    	os_printf("found one device\n");
        if (d18b20_validAddress(deviceAddress)){
//			os_printf("valid address\n");
            if (!parasite && d18b20_readPowerSupply(deviceAddress)) parasite = true;

            ScratchPad scratchPad;

            d18b20_readScratchPad(deviceAddress, scratchPad);

            bitResolution = d18b20_max(bitResolution, d18b20_getResolution(deviceAddress));

            devices++;
        }
    }
//    os_printf("found %d devices\n", devices);
}

// returns the number of devices found on the bus
uint8_t d18b20_getDeviceCount(void){
    return devices;
}

// returns true if address is valid
ICACHE_FLASH_ATTR
bool d18b20_validAddress(const uint8_t* deviceAddress){
    return (oneWire_crc8(deviceAddress, 7) == deviceAddress[7]);
}

// finds an address at a given index on the bus
// returns true if the device was found
ICACHE_FLASH_ATTR
bool d18b20_getAddress(uint8_t* deviceAddress, uint8_t index){

    uint8_t depth = 0;

    oneWire_reset_search();

    while (depth <= index && oneWire_search(deviceAddress)) {
        if (depth == index && d18b20_validAddress(deviceAddress)) return true;
        depth++;
    }

    return false;

}

// attempt to determine if the device at the given address is connected to the bus
// also allows for updating the read scratchpad
ICACHE_FLASH_ATTR
bool d18b20_isConnected2(const uint8_t* deviceAddress, uint8_t* scratchPad)
{
    bool b = d18b20_readScratchPad(deviceAddress, scratchPad);
    return b && (oneWire_crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}

// attempt to determine if the device at the given address is connected to the bus
ICACHE_FLASH_ATTR
bool d18b20_isConnected(const uint8_t* deviceAddress){

    ScratchPad scratchPad;
    return d18b20_isConnected2(deviceAddress, scratchPad);

}

ICACHE_FLASH_ATTR
bool d18b20_readScratchPad(const uint8_t* deviceAddress, uint8_t* scratchPad){

    // send the reset command and fail fast
    int b = oneWire_reset();
    if (b == 0) return false;

    oneWire_select(deviceAddress);
    oneWire_write(READSCRATCH);

    // Read all registers in a simple loop
    // byte 0: temperature LSB
    // byte 1: temperature MSB
    // byte 2: high alarm temp
    // byte 3: low alarm temp
    // byte 4: DS18S20: store for crc
    //         DS18B20 & DS1822: configuration register
    // byte 5: internal use & crc
    // byte 6: DS18S20: COUNT_REMAIN
    //         DS18B20 & DS1822: store for crc
    // byte 7: DS18S20: COUNT_PER_C
    //         DS18B20 & DS1822: store for crc
    // byte 8: SCRATCHPAD_CRC
    uint8_t i;
    for(i = 0; i < 9; i++){
        scratchPad[i] = oneWire_read();
    }

    b = oneWire_reset();
    return (b == 1);
}


ICACHE_FLASH_ATTR
void d18b20_writeScratchPad(const uint8_t* deviceAddress, const uint8_t* scratchPad){

    oneWire_reset();
    oneWire_select(deviceAddress);
    oneWire_write(WRITESCRATCH);
    oneWire_write(scratchPad[HIGH_ALARM_TEMP]); // high alarm temp
    oneWire_write(scratchPad[LOW_ALARM_TEMP]); // low alarm temp

    // DS1820 and DS18S20 have no configuration register
    if (deviceAddress[0] != DS18S20MODEL) oneWire_write(scratchPad[CONFIGURATION]);

    oneWire_reset();
    oneWire_select(deviceAddress);

    // save the newly written values to eeprom
    oneWire_write(COPYSCRATCH, parasite);
    d18b20_delay(20);  // <--- added 20ms delay to allow 10ms long EEPROM write operation (as specified by datasheet)

    if (parasite) d18b20_delay(10); // 10ms delay
    oneWire_reset();

}

ICACHE_FLASH_ATTR
bool d18b20_readPowerSupply(const uint8_t* deviceAddress){

    bool ret = false;
    oneWire_reset();
    oneWire_select(deviceAddress);
    oneWire_write(READPOWERSUPPLY);
    if (oneWire_read_bit() == 0) ret = true;
    oneWire_reset();
    return ret;

}

// set resolution of a device to 9, 10, 11, or 12 bits
// if new resolution is out of range, 9 bits is used.
ICACHE_FLASH_ATTR
bool d18b20_setResolution2(const uint8_t* deviceAddress, uint8_t newResolution){

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)){

        // DS1820 and DS18S20 have no resolution configuration register
        if (deviceAddress[0] != DS18S20MODEL){

            switch (newResolution){
            case 12:
                scratchPad[CONFIGURATION] = TEMP_12_BIT;
                break;
            case 11:
                scratchPad[CONFIGURATION] = TEMP_11_BIT;
                break;
            case 10:
                scratchPad[CONFIGURATION] = TEMP_10_BIT;
                break;
            case 9:
            default:
                scratchPad[CONFIGURATION] = TEMP_9_BIT;
                break;
            }
            d18b20_writeScratchPad(deviceAddress, scratchPad);
        }
        return true;  // new value set
    }

    return false;

}


ICACHE_FLASH_ATTR
uint8_t d18b20_constrain(uint8_t newres, uint8_t v1, uint8_t v2){
	if(newres >= v1 && newres <= v2)
		return newres;

	if(newres < v1)
		return v1;

	return v2;

}

// set resolution of all devices to 9, 10, 11, or 12 bits
// if new resolution is out of range, it is constrained.
ICACHE_FLASH_ATTR
void d18b20_setResolution(uint8_t newResolution){

    bitResolution = d18b20_constrain(newResolution, 9, 12);
    DeviceAddress deviceAddress;
    uint16_t i;
    for (i = 0; i<devices; i++)
    {
    	d18b20_getAddress(deviceAddress, i);
        d18b20_setResolution2(deviceAddress, bitResolution);
    }

}

// returns the global resolution
ICACHE_FLASH_ATTR
uint8_t d18b20_getResolution(){
    return bitResolution;
}

//// returns the current resolution of the device, 9-12
//// returns 0 if device not found
//uint8_t d18b20_getResolution(const uint8_t* deviceAddress){
//
//    // DS1820 and DS18S20 have no resolution configuration register
//    if (deviceAddress[0] == DS18S20MODEL) return 12;
//
//    ScratchPad scratchPad;
//    if (isConnected(deviceAddress, scratchPad))
//    {
//        switch (scratchPad[CONFIGURATION])
//        {
//        case TEMP_12_BIT:
//            return 12;
//
//        case TEMP_11_BIT:
//            return 11;
//
//        case TEMP_10_BIT:
//            return 10;
//
//        case TEMP_9_BIT:
//            return 9;
//        }
//    }
//    return 0;
//
//}


// sets the value of the waitForConversion flag
// TRUE : function requestTemperature() etc returns when conversion is ready
// FALSE: function requestTemperature() etc returns immediately (USE WITH CARE!!)
//        (1) programmer has to check if the needed delay has passed
//        (2) but the application can do meaningful things in that time
ICACHE_FLASH_ATTR
void d18b20_setWaitForConversion(bool flag){
    waitForConversion = flag;
}

// gets the value of the waitForConversion flag
ICACHE_FLASH_ATTR
bool d18b20_getWaitForConversion(){
    return waitForConversion;
}


// sets the value of the checkForConversion flag
// TRUE : function requestTemperature() etc will 'listen' to an IC to determine whether a conversion is complete
// FALSE: function requestTemperature() etc will wait a set time (worst case scenario) for a conversion to complete
ICACHE_FLASH_ATTR
void d18b20_setCheckForConversion(bool flag){
    checkForConversion = flag;
}

// gets the value of the waitForConversion flag
ICACHE_FLASH_ATTR
bool d18b20_getCheckForConversion(){
    return checkForConversion;
}

ICACHE_FLASH_ATTR
bool d18b20_isConversionAvailable(const uint8_t* deviceAddress){

    // Check if the clock has been raised indicating the conversion is complete
    ScratchPad scratchPad;
    d18b20_readScratchPad(deviceAddress, scratchPad);
    return scratchPad[0];

}

// sends command for all devices on the bus to perform a temperature conversion
ICACHE_FLASH_ATTR
void d18b20_requestTemperatures(){

    oneWire_reset();
    oneWire_skip();
    oneWire_write(STARTCONVO, parasite);

    // ASYNC mode?
    if (!waitForConversion) return;
    d18b20_blockTillConversionComplete(bitResolution, NULL);

}

// sends command for one device to perform a temperature by address
// returns FALSE if device is disconnected
// returns TRUE  otherwise
ICACHE_FLASH_ATTR
bool d18b20_requestTemperaturesByAddress(const uint8_t* deviceAddress){

    uint8_t bitResolution = d18b20_getResolution(deviceAddress);
    if (bitResolution == 0){
     return false; //Device disconnected
    }

    if (oneWire_reset() == 0){
        return false;
    }

    oneWire_select(deviceAddress);
    oneWire_write(STARTCONVO, parasite);


    // ASYNC mode?
    if (!waitForConversion) return true;

    d18b20_blockTillConversionComplete(bitResolution, deviceAddress);

    return true;

}

// Continue to check if the IC has responded with a temperature
ICACHE_FLASH_ATTR
void d18b20_blockTillConversionComplete(uint8_t bitResolution, const uint8_t* deviceAddress){

    int delms = d18b20_millisToWaitForConversion(bitResolution);
    if (deviceAddress != NULL && checkForConversion && !parasite){
        unsigned long now = d18b20_millis();
        while(!d18b20_isConversionAvailable(deviceAddress) && (d18b20_millis() - delms < now));
    } else {
    	d18b20_delay(delms);
    }

}

// returns number of milliseconds to wait till conversion is complete (based on IC datasheet)
ICACHE_FLASH_ATTR
int16_t d18b20_millisToWaitForConversion(uint8_t bitResolution){

    switch (bitResolution){
    case 9:
        return 94;
    case 10:
        return 188;
    case 11:
        return 375;
    default:
        return 750;
    }

}


// sends command for one device to perform a temp conversion by index
ICACHE_FLASH_ATTR
bool d18b20_requestTemperaturesByIndex(uint8_t deviceIndex){

    DeviceAddress deviceAddress;
    d18b20_getAddress(deviceAddress, deviceIndex);

    return d18b20_requestTemperaturesByAddress(deviceAddress);

}

// Fetch temperature for device index
ICACHE_FLASH_ATTR
float d18b20_getTempCByIndex(uint8_t deviceIndex){

    DeviceAddress deviceAddress;
    if (!d18b20_getAddress(deviceAddress, deviceIndex)){
        return DEVICE_DISCONNECTED_C;
    }

    return d18b20_getTempC((uint8_t*)deviceAddress);

}

// Fetch temperature for device index
ICACHE_FLASH_ATTR
float d18b20_getTempFByIndex(uint8_t deviceIndex){

    DeviceAddress deviceAddress;

    if (!d18b20_getAddress(deviceAddress, deviceIndex)){
        return DEVICE_DISCONNECTED_F;
    }

    return d18b20_getTempF((uint8_t*)deviceAddress);

}

// reads scratchpad and returns fixed-point temperature, scaling factor 2^-7
ICACHE_FLASH_ATTR
int16_t d18b20_calculateTemperature(const uint8_t* deviceAddress, uint8_t* scratchPad){

    int16_t fpTemperature =
    (((int16_t) scratchPad[TEMP_MSB]) << 11) |
    (((int16_t) scratchPad[TEMP_LSB]) << 3);

    /*
    DS1820 and DS18S20 have a 9-bit temperature register.

    Resolutions greater than 9-bit can be calculated using the data from
    the temperature, and COUNT REMAIN and COUNT PER °C registers in the
    scratchpad.  The resolution of the calculation depends on the model.

    While the COUNT PER °C register is hard-wired to 16 (10h) in a
    DS18S20, it changes with temperature in DS1820.

    After reading the scratchpad, the TEMP_READ value is obtained by
    truncating the 0.5°C bit (bit 0) from the temperature data. The
    extended resolution temperature can then be calculated using the
    following equation:

                                    COUNT_PER_C - COUNT_REMAIN
    TEMPERATURE = TEMP_READ - 0.25 + --------------------------
                                            COUNT_PER_C

    Hagai Shatz simplified this to integer arithmetic for a 12 bits
    value for a DS18S20, and James Cameron added legacy DS1820 support.

    See - http://myarduinotoy.blogspot.co.uk/2013/02/12bit-result-from-ds18s20.html
    */

    if (deviceAddress[0] == DS18S20MODEL){
        fpTemperature = ((fpTemperature & 0xfff0) << 3) - 16 +
            (
                ((scratchPad[COUNT_PER_C] - scratchPad[COUNT_REMAIN]) << 7) /
                  scratchPad[COUNT_PER_C]
            );
    }

    return fpTemperature;
}


// returns temperature in 1/128 degrees C or DEVICE_DISCONNECTED_RAW if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED_RAW is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
ICACHE_FLASH_ATTR
int16_t d18b20_getTemp(const uint8_t* deviceAddress){

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)) return d18b20_calculateTemperature(deviceAddress, scratchPad);
    return DEVICE_DISCONNECTED_RAW;

}

// returns temperature in degrees C or DEVICE_DISCONNECTED_C if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED_C is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
ICACHE_FLASH_ATTR
float d18b20_getTempC(const uint8_t* deviceAddress){
    return d18b20_rawToCelsius(d18b20_getTemp(deviceAddress));
}

// returns temperature in degrees F or DEVICE_DISCONNECTED_F if the
// device's scratch pad cannot be read successfully.
// the numeric value of DEVICE_DISCONNECTED_F is defined in
// DallasTemperature.h. It is a large negative number outside the
// operating range of the device
ICACHE_FLASH_ATTR
float d18b20_getTempF(const uint8_t* deviceAddress){
    return d18b20_rawToFahrenheit(d18b20_getTemp(deviceAddress));
}

// returns true if the bus requires parasite power
ICACHE_FLASH_ATTR
bool d18b20_isParasitePowerMode(void){
    return parasite;
}


// IF alarm is not used one can store a 16 bit int of userdata in the alarm
// registers. E.g. an ID of the sensor.
// See github issue #29

// note if device is not connected it will fail writing the data.
ICACHE_FLASH_ATTR
void d18b20_setUserData(const uint8_t* deviceAddress, int16_t data)
{
    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad))
    {
        scratchPad[HIGH_ALARM_TEMP] = data >> 8;
        scratchPad[LOW_ALARM_TEMP] = data & 255;
        d18b20_writeScratchPad(deviceAddress, scratchPad);
    }
}

ICACHE_FLASH_ATTR
int16_t d18b20_getUserData(const uint8_t* deviceAddress)
{
    int16_t data = 0;
    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad))
    {
        data = scratchPad[HIGH_ALARM_TEMP] << 8;
        data += scratchPad[LOW_ALARM_TEMP];
    }
    return data;
}

// note If address cannot be found no error will be reported.
ICACHE_FLASH_ATTR
int16_t d18b20_getUserDataByIndex(uint8_t deviceIndex)
{
    DeviceAddress deviceAddress;
    d18b20_getAddress(deviceAddress, deviceIndex);
    return d18b20_getUserData((uint8_t*) deviceAddress);
}

ICACHE_FLASH_ATTR
void d18b20_setUserDataByIndex(uint8_t deviceIndex, int16_t data)
{
    DeviceAddress deviceAddress;
    d18b20_getAddress(deviceAddress, deviceIndex);
    d18b20_setUserData((uint8_t*) deviceAddress, data);
}


// Convert float Celsius to Fahrenheit
ICACHE_FLASH_ATTR
float d18b20_toFahrenheit(float celsius){
    return (celsius * 1.8) + 32;
}

// Convert float Fahrenheit to Celsius
ICACHE_FLASH_ATTR
float d18b20_toCelsius(float fahrenheit){
    return (fahrenheit - 32) * 0.555555556;
}

// convert from raw to Celsius
ICACHE_FLASH_ATTR
float d18b20_rawToCelsius(int16_t raw){

    if (raw <= DEVICE_DISCONNECTED_RAW)
    return DEVICE_DISCONNECTED_C;
    // C = RAW/128
    return (float)raw * 0.0078125;

}

// convert from raw to Fahrenheit
ICACHE_FLASH_ATTR
float d18b20_rawToFahrenheit(int16_t raw){

    if (raw <= DEVICE_DISCONNECTED_RAW)
    return DEVICE_DISCONNECTED_F;
    // C = RAW/128
    // F = (C*1.8)+32 = (RAW/128*1.8)+32 = (RAW*0.0140625)+32
    return ((float)raw * 0.0140625) + 32;

}

#if REQUIRESALARMS

/*

ALARMS:

TH and TL Register Format

BIT 7 BIT 6 BIT 5 BIT 4 BIT 3 BIT 2 BIT 1 BIT 0
S    2^6   2^5   2^4   2^3   2^2   2^1   2^0

Only bits 11 through 4 of the temperature register are used
in the TH and TL comparison since TH and TL are 8-bit
registers. If the measured temperature is lower than or equal
to TL or higher than or equal to TH, an alarm condition exists
and an alarm flag is set inside the DS18B20. This flag is
updated after every temperature measurement; therefore, if the
alarm condition goes away, the flag will be turned off after
the next temperature conversion.

*/

// sets the high alarm temperature for a device in degrees Celsius
// accepts a float, but the alarm resolution will ignore anything
// after a decimal point.  valid range is -55C - 125C
ICACHE_FLASH_ATTR
void d18b20_setHighAlarmTemp(const uint8_t* deviceAddress, char celsius){

    // make sure the alarm temperature is within the device's range
    if (celsius > 125) celsius = 125;
    else if (celsius < -55) celsius = -55;

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)){
        scratchPad[HIGH_ALARM_TEMP] = (uint8_t)celsius;
        d18b20_writeScratchPad(deviceAddress, scratchPad);
    }

}

// sets the low alarm temperature for a device in degrees Celsius
// accepts a float, but the alarm resolution will ignore anything
// after a decimal point.  valid range is -55C - 125C
ICACHE_FLASH_ATTR
void d18b20_setLowAlarmTemp(const uint8_t* deviceAddress, char celsius){
    // make sure the alarm temperature is within the device's range
    if (celsius > 125) celsius = 125;
    else if (celsius < -55) celsius = -55;

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)){
        scratchPad[LOW_ALARM_TEMP] = (uint8_t)celsius;
        d18b20_writeScratchPad(deviceAddress, scratchPad);
    }

}

// returns a char with the current high alarm temperature or
// DEVICE_DISCONNECTED for an address
ICACHE_FLASH_ATTR
char d18b20_getHighAlarmTemp(const uint8_t* deviceAddress){

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)) return (char)scratchPad[HIGH_ALARM_TEMP];
    return DEVICE_DISCONNECTED_C;

}

// returns a char with the current low alarm temperature or
// DEVICE_DISCONNECTED for an address
ICACHE_FLASH_ATTR
char d18b20_getLowAlarmTemp(const uint8_t* deviceAddress){

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)) return (char)scratchPad[LOW_ALARM_TEMP];
    return DEVICE_DISCONNECTED_C;

}

// resets internal variables used for the alarm search
ICACHE_FLASH_ATTR
void d18b20_resetAlarmSearch(){

    alarmSearchJunction = -1;
    alarmSearchExhausted = 0;
    uint8_t i;
    for(i = 0; i < 7; i++){
        alarmSearchAddress[i] = 0;
    }

}

// This is a modified version of the OneWire::search method.
//
// Also added the OneWire search fix documented here:
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1238032295
//
// Perform an alarm search. If this function returns a '1' then it has
// enumerated the next device and you may retrieve the ROM from the
// OneWire::address variable. If there are no devices, no further
// devices, or something horrible happens in the middle of the
// enumeration then a 0 is returned.  If a new device is found then
// its address is copied to newAddr.  Use
// d18b20_resetAlarmSearch() to start over.
ICACHE_FLASH_ATTR
bool d18b20_alarmSearch(uint8_t* newAddr){

    uint8_t i;
    char lastJunction = -1;
    uint8_t done = 1;

    if (alarmSearchExhausted) return false;
    if (!oneWire_reset()) return false;

    // send the alarm search command
    oneWire_write(0xEC, 0);

    for(i = 0; i < 64; i++){

        uint8_t a = oneWire_read_bit( );
        uint8_t nota = oneWire_read_bit( );
        uint8_t ibyte = i / 8;
        uint8_t ibit = 1 << (i & 7);

        // I don't think this should happen, this means nothing responded, but maybe if
        // something vanishes during the search it will come up.
        if (a && nota) return false;

        if (!a && !nota){
            if (i == alarmSearchJunction){
                // this is our time to decide differently, we went zero last time, go one.
                a = 1;
                alarmSearchJunction = lastJunction;
            }else if (i < alarmSearchJunction){

                // take whatever we took last time, look in address
                if (alarmSearchAddress[ibyte] & ibit){
                    a = 1;
                }else{
                    // Only 0s count as pending junctions, we've already exhausted the 0 side of 1s
                    a = 0;
                    done = 0;
                    lastJunction = i;
                }
            }else{
                // we are blazing new tree, take the 0
                a = 0;
                alarmSearchJunction = i;
                done = 0;
            }
            // OneWire search fix
            // See: http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1238032295
        }

        if (a) alarmSearchAddress[ibyte] |= ibit;
        else alarmSearchAddress[ibyte] &= ~ibit;

        oneWire_write_bit(a);
    }

    if (done) alarmSearchExhausted = 1;
    for (i = 0; i < 8; i++) newAddr[i] = alarmSearchAddress[i];
    return true;

}

// returns true if device address might have an alarm condition
// (only an alarm search can verify this)
ICACHE_FLASH_ATTR
bool d18b20_hasAlarm(const uint8_t* deviceAddress){

    ScratchPad scratchPad;
    if (d18b20_isConnected2(deviceAddress, scratchPad)){

        char temp = d18b20_calculateTemperature(deviceAddress, scratchPad) >> 7;

        // check low alarm
        if (temp <= (char)scratchPad[LOW_ALARM_TEMP]) return true;

        // check high alarm
        if (temp >= (char)scratchPad[HIGH_ALARM_TEMP]) return true;
    }

    // no alarm
    return false;

}

// returns true if any device is reporting an alarm condition on the bus
ICACHE_FLASH_ATTR
bool d18b20_hasAlarm2(void){

    DeviceAddress deviceAddress;
    d18b20_resetAlarmSearch();
    return d18b20_alarmSearch(deviceAddress);

}

// runs the alarm handler for all devices returned by alarmSearch()
ICACHE_FLASH_ATTR
void d18b20_processAlarms(void){

    d18b20_resetAlarmSearch();
    DeviceAddress alarmAddr;

    while (d18b20_alarmSearch(alarmAddr)){

        if (d18b20_validAddress(alarmAddr)){
            _AlarmHandler(alarmAddr);
        }

    }
}

// sets the alarm handler
ICACHE_FLASH_ATTR
void d18b20_setAlarmHandler(const AlarmHandler *handler){
    _AlarmHandler = handler;
}

// The default alarm handler
ICACHE_FLASH_ATTR
void d18b20_defaultAlarmHandler(const uint8_t* deviceAddress){}

#endif
