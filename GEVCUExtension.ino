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

void setup()
{
    SerialUSB.begin(CFG_SERIAL_SPEED);

    // delay startup to have enough time to activate logging
//    for (int i = 5; i > 0; i--) {
//        SerialUSB.println(i);
//        delay(1000);
//    }

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
