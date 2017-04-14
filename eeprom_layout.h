/*
 * eeprom_layout.h
 *
 *EEPROM Map. There is support for up to 6 devices: A motor controller, display, charger, BMS, Throttle,  and a misc device (EPAS, WOC, etc)
 *
 *There is a 256KB eeprom chip which stores these settings. The 4K is allocated to primary storage and 4K is allocated to a "known good"
 * storage location. This leaves most of EEPROM free for something else, probably logging.

 Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "config.h"

/*
 The device table is just a list of IDs. The devices register for a spot in the table.
 Since each device has a 16 bit ID and the reserved space is 128 bytes we can support
 64 different devices in the table and EEPROM
 Devices are considered enabled if their highest ID bit is set (0x8000) otherwise
 they're disabled.
 This means that valid IDs must be under 0x8000 but that still leaves a couple of open IDs ;)
 First device entry is 0xCAFE if valid - otherwise table is initialized

 Layout :

 #1 32kB block (EE_DEVICES_TABLE to EE_DEVICES_BASE + EE_NUM_DEVICES * EE_DEVICE_SIZE - 1)
 0000-0001 : GEVCU Extension marker (0xCAFE)
 0002-0003 : ID of device 1 (enabled if bit 0x8000 of ID is set)
 0004-0005 : ID of device 2 (enabled if bit 0x8000 of ID is set)
 0006-0007 : ID of device 3 (enabled if bit 0x8000 of ID is set)
 0008-0009 : ID of device 4 (enabled if bit 0x8000 of ID is set)
 ...
 0126-0127 : ID of device 63 (enabled if bit 0x8000 of ID is set)
 0128-511 : unused
 0512-1023 : config device 1 (first byte = checksum)
 1024-1535 : config device 2 (first byte = checksum)
 1536-2047 : config device 3 (first byte = checksum)
 2048-2559 : config device 4 (first byte = checksum)
 ...
 32256-32767 : config device 63 (first byte = checksum)

 #2 32kB block (EE_LKG_OFFSET to LGK_OFFSET + EE_DEVICES_BASE + EE_NUM_DEVICES * EE_DEVICE_SIZE - 1)
 32768-32769 : GEVCU Extension marker (0xCAFE)
 32770-32771 : ID of device 1 (enabled if bit 0x8000 of ID is set)
 32772-32773 : ID of device 2 (enabled if bit 0x8000 of ID is set)
 ...
 32894-32895 : ID of device 63 (enabled if bit 0x8000 of ID is set)
 32896-33279 : unused
 33280-33791 : lkg config device 1 (first byte = checksum)
 33792-34303 : lkg config device 2 (first byte = checksum)
 ...
 65024-65535 : lkg config device 63 (first byte = checksum)

 #3 and #4 32kB block (EE_SYS_LOG to EE_FAULT_LOG - 1)
 65536-98303 : system log

 #5 and #6 32kB block (EE_FAULT_LOG to 131071)
 98304-131071 : fault log

 #7 and #8 32kB block:
 131072-... : tbd

 */
#define EE_DEVICE_TABLE     0 //where is the table of devices found in EEPROM?
#define EE_NUM_DEVICES		63 // the number of supported device entries
#define EE_GEVCU_MARKER     0xCAFE // marker at position 0 to identify EEPROM was initialized

#define EE_DEVICE_SIZE      512 //# of bytes allocated to each device
#define EE_DEVICES_BASE     512 //start of where devices in the table can use

#define EE_MAIN_OFFSET      0 //offset from start of EEPROM where main config is
#define EE_LKG_OFFSET       32768  //start EEPROM addr where last known good config is

//start EEPROM addr where the system log starts. <SYS LOG YET TO BE DEFINED>
#define EE_SYS_LOG          65536

//start EEPROM addr for fault log (Used by fault_handler)
#define EE_FAULT_LOG        98304

/*Now, all devices also have a default list of things that WILL be stored in EEPROM. Each actual
 implementation for a given device can store it's own custom info as well. This data must come after
 the end of the stardard data. The below numbers are offsets from the device's eeprom section
 */

//first, things in common to all devices - leave 10 bytes for this
#define EE_CHECKSUM                         0 //1 byte - checksum for this section of EEPROM to makesure it is valid

// heater data
#define EEHEAT_MAX_POWER                    10 // 2 bytes
#define EEHEAT_TARGET_TEMPERATURE           12 // 1 bytes
#define EEHEAT_DERATING_TEMPERATURE         13 // 1 byte
#define EEHEAT_EXT_TEMPERATURE_ON           14 // 1 byte
#define EEHEAT_EXT_TEMPERATURE_ADDRESS      15 // 8 bytes

// can i/o data
#define EECAN_PRE_CHARGE_RELAY_OUTPUT       10 // 1 byte, output pin
#define EECAN_MAIN_CONTACTOR_OUTPUT         11 // 1 byte, output pin
#define EECAN_SECONDAY_CONTACTOR_OUTPUT     12 // 1 byte, output pin
#define EECAN_FAST_CHARGE_CONTACTOR_OUTPUT  13 // 1 byte, output pin
#define EECAN_ENABLE_MOTOR_OUTPUT           14 // 1 byte, output pin
#define EECAN_ENABLE_CHARGER_OUTPUT         15 // 1 byte, output pin
#define EECAN_ENABLE_DCDC_OUTPUT            16 // 1 byte, output pin
#define EECAN_ENABLE_HEATER_OUTPUT          17 // 1 byte, output pin
#define EECAN_HEATER_VALVE_OUTPUT           18 // 1 byte, output pin
#define EECAN_HEATER_PUMP_OUTPUT            19 // 1 byte, output pin
#define EECAN_COOLING_PUMP_OUTPUT           20 // 1 byte, output pin
#define EECAN_COOLING_FAN_OUTPUT            21 // 1 byte, output pin
#define EECAN_BRAKE_LIGHT_OUTPUT            22 // 1 byte, output pin
#define EECAN_REVERSE_LIGHT_OUTPUT          23 // 1 byte, output pin
#define EECAN_WARNING_OUTPUT                24 // 1 byte, output pin
#define EECAN_POWER_LIMITATION_OUTPUT       25 // 1 byte, output pin
#define EECAN_yyy                           12 // 1 byte - flag...

// flow meter data
#define EEFLOW_CALIBRATION_FACTOR           10 // 2 bytes, value

#endif
