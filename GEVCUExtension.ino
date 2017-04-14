#include "GEVCUExtension.h"

void createDevices()
{
    deviceManager.addDevice(new Heartbeat());
    deviceManager.addDevice(new Temperature());
    deviceManager.addDevice(new EberspaecherHeater());
    deviceManager.addDevice(new CanIO());
    deviceManager.addDevice(new FlowMeter(FLOW_METER_COOLING, CFG_FLOW_METER_COOLING));
    deviceManager.addDevice(new FlowMeter(FLOW_METER_HEATER, CFG_FLOW_METER_HEATER));
}

void delayStart(uint8_t seconds)
{
    for (int i = seconds; i > 0; i--) {
        SerialUSB.println(i);
        delay(1000);
    }
}

void setup()
{
    SerialUSB.begin(CFG_SERIAL_SPEED);
//    delayStart(10);
    SerialUSB.println(CFG_VERSION);

    memCache.setup();
    canHandlerEv.setup();
    canHandlerCar.setup();

    createDevices();
    serialConsole.printMenu();

    status.setSystemState(Status::init);
}

void loop()
{
    tickHandler.process();
    canHandlerEv.process();
    canHandlerCar.process();
    serialConsole.loop();
}
