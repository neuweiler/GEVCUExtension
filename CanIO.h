/*
 * CanIO.h
 *
 */

#ifndef CANIO_H_
#define CANIO_H_

#include "Device.h"
#include "config.h"
#include "TickHandler.h"
#include "CanHandler.h"
#include "DeviceManager.h"

// CAN bus id's for frames sent to the heater
//TODO: define correct can ID's, mask and masked id's
#define CAN_ID_WAKEUP       0x100 // wake up the device
#define CAN_ID_CONTROL      0x640 // send commands

// CAN bus id's for frames received from the heater

#define CAN_ID_STATUS           0x258 // receive status message                  01001011000
#define CAN_MASK_1              0x7cc // mask for above id's                     11111001100
#define CAN_MASKED_ID_1         0x248 // masked id for id's from 0x258 to 0x268  01001001000

#define CAN_ID_TEMP         0x458 // receive temperature information             10001011000
#define CAN_MASK_2          0x7ff // mask for above id's                         11111111111
#define CAN_MASKED_ID_2     0x458 // masked id for id's from 0x258 to 0x268      10001011000

class CanIO: public Device, CanObserver
{
public:
    CanIO();
    void setup();
    void handleTick();
    void handleCanFrame(CAN_FRAME *frame);
    void processStatus(uint8_t data[]);
    DeviceId getId();
    DeviceType getType();

protected:

private:
    bool faulted;
    long lastReception;
    CanHandler *canHandlerEv;
    CAN_FRAME outputFrame; // the output CAN frame;

    void resetOutput();
};

#endif /* CANIO_H_ */
