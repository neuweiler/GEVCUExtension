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

// CAN bus id's for frames sent to the heater
#define CAN_ID_WAKEUP       0x100 // wake up the device
#define CAN_ID_KEEP_ALIVE   0x621 // keep alive message
#define CAN_ID_CONTROL      0x // send power conmtrol message

// CAN bus id's for frames received from the heater

//TODO: define correct can ID's, mask and masked id's
#define CAN_ID_STATUS           0x258 // receive status message                  01001011000
#define CAN_MASK_1              0x7cc // mask for above id's                     11111001100
#define CAN_MASKED_ID_1         0x248 // masked id for id's from 0x258 to 0x268  01001001000

#define CAN_ID_TEMP         0x458 // receive temperature information             10001011000
#define CAN_MASK_2          0x7ff // mask for above id's                         11111111111
#define CAN_MASKED_ID_2     0x458 // masked id for id's from 0x258 to 0x268      10001011000

class EberspaecherHeater: public Device, CanObserver
{
public:
    EberspaecherHeater();
    void setup();
    void handleTick();
    void handleCanFrame(CAN_FRAME *frame);
    void processStatus(uint8_t data[]);
    DeviceId getId();
    DeviceType getType();

protected:

private:
    CanHandler *canHandlerCar;
    CAN_FRAME outputFrame; // the output CAN frame;

    void sendControl();

};

#endif /* EBERSPAECHERHEATER_H_ */
