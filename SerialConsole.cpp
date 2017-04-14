/*
 * SerialConsole.cpp
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

#include "SerialConsole.h"

SerialConsole serialConsole;

SerialConsole::SerialConsole()
{
    handlingEvent = false;
    ptrBuffer = 0;
    state = STATE_ROOT_MENU;
}

void SerialConsole::loop()
{
    if (handlingEvent == false) {
        if (SerialUSB.available()) {
            serialEvent();
        }
    }
}

void SerialConsole::printMenu()
{
    //Show build # here as well in case people are using the native port and don't get to see the start up messages
    Logger::console("\n%s", CFG_VERSION);
    Logger::console("System State: %s", status.systemStateToStr(status.getSystemState()));
    Logger::console("System Menu:\n");
    Logger::console("Enable line endings of some sort (LF, CR, CRLF)\n");
    Logger::console("Short Commands:");
    Logger::console("h = help (displays this message)");
    Logger::console("S = show list of devices");

    Logger::console("\nConfig Commands (enter command=newvalue)\n");
    Logger::console("LOGLEVEL=%d - set log level (0=debug, 1=info, 2=warn, 3=error, 4=off)", Logger::getLogLevel());

    deviceManager.printDeviceList();

    printMenuCanIO();
    printMenuHeater();
    printMenuFlowMeter();
}

void SerialConsole::printMenuCanIO()
{
    CanIO *canIO = (CanIO *) deviceManager.getDeviceByID(CAN_IO);

    if (canIO && canIO->getConfiguration()) {
        CanIOConfiguration *config = (CanIOConfiguration *) canIO->getConfiguration();

        Logger::console("\nCAN I/O\n");
        Logger::console("PRELAY=%d - Digital output to use for precharge contactor (255 to disable)", config->prechargeRelayOutput);
        Logger::console("MRELAY=%d - Digital output to use for main contactor (255 to disable)", config->mainContactorOutput);
        Logger::console("NRELAY=%d - Digital output to use for secondary contactor (255 to disable)", config->secondaryContactorOutput);
        Logger::console("FRELAY=%d - Digital output to use for fast charge contactor (255 to disable)\n", config->fastChargeContactorOutput);

        Logger::console("ENABLEM=%d - Digital output to use for enable motor signal (255 to disable)", config->enableMotorOutput);
        Logger::console("ENABLEC=%d - Digital output to use for enable charger signal (255 to disable)", config->enableChargerOutput);
        Logger::console("ENABLED=%d - Digital output to use for enable dc-dc converter signal (255 to disable)", config->enableDcDcOutput);
        Logger::console("ENABLEH=%d - Digital output to use for enable heater signal (255 to disable)\n", config->enableHeaterOutput);

        Logger::console("HEATVALV=%d - Digital output to actuate heater valve (255 to disable)", config->heaterValveOutput);
        Logger::console("HEATPUMP=%d - Digital output to turn on heater pump (255 to disable)", config->heaterPumpOutput);
        Logger::console("COOLPUMP=%d - Digital output to turn on cooling pump (255 to disable)", config->coolingPumpOutput);
        Logger::console("COOLFAN=%d - Digital output to turn on cooling fan (255 to disable)", config->coolingFanOutput);

        Logger::console("BRAKELT=%d - Digital output to use for brake light (255 to disable)", config->brakeLightOutput);
        Logger::console("REVLT=%d - Digital output to use for reverse light (255 to disable)", config->reverseLightOutput);
        Logger::console("PWRST=%d - Digital output to use for power steering (255 to disable)", config->powerSteeringOutput);
//        Logger::console("TBD=%d - Digital output to use for xxxx (255 to disable)", config->unusedOutput);
    }
}

void SerialConsole::printMenuHeater()
{
    EberspaecherHeater *heater = (EberspaecherHeater *) deviceManager.getDeviceByID(EBERSPAECHER);

    if (heater && heater->getConfiguration()) {
        EberspaecherHeaterConfiguration *config = (EberspaecherHeaterConfiguration *) heater->getConfiguration();

        Logger::console("\HEATER CONTROLS\n");
        Logger::console("HTMAXW=%d - Maximum power (0 - 6000 Watt)", config->maxPower);
        Logger::console("HTTEMP=%d - Desired water temperature (0 - 100 deg C)", config->targetTemperature);
        Logger::console(
                "HTDERT=%d - Temperature at which power will be derated from maxPower to 0%% at target temperature (0 - 100 deg C, 255 = ignore)",
                config->deratingTemperature);
        Logger::console("HTTON=%d - external temperature at which heater is turned on (0 - 40 deg C, 255 = ignore)", config->extTemperatureOn);
        Logger::console("HTTADDR=%#x,%#x,%#x,%#x,%#x,%#x,%#x,%#x, - address of external temperature sensor", config->extTemperatureSensorAddress[0],
                config->extTemperatureSensorAddress[1], config->extTemperatureSensorAddress[2], config->extTemperatureSensorAddress[3],
                config->extTemperatureSensorAddress[4], config->extTemperatureSensorAddress[5], config->extTemperatureSensorAddress[6],
                config->extTemperatureSensorAddress[7]);
    }
}

void SerialConsole::printMenuFlowMeter()
{
    FlowMeter *heating = (FlowMeter *) deviceManager.getDeviceByID(FLOW_METER_HEATER);
    FlowMeter *cooling = (FlowMeter *) deviceManager.getDeviceByID(FLOW_METER_COOLING);

    if (heating && heating->getConfiguration()) {
        FlowMeterConfiguration *config = (FlowMeterConfiguration *) heating->getConfiguration();

        Logger::console("\FLOW METER - HEATING CONTROLS\n");
        Logger::console("FMHCALIB=%d - Calibration Factor (pulses per liter)", config->calibrationFactor);
    }
    if (cooling && cooling->getConfiguration()) {
        FlowMeterConfiguration *config = (FlowMeterConfiguration *) cooling->getConfiguration();

        Logger::console("\FLOW METER - COOLING CONTROLS\n");
        Logger::console("FMCCALIB=%d - Calibration Factor (pulses per liter)", config->calibrationFactor);
    }
}

/*  There is a help menu (press H or h or ?)

 This is no longer going to be a simple single character console.
 Now the system can handle up to 80 input characters. Commands are submitted
 by sending line ending (LF, CR, or both)
 */
