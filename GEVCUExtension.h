/*
 * GEVCUExtension.h
 *
 */

#ifndef GEVCUEXTENSION_H_
#define GEVCUEXTENSION_H_

#include <Arduino.h>
#include "DeviceManager.h"
#include "CanHandler.h"
#include "TickHandler.h"
#include "Heartbeat.h"
#include "Temperature.h"

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GEVCUEXTENSION_H_ */
