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

    // delay startup to have enough time to activate logging
    for (int i = 5; i > 0; i--) {
        SerialUSB.println(i);
        delay(1000);
    }

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
