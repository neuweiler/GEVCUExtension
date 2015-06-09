#include "GEVCUExtension.h"

CanHandler *canHandlerEV;
CanHandler *canHandlerCar;
TickHandler *tickHandler;

void createDevices()
{
    DeviceManager *deviceManager = DeviceManager::getInstance();

    deviceManager->addDevice(new Heartbeat());
    deviceManager->addDevice(new Temperature());
    deviceManager->addDevice(new EberspaecherHeater());
    deviceManager->addDevice(new CanIO());
}

void setup()
{
    pinMode(CFG_BLINK_LED, OUTPUT);
    digitalWrite(CFG_BLINK_LED, LOW);

    SerialUSB.begin(CFG_SERIAL_SPEED);

    tickHandler = TickHandler::getInstance();

    canHandlerEV = CanHandler::getInstanceEV();
    canHandlerCar = CanHandler::getInstanceCar();
    canHandlerEV->initialize();
    canHandlerCar->initialize();

    createDevices();
    DeviceManager::getInstance()->sendMessage(DEVICE_ANY, INVALID, MSG_STARTUP, NULL);
}

void loop()
{
    tickHandler->process();
    canHandlerEV->process();
    canHandlerCar->process();
}
