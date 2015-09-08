/*
 * DeviceTypes.h
 *
 */

#ifndef DEVICE_TYPES_H_
#define DEVICE_TYPES_H_

enum DeviceType
{
    DEVICE_ANY,
    DEVICE_SENSOR,
    DEVICE_HEATER,
    DEVICE_IO,
    DEVICE_NONE
};

enum DeviceId
{ //unique device ID for every piece of hardware possible
    SYSTEM = 0x5000,
    HEARTBEAT = 0x5001,
    EBERSPAECHER = 0x6000,
    CAN_IO = 0x6100,
    TEMPERATURE = 0x6200,
    INVALID = 0xFFFF
};

#endif /* DEVICE_TYPES_H_ */
