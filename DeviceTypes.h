/*
 * DeviceTypes.h
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

#ifndef DEVICE_TYPES_H_
#define DEVICE_TYPES_H_

enum DeviceType
{
    DEVICE_ANY,
    DEVICE_MISC,
    DEVICE_SENSOR,
    DEVICE_HEATER,
    DEVICE_IO,
    DEVICE_FLOW_METER,
    DEVICE_NONE
};

enum DeviceId
{ //unique device ID for every piece of hardware possible
    NEW = 0x0000,
    SYSTEM = 0x5000,
    HEARTBEAT = 0x5001,
    MEMCACHE = 0x5002,
    EBERSPAECHER = 0x6000,
    CAN_IO = 0x6100,
    TEMPERATURE = 0x6200,
    FLOW_METER_COOLING = 0x6300,
    FLOW_METER_HEATER = 0x6301,
    INVALID = 0xFFFF
};

const DeviceId deviceIds[] = {
        HEARTBEAT,
        EBERSPAECHER,
        CAN_IO,
        TEMPERATURE,
        FLOW_METER_COOLING,
        FLOW_METER_HEATER
};

const uint8_t deviceIdsSize = (sizeof(deviceIds) / sizeof(DeviceId));

#endif /* DEVICE_TYPES_H_ */
