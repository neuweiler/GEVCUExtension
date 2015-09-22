/*
 * config.h
 *
 * Defines the components to be used in the GEVCU and allows the user to configure
 * static parameters.
 *
 * Note: Make sure with all pin defintions of your hardware that each pin number is
 *       only defined once.

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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <due_can.h>

#define CFG_VERSION "GEVCU extension 2015-03-30"
#define CFG_DEFAULT_LOGLEVEL Logger::Debug

/*
 * SERIAL CONFIGURATION
 */
#define CFG_SERIAL_SPEED 115200
#define SerialUSB Serial // re-route serial-usb output to programming port ;) comment if output should go to std usb

/*
 * TIMER INTERVALS
 *
 * specify the intervals (microseconds) at which each device type should be "ticked"
 * try to use the same numbers for several devices because then they will share
 * the same timer (out of a limited number of 9 timers).
 */
#define CFG_TICK_INTERVAL_MEM_CACHE                   40000
#define CFG_TICK_INTERVAL_HEARTBEAT                 2000000
#define CFG_TICK_INTERVAL_TEMPERATURE               2000000
#define CFG_TICK_INTERVAL_EBERSPAECHER_HEATER         60000
#define CFG_TICK_INTERVAL_CAN_IO                     200000

/*
 * CAN BUS CONFIGURATION
 */
#define CFG_CAN0_SPEED CAN_BPS_500K // specify the speed of the CAN0 bus (EV)
#define CFG_CAN1_SPEED CAN_BPS_33333 // specify the speed of the CAN1 bus (Car / SW-CAN)
#define CFG_CAN0_NUM_TX_MAILBOXES 3 // how many of 8 mailboxes are used for TX for CAN0, rest is used for RX
#define CFG_CAN1_NUM_TX_MAILBOXES 5 // how many of 8 mailboxes are used for TX for CAN1, rest is used for RX
#define CFG_CAN_IO_MSG_TIMEOUT 1000 // milliseconds a can IO message may be missing before the device faults
#define CFG_CAN1_HV_MODE_PIN 51 // pin to use to set SW-CAN chip to HV mode (for wake-up)

/*
 * HARD CODED PARAMETERS
 *
 * If USE_HARD_CODED is defined or the checksum of the parameters stored in EEPROM,
 * the parameter values defined here are used instead of those stored in the EEPROM.
 */
//#define USE_HARD_CODED
#define PrechargeRelayOutput 0
#define MainContactorRelayOutput 1
#define SecondaryContactorRelayOutput 2
#define EnableRelayOutput   3
#define BrakeLightOutput    4
#define ReverseLightOutput  5
#define CoolingFanRelayOutput  7  //output to use for cooling fan

/*
 * ARRAY SIZE
 *
 * Define the maximum number of various object lists.
 * These values should normally not be changed.
 */
#define CFG_DEV_MGR_MAX_DEVICES 20 // the maximum number of devices supported by the DeviceManager
#define CFG_CAN_NUM_OBSERVERS 10 // maximum number of device subscriptions per CAN bus
#define CFG_TIMER_NUM_OBSERVERS 9 // the maximum number of supported observers per timer
#define CFG_TIMER_BUFFER_SIZE 100 // the size of the queueing buffer for TickHandler
#define CFG_SERIAL_SEND_BUFFER_SIZE 50
#define CFG_MAX_NUM_TEMPERATURE_SENSORS 32

/*
 * PIN ASSIGNMENT
 */
#define CFG_OUTPUT_NONE                 255
#define CFG_BLINK_LED                   13 //13 is L, 73 is TX, 72 is RX

#define CFG_IO_TEMPERATURE_SENSOR       20

#define CFG_IO_PRE_CHARGE_RELAY         22
#define CFG_IO_MAIN_CONTACTOR           23
#define CFG_IO_SECONDAY_CONTACTOR       24
#define CFG_IO_FAST_CHARGE_CONTACTOR    25

#define CFG_IO_ENABLE_MOTOR             29
#define CFG_IO_ENABLE_CHARGER           27
#define CFG_IO_ENABLE_DCDC              28
#define CFG_IO_ENABLE_HEATER            26

#define CFG_IO_HEATER_VALVE             30
#define CFG_IO_HEATER_PUMP              31
#define CFG_IO_COOLING_PUMP             32
#define CFG_IO_COOLING_FAN              33

#define CFG_IO_BRAKE_LIGHT              34
#define CFG_IO_REVERSE_LIGHT            35
#define CFG_IO_WARNING                  36
#define CFG_IO_POWER_LIMITATION         37

#endif /* CONFIG_H_ */
