/*
 * TemperatureSensor.cpp
 *
 */

#include "TemperatureSensor.h"

static OneWire ds(CFG_IO_TEMPERATURE_SENSOR); // DS18B20 Temperature chip i/o

/**
 * Constructor
 */
TemperatureSensor::TemperatureSensor(byte addr[]) {
	memcpy(address, addr, 8); // copy over the contents of the source array
	temperature = 0.0;

	if (address[0] == 0x10) {
		type = DS18S20;
	} else if (address[0] == 0x28) {
		type = DS18B20;
	} else if (address[0] == 0x22) {
		type = DS1822;
	} else {
		type = UNKNOWN;
	}
}

/**
 * Get the type of the device
 */
TemperatureSensor::DeviceType TemperatureSensor::getType() {
	return type;
}

/**
 * Get the type of the device as string
 */
char *TemperatureSensor::getTypeStr() {
	switch (type) {
	case TemperatureSensor::DS18S20:
		return "DS18S20";
		break;
	case TemperatureSensor::DS18B20:
		return "DS18B20";
		break;
	case TemperatureSensor::DS1822:
		return "DS1822";
		break;
	default:
		return "unknown";
		break;
	}
}

/**
 * Get the address of the device
 */
byte *TemperatureSensor::getAddress() {
	return address;
}

/**
 * Set the resolution of the DS18B20 between 9 or 12 bits.
 */
void TemperatureSensor::setResolution(byte resolution) {
	if (resolution > 12 || resolution < 9)
		return;
	if (type != DS18B20)
		return;

	// Get byte for desired resolution
	byte resolutionByte = 0x1F; // 9 bit
	if (resolution == 12) {
		resolutionByte = 0x7F;
	} else if (resolution == 11) {
		resolutionByte = 0x5F;
	} else if (resolution == 10) {
		resolutionByte = 0x3F;
	}

	// set configuration
	ds.reset();
	ds.select(address);
	ds.write(0x4E);			// write scratchpad
	ds.write(0);				// TL
	ds.write(0);				// TH
	ds.write(resolutionByte);	// configuration register
	ds.write(0x48);			// copy scratchpad
}

/**
 * Order all temperature sensors to prepare data.
 */
void TemperatureSensor::prepareData() {
	ds.reset();
	ds.skip(); // skip ROM - send to all devices
	ds.write(0x44); // start conversion
}

/**
 * Retrieve prepared data from temperature sensors
 */
void TemperatureSensor::retrieveData() {
	byte data[9];

	ds.reset();
	ds.select(address);
	ds.write(0xBE); // read scratchpad
	ds.read_bytes(data, 9); // 9 bytes are required

	temperature = (data[1] << 8) | data[0];

	if (type == DS18S20) {
		temperature = temperature << 3; // 9 bit resolution default
		if (data[7] == 0x10) { // "count remain" gives full 12 bit resolution
			temperature = (temperature & 0xFFF0) + 12 - data[6];
		}
	} else {
		byte cfg = (data[4] & 0x60);
		// at lower res, the low bits are undefined, so let's zero them
		if (cfg == 0x00)
			temperature = temperature & ~7; // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20)
			temperature = temperature & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40)
			temperature = temperature & ~1; // 11 bit res, 375 ms
	}
}

/**
 * Return the measured temperature in degree celsius
 */
float TemperatureSensor::getTemperatureCelsius() {
	return (float) temperature / 16.0;
}

/**
 * Return the measured temperature in degree fahrenheit
 */
float TemperatureSensor::getTemperatureFahrenheit() {
	return (float) temperature / 16.0 * 1.8 + 32.0;
}

/**
 * A static wrapper for the OneWire::reset_search()
 */
void TemperatureSensor::resetSearch() {
	ds.reset_search();
}

/**
 * A static function to search for more devices and instanciate an object
 * for each found device.
 */
TemperatureSensor *TemperatureSensor::search() {
	byte addr[8];

	if (ds.search(addr)) {
		if (OneWire::crc8(addr, 7) != addr[7]) {
			Serial.print("invalid CRC!\n");
			return NULL;
		}
		return new TemperatureSensor(addr);
	}

	return NULL;
}