void SerialConsole::serialEvent()
{
    int incoming;
    incoming = SerialUSB.read();

    if (incoming == -1) { //false alarm....
        return;
    }

    if (incoming == 10 || incoming == 13) { //command done. Parse it.
        handleConsoleCmd();
        ptrBuffer = 0; //reset line counter once the line has been processed
    } else {
        cmdBuffer[ptrBuffer++] = (unsigned char) incoming;

        if (ptrBuffer > 79) {
            ptrBuffer = 79;
        }
    }
}

void SerialConsole::handleConsoleCmd()
{
    handlingEvent = true;

    if (state == STATE_ROOT_MENU) {
        if (ptrBuffer == 1) { //command is a single ascii character
            handleShortCmd();
        } else { //if cmd over 1 char then assume (for now) that it is a config line
            handleConfigCmd();
        }
    }

    handlingEvent = false;
}

/*For simplicity the configuration setting code uses four characters for each configuration choice. This makes things easier for
 comparison purposes.
 */
void SerialConsole::handleConfigCmd()
{
    int i;
    int value;
    bool updateWifi = true;

    //Logger::debug("Cmd size: %d", ptrBuffer);
    if (ptrBuffer < 6) {
        return;    //4 digit command, =, value is at least 6 characters
    }

    cmdBuffer[ptrBuffer] = 0; //make sure to null terminate
    String command = String();
    unsigned char whichEntry = '0';
    i = 0;

    while (cmdBuffer[i] != '=' && i < ptrBuffer) {
        /*if (cmdBuffer[i] >= '0' && cmdBuffer[i] <= '9') {
         whichEntry = cmdBuffer[i++] - '0';
         }
         else */command.concat(String(cmdBuffer[i++]));
    }

    i++; //skip the =

    if (i >= ptrBuffer) {
        Logger::console("Command needs a value..ie TORQ=3000\n");
        return; //or, we could use this to display the parameter instead of setting
    }

    // strtol() is able to parse also hex values (e.g. a string "0xCAFE"), useful for enable/disable by device id
    value = strtol((char *) (cmdBuffer + i), NULL, 0);
    command.toUpperCase();

    if (!handleConfigCmdCanIO(command, value) && !handleConfigCmdHeater(command, value) && !handleConfigCmdFlowMeter(command, value)
            && !handleConfigCmdSystem(command, value, (cmdBuffer + i))) {
        Logger::warn("unknown command: %s", command.c_str());
    }
}

