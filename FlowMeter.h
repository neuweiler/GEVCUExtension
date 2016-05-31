/*
 * FlowMeter.h
 *
 *  Created on: 10.01.2016
 *      Author: d4220
 */

#ifndef FLOWMETER_H_
#define FLOWMETER_H_

#include "Device.h"
#include "config.h"
#include "TickHandler.h"
#include "CanHandler.h"

#define CAN_ID_GEVCU_FLOW_HEAT     0x729 // Flow CAN message heater
#define CAN_ID_GEVCU_FLOW_COOL     0x72a // Flow CAN message cooling

class FlowMeterConfiguration: public DeviceConfiguration
{
public:
    uint16_t calibrationFactor; // the number of pulses per liter (usually 270)
};

class FlowMeter: public Device
{
public:
    FlowMeter(DeviceId id, uint8_t pin);
    void setup();
    void tearDown();
    void handleTick();
    DeviceId getId();
    DeviceType getType();
    void loadConfiguration();
    void saveConfiguration();
    float getFlowLiterPerMin();
    uint32_t getFlowMilliLiterPerSec();
    uint32_t getTotalMilliLiter();

protected:

private:
    CAN_FRAME outputFrame; // the output CAN frame
    DeviceId id;
    uint8_t sensorPin;
    float flowLiterPerMin; // flow rate in liter/min
    uint32_t flowMilliLiterPerSec; // flow rate in milliliters/second
    uint32_t totalMilliLiter; // total milliliters measured
    unsigned long oldTime;

};

#endif /* FLOWMETER_H_ */
