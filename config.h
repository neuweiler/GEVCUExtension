/*
 * config.h
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <due_can.h>

#define CFG_VERSION "GEVCU extension 2015-03-16"
#define CFG_DEFAULT_LOGLEVEL Logger::Info

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
#define CFG_TICK_INTERVAL_HEARTBEAT                 2000000
#define CFG_TICK_INTERVAL_TEMPERATURE               2000000
#define CFG_TICK_INTERVAL_EBERSPAECHER_HEATER         20000
#define CFG_TICK_INTERVAL_CAN_IO                     200000

/*
 * CAN BUS CONFIGURATION
 */
#define CFG_CAN0_SPEED CAN_BPS_500K // specify the speed of the CAN0 bus (EV)
#define CFG_CAN1_SPEED CAN_BPS_500K // specify the speed of the CAN1 bus (Car)
#define CFG_CAN0_NUM_RX_MAILBOXES 7 // amount of CAN bus receive mailboxes for CAN0
#define CFG_CAN1_NUM_RX_MAILBOXES 7 // amount of CAN bus receive mailboxes for CAN1
#define CFG_CAN_IO_MSG_TIMEOUT 1000 // milliseconds a can IO message may be missing before the device faults

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
#define CFG_TIMER_BUFFER_SIZE 100 // the size of the queuing buffer for TickHandler
#define CFG_SERIAL_SEND_BUFFER_SIZE 50
#define CFG_MAX_NUM_TEMPERATURE_SENSORS 32

/*
 * PIN ASSIGNMENT
 */
#define CFG_IO_TEMPERATURE_SENSOR 24
#define CFG_OUTPUT_NONE    255
#define BLINK_LED          73 //13 is L, 73 is TX, 72 is RX

#endif /* CONFIG_H_ */
