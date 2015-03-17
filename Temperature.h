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
#include "TemperatureSensor.h"

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
    TemperatureSensor *devices[CFG_MAX_NUM_TEMPERATURE_SENSORS];
};

#endif /* TEMPERATURE_H_ */
