/*
 * DeviceManager.cpp
 *
 The device manager keeps a list of all devices which are installed into the system.
 Anything that needs either a tick handler, a canbus handler, or to communicate to other
 devices on the system must be registered with the manager. The manager then handles
 making sure the tick handler is set up (if needed), the canbus handler is set up (if need),
 and that a device can send information to other devices by either type (BMS, motor ctrl, etc)
 or by device ID.

 The device class itself has the handlers defined so the tick and canbus handling code
 need only call these existing functions but the manager interface needs to
 expose a way to register them with the system.

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

#include "DeviceManager.h"

DeviceManager deviceManager;

DeviceManager::DeviceManager()
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        devices[i] = NULL;
    }
}

/*
 * Add the specified device to the list of registered devices
 */
void DeviceManager::addDevice(Device *device)
{
    Logger::info(device, "add device: %s (id: %#x)", device->getCommonName(), device->getId());

    if (findDevice(device) == -1) {
        int8_t i = findDevice(NULL);

        if (i != -1) {
            devices[i] = device;
        } else {
            Logger::error(device, "unable to register device, max number of devices reached.");
        }
    }
}

/*
 * Remove the specified device from the list of registered devices
 */
void DeviceManager::removeDevice(Device *device)
{
    int8_t i = findDevice(NULL);

    if (i != -1) {
        devices[i] = NULL;
    }
}

/*
 Send an inter-device message. Devtype has to be filled out but could be DEVICE_ANY.
 If devId is anything other than INVALID (0xFFFF) then the message will be targetted to only
 one device. Otherwise it will broadcast to any device that matches the device type (or all
 devices in the case of DEVICE_ANY).
 DeviceManager.h has a list of standard message types but you're allowed to send
 whatever you want. The standard message types are to enforce standard messages for easy
 intercommunication.
 */
bool DeviceManager::sendMessage(DeviceType devType, DeviceId devId, uint32_t msgType, void* message)
{
    bool foundDevice = false;
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i] && (devices[i]->isEnabled() || msgType == MSG_ENABLE)) { //does this object exist and is it enabled?
            if (devType == DEVICE_ANY || devType == devices[i]->getType()) {
                if (devId == INVALID || devId == devices[i]->getId()) {
                    if (Logger::isDebug()) {
                        Logger::debug("Sending msg %#x to device %#x", msgType, devices[i]->getId());
                    }
                    devices[i]->handleMessage(msgType, message);
                    foundDevice = true;
                }
            }
        }
    }
    return foundDevice;
}

void DeviceManager::setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, char *value)
{
    char *params[] = { key, value };
    sendMessage(deviceType, deviceId, msgType, params);
}

void DeviceManager::setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, uint32_t value)
{
    char buffer[15];
    sprintf(buffer, "%lu", value);
    setParameter(deviceType, deviceId, msgType, key, buffer);
}

/*
Allows one to request a reference to a device with the given ID. This lets code specifically request a certain
device. Normally this would be a bad idea because it sort of breaks the OOP design philosophy of polymorphism
but sometimes you can't help it.
*/
Device *DeviceManager::getDeviceByID(DeviceId id)
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]) {
            if (devices[i]->getId() == id) {
                return devices[i];
            }
        }
    }

    Logger::debug("getDeviceByID - No device with ID: %#x", (int) id);
    return NULL;
}

/*
The more object oriented version of the above function. Allows one to find the first device that matches
a given type and that is enabled.
*/
Device *DeviceManager::getDeviceByType(DeviceType type)
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]) {
            if (devices[i]->getType() == type && devices[i]->isEnabled()) {
                return devices[i];
            }
        }
    }
    return NULL;
}

/*
 * Find the position of a device in the devices array
 * /retval the position of the device or -1 if not found.
 */
int8_t DeviceManager::findDevice(Device *device)
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (device == devices[i]) {
            return i;
        }
    }

    return -1;
}

void DeviceManager::printDeviceList()
{
    Logger::console("Currently enabled devices: (DISABLE= to disable)");

    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i] && devices[i]->isEnabled()) {
            Logger::console("     %#x     %s", devices[i]->getId(), devices[i]->getCommonName());
        }
    }

    Logger::console("Currently disabled devices: (ENABLE= to enable)");

    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i] && !devices[i]->isEnabled()) {
            Logger::console("     %#x     %s", devices[i]->getId(), devices[i]->getCommonName());
        }
    }
}
