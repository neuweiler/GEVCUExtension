/*
 * DeviceManager.h
 *
*/

#ifndef DEVICEMGR_H_
#define DEVICEMGR_H_

#include "config.h"
#include "Logger.h"
#include "Device.h"
#include "Sys_Messages.h"
#include "DeviceTypes.h"

//class Device;

class DeviceManager
{
public:
    static DeviceManager *getInstance();
    void addDevice(Device *device);
    void removeDevice(Device *device);
    void sendMessage(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, void* message);
    void setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, char *value);
    void setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, uint32_t value);
    Device *getDeviceByID(DeviceId);
    Device *getDeviceByType(DeviceType);
    void printDeviceList();

protected:

private:
    Device *devices[CFG_DEV_MGR_MAX_DEVICES];

    DeviceManager();    // private constructor
    int8_t findDevice(Device *device);
    uint8_t countDeviceType(DeviceType deviceType);
};

#endif
