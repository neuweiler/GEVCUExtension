#include "GEVCUExtension.h"

void createDevices()
{
    deviceManager.addDevice(new Heartbeat());
    deviceManager.addDevice(new Temperature());
    deviceManager.addDevice(new EberspaecherHeater());
    deviceManager.addDevice(new CanIO());
}

void setup()
{
    SerialUSB.begin(CFG_SERIAL_SPEED);

    memCache.setup();
    canHandlerEv.setup();
    canHandlerCar.setup();

    createDevices();
    status.setSystemState(Status::init);
}

void loop()
{
    tickHandler.process();
    canHandlerEv.process();
    canHandlerCar.process();
}
