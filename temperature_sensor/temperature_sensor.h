#ifndef _TEMPERATURE_SENSOR_H_
#define _TEMPERATURE_SENSOR_H_
#include "hardware/adc.h"

void temperature_sensor_init();
float temperature_sensor_read();

#endif