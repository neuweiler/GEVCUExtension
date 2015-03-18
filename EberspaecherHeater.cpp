/*
 * Temperature.c
 *
 */

#include "EberspaecherHeater.h"

EberspaecherHeater::EberspaecherHeater(): Device()
{
    canHandlerEv = CanHandler::getInstanceEV();

    commonName = "Eberspächer Heater";
}

void EberspaecherHeater::setup()
{
    TickHandler::getInstance()->detach(this);

    Device::setup(); //call base class

    // register ourselves as observer of 0x... and 0x... can frames
    canHandlerEv->attach(this, CAN_MASKED_ID_1, CAN_MASK_1, false);
    canHandlerEv->attach(this, CAN_MASKED_ID_2, CAN_MASK_2, false);

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_EBERSPAECHER_HEATER);
}

void EberspaecherHeater::handleTick()
{
    sendControl();
}

/*
 * Processes an event from the CanHandler.
 *
 * In case a CAN message was received which pass the masking and id filtering,
 * this method is called. Depending on the ID of the CAN message, the data of
 * the incoming message is processed.
 */
void EberspaecherHeater::handleCanFrame(CAN_FRAME *frame)
{
    switch (frame->id) {
        case CAN_ID_STATUS:
            processStatus(frame->data.bytes);
            break;
    }
}

/*
 * Send control message to the heater.
 *
 * This message controls the power-stage in the heater
 */
void EberspaecherHeater::sendControl()
{
    long powerRequested = 0;

    canHandlerEv->prepareOutputFrame(&outputFrame, CAN_ID_CONTROL);

    //TODO: implement preparation of control frame

    if (Logger::isDebug()) {
        Logger::debug(EBERSPAECHER, "requested power: %l", powerRequested);
    }

    canHandlerEv->sendFrame(outputFrame);
}

/*
 * Process a status message which was received from the heater.
 *
 */
void EberspaecherHeater::processStatus(uint8_t data[])
{
    //TODO: implement processing of bits and bytes
}

DeviceType EberspaecherHeater::getType()
{
    return DEVICE_HEATER;
}


DeviceId EberspaecherHeater::getId()
{
    return EBERSPAECHER;
}
