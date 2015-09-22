/*
 * CanIO.c
 *
 Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "CanIO.h"

/**
 * Constructor to initialize class variables
 */
CanIO::CanIO() : Device()
{
    lastReception = -1;
    commonName = "Can I/O";
}

/**
 * set-up the device
 */
void CanIO::setup()
{
    Device::setup(); //call base class

    resetOutput(); // safety: make sure no output is activated when setting the pin mode

    // initialize digital output
    setPinMode(CFG_IO_PRE_CHARGE_RELAY);
    setPinMode(CFG_IO_MAIN_CONTACTOR);
    setPinMode(CFG_IO_SECONDAY_CONTACTOR);
    setPinMode(CFG_IO_FAST_CHARGE_CONTACTOR);

    setPinMode(CFG_IO_ENABLE_MOTOR);
    setPinMode(CFG_IO_ENABLE_CHARGER);
    setPinMode(CFG_IO_ENABLE_DCDC);
    setPinMode(CFG_IO_ENABLE_HEATER);

    setPinMode(CFG_IO_HEATER_VALVE);
    setPinMode(CFG_IO_HEATER_PUMP);
    setPinMode(CFG_IO_COOLING_PUMP);
    setPinMode(CFG_IO_COOLING_FAN);

    setPinMode(CFG_IO_BRAKE_LIGHT);
    setPinMode(CFG_IO_REVERSE_LIGHT);
    setPinMode(CFG_IO_WARNING);
    setPinMode(CFG_IO_POWER_LIMITATION);
    resetOutput(); // safety: to be 100% sure no relay pulls by accident, reset the output again

    ready = true;
    powerOn = true;

    canHandlerEv.attach(this, CAN_MASKED_ID, CAN_MASK, false);
    tickHandler.attach(this, CFG_TICK_INTERVAL_CAN_IO);
}

/**
 * Tear down the device in a safe way.
 */
void CanIO::tearDown()
{
    Device::tearDown();
    canHandlerEv.detach(this, CAN_MASKED_ID, CAN_MASK);

    resetOutput(); // safety: release all output signals
}

/**
 * process a tick event from the timer the device is registered to.
 */
void CanIO::handleTick()
{
    // safety: if CAN messages from GEVCU are missing, fault the system
    if (millis() > lastReception + CFG_CAN_IO_MSG_TIMEOUT) {
        Logger::error(CAN_IO, "too many lost messages !");
        status.setSystemState(Status::error);
    }
}

/*
 * check if a pin is configured and initialize it for digital output
 */
void CanIO::setPinMode(uint8_t pin)
{
    if (pin != CFG_OUTPUT_NONE) {
        pinMode(pin, OUTPUT);
    }
}

/**
 * check if a pin is configured and set it to HIGH or LOW depending on the input
 * note: if the output is logically active, it has to be set to LOW
 */
void CanIO::setOutput(uint8_t pin, bool active)
{
    if (pin != CFG_OUTPUT_NONE) {
        digitalWrite(pin, (active && status.getSystemState() != Status::error) ? LOW : HIGH);
    }
}

/**
 * deactivate all output signals
 */
void CanIO::resetOutput()
{
    setOutput(CFG_IO_PRE_CHARGE_RELAY, false);
    setOutput(CFG_IO_MAIN_CONTACTOR, false);
    setOutput(CFG_IO_SECONDAY_CONTACTOR, false);
    setOutput(CFG_IO_FAST_CHARGE_CONTACTOR, false);

    setOutput(CFG_IO_ENABLE_MOTOR, false);
    setOutput(CFG_IO_ENABLE_CHARGER, false);
    setOutput(CFG_IO_ENABLE_DCDC, false);
    setOutput(CFG_IO_ENABLE_HEATER, false);

    setOutput(CFG_IO_HEATER_VALVE, false);
    setOutput(CFG_IO_HEATER_PUMP, false);
    setOutput(CFG_IO_COOLING_PUMP, false);
    setOutput(CFG_IO_COOLING_FAN, false);

    setOutput(CFG_IO_BRAKE_LIGHT, false);
    setOutput(CFG_IO_REVERSE_LIGHT, false);
    setOutput(CFG_IO_WARNING, false);
    setOutput(CFG_IO_POWER_LIMITATION, false);
}

/*
 * Processes an event from the CanHandler.
 */
void CanIO::handleCanFrame(CAN_FRAME *frame)
{
    // safety: do not process any input when error occurred or device is not flagged
    // as powered on, this should prevent triggering any output
    if (status.getSystemState() == Status::error || !powerOn) {
        return;
    }

    switch (frame->id) {
    case CAN_ID_GEVCU_STATUS:
        processGevcuStatus(frame->data.bytes);
        running = true;
        lastReception = millis();
        break;

    case CAN_ID_GEVCU_ANALOG_IO:
        processGevcuAnalogIO(frame->data.bytes);
        break;
    }
}