bool SerialConsole::handleConfigCmdCanIO(String command, long value)
{
    CanIO *canIO = (CanIO *) deviceManager.getDeviceByID(CAN_IO);

    if (canIO && canIO->getConfiguration()) {
        CanIOConfiguration *config = (CanIOConfiguration *) canIO->getConfiguration();

        if (command == String("PRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting precharge relay to output %d", value);
            config->prechargeRelayOutput = value;
        } else if (command == String("MRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting main contactor to output %d", value);
            config->mainContactorOutput = value;
        } else if (command == String("NRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting secondary contactor to output %d", value);
            config->secondaryContactorOutput = value;
        } else if (command == String("FRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting fast charge contactor to output %d", value);
            config->fastChargeContactorOutput = value;
        } else if (command == String("ENABLEM")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable motor signal to output %d", value);
            config->enableMotorOutput = value;
        } else if (command == String("ENABLEC")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable charger signal to output %d", value);
            config->enableChargerOutput = value;
        } else if (command == String("ENABLED")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable DC-DC converter signal to output %d", value);
            config->enableDcDcOutput = value;
        } else if (command == String("ENABLEH")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable heater signal to output %d", value);
            config->enableHeaterOutput = value;
        } else if (command == String("HEATVALV")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting heater valve signal output %d", value);
            config->heaterValveOutput = value;
        } else if (command == String("HEATPUMP")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting heater pump signal to output %d", value);
            config->heaterPumpOutput = value;
        } else if (command == String("COOLPUMP")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting cooling pump signal to output %d", value);
            config->coolingPumpOutput = value;
        } else if (command == String("COOLFAN")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting cooling fan signal to output %d", value);
            config->coolingFanOutput = value;
        } else if (command == String("BRAKELT")) {
            value = constrain(value, 0, 255);
            Logger::console("Brake light signal set to output %d.", value);
            config->brakeLightOutput = value;
        } else if (command == String("REVLT")) {
            value = constrain(value, 0, 255);
            Logger::console("Reverse light signal set to output %d.", value);
            config->reverseLightOutput = value;
        } else if (command == String("PWRST")) {
            value = constrain(value, 0, 255);
            Logger::console("Power steering set to output %d.", value);
            config->powerSteeringOutput = value;
        } else if (command == String("TBD")) {
            value = constrain(value, 0, 255);
            Logger::console("xxxxx set to output %d.", value);
            config->unusedOutput = value;
        } else {
            return false;
        }
    } else {
        return false;
    }

    canIO->saveConfiguration();
    return true;
}

bool SerialConsole::handleConfigCmdHeater(String command, long value)
{
    EberspaecherHeater *heater = (EberspaecherHeater *) deviceManager.getDeviceByID(EBERSPAECHER);

    if (heater && heater->getConfiguration()) {
        EberspaecherHeaterConfiguration *config = (EberspaecherHeaterConfiguration *) heater->getConfiguration();

        if (command == String("HTMAXW")) {
            value = constrain(value, 0, 6000);
            Logger::console("Setting maximum power to %dW", value);
            config->maxPower = value;
        } else if (command == String("HTTEMP")) {
            value = constrain(value, 0, 100);
            Logger::console("Setting desired water temperature to %d deg C", value);
            config->targetTemperature = value;
        } else if (command == String("HTDERT")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting derating temperature to %d deg C", value);
            config->deratingTemperature = value;
        } else if (command == String("HTTON")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting external temp on to %d deg C", value);
            config->extTemperatureOn = value;
        } else if (command == String("HTTADDR")) {
            Logger::console("Setting temp sensor address");
            config->extTemperatureSensorAddress[0] = (value >> 7) & 0xff;
            config->extTemperatureSensorAddress[1] = (value >> 6) & 0xff;
            config->extTemperatureSensorAddress[2] = (value >> 5) & 0xff;
            config->extTemperatureSensorAddress[3] = (value >> 4) & 0xff;
            config->extTemperatureSensorAddress[4] = (value >> 3) & 0xff;
            config->extTemperatureSensorAddress[5] = (value >> 2) & 0xff;
            config->extTemperatureSensorAddress[6] = (value >> 1) & 0xff;
            config->extTemperatureSensorAddress[7] = (value >> 0) & 0xff;
        } else {
            return false;
        }
    } else {
        return false;
    }
    heater->saveConfiguration();
    return true;
}

