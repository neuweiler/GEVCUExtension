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

protected:

private:
    CanHandler *canHandlerEv;
    CAN_FRAME outputFrame; // the output CAN frame;
    TemperatureSensor *devices[CFG_MAX_NUM_TEMPERATURE_SENSORS];
};

#endif /* TEMPERATURE_H_ */
