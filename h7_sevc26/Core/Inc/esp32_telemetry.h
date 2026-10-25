/*
 * esp32_telemetry.h
 *
 *  Created on: 23-Oct-2026
 *      Author: khunt
 */

#ifndef INC_ESP32_TELEMETRY_H_
#define INC_ESP32_TELEMETRY_H_

#include "main.h"

void Telemetry_Init(void);
void Telemetry_PollTransmit(void);

#endif /* INC_ESP32_TELEMETRY_H_ */
