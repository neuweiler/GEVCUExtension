/*
 * Heartbeat.c
 *
 */

#include "Heartbeat.h"

Heartbeat::Heartbeat() : Device() {
    led = false;
    dotCount = 0;
    commonName = "Heartbeat";
}

void Heartbeat::setup() {
    TickHandler::getInstance()->detach(this);

    Logger::info("add device: Heartbeat");
    Device::setup(); //call base class

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_HEARTBEAT);
}

void Heartbeat::handleTick() {
    SerialUSB.print('.');
    // Print a dot if no other output has been made since the last tick
    if ((++dotCount % 80) == 0) {
        SerialUSB.println();
    }

    if (led) {
        digitalWrite(BLINK_LED, HIGH);
    } else {
        digitalWrite(BLINK_LED, LOW);
    }
    led = !led;
}

DeviceId Heartbeat::getId() {
    return HEARTBEAT;
}

