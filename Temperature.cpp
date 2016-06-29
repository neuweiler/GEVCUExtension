/*
 * Temperature.c
 *
 */

#include "Temperature.h"

/**
 * Constructor to initialize class variables
 */
Temperature::Temperature() :
        Device()
{
    prefsHandler = new PrefHandler(TEMPERATURE);
    commonName = "TemperatureProbe";
}

/**
 * set-up the device
 */
void Temperature::setup()
{
    Device::setup(); //call base class

    Logger::info(this, "locating temperature sensors...");

    TemperatureSensor::resetSearch();
    int i;
    for (i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS; i++) {
        devices[i] = TemperatureSensor::search();
        if (devices[i] == NULL)
            break;
        byte *addr = devices[i]->getAddress();
        Logger::info(this, "found sensor #%d: addr=0x%#x %#x %#x %#x %#x %#x %#x %#x, %s", i, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], devices[i]->getTypeStr());
    }
    devices[i] = NULL;
    TemperatureSensor::prepareData();

    ready = true;

    tickHandler.attach(this, CFG_TICK_INTERVAL_TEMPERATURE);
}


/**
 * process a tick event from the timer the device is registered to.
 */
void Temperature::handleTick()
{
    sendTemperature();
    // order sensors to calculate temperatures for next tick
    TemperatureSensor::prepareData();
}

DeviceType Temperature::getType()
{
    return DEVICE_SENSOR;
}

DeviceId Temperature::getId()
{
    return TEMPERATURE;
}

/*
 * read temperatures and send them via CAN bus.
 * The first 6 bytes are used for battery temperature. Byte 6 is the coolant temperature
 * and byte 7 is the exterior temperature.
 * All temperatures are added a offset of 50 degree celsius so a range of -50 to +204 fits into one ubyte
 */
void Temperature::sendTemperature()
{
    canHandlerEv.prepareOutputFrame(&outputFrame, CAN_ID_GEVCU_EXT_TEMPERATURE);

    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        devices[i]->retrieveData();
        running = true;
        if (Logger::isDebug()) {
            Logger::debug(this, "sensor #%d: %f C", i, devices[i]->getTemperatureCelsius());
        }

        int byteNum = -1;
        if (!memcmp(devices[i]->getAddress(), addrBatteryFrontUpper, 8)) {
            byteNum = 0;
        } else if (!memcmp(devices[i]->getAddress(), addrBatteryFrontLower, 8)) {
            byteNum = 1;
        } else if (!memcmp(devices[i]->getAddress(), addrBatteryMid, 8)) {
            byteNum = 2;
        } else if (!memcmp(devices[i]->getAddress(), addrBatteryRearLeft, 8)) {
            byteNum = 3;
        } else if (!memcmp(devices[i]->getAddress(), addrBatteryRearRight, 8)) {
            byteNum = 4;
        } else if (!memcmp(devices[i]->getAddress(), addrBatteryTrunk, 8)) {
            byteNum = 5;
        } else if (!memcmp(devices[i]->getAddress(), addrCoolant, 8)) {
            byteNum = 6;
        } else if (!memcmp(devices[i]->getAddress(), addrExterior, 8)) {
            byteNum = 7;
        }
        if (byteNum != -1) {
            outputFrame.data.byte[byteNum] = constrain(round(devices[i]->getTemperatureCelsius()) + CFG_CAN_TEMPERATURE_OFFSET, 0, 255);
        }
    }
    canHandlerEv.sendFrame(outputFrame);
}

/*
 * Find the minimum temperature in celsius from all found sensors
 */
float Temperature::getMinimum()
{
    float minimum = 999.0;

    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        float temp = devices[i]->getTemperatureCelsius();
        if (temp < minimum) {
            minimum = temp;
        }
    }
    return minimum;
}

/*
 * Find the maximum temperature in celsius from all found sensors
 */
float Temperature::getMaximum()
{
    float maximum = -999.0;

    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        float temp = devices[i]->getTemperatureCelsius();
        if (temp > maximum) {
            maximum = temp;
        }
    }
    return maximum;
}

/*
 * Get the temperature from a sensor by specifying its address
 *
 * returns 999 if sensor was not found
 */
float Temperature::getSensorTemperature(byte *address) {
    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        if (memcmp(devices[i]->getAddress(), address, 8) == 0) {
            return devices[i]->getTemperatureCelsius();
        }
    }
    return 999;
}
