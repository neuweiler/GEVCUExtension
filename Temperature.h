/*
 * Temperature.h
 *
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include "Device.h"
#include "config.h"
#include "TickHandler.h"
#include "DeviceManager.h"
#include "CanHandler.h"
#include "TemperatureSensor.h"

#define CAN_ID_GEVCU_EXT_TEMPERATURE     0x728 // Temperature CAN message

class Temperature: public Device
{
public:
    Temperature();
    void setup();
    void handleTick();
    DeviceId getId();
    DeviceType getType();
    float getMinimum();
    float getMaximum();
    float getSensorTemperature(byte[]);

protected:

private:
    CAN_FRAME outputFrame; // the output CAN frame;
    TemperatureSensor *devices[CFG_MAX_NUM_TEMPERATURE_SENSORS];

    // The following are addresses of temperature sensors, adapt them for your own
    byte addrBatteryTrunk[8] = { 0x28, 0xFF, 0x5F, 0x3C, 0x64, 0x14, 0x01, 0x5A };
    byte addrBatteryRearLeft[8] = { 0x28, 0xFF, 0x8C, 0xD3, 0x64, 0x14, 0x02, 0x7B };
    byte addrBatteryRearRight[8] = { 0x28, 0xFF, 0xE8, 0xA8, 0x64, 0x14, 0x02, 0x49 };
    byte addrBatteryMid[8] = { 0x28, 0xFF, 0xBE, 0x48, 0x64, 0x14, 0x01, 0xEB };
    byte addrBatteryFrontUpper[8] = { 0x28, 0xFF, 0xDD, 0x99, 0x64, 0x14, 0x02, 0xD8 };
    byte addrBatteryFrontLower[8] = { 0x28, 0xFF, 0xA6, 0x69, 0x64, 0x14, 0x02, 0xB4 };
    byte addrCoolant[8] = { 0x28, 0xFF, 0x28, 0x67, 0xA8, 0x15, 0x01, 0x06 };
    byte addrExterior[8] = { 0x28, 0xFF, 0xDE, 0x26, 0xA8, 0x15, 0x04, 0x6C };

    void sendTemperature();
};

#endif /* TEMPERATURE_H_ */
