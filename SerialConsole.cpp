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
    Logger::console("\nBuild number: %i", CFG_VERSION);
    Logger::console("System State: %s", status.systemStateToStr(status.getSystemState()));
    Logger::console("System Menu:\n");
    Logger::console("Enable line endings of some sort (LF, CR, CRLF)\n");
    Logger::console("Short Commands:");
    Logger::console("h = help (displays this message)");
    Logger::console("S = show list of devices");

    Logger::console("\nConfig Commands (enter command=newvalue)\n");
    Logger::console("LOGLEVEL=%i - set log level (0=debug, 1=info, 2=warn, 3=error, 4=off)", Logger::getLogLevel());

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
        Logger::console("PRELAY=%i - Digital output to use for precharge contactor (255 to disable)", config->prechargeRelayOutput);
        Logger::console("MRELAY=%i - Digital output to use for main contactor (255 to disable)", config->mainContactorOutput);
        Logger::console("NRELAY=%i - Digital output to use for secondary contactor (255 to disable)", config->secondaryContactorOutput);
        Logger::console("FRELAY=%i - Digital output to use for fast charge contactor (255 to disable)\n", config->fastChargeContactorOutput);

        Logger::console("ENABLEM=%i - Digital output to use for enable motor signal (255 to disable)", config->enableMotorOutput);
        Logger::console("ENABLEC=%i - Digital output to use for enable charger signal (255 to disable)", config->enableChargerOutput);
        Logger::console("ENABLED=%i - Digital output to use for enable dc-dc converter signal (255 to disable)", config->enableDcDcOutput);
        Logger::console("ENABLEH=%i - Digital output to use for enable heater signal (255 to disable)\n", config->enableHeaterOutput);

        Logger::console("HEATVALV=%i - Digital output to actuate heater valve (255 to disable)", config->heaterValveOutput);
        Logger::console("HEATPUMP=%i - Digital output to turn on heater pump (255 to disable)", config->heaterPumpOutput);
        Logger::console("COOLPUMP=%i - Digital output to turn on cooling pump (255 to disable)", config->coolingPumpOutput);
        Logger::console("COOLFAN=%i - Digital output to turn on cooling fan (255 to disable)", config->coolingFanOutput);

        Logger::console("BRAKELT=%i - Digital output to use for brake light (255 to disable)", config->brakeLightOutput);
        Logger::console("REVLT=%i - Digital output to use for reverse light (255 to disable)", config->reverseLightOutput);
        Logger::console("PWRST=%i - Digital output to use for power steering (255 to disable)", config->powerSteeringOutput);
//        Logger::console("TBD=%i - Digital output to use for xxxx (255 to disable)", config->unusedOutput);
    }
}

