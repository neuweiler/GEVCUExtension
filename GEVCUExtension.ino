#include "GEVCUExtension.h"

CanHandler *canHandlerEV;
CanHandler *canHandlerCar;
TickHandler *tickHandler;

void createDevices() {
    Heartbeat *heartbeat = new Heartbeat();
    Temperature *temperatureProbe = new Temperature();
}

void setup() {
    pinMode(BLINK_LED, OUTPUT);
    digitalWrite(BLINK_LED, LOW);

    SerialUSB.begin(CFG_SERIAL_SPEED);

    tickHandler = TickHandler::getInstance();

    canHandlerEV = CanHandler::getInstanceEV();
    canHandlerCar = CanHandler::getInstanceCar();
    canHandlerEV->initialize();
    canHandlerCar->initialize();

    createDevices();
    DeviceManager::getInstance()->sendMessage(DEVICE_ANY, INVALID, MSG_STARTUP, NULL);
}

void loop() {
    tickHandler->process();
    canHandlerEV->process();
    canHandlerCar->process();
}
