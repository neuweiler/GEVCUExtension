/*
 * DeviceManager.cpp
 *
 The device manager keeps a list of all devices which are installed into the system.
 Anything that needs either a tick handler, a canbus handler, or to communicate to other
 devices on the system must be registered with the manager. The manager then handles
 making sure the tick handler is set up (if needed), the canbus handler is set up (if need),
 and that a device can send information to other devices by either type (BMS, motor ctrl, etc)
 or by device ID.

 */

#include "DeviceManager.h"

DeviceManager::DeviceManager() {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        devices[i] = NULL;
    }
}

/*
 * Get the instance of the DeviceManager (singleton pattern)
 *
 * Note: It's a simple singleton implementation - no worries about
 * thread-safety and memory-leaks, this object lives as long as the
 * Arduino has power.
 */
DeviceManager *DeviceManager::getInstance() {
    static DeviceManager* deviceManager = new DeviceManager();
    return deviceManager;
}

/*
 * Add the specified device to the list of registered devices
 */
void DeviceManager::addDevice(Device *device) {
    Logger::info(device->getId(), "add device: %s", device->getCommonName());
    if (findDevice(device) == -1) {
        int8_t i = findDevice(NULL);

        if (i != -1) {
            devices[i] = device;
        } else {
            Logger::error("unable to register device, max number of devices reached.");
        }
    }
}

/*
 * Remove the specified device from the list of registered devices
 */
void DeviceManager::removeDevice(Device *device) {
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
void DeviceManager::sendMessage(DeviceType devType, DeviceId devId, uint32_t msgType, void* message) {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]) {
            if (devType == DEVICE_ANY || devType == devices[i]->getType()) {
                if (devId == INVALID || devId == devices[i]->getId()) {
                    Logger::debug("Sending msg %X to device with ID %X", msgType, devices[i]->getId());
                    devices[i]->handleMessage(msgType, message);
                }
            }
        }
    }
}

void DeviceManager::setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, char *value) {
    char *params[] = { key, value };
    sendMessage(deviceType, deviceId, msgType, params);
}

void DeviceManager::setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, uint32_t value) {
    char buffer[15];
    sprintf(buffer, "%lu", value);
    setParameter(deviceType, deviceId, msgType, key, buffer);
}

/*
 Allows one to request a reference to a device with the given ID. This lets code specifically request a certain
 device. Normally this would be a bad idea because it sort of breaks the OOP design philosophy of polymorphism
 but sometimes you can't help it.
 */
Device *DeviceManager::getDeviceByID(DeviceId id) {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]) {
            if (devices[i]->getId() == id) {
                return devices[i];
            }
        }
    }

    Logger::debug("getDeviceByID - No device with ID: %X", (int) id);
    return 0; //NULL!
}

/*
 The more object oriented version of the above function. Allows one to find the first device that matches
 a given type and that is enabled.
 */
Device *DeviceManager::getDeviceByType(DeviceType type) {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i] && devices[i]->getType() == type) {
            return devices[i];
        }
    }
    return 0; //NULL!
}

/*
 * Find the position of a device in the devices array
 * /retval the position of the device or -1 if not found.
 */
int8_t DeviceManager::findDevice(Device *device) {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (device == devices[i]) {
            return i;
        }
    }

    return -1;
}

/*
 * Count the number of registered devices of a certain type.
 */
uint8_t DeviceManager::countDeviceType(DeviceType deviceType) {
    uint8_t count = 0;

    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]->getType() == deviceType) {
            count++;
        }
    }

    return count;
}

void DeviceManager::printDeviceList() {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]) {
            Logger::console("     %X     %s", devices[i]->getId(), devices[i]->getCommonName());
        }
    }
}
