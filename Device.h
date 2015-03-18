/*
 * Device.h
 *
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <Arduino.h>
#include "config.h"
#include "DeviceTypes.h"
#include "Sys_Messages.h"
#include "TickHandler.h"

class DeviceManager;

/*
 * A abstract class for all Devices.
 */
class Device: public TickObserver
{
public:
    Device();
    virtual void setup();
    virtual void handleMessage(uint32_t, void*);
    virtual DeviceType getType();
    virtual DeviceId getId();
    void handleTick();
    virtual uint32_t getTickInterval();
    char* getCommonName();

protected:
    char *commonName;

private:
};

#endif /* DEVICE_H_ */
