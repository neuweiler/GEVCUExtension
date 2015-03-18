/*
 * CanIO.c
 *
 */

#include "CanIO.h"

CanIO::CanIO(): Device()
{
    lastReception = -1;
    faulted = false;
    canHandlerEv = CanHandler::getInstanceEV();
    commonName = "Can I/O";
}

void CanIO::setup()
{
    TickHandler::getInstance()->detach(this);

    Device::setup(); //call base class

    // register ourselves as observer of 0x... and 0x... can frames
    canHandlerEv->attach(this, CAN_MASKED_ID_1, CAN_MASK_1, false);
    canHandlerEv->attach(this, CAN_MASKED_ID_2, CAN_MASK_2, false);

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_CAN_IO);
}

void CanIO::handleTick()
{
    if (millis() > lastReception + CFG_CAN_IO_MSG_TIMEOUT) {

        // in case the system's milli counter overflows, adapt to much lower values
        if (millis() < 3000) {
            lastReception = millis();
            return;
        }

        Logger::error(CAN_IO, "Too many lost messages, faulting and resetting all output. Reset device to restart.");

        faulted = true;
        resetOutput();
        canHandlerEv->detach(this, CAN_MASKED_ID_1, CAN_MASK_1);
        canHandlerEv->detach(this, CAN_MASKED_ID_2, CAN_MASK_2);
        TickHandler::getInstance()->detach(this);
    }
}

void CanIO::resetOutput() {
    //TODO: set all pins to low
}

/*
 * Processes an event from the CanHandler.
 *
 * In case a CAN message was received which pass the masking and id filtering,
 * this method is called. Depending on the ID of the CAN message, the data of
 * the incoming message is processed.
 */
void CanIO::handleCanFrame(CAN_FRAME *frame)
{
    if (faulted) {
        return;
    }

    switch (frame->id) {
        case CAN_ID_STATUS:
            processStatus(frame->data.bytes);
            break;
    }
}

/*
 * Process a status message which was received from the heater.
 *
 */
void CanIO::processStatus(uint8_t data[])
{
    //TODO: implement processing of bits and bytes
}

DeviceType CanIO::getType()
{
    return DEVICE_IO;
}


DeviceId CanIO::getId()
{
    return CAN_IO;
}
