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
CanIO::CanIO() :
        Device()
{
    prefsHandler = new PrefHandler(CAN_IO);
    lastReception = -1;
    commonName = "Can I/O";
}

/**
 * set-up the device
 */
void CanIO::setup()
{
    Device::setup(); //call base class

    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();
    resetOutput(); // safety: make sure no output is activated when setting the pin mode

    // initialize digital output
    setPinMode(config->preChargeRelayOutput);
    setPinMode(config->mainContactorOutput);
    setPinMode(config->secondayContactorOutput);
    setPinMode(config->fastChargeContactorOutput);

    setPinMode(config->enableMotorOutput);
    setPinMode(config->enableChargerOutput);
    setPinMode(config->enableDcDcOutput);
    setPinMode(config->enableHeaterOutput);

    setPinMode(config->heaterValveOutput);
    setPinMode(config->heaterPumpOutput);
    setPinMode(config->coolingPumpOutput);
    setPinMode(config->coolingFanOutput);

    setPinMode(config->brakeLightOutput);
    setPinMode(config->reverseLightOutput);
    setPinMode(config->warningOutput);
    setPinMode(config->powerLimitationOutput);
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
    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();

    setOutput(config->preChargeRelayOutput, false);
    setOutput(config->mainContactorOutput, false);
    setOutput(config->secondayContactorOutput, false);
    setOutput(config->fastChargeContactorOutput, false);

    setOutput(config->enableMotorOutput, false);
    setOutput(config->enableChargerOutput, false);
    setOutput(config->enableDcDcOutput, false);
    setOutput(config->enableHeaterOutput, false);

    setOutput(config->heaterValveOutput, false);
    setOutput(config->heaterPumpOutput, false);
    setOutput(config->coolingPumpOutput, false);
    setOutput(config->coolingFanOutput, false);

    setOutput(config->brakeLightOutput, false);
    setOutput(config->reverseLightOutput, false);
    setOutput(config->warningOutput, false);
    setOutput(config->powerLimitationOutput, false);
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
        processGevcuStatus(frame);
        running = true;
        lastReception = millis();
        break;

    case CAN_ID_GEVCU_ANALOG_IO:
        processGevcuAnalogIO(frame);
        break;
    }
}

/*
 * Process a status message which was received from the GEVCU
 * and set system state and I/O accordingly
 */