bool SerialConsole::handleConfigCmdFlowMeter(String command, long value)
{
    FlowMeter *heating = (FlowMeter *) deviceManager.getDeviceByID(FLOW_METER_HEATER);
    FlowMeter *cooling = (FlowMeter *) deviceManager.getDeviceByID(FLOW_METER_COOLING);

    if (command == String("FMHCALIB") && heating && heating->getConfiguration()) {
        FlowMeterConfiguration *config = (FlowMeterConfiguration *) heating->getConfiguration();
        value = constrain(value, 1, 100000);
        Logger::console("Setting flow meter heating calibration factor to %d", value);
        config->calibrationFactor = value;
        heating->saveConfiguration();
    } else if (command == String("FMCCALIB") && cooling && cooling->getConfiguration()) {
        FlowMeterConfiguration *config = (FlowMeterConfiguration *) cooling->getConfiguration();
        value = constrain(value, 1, 100000);
        Logger::console("Setting flow meter cooling calibration factor to %d", value);
        config->calibrationFactor = value;
        cooling->saveConfiguration();
    } else {
        return false;
    }

    return true;
}

bool SerialConsole::handleConfigCmdSystem(String command, long value, char *parameter)
{

    if (command == String("ENABLE")) {
        if (!deviceManager.sendMessage(DEVICE_ANY, (DeviceId) value, MSG_ENABLE, NULL)) {
            Logger::console("Invalid device ID (%#x, %d)", value, value);
        }
    } else if (command == String("DISABLE")) {
        if (!deviceManager.sendMessage(DEVICE_ANY, (DeviceId) value, MSG_DISABLE, NULL)) {
            Logger::console("Invalid device ID (%#x, %d)", value, value);
        }
    } else if (command == String("LOGLEVEL")) {
        if (strchr(parameter, ',') == NULL) {
            Logger::setLoglevel((Logger::LogLevel) value);
            Logger::console("setting loglevel to %d", value);
        } else {
            DeviceId deviceId = (DeviceId) strtol(strtok(parameter, ","), NULL, 0);
            Device *device = deviceManager.getDeviceByID(deviceId);
            if (device != NULL) {
                value = atol(strtok(NULL, ","));
                Logger::setLoglevel(device, (Logger::LogLevel) value);
            }
        }
        //TODO save log level to eeprom !
    } else {
        return false;
    }
    return true;
}

void SerialConsole::handleShortCmd()
{
    uint8_t val;

    switch (cmdBuffer[0]) {
    case 'h':
    case '?':
    case 'H':
        printMenu();
        break;

    case 'S':
        deviceManager.printDeviceList();
        break;
    case 'c':
        Logger::console("state: %d, pre-charge: %d, main: %d, secondary: %d, fast chrg: %d, motor: %d, charger: %d, DCDC: %d",
                status.getSystemState(), status.preChargeRelay, status.mainContactor, status.secondaryContactor, status.fastChargeContactor,
                status.enableMotor, status.enableCharger, status.enableDcDc);
        Logger::console("heater: %d, valve: %d, heater pump: %d, cooling pump: %d, fan: %d, brake: %d, reverse: %d, power steer: %d, unused: %d",
                status.enableHeater, status.heaterValve, status.heaterPump, status.coolingPump, status.coolingFan, status.brakeLight,
                status.reverseLight, status.powerSteering, status.unused);
        break;
    }
}
