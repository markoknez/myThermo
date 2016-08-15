/*
 * one_wire.h
 *
 *  Created on: 14. lis 2015.
 *      Author: Marko
 */

#ifndef IOT_DEMO_INCLUDE_DRIVER_ONE_WIRE_H_
#define IOT_DEMO_INCLUDE_DRIVER_ONE_WIRE_H_

#define ONEWIRE_PIN 	5
#define ONEWIRE_PIN_INIT()	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5)

// you can exclude onewire_search by defining that to 0
#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif

// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif

// Select the table-lookup method of computing the 8-bit CRC
// by setting this to 1.  The lookup table enlarges code size by
// about 250 bytes.  It does NOT consume RAM (but did in very
// old versions of OneWire).  If you disable this, a slower
// but very compact algorithm is used.
#ifndef ONEWIRE_CRC8_TABLE
#define ONEWIRE_CRC8_TABLE 0
#endif

// You can allow 16-bit CRC checks by defining this to 1
// (Note that ONEWIRE_CRC must also be 1.)
#ifndef ONEWIRE_CRC16
#define ONEWIRE_CRC16 0
#endif



#endif /* IOT_DEMO_INCLUDE_DRIVER_ONE_WIRE_H_ */
