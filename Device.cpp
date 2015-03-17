/*
 * Device.cpp
 *
 */

#include "Device.h"
#include "DeviceManager.h"

Device::Device()
{
    deviceManager = DeviceManager::getInstance();
//    status = Status::getInstance();
//    systemIO = SystemIO::getInstance();

    //since all derived classes eventually call this base method this will cause every device to auto register itself with the device manager
    deviceManager->addDevice(this);
    commonName = "Generic Device";
}

//Empty functions to handle these callbacks if the derived classes don't

void Device::setup()
{
}

char* Device::getCommonName()
{
    return commonName;
}

void Device::handleTick()
{
}

uint32_t Device::getTickInterval()
{
    return 0;
}

void Device::handleMessage(uint32_t msgType, void* message)
{
    switch (msgType) {
        case MSG_STARTUP:
            this->setup();
            break;
    }
}

DeviceType Device::getType()
{
    return DEVICE_NONE;
}

DeviceId Device::getId()
{
    return INVALID;
}