/*
 * Process a status message which was received from the GEVCU
 * and set system state and I/O accordingly
 */
void CanIO::processGevcuStatus(uint8_t data[])
{
    // safety: set the system state, a error will cause a call of tearDown()
    // and prevent any activation of outputs.
    status.setSystemState((Status::SystemState) data[4]);

    uint16_t logicIO = (data[3] << 0) | (data[2] << 8);
    uint8_t status = data[5];

    setOutput(CFG_IO_PRE_CHARGE_RELAY, logicIO & preChargeRelay);
    setOutput(CFG_IO_MAIN_CONTACTOR, logicIO & mainContactor);
    setOutput(CFG_IO_SECONDAY_CONTACTOR, logicIO & secondaryContactor);
    setOutput(CFG_IO_FAST_CHARGE_CONTACTOR, logicIO & fastChargeContactor);

    setOutput(CFG_IO_ENABLE_MOTOR, logicIO & enableMotor);
    setOutput(CFG_IO_ENABLE_CHARGER, logicIO & enableCharger);
    setOutput(CFG_IO_ENABLE_DCDC, logicIO & enableDcDc);
    setOutput(CFG_IO_ENABLE_HEATER, logicIO & enableHeater);

    setOutput(CFG_IO_HEATER_VALVE, logicIO & heaterValve);
    setOutput(CFG_IO_HEATER_PUMP, logicIO & heaterPump);
    setOutput(CFG_IO_COOLING_PUMP, logicIO & coolingPump);
    setOutput(CFG_IO_COOLING_FAN, logicIO & coolingFan);

    setOutput(CFG_IO_BRAKE_LIGHT, logicIO & brakeLight);
    setOutput(CFG_IO_REVERSE_LIGHT, logicIO & reverseLight);
    setOutput(CFG_IO_WARNING, logicIO & warning);
    setOutput(CFG_IO_POWER_LIMITATION, logicIO & powerLimitation);

    if (Logger::isDebug()) {
        Logger::debug(CAN_IO, "pre-charge: %t, main cont: %t, sec cont: %t, fast charge: %t", logicIO & preChargeRelay, logicIO & mainContactor,
                logicIO & secondaryContactor, logicIO & fastChargeContactor);
        Logger::debug(CAN_IO, "enable motor: %t, enable charger: %t, enable DCDC: %t, enable heater: %t", logicIO & enableMotor,
                logicIO & enableCharger, logicIO & enableDcDc, logicIO & enableHeater);
        Logger::debug(CAN_IO, "heater valve: %t, heater pump: %t, cooling pump: %t, cooling fan: %t", logicIO & heaterValve, logicIO & heaterPump,
                logicIO & coolingPump, logicIO & coolingFan);
        Logger::debug(CAN_IO, "brake: %t, reverse: %t, warning: %t, power limit: %t", logicIO & brakeLight, logicIO & reverseLight, logicIO & warning,
                logicIO & powerLimitation);
    }
}

/*
 * process a status message which was received e.g. from the heater's temperature sensor.
 */
void CanIO::processGevcuAnalogIO(uint8_t data[])
{
    //TODO: implement processing of bits and bytes
}

DeviceType CanIO::getType()
{
    return DEVICE_IO;
}

DeviceId CanIO::getId()
{
    return CAN_IO;
}

/*
 * Load configuration data from EEPROM.
 *
 * If not available or the checksum is invalid, default values are chosen.
 */
void CanIO::loadConfiguration()
{
    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();

    if (!config) { // as lowest sub-class make sure we have a config object
        config = new CanIOConfiguration();
        setConfiguration(config);
    }

    Device::loadConfiguration(); // call parent

#ifdef USE_HARD_CODED

    if (false) {
#else
    if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
#endif
        uint8_t temp;
        prefsHandler->read(EECAN_xxx, &config->xxx);
        prefsHandler->read(EECAN_yyy, &temp);
        config->yyy = (temp != 0);
    } else { //checksum invalid, reinitialize values and store to EEPROM
        config->xxx = 0;
        config->yyy = false;
        saveConfiguration();
    }
    Logger::info(CAN_IO, "xxx: %d, yyy: %B", config->xxx, config->yyy);
}

/*
 * Store the current configuration parameters to EEPROM.
 */
void CanIO::saveConfiguration()
{
    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();

    Device::saveConfiguration(); // call parent

    prefsHandler->write(EECAN_xxx, config->xxx);
    prefsHandler->write(EECAN_yyy, (uint8_t) (config->yyy ? 1 : 0));
    prefsHandler->saveChecksum();
}
