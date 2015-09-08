/*
 * EberspaecherHeater.c
 *
 * The heater communicates using J1939 protocol. It has to be "woken up" one time with a 0x100 message and then
 * must see a "keep alive" to stay active, which is the 0x621 message. The message repetition rate is between
 * 25 and 100ms intervals.
 *
 * The Eberspacher CAN version will work when used with a 33.33Kb SWCAN. The data below is the minimum required
 * to turn on the heater. It will operate at approximately 33% of full power. To command higher power, increase
 * the value of message 0x1072099 byte 1 (it begins with byte 0) which is 3E below.
 * Full power is applied when 85 is used as the value for byte 1. The power will vary based upon inlet
 * temperature as the PTC elements increase the resistance with higher temperature.
 *
 * ID,         Ext,  LEN,D0,D1,D2,D3,D4,D5,D6,D7
 *
 * 0x100,      False, 0, 00,00,00,00,00,00,00,00
 *
 * 0x621,      False, 8, 00,40,00,00,00,00,00,00 - keep alive
 * 0x13FFE060, True,  0, 00,00,00,00,00,00,00,00 - cmd1
 * 0x10720099, True,  5, 02,3E,00,00,00,00,00,00 - control
 * 0x102CC040, True,  8, 01,01,CF,0F,00,51,46,60 - cmd2
 * 0x10242040, True,  1, 00,00,00,00,00,00,00,00 - cmd3
 * 0x102740CB, True,  3, 2D,00,00,00,00,00,00,00 - cmd4
 * 0x102740CB, True,  3, 19,00,00,00,00,00,00,00 - cmd5
 */

#include "EberspaecherHeater.h"

EberspaecherHeater::EberspaecherHeater() : Device()
{
    canHandlerCar = CanHandler::getInstanceCar();
    commonName = "Eberspaecher Heater";
}

void EberspaecherHeater::setup()
{
    TickHandler::getInstance()->detach(this);

    Device::setup(); //call base class

    sendWakeup();
    prepareFrames();

    // register ourselves as observer of 0x... and 0x... can frames
    // canHandlerCar->attach(this, CAN_MASKED_ID_1, CAN_MASK_1, false);
    // canHandlerCar->attach(this, CAN_MASKED_ID_2, CAN_MASK_2, false);

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_EBERSPAECHER_HEATER);
}

void EberspaecherHeater::sendWakeup()
{
    // 0x100, False, 0, 00,00,00,00,00,00,00,00
    canHandlerCar->prepareOutputFrame(&frameControl, CAN_ID_WAKEUP);
    canHandlerCar->sendFrame(frameControl);
}

void EberspaecherHeater::prepareFrames()
{
    // 0x621, False, 8, 00,40,00,00,00,00,00,00 - keep alive
    canHandlerCar->prepareOutputFrame(&frameKeepAlive, CAN_ID_KEEP_ALIVE);
    frameKeepAlive.length = 8;
    frameKeepAlive.data.byte[1] = 0x40;
    // 0x13FFE060, True, 0, 00,00,00,00,00,00,00,00 - cmd1
    canHandlerCar->prepareOutputFrame(&frameCmd1, CAN_ID_CMD1);
    frameCmd1.extended = 1;
    // 0x102CC040, True, 8, 01,01,CF,0F,00,51,46,60 - cmd2
    canHandlerCar->prepareOutputFrame(&frameCmd2, CAN_ID_CMD2);
    frameCmd2.extended = 1;
    frameCmd2.length = 8;
    frameCmd2.data.value = 0x0101CF0F00514660;
    // 0x10242040, True, 1, 00,00,00,00,00,00,00,00 - cmd3
    canHandlerCar->prepareOutputFrame(&frameCmd3, CAN_ID_CMD3);
    frameCmd3.extended = 1;
    frameCmd3.length = 1;
    // 0x102740CB, True, 3, 2D,00,00,00,00,00,00,00 - cmd4
    canHandlerCar->prepareOutputFrame(&frameCmd4, CAN_ID_CMD4);
    frameCmd4.extended = 1;
    frameCmd4.length = 3;
    frameCmd4.data.byte[0] = 0x2d;
    // 0x102740CB, True, 3, 19,00,00,00,00,00,00,00 - cmd5
    canHandlerCar->prepareOutputFrame(&frameCmd5, CAN_ID_CMD5);
    frameCmd5.extended = 1;
    frameCmd5.length = 3;
    frameCmd5.data.byte[0] = 0x19;
    // 0x10720099, True, 5, 02,3E,00,00,00,00,00,00 - control
    canHandlerCar->prepareOutputFrame(&frameControl, CAN_ID_CONTROL);
    frameControl.extended = 1;
    frameControl.length = 5;
    frameControl.data.byte[0] = 0x02;
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
    uint8_t powerRequested = 0; // value from 0 to 100 percent power

    frameControl.data.byte[1] = map(powerRequested, 0, 100, 0, 0x85);

    if (Logger::isDebug()) {
        Logger::debug(EBERSPAECHER, "requested power: %l", powerRequested);
        canHandlerCar->logFrame(frameKeepAlive);
        canHandlerCar->logFrame(frameCmd1);
        canHandlerCar->logFrame(frameControl);
        canHandlerCar->logFrame(frameCmd2);
        canHandlerCar->logFrame(frameCmd3);
        canHandlerCar->logFrame(frameCmd4);
        canHandlerCar->logFrame(frameCmd5);
    }

    canHandlerCar->sendFrame(frameKeepAlive);
    canHandlerCar->sendFrame(frameCmd1);
    canHandlerCar->sendFrame(frameControl);
    canHandlerCar->sendFrame(frameCmd2);
    canHandlerCar->sendFrame(frameCmd3);
    canHandlerCar->sendFrame(frameCmd4);
    canHandlerCar->sendFrame(frameCmd5);
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