void SerialConsole::printMenuHeater()
{
    EberspaecherHeater *heater = (EberspaecherHeater *) deviceManager.getDeviceByID(EBERSPAECHER);

    if (heater && heater->getConfiguration()) {
        EberspaecherHeaterConfiguration *config = (EberspaecherHeaterConfiguration *) heater->getConfiguration();

        Logger::console("\HEATER CONTROLS\n");
        Logger::console("HTMAXW=%i - Maximum power (0 - 6000 Watt)", config->maxPower);
        Logger::console("HTTEMP=%i - Desired water temperature (0 - 100 deg C)", config->targetTemperature);
        Logger::console(
                "HTDERT=%i - Temperature at which power will be derated from maxPower to 0% at target temperature (0 - 100 deg C, 255 = ignore)",
                config->deratingTemperature);
        Logger::console("HTTON=%i - external temperature at which heater is turned on (0 - 40 deg C, 255 = ignore)", config->extTemperatureOn);
        Logger::console("HTTADDR=%X,%X,%X,%X,%X,%X,%X,%X, - address of external temperature sensor", config->extTemperatureSensorAddress[0],
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
        Logger::console("FMHCALIB=%i - Calibration Factor (pulses per liter)", config->calibrationFactor);
    }
    if (cooling && cooling->getConfiguration()) {
        FlowMeterConfiguration *config = (FlowMeterConfiguration *) cooling->getConfiguration();

        Logger::console("\FLOW METER - COOLING CONTROLS\n");
        Logger::console("FMCCALIB=%i - Calibration Factor (pulses per liter)", config->calibrationFactor);
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

    //Logger::debug("Cmd size: %i", ptrBuffer);
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
            && !handleConfigCmdSystem(command, value)) {
        Logger::error("unknown command: %s", command.c_str());
    }
}

bool SerialConsole::handleConfigCmdCanIO(String command, long value)
{
    CanIO *canIO = (CanIO *) deviceManager.getDeviceByID(CAN_IO);

    if (canIO && canIO->getConfiguration()) {
        CanIOConfiguration *config = (CanIOConfiguration *) canIO->getConfiguration();

        if (command == String("PRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting precharge relay to output %i", value);
            config->prechargeRelayOutput = value;
        } else if (command == String("MRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting main contactor to output %i", value);
            config->mainContactorOutput = value;
        } else if (command == String("NRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting secondary contactor to output %i", value);
            config->secondaryContactorOutput = value;
        } else if (command == String("FRELAY")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting fast charge contactor to output %i", value);
            config->fastChargeContactorOutput = value;
        } else if (command == String("ENABLEM")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable motor signal to output %i", value);
            config->enableMotorOutput = value;
        } else if (command == String("ENABLEC")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable charger signal to output %i", value);
            config->enableChargerOutput = value;
        } else if (command == String("ENABLED")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable DC-DC converter signal to output %i", value);
            config->enableDcDcOutput = value;
        } else if (command == String("ENABLEH")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting enable heater signal to output %i", value);
            config->enableHeaterOutput = value;
        } else if (command == String("HEATVALV")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting heater valve signal output %i", value);
            config->heaterValveOutput = value;
        } else if (command == String("HEATPUMP")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting heater pump signal to output %i", value);
            config->heaterPumpOutput = value;
        } else if (command == String("COOLPUMP")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting cooling pump signal to output %i", value);
            config->coolingPumpOutput = value;
        } else if (command == String("COOLFAN")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting cooling fan signal to output %i", value);
            config->coolingFanOutput = value;
        } else if (command == String("BRAKELT")) {
            value = constrain(value, 0, 255);
            Logger::console("Brake light signal set to output %i.", value);
            config->brakeLightOutput = value;
        } else if (command == String("REVLT")) {
            value = constrain(value, 0, 255);
            Logger::console("Reverse light signal set to output %i.", value);
            config->reverseLightOutput = value;
        } else if (command == String("PWRST")) {
            value = constrain(value, 0, 255);
            Logger::console("Power steering set to output %i.", value);
            config->powerSteeringOutput = value;
        } else if (command == String("TBD")) {
            value = constrain(value, 0, 255);
            Logger::console("xxxxx set to output %i.", value);
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
            Logger::console("Setting maximum power to %iW", value);
            config->maxPower = value;
        } else if (command == String("HTTEMP")) {
            value = constrain(value, 0, 100);
            Logger::console("Setting desired water temperature to %i deg C", value);
            config->targetTemperature = value;
        } else if (command == String("HTDERT")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting derating temperature to %i deg C", value);
            config->deratingTemperature = value;
        } else if (command == String("HTTON")) {
            value = constrain(value, 0, 255);
            Logger::console("Setting external temp on to %i deg C", value);
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
        Logger::console("Setting flow meter heating calibration factor to %i", value);
        config->calibrationFactor = value;
        heating->saveConfiguration();
    } else if (command == String("FMCCALIB") && cooling && cooling->getConfiguration()) {
        FlowMeterConfiguration *config = (FlowMeterConfiguration *) cooling->getConfiguration();
        value = constrain(value, 1, 100000);
        Logger::console("Setting flow meter cooling calibration factor to %i", value);
        config->calibrationFactor = value;
        cooling->saveConfiguration();
    } else {
        return false;
    }

    return true;
}

bool SerialConsole::handleConfigCmdSystem(String command, long value)
{

    if (command == String("ENABLE")) {
        if (!deviceManager.sendMessage(DEVICE_ANY, (DeviceId) value, MSG_ENABLE, NULL)) {
            Logger::console("Invalid device ID (%X, %d)", value, value);
        }
    } else if (command == String("DISABLE")) {
        if (!deviceManager.sendMessage(DEVICE_ANY, (DeviceId) value, MSG_DISABLE, NULL)) {
            Logger::console("Invalid device ID (%X, %d)", value, value);
        }
    } else if (command == String("LOGLEVEL")) {
        switch (value) {
        case 0:
            Logger::setLoglevel(Logger::Debug);
            Logger::console("setting loglevel to 'debug'");
            break;

        case 1:
            Logger::setLoglevel(Logger::Info);
            Logger::console("setting loglevel to 'info'");
            break;

        case 2:
            Logger::console("setting loglevel to 'warning'");
            Logger::setLoglevel(Logger::Warn);
            break;

        case 3:
            Logger::console("setting loglevel to 'error'");
            Logger::setLoglevel(Logger::Error);
            break;

        case 4:
            Logger::console("setting loglevel to 'off'");
            Logger::setLoglevel(Logger::Off);
            break;
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
    }
}
