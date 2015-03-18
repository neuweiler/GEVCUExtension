/*
 * Heartbeat.h
 *
 */

#ifndef HEARTBEAT_H_
#define HEARTBEAT_H_

#include "Device.h"
#include "config.h"
#include "TickHandler.h"
#include "DeviceManager.h"

class Heartbeat: public Device
{
public:
    Heartbeat();
    void setup();
    void handleTick();
    DeviceId getId();

protected:

private:
    bool led;
    int dotCount;
};

#endif /* HEARTBEAT_H_ */
