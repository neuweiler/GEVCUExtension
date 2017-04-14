/*
 * Status.h
 *
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

#ifndef STATUS_H_
#define STATUS_H_

#include <Arduino.h>
#include "Logger.h"
#include "Sys_Messages.h"

class Status
{
public:
    enum SystemState
    {
        startup = 0, // the system is starting-up (next states: init, error)
        init = 1, // the system is being initialized and is not ready for operation yet (next states: preCharge, ready, error)
        preCharge = 2, // the system is initialized and executing the pre-charge cycle (next states: ready, error)
        preCharged = 3, // the system is pre-charged, the pre-charge cycle is finished
        batteryHeating = 4, // before charging, the batteries need to be heated
        charging = 5, // the batteries are being charged
        charged = 6, // the charging is finished
        ready = 7, // the system is ready to accept commands but the motor controller is not enabled yet (next states: running, error)
        running = 8, // the system is running and the motor controller is to be enabled (next states: ready, error)
        shutdown = 9, // the system is shutdown and must be restarted to get operational again
        error = 99 // the system is in an error state and not operational (no power on motor, turn of power stage)
    };
    uint16_t analogIn[4];

    bool preChargeRelay;
    bool mainContactor;
    bool secondaryContactor;
    bool fastChargeContactor;
    bool enableMotor;
    bool enableCharger;
    bool enableDcDc;
    bool enableHeater;
    bool heaterValve;
    bool heaterPump;
    bool coolingPump;
    bool coolingFan;
    bool brakeLight;
    bool reverseLight;
    bool powerSteering;
    bool unused;

    Status();
    SystemState getSystemState();
    SystemState setSystemState(SystemState);
    char *systemStateToStr(SystemState);

private:
    SystemState systemState; // the current state of the system, to be modified by the state machine of this class only
};

extern Status status;

#endif /* STATUS_H_ */
