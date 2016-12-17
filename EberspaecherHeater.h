/*
 * EberspaecherHeater.h
 *
 */

#ifndef EBERSPAECHERHEATER_H_
#define EBERSPAECHERHEATER_H_

#include "Device.h"
#include "config.h"
#include "TickHandler.h"
#include "CanHandler.h"
#include "DeviceManager.h"
#include "Temperature.h"

#define MAX_POWER_WATT 6000

// CAN bus id's for frames sent to the heater
#define CAN_ID_WAKEUP       0x100 // wake up the device
#define CAN_ID_KEEP_ALIVE   0x621 // keep alive message
#define CAN_ID_CONTROL      0x10720099 // send power control message
#define CAN_ID_CMD1 0x13FFE060 // dummy message
#define CAN_ID_CMD2 0x102CC040 // dummy message
#define CAN_ID_CMD3 0x10242040 // dummy message
#define CAN_ID_CMD4 0x102740CB // dummy message
#define CAN_ID_CMD5 0x102740CB // dummy message

// CAN bus id's for frames received from the heater

//TODO: define correct can ID's, mask and masked id's
#define CAN_ID_STATUS           0x13FFE09D // receive status message             10011111111111110000010011101
#define CAN_MASK                0x0   // mask for above id's                     00000000000
#define CAN_MASKED_ID           0x0   // masked id for id's from 0x258 to 0x268  00000000000

class EberspaecherHeaterConfiguration: public DeviceConfiguration
{
public:
    uint16_t maxPower; // maximum power in watt (0 - 6000)
    uint8_t targetTemperature; // desired water temperature in deg C (0 - 100)
    uint8_t deratingTemperature; // temperature at which power will be derated from maxPower to 0% at target temperature in deg C (0 - 100, 255 = ignore)
    uint8_t extTemperatureOn; // external temperature at which heater is turned on in deg C (0 - 40, 255 = ignore)
    uint8_t extTemperatureSensorAddress[8]; // address of external temperature sensor
};

class EberspaecherHeater: public Device, CanObserver
{
public:
    EberspaecherHeater();
    void setup();
    void tearDown();
    void handleTick();
    void handleCanFrame(CAN_FRAME *frame);
    void handleStateChange(Status::SystemState, Status::SystemState);
    void processStatus(uint8_t *data);
    DeviceId getId();
    DeviceType getType();
    void loadConfiguration();
    void saveConfiguration();

protected:

private:
    CAN_FRAME frameWakeup; // frame to send wake-up message
    CAN_FRAME frameControl; // frame to send control messages
    CAN_FRAME frameKeepAlive; // frame to send heart beat
    CAN_FRAME frameCmd1; // frame to send cmd1 message
    CAN_FRAME frameCmd2; // frame to send cmd2 message
    CAN_FRAME frameCmd3; // frame to send cmd3 message
    CAN_FRAME frameCmd4; // frame to send cmd4 message
    CAN_FRAME frameCmd5; // frame to send cmd5 message
    uint16_t powerRequested; // value from 0 to 6000 watt
    Temperature *temperatureDevice;

    void calculatePower();
    void sendControl();
    void sendWakeup();
    void prepareFrames();
};

#endif /* EBERSPAECHERHEATER_H_ */