void CanIO::processGevcuStatus(CAN_FRAME *frame)
{
    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();

    // safety: set the system state, a error will cause a call of tearDown()
    // and prevent any activation of outputs.
    status.setSystemState((Status::SystemState) frame->data.byte[4]);

    uint16_t logicIO = frame->data.s1;

    setOutput(config->preChargeRelayOutput, logicIO & preChargeRelay);
    setOutput(config->mainContactorOutput, logicIO & mainContactor);
    setOutput(config->secondayContactorOutput, logicIO & secondaryContactor);
    setOutput(config->fastChargeContactorOutput, logicIO & fastChargeContactor);

    setOutput(config->enableMotorOutput, logicIO & enableMotor);
    setOutput(config->enableChargerOutput, logicIO & enableCharger);
    setOutput(config->enableDcDcOutput, logicIO & enableDcDc);
    setOutput(config->enableHeaterOutput, logicIO & enableHeater);

    setOutput(config->heaterValveOutput, logicIO & heaterValve);
    setOutput(config->heaterPumpOutput, logicIO & heaterPump);
    setOutput(config->coolingPumpOutput, logicIO & coolingPump);
    setOutput(config->coolingFanOutput, logicIO & coolingFan);

    setOutput(config->brakeLightOutput, logicIO & brakeLight);
    setOutput(config->reverseLightOutput, logicIO & reverseLight);
    setOutput(config->warningOutput, logicIO & warning);
    setOutput(config->powerLimitationOutput, logicIO & powerLimitation);

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
void CanIO::processGevcuAnalogIO(CAN_FRAME *frame)
{
    status.analogIn[0] = frame->data.s0;
    status.analogIn[1] = frame->data.s1;
    status.analogIn[2] = frame->data.s2;
    status.analogIn[3] = frame->data.s3;
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
        prefsHandler->read(EECAN_PRE_CHARGE_RELAY_OUTPUT, &config->preChargeRelayOutput);
        prefsHandler->read(EECAN_MAIN_CONTACTOR_OUTPUT, &config->mainContactorOutput);
        prefsHandler->read(EECAN_SECONDAY_CONTACTOR_OUTPUT, &config->secondayContactorOutput);
        prefsHandler->read(EECAN_FAST_CHARGE_CONTACTOR_OUTPUT, &config->fastChargeContactorOutput);

        prefsHandler->read(EECAN_ENABLE_MOTOR_OUTPUT, &config->enableMotorOutput);
        prefsHandler->read(EECAN_ENABLE_CHARGER_OUTPUT, &config->enableChargerOutput);
        prefsHandler->read(EECAN_ENABLE_DCDC_OUTPUT, &config->enableDcDcOutput);
        prefsHandler->read(EECAN_ENABLE_HEATER_OUTPUT, &config->enableHeaterOutput);

        prefsHandler->read(EECAN_HEATER_VALVE_OUTPUT, &config->heaterValveOutput);
        prefsHandler->read(EECAN_HEATER_PUMP_OUTPUT, &config->heaterPumpOutput);
        prefsHandler->read(EECAN_COOLING_PUMP_OUTPUT, &config->coolingPumpOutput);
        prefsHandler->read(EECAN_COOLING_FAN_OUTPUT, &config->coolingFanOutput);

        prefsHandler->read(EECAN_BRAKE_LIGHT_OUTPUT, &config->brakeLightOutput);
        prefsHandler->read(EECAN_REVERSE_LIGHT_OUTPUT, &config->reverseLightOutput);
        prefsHandler->read(EECAN_WARNING_OUTPUT, &config->warningOutput);
        prefsHandler->read(EECAN_POWER_LIMITATION_OUTPUT, &config->powerLimitationOutput);

        prefsHandler->read(EECAN_yyy, &temp);
        config->yyy = (temp != 0);
    } else { //checksum invalid, reinitialize values and store to EEPROM
        config->preChargeRelayOutput = 22;
        config->mainContactorOutput = 23;
        config->secondayContactorOutput = 24;
        config->fastChargeContactorOutput = 25;

        config->enableMotorOutput = 29;
        config->enableChargerOutput = 27;
        config->enableDcDcOutput = 28;
        config->enableHeaterOutput = 26;

        config->heaterValveOutput = 30;
        config->heaterPumpOutput = 31;
        config->coolingPumpOutput = 32;
        config->coolingFanOutput = 33;

        config->brakeLightOutput = 34;
        config->reverseLightOutput = 35;
        config->warningOutput = 36;
        config->powerLimitationOutput = 37;
        saveConfiguration();
    }
    Logger::info(CAN_IO, "preChargeRelay: %d, mainContactor: %d, secondaryContactor:%d, fastChargeContactor: %d", config->preChargeRelayOutput,
            config->mainContactorOutput, config->secondayContactorOutput, config->fastChargeContactorOutput);
    Logger::info(CAN_IO, "enableMotor: %d, enableCharger: %d, enableDcDc:%d, enableHeater: %d", config->enableMotorOutput,
            config->enableChargerOutput, config->enableDcDcOutput, config->enableHeaterOutput);
    Logger::info(CAN_IO, "heaterValve: %d, heaterPump: %d, coolingPump:%d, coolingFan: %d", config->heaterValveOutput, config->heaterPumpOutput,
            config->coolingPumpOutput, config->coolingFanOutput);
    Logger::info(CAN_IO, "brakeLight: %d, reverseLight: %d, warning:%d, powerLimitation: %d", config->brakeLightOutput, config->reverseLightOutput,
            config->warningOutput, config->powerLimitationOutput);
}

/*
 * Store the current configuration parameters to EEPROM.
 */
void CanIO::saveConfiguration()
{
    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();

    Device::saveConfiguration(); // call parent

    prefsHandler->write(EECAN_PRE_CHARGE_RELAY_OUTPUT, config->preChargeRelayOutput);
    prefsHandler->write(EECAN_MAIN_CONTACTOR_OUTPUT, config->mainContactorOutput);
    prefsHandler->write(EECAN_SECONDAY_CONTACTOR_OUTPUT, config->secondayContactorOutput);
    prefsHandler->write(EECAN_FAST_CHARGE_CONTACTOR_OUTPUT, config->fastChargeContactorOutput);

    prefsHandler->write(EECAN_ENABLE_MOTOR_OUTPUT, config->enableMotorOutput);
    prefsHandler->write(EECAN_ENABLE_CHARGER_OUTPUT, config->enableChargerOutput);
    prefsHandler->write(EECAN_ENABLE_DCDC_OUTPUT, config->enableDcDcOutput);
    prefsHandler->write(EECAN_ENABLE_HEATER_OUTPUT, config->enableHeaterOutput);

    prefsHandler->write(EECAN_HEATER_VALVE_OUTPUT, config->heaterValveOutput);
    prefsHandler->write(EECAN_HEATER_PUMP_OUTPUT, config->heaterPumpOutput);
    prefsHandler->write(EECAN_COOLING_PUMP_OUTPUT, config->coolingPumpOutput);
    prefsHandler->write(EECAN_COOLING_FAN_OUTPUT, config->coolingFanOutput);

    prefsHandler->write(EECAN_BRAKE_LIGHT_OUTPUT, config->brakeLightOutput);
    prefsHandler->write(EECAN_REVERSE_LIGHT_OUTPUT, config->reverseLightOutput);
    prefsHandler->write(EECAN_WARNING_OUTPUT, config->warningOutput);
    prefsHandler->write(EECAN_POWER_LIMITATION_OUTPUT, config->powerLimitationOutput);

    prefsHandler->write(EECAN_yyy, (uint8_t) (config->yyy ? 1 : 0));
    prefsHandler->saveChecksum();
}
