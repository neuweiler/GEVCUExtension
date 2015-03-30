/*
 * Temperature.c
 *
 */

#include "Temperature.h"

Temperature::Temperature(): Device()
{
    canHandlerEv = CanHandler::getInstanceEV();
    commonName = "TemperatureProbe";
}

void Temperature::setup()
{
    TickHandler::getInstance()->detach(this);

    Device::setup(); //call base class

    Logger::info(TEMPERATURE, "locating temperature sensors...");

    TemperatureSensor::resetSearch();
    int i;
    for (i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS; i++) {
        devices[i] = TemperatureSensor::search();
        if (devices[i] == NULL)
            break;
        Logger::info(TEMPERATURE, "found sensor #%d: addr=%X, %s", i, devices[i]->getAddress(), devices[i]->getTypeStr());
    }
    devices[i] = NULL;
    TemperatureSensor::prepareData();

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_TEMPERATURE);
}

void Temperature::handleTick()
{
    canHandlerEv->prepareOutputFrame(&outputFrame, CAN_ID_GEVCU_EXT_TEMPERATURE);

    // read temperatures and send them via CAN bus
    for (int i = 0; i < CFG_MAX_NUM_TEMPERATURE_SENSORS && devices[i] != NULL; i++) {
        devices[i]->retrieveData();
        if (Logger::isDebug()) {
            Logger::debug(TEMPERATURE, "sensor #%d: %f C", i, devices[i]->getTemperatureCelsius());
        }
        if (i < 8) {
            outputFrame.data.byte[i] = constrain(round(devices[i]->getTemperatureCelsius()) + 50, 0, 255);
        }
    }
    canHandlerEv->sendFrame(outputFrame);

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
