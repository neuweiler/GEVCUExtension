/*
 * DeviceTypes.h
 *
 */

#ifndef DEVICE_TYPES_H_
#define DEVICE_TYPES_H_

enum DeviceType {
    DEVICE_ANY,
    DEVICE_SENSOR,
    DEVICE_NONE
};

enum DeviceId { //unique device ID for every piece of hardware possible
    SYSTEM = 0x5000,
    HEARTBEAT = 0x5001,
    TEMPERATURE = 0x5002,
    INVALID = 0xFFFF
};

#endif /* DEVICE_TYPES_H_ */
