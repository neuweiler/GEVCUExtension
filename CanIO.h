/*
 * CanIO.h
 *
 Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

#define CAN_ID_GEVCU_STATUS     0x724 // receive status message                  11100100100
#define CAN_ID_GEVCU_ANALOG_IO  0x725 // receive status message                  11100100101
#define CAN_MASK                0x7fe // mask for above id's                     11111111110
#define CAN_MASKED_ID           0x724 // masked id for id's from 0x258 to 0x268  11100100100

class CanIOConfiguration: public DeviceConfiguration
{
public:
    uint8_t prechargeRelayOutput;
    uint8_t mainContactorOutput;
    uint8_t secondaryContactorOutput;
    uint8_t fastChargeContactorOutput;

    uint8_t enableMotorOutput;
    uint8_t enableChargerOutput;
    uint8_t enableDcDcOutput;
    uint8_t enableHeaterOutput;

    uint8_t heaterValveOutput;
    uint8_t heaterPumpOutput;
    uint8_t coolingPumpOutput;
    uint8_t coolingFanOutput;

    uint8_t brakeLightOutput;
    uint8_t reverseLightOutput;
    uint8_t powerSteeringOutput;
    uint8_t unusedOutput;
};

class CanIO: public Device, CanObserver
{
public:
    // Message id=0x724, GEVCU_STATUS
    // The value is composed of 2 bytes: (data[1] << 0) | (data[0] << 8)
    enum GEVCU_RawIO
    {
        digitalOut8 = 1 << 0,  // 0x0001, data[1], Motorola bit 15
        digitalOut7 = 1 << 1,  // 0x0002, data[1], Motorola bit 14
        digitalOut6 = 1 << 2,  // 0x0004, data[1], Motorola bit 13
        digitalOut5 = 1 << 3,  // 0x0008, data[1], Motorola bit 12
        digitalOut4 = 1 << 4,  // 0x0010, data[1], Motorola bit 11
        digitalOut3 = 1 << 5,  // 0x0020, data[1], Motorola bit 10
        digitalOut2 = 1 << 6,  // 0x0040, data[1], Motorola bit 9
        digitalOut1 = 1 << 7,  // 0x0080, data[1], Motorola bit 8

        digitalIn4 = 1 << 12, // 0x1000, data[0], Motorola bit 3
        digitalIn3 = 1 << 13, // 0x2000, data[0], Motorola bit 2
        digitalIn2 = 1 << 14, // 0x4000, data[0], Motorola bit 1
        digitalIn1 = 1 << 15  // 0x8000, data[0], Motorola bit 0
    };

    // The value is composed of 2 bytes: (data[3] << 0) | (data[2] << 8)
    enum GEVCU_LogicIO
    {
        preChargeRelay = 1 << 0,  // 0x0001, data[3], Motorola bit 15
        mainContactor = 1 << 1,  // 0x0002, data[3], Motorola bit 14
        secondaryContactor = 1 << 2,  // 0x0004, data[3], Motorola bit 13
        fastChargeContactor = 1 << 3,  // 0x0008, data[3], Motorola bit 12

        enableMotor = 1 << 4,  // 0x0010, data[3], Motorola bit 11
        enableCharger = 1 << 5,  // 0x0020, data[3], Motorola bit 10
        enableDcDc = 1 << 6,  // 0x0040, data[3], Motorola bit 9
        enableHeater = 1 << 7,  // 0x0080, data[3], Motorola bit 8

        heaterValve = 1 << 8,  // 0x0100, data[2], Motorola bit 7
        heaterPump = 1 << 9,  // 0x0200, data[2], Motorola bit 6
        coolingPump = 1 << 10, // 0x0400, data[2], Motorola bit 5
        coolingFan = 1 << 11, // 0x0800, data[2], Motorola bit 4

        brakeLight = 1 << 12, // 0x1000, data[2], Motorola bit 3
        reverseLight = 1 << 13, // 0x2000, data[2], Motorola bit 2
        powerSteering = 1 << 14, // 0x4000, data[2], Motorola bit 1
        unused = 1 << 15  // 0x8000, data[2], Motorola bit 0
    };

    CanIO();
    void setup();
    void tearDown();
    void handleTick();
    void handleCanFrame(CAN_FRAME *frame);
    void processGevcuStatus(CAN_FRAME *frame);
    void processGevcuAnalogIO(CAN_FRAME *frame);
    void printStatus();
    DeviceId getId();
    DeviceType getType();
    void loadConfiguration();
    void saveConfiguration();

protected:

private:
    long lastReception;
    CAN_FRAME outputFrame; // the output CAN frame;

    void resetOutput();
    void setPinMode(uint8_t pin);
    void setOutput(uint8_t pin, bool value);
    bool getOutput(uint8_t pin);
};

#endif /* CANIO_H_ */
