/*
 * Status.cpp
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

#include "Status.h"
#include "DeviceManager.h"

Status status;

/*
 * Constructor
 */
Status::Status()
{
    systemState = startup;
    for (int i = 0; i < 4; i++) {
        analogIn[i] = 0;
    }
}

/*
 * Retrieve the current system state.
 */
Status::SystemState Status::getSystemState()
{
    return systemState;
}

/*
 * Set a new system state. The new system state is validated if the
 * transition is allowed from the old state. If an invalid transition is
 * attempted, the new state will be 'error'.
 * The old and new state are broadcast to all devices.
 */
Status::SystemState Status::setSystemState(SystemState newSystemState)
{
    if (systemState == newSystemState) {
        return systemState;
    }

    SystemState oldSystemState = systemState;

    if (newSystemState == error) {
        systemState = error;
    } else {
        switch (systemState) {
        case startup:
            if (newSystemState == init) {
                systemState = newSystemState;
            }
            break;
        case init:
            if (newSystemState == preCharge) {
                systemState = newSystemState;
            }
            break;
        case preCharge:
            if (newSystemState == preCharged) {
                systemState = newSystemState;
            }
            break;
        case preCharged:
            if (newSystemState == ready) {
                systemState = newSystemState;
            }
            break;
        case batteryHeating:
            if (newSystemState == charging || newSystemState == ready) {
                systemState = newSystemState;
            }
            break;
        case charging:
            if (newSystemState == charged || newSystemState == ready) {
                systemState = newSystemState;
            }
            break;
        case charged:
            if (newSystemState == ready) {
                systemState = newSystemState;
            }
            break;
        case ready:
            if (newSystemState == running || newSystemState == charging || newSystemState == batteryHeating) {
                systemState = newSystemState;
            }
            break;
        case running:
            if (newSystemState == ready) {
                systemState = newSystemState;
            }
            break;
        case error:
            if (newSystemState == init) {
                systemState = newSystemState;
            }
            break;
        }
    }
    if (systemState == newSystemState) {
        Logger::info("switching to system state '%s'", systemStateToStr(systemState));
    } else {
        Logger::error("switching from system state '%s' to '%s' is not allowed", systemStateToStr(systemState), systemStateToStr(newSystemState));
        systemState = error;
    }

    SystemState params[] = { oldSystemState, newSystemState };
    deviceManager.sendMessage(DEVICE_ANY, INVALID, MSG_STATE_CHANGE, params);

    return systemState;
}

/*
 * Convert the current state into a string.
 */
char *Status::systemStateToStr(SystemState state)
{
    switch (state) {
    case startup:
        return "unknown";
    case init:
        return "init";
    case preCharge:
        return "pre-charge";
    case preCharged:
        return "pre-charged";
    case ready:
        return "ready";
    case running:
        return "running";
    case error:
        return "error";
    case batteryHeating:
        return "battery heating";
    case charging:
        return "charging";
    case charged:
        return "charged";
    }
    Logger::error("the system state is invalid, contact your support!");
    return "invalid";
}
