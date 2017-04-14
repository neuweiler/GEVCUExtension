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
    lastReception = 0xffffff;
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
    setPinMode(config->prechargeRelayOutput);
    setPinMode(config->mainContactorOutput);
    setPinMode(config->secondaryContactorOutput);
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
    setPinMode(config->powerSteeringOutput);
    setPinMode(config->unusedOutput);
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
        Logger::error(this, "too many lost messages !");
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
void CanIO::setOutput(uint8_t pin, bool active, bool *flag)
{
    *flag = active;
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

    setOutput(config->prechargeRelayOutput, false, &status.preChargeRelay);
    setOutput(config->mainContactorOutput, false, &status.mainContactor);
    setOutput(config->secondaryContactorOutput, false, &status.secondaryContactor);
    setOutput(config->fastChargeContactorOutput, false, &status.fastChargeContactor);

    setOutput(config->enableMotorOutput, false, &status.enableMotor);
    setOutput(config->enableChargerOutput, false, &status.enableCharger);
    setOutput(config->enableDcDcOutput, false, &status.enableDcDc);
    setOutput(config->enableHeaterOutput, false, &status.enableHeater);

    setOutput(config->heaterValveOutput, false, &status.heaterValve);
    setOutput(config->heaterPumpOutput, false, &status.heaterPump);
    setOutput(config->coolingPumpOutput, false, &status.coolingPump);
    setOutput(config->coolingFanOutput, false, &status.coolingFan);

    setOutput(config->brakeLightOutput, false, &status.brakeLight);
    setOutput(config->reverseLightOutput, false, &status.reverseLight);
    setOutput(config->powerSteeringOutput, false, &status.powerSteering);
    setOutput(config->unusedOutput, false, &status.unused);
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

    // safety: set the system state, a error will cause a call of tearDown() and prevent any activation of outputs.
    if (status.setSystemState((Status::SystemState) frame->data.byte[4]) == Status::error) {
        return;
    }

    uint16_t logicIO = frame->data.s1;

    if ((logicIO & preChargeRelay) && status.getSystemState() != Status::preCharge) {
        Logger::error(this, "Pre-Charge relay set in wrong system state %d", status.getSystemState());
        status.setSystemState(Status::error);
        return;
    }

    if ((logicIO & mainContactor) && !status.mainContactor && status.getSystemState() != Status::preCharge) {
        Logger::error(this, "Trying to enable main contactor in system state: %d", status.getSystemState());
        status.setSystemState(Status::error);
        return;
    }

    setOutput(config->prechargeRelayOutput, logicIO & preChargeRelay, &status.preChargeRelay);
    setOutput(config->mainContactorOutput, logicIO & mainContactor, &status.mainContactor);
    setOutput(config->secondaryContactorOutput, logicIO & secondaryContactor, &status.secondaryContactor);
    setOutput(config->fastChargeContactorOutput, logicIO & fastChargeContactor, &status.fastChargeContactor);

    setOutput(config->enableMotorOutput, logicIO & enableMotor, &status.enableMotor);
    setOutput(config->enableChargerOutput, logicIO & enableCharger, &status.enableCharger);
    setOutput(config->enableDcDcOutput, logicIO & enableDcDc, &status.enableDcDc);
    setOutput(config->enableHeaterOutput, logicIO & enableHeater, &status.enableHeater);

    setOutput(config->heaterValveOutput, logicIO & heaterValve, &status.heaterValve);
    setOutput(config->heaterPumpOutput, logicIO & heaterPump, &status.heaterPump);
    setOutput(config->coolingPumpOutput, logicIO & coolingPump, &status.coolingPump);
    setOutput(config->coolingFanOutput, logicIO & coolingFan, &status.coolingFan);

    setOutput(config->brakeLightOutput, logicIO & brakeLight, &status.brakeLight);
    setOutput(config->reverseLightOutput, logicIO & reverseLight, &status.reverseLight);
    setOutput(config->powerSteeringOutput, logicIO & powerSteering, &status.powerSteering);
    setOutput(config->unusedOutput, logicIO & unused, &status.unused);

    if (Logger::isDebug()) {
        Logger::debug(this, "state: %d, pre-charge: %d, main: %d, secondary: %d, fast chrg: %d, motor: %d, charger: %d, DCDC: %d",
                frame->data.byte[4], status.preChargeRelay, status.mainContactor, status.secondaryContactor, status.fastChargeContactor,
                status.enableMotor, status.enableCharger, status.enableDcDc);
        Logger::debug(this, "heater: %d, valve: %d, heater pump: %d, cooling pump: %d, fan: %d, brake: %d, reverse: %d, power steer: %d, unused: %d",
                status.enableHeater, status.heaterValve, status.heaterPump, status.coolingPump, status.coolingFan, status.brakeLight,
                status.reverseLight, status.powerSteering, status.unused);
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
        prefsHandler->read(EECAN_PRE_CHARGE_RELAY_OUTPUT, &config->prechargeRelayOutput);
        prefsHandler->read(EECAN_MAIN_CONTACTOR_OUTPUT, &config->mainContactorOutput);
        prefsHandler->read(EECAN_SECONDAY_CONTACTOR_OUTPUT, &config->secondaryContactorOutput);
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
        prefsHandler->read(EECAN_WARNING_OUTPUT, &config->powerSteeringOutput);
        prefsHandler->read(EECAN_POWER_LIMITATION_OUTPUT, &config->unusedOutput);
    } else { //checksum invalid, reinitialize values and store to EEPROM
        config->prechargeRelayOutput = 22;
        config->mainContactorOutput = 23;
        config->secondaryContactorOutput = 24;
        config->fastChargeContactorOutput = 25;

        config->enableMotorOutput = 29;
        config->enableChargerOutput = 27;
        config->enableDcDcOutput = 28;
        config->enableHeaterOutput = 37;

        config->heaterValveOutput = 30;
        config->heaterPumpOutput = 31;
        config->coolingPumpOutput = 32;
        config->coolingFanOutput = 33;

        config->brakeLightOutput = 34;
        config->reverseLightOutput = 35;
        config->powerSteeringOutput = 36;
        config->unusedOutput = 26;
        saveConfiguration();
    }
    Logger::info(this, "prechargeRelay: %d, mainContactor: %d, secondaryContactor:%d, fastChargeContactor: %d", config->prechargeRelayOutput,
            config->mainContactorOutput, config->secondaryContactorOutput, config->fastChargeContactorOutput);
    Logger::info(this, "enableMotor: %d, enableCharger: %d, enableDcDc:%d, enableHeater: %d", config->enableMotorOutput, config->enableChargerOutput,
            config->enableDcDcOutput, config->enableHeaterOutput);
    Logger::info(this, "heaterValve: %d, heaterPump: %d, coolingPump:%d, coolingFan: %d", config->heaterValveOutput, config->heaterPumpOutput,
            config->coolingPumpOutput, config->coolingFanOutput);
    Logger::info(this, "brakeLight: %d, reverseLight: %d, powerSteering:%d, unused: %d", config->brakeLightOutput, config->reverseLightOutput,
            config->powerSteeringOutput, config->unusedOutput);
}

/*
 * Store the current configuration parameters to EEPROM.
 */
void CanIO::saveConfiguration()
{
    CanIOConfiguration *config = (CanIOConfiguration *) getConfiguration();

    Device::saveConfiguration(); // call parent

    prefsHandler->write(EECAN_PRE_CHARGE_RELAY_OUTPUT, config->prechargeRelayOutput);
    prefsHandler->write(EECAN_MAIN_CONTACTOR_OUTPUT, config->mainContactorOutput);
    prefsHandler->write(EECAN_SECONDAY_CONTACTOR_OUTPUT, config->secondaryContactorOutput);
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
    prefsHandler->write(EECAN_WARNING_OUTPUT, config->powerSteeringOutput);
    prefsHandler->write(EECAN_POWER_LIMITATION_OUTPUT, config->unusedOutput);

    prefsHandler->saveChecksum();
}
