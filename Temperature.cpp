/*
 * Temperature.c
 *
 */

#include "Temperature.h"

Temperature::Temperature(): Device()
{
    commonName = "TemperatureProbe";
}

void Temperature::setup()
{
    TickHandler::getInstance()->detach(this);

    Device::setup(); //call base class

    Logger::info("locating temperature sensors...");

    TemperatureSensor::resetSearch();
    int i;
    for (i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS; i++) {
        devices[i] = TemperatureSensor::search();
        if (devices[i] == NULL)
            break;
        Logger::info("found sensor #%d: addr=%X, %s", i, devices[i]->getAddress(), devices[i]->getTypeStr());
    }
    devices[i] = NULL;
    TemperatureSensor::prepareData();

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_TEMPERATURE);
}

void Temperature::handleTick()
{
    // read temperatures
    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        devices[i]->retrieveData();
        if (Logger::isDebug()) {
            Logger::debug("Sensor #%d: %f C", i, devices[i]->getTemperatureCelsius());
        }
    }
    // calculate temperatures for next tick
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

float Temperature::getMinimum() {
    float minimum = 999.0;

    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        float temp = devices[i]->getTemperatureCelsius();
        if (temp < minimum) {
            minimum = temp;
        }
    }
    return minimum;
}

float Temperature::getMaximum() {
    float maximum = -999.0;

    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        float temp = devices[i]->getTemperatureCelsius();
        if (temp > maximum) {
            maximum = temp;
        }
    }
    return maximum;
}
