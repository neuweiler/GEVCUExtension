/*
 * TemperatureSensor.h
 *
 */

#ifndef TEMPERATURESENSOR_H_
#define TEMPERATURESENSOR_H_

#include "config.h"
#include <OneWire.h>

class TemperatureSensor
{
public:
    enum DeviceType
    {
        UNKNOWN,
        DS18S20,
        DS18B20,
        DS1822
    };

    TemperatureSensor(byte address[]);
    static void prepareData();
    static void resetSearch();
    static TemperatureSensor *search();
    DeviceType getType();
    char *getTypeStr();
    byte *getAddress();
    void setResolution(byte resolution);
    void retrieveData();
    float getTemperatureCelsius();
    float getTemperatureFahrenheit();
protected:

private:
    byte address[8];
    DeviceType type;
    int16_t temperature; // integer representation of temperature
};

#endif /* TEMPERATURESENSOR_H_ */
