/*
 * CanIO.c
 *
 */

#include "CanIO.h"

/**
 * Constructor to initialize class variables
 */
CanIO::CanIO() : Device() {
    lastReception = -1;
    faulted = false;
    passedPreCharging = false;
    canHandlerEv = CanHandler::getInstanceEV();
    commonName = "Can I/O";
}

/**
 * set-up the device
 */
void CanIO::setup() {
    TickHandler::getInstance()->detach(this);
    Device::setup(); //call base class

    resetOutput();
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
    resetOutput();

    // register ourselves as observer of can frames
    canHandlerEv->attach(this, CAN_MASKED_ID, CAN_MASK, false);

    TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_CAN_IO);
}

/**
 * process a tick event from the timer the device is registered to.
 */
void CanIO::handleTick() {
    // if CAN messages from GEVCU are missing, fault this device and disable everything
    if (millis() > lastReception + CFG_CAN_IO_MSG_TIMEOUT) {
        // in case the system's milli counter overflows, adapt to much lower values
        if (millis() < 3000) {
            lastReception = millis();
            return;
        }
        Logger::error(CAN_IO, "too many lost messages !");
        fault();
    }
}

/**
 * In case of an error, fault the device, reset all output and stop processing further I/O data via can
 */
void CanIO::fault() {
    Logger::error(CAN_IO, "Faulting and resetting all output. Reset device to restart.");

    faulted = true;
    resetOutput();
    canHandlerEv->detach(this, CAN_MASKED_ID, CAN_MASK);
    TickHandler::getInstance()->detach(this);
}

/*
 * check if a pin is configured and initialize it for digital output
 */
void CanIO::setPinMode(uint8_t pin) {
    if (pin != CFG_OUTPUT_NONE) {
        pinMode(pin, OUTPUT);
    }
}

/**
 * check if a pin is configured and set it to HIGH or LOW depending on the input
 * note: if the output is logically active, it has to be set to LOW
 */
void CanIO::setOutput(uint8_t pin, bool active) {
    if (pin != CFG_OUTPUT_NONE) {
        digitalWrite(pin, (active && !faulted) ? LOW : HIGH);
    }
}

/**
 * deactivate all output signals
 */
void CanIO::resetOutput() {
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
 *
 * In case a CAN message was received which pass the masking and id filtering,
 * this method is called. Depending on the ID of the CAN message, the data of
 * the incoming message is processed.
 */
void CanIO::handleCanFrame(CAN_FRAME *frame) {
    if (faulted) {
        return;
    }

    switch (frame->id) {
    case CAN_ID_GEVCU_STATUS:
        processGevcuStatus(frame->data.bytes);
        lastReception = millis();
        break;

    case CAN_ID_GEVCU_ANALOG_IO:
        processGevcuAnalogIO(frame->data.bytes);
        break;
    }
}

/*
 * Process a status message which was received from the GEVCU
 * and set I/O accordingly
 */
void CanIO::processGevcuStatus(uint8_t data[]) {
    uint8_t state = data[4];
    switch (state) {
    case startup:
        Logger::info(CAN_IO, "state: startup");
        break;
    case init:
        Logger::info(CAN_IO, "state: init");
        break;
    case preCharge:
        Logger::info(CAN_IO, "state: preCharge");
        passedPreCharging = true;
        break;
    case preCharged:
        Logger::info(CAN_IO, "state: preCharged");
        break;
    case batteryHeating:
        Logger::info(CAN_IO, "state: batteryHeating");
        break;
    case charging:
        Logger::info(CAN_IO, "state: charging");
        break;
    case charged:
        Logger::info(CAN_IO, "state: charged");
        break;
    case ready:
        Logger::info(CAN_IO, "state: ready");
        break;
    case running:
        Logger::info(CAN_IO, "state: running");
        break;
    case error:
        Logger::error(CAN_IO, "state: error");
        fault();
        break;
    }

    // make sure that a power cycle or reset of the extension only does not skip the pre-charge cycle
    // this would be potentially dangerous, so this device will fault
    if (!passedPreCharging && (state > preCharge) && !faulted) {
    	Logger::error(CAN_IO, "GEVCU reports its status is higher than 'preCharge' but extension did not pass pre-charge cycle");
    	fault();
    	return;
    }

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
        Logger::debug(CAN_IO, "pre-charge: %t, main cont: %t, sec cont: %t, fast charge: %t", logicIO & preChargeRelay, logicIO & mainContactor, logicIO & secondaryContactor, logicIO & fastChargeContactor);
        Logger::debug(CAN_IO, "enable motor: %t, enable charger: %t, enable DCDC: %t, enable heater: %t", logicIO & enableMotor, logicIO & enableCharger, logicIO & enableDcDc, logicIO & enableHeater);
        Logger::debug(CAN_IO, "heater valve: %t, heater pump: %t, cooling pump: %t, cooling fan: %t", logicIO & heaterValve, logicIO & heaterPump, logicIO & coolingPump, logicIO & coolingFan);
        Logger::debug(CAN_IO, "brake: %t, reverse: %t, warning: %t, power limit: %t", logicIO & brakeLight, logicIO & reverseLight, logicIO & warning, logicIO & powerLimitation);
    }
}

/*
 * process a status message which was received from the heater.
 */
void CanIO::processGevcuAnalogIO(uint8_t data[]) {
    //TODO: implement processing of bits and bytes
}

DeviceType CanIO::getType() {
    return DEVICE_IO;
}

DeviceId CanIO::getId() {
    return CAN_IO;
}
