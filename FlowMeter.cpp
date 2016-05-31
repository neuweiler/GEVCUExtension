/*
 * FlowMeter.cpp
 *
 *  Created on: 10.01.2016
 *      Author: d4220
 */

#include "FlowMeter.h"

volatile uint16_t pulseCountCooling;
volatile uint16_t pulseCountHeater;

/*
 Interrupt Service Routine
 */
void pulseCounterCooling()
{
    pulseCountCooling++;
}

void pulseCounterHeater()
{
    pulseCountHeater++;
}

/**
 * Constructor to initialize class variables
 */
FlowMeter::FlowMeter(DeviceId devId, uint8_t pin) :
        Device()
{
    id = devId;
    sensorPin = pin;
    flowLiterPerMin = 0;
    flowMilliLiterPerSec = 0;
    totalMilliLiter = 0;
    oldTime = 0;

    prefsHandler = new PrefHandler(id);
    if (id == FLOW_METER_COOLING) {
        commonName =  "Flow Meter Cooling";
    } else {
        commonName = "Flow Meter Heater";
    }
}

void FlowMeter::setup()
{
    Device::setup(); //call base class

    FlowMeterConfiguration *config = (FlowMeterConfiguration *) getConfiguration();

    pinMode(sensorPin, INPUT);
    digitalWrite(sensorPin, HIGH);

    ready = true;
    powerOn = true;

    switch (id) {
    case FLOW_METER_COOLING:
        pulseCountCooling = 0;
        attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounterCooling, FALLING);
        canHandlerEv.prepareOutputFrame(&outputFrame, CAN_ID_GEVCU_FLOW_COOL);
        break;
    case FLOW_METER_HEATER:
        pulseCountHeater = 0;
        attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounterHeater, FALLING);
        canHandlerEv.prepareOutputFrame(&outputFrame, CAN_ID_GEVCU_FLOW_HEAT);
        break;
    }
    tickHandler.attach(this, CFG_TICK_INTERVAL_FLOW_METER);
}

/**
 * Tear down the device in a safe way.
 */
void FlowMeter::tearDown()
{
    Device::tearDown();
    detachInterrupt(digitalPinToInterrupt(sensorPin));
}

/**
 * process a tick event from the timer the device is registered to.
 */
void FlowMeter::handleTick()
{
    FlowMeterConfiguration *config = (FlowMeterConfiguration *) getConfiguration();
    unsigned long newTime = millis();

    // copy over the value of pulseCount and reset the pulse counter so we can start incrementing again in interrupts
    uint16_t pulses = 0;
    switch (id) {
    case FLOW_METER_COOLING:
        pulses = pulseCountCooling;
        pulseCountCooling = 0;
        break;
    case FLOW_METER_HEATER:
        pulses = pulseCountHeater;
        pulseCountHeater = 0;
        break;
    }

    // part-of-one-second * 1000ml * pulses / pulses-per-liter
    flowMilliLiterPerSec = ((1000.0 / (newTime - oldTime)) * 1000 * pulses) / config->calibrationFactor;
    flowLiterPerMin = 60.0 * flowMilliLiterPerSec / 1000;
    totalMilliLiter += flowMilliLiterPerSec;
    oldTime = newTime;

    if (Logger::isDebug()) {
        Logger::info(id, "flow: %dml/sec, total: %dml", flowMilliLiterPerSec, totalMilliLiter);
    }
    outputFrame.data.high = flowMilliLiterPerSec;
    outputFrame.data.low = totalMilliLiter;
    canHandlerEv.sendFrame(outputFrame);
}

float FlowMeter::getFlowLiterPerMin()
{
    return flowLiterPerMin;
}

uint32_t FlowMeter::getFlowMilliLiterPerSec()
{
    return flowMilliLiterPerSec;
}

uint32_t FlowMeter::getTotalMilliLiter()
{
    return totalMilliLiter;
}

DeviceType FlowMeter::getType()
{
    return DEVICE_FLOW_METER;
}

DeviceId FlowMeter::getId()
{
    return id;
}

/*
 * Load configuration data from EEPROM.
 *
 * If not available or the checksum is invalid, default values are chosen.
 */
void FlowMeter::loadConfiguration()
{
    FlowMeterConfiguration *config = (FlowMeterConfiguration *) getConfiguration();

    if (!config) { // as lowest sub-class make sure we have a config object
        config = new FlowMeterConfiguration();
        setConfiguration(config);
    }

    Device::loadConfiguration(); // call parent

#ifdef USE_HARD_CODED

    if (false) {
#else
    if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
#endif
        uint16_t temp;
        prefsHandler->read(EEFLOW_CALIBRATION_FACTOR, &config->calibrationFactor);
    } else { //checksum invalid, reinitialize values and store to EEPROM
        config->calibrationFactor = 270; // some devices also give 450 pulses per liter
        saveConfiguration();
    }
    Logger::info(id, "calibration factor: %d pulses per liter", config->calibrationFactor);
}

/*
 * Store the current configuration parameters to EEPROM.
 */
void FlowMeter::saveConfiguration()
{
    FlowMeterConfiguration *config = (FlowMeterConfiguration *) getConfiguration();

    Device::saveConfiguration(); // call parent

    prefsHandler->write(EEFLOW_CALIBRATION_FACTOR, config->calibrationFactor);

    prefsHandler->saveChecksum();
}
