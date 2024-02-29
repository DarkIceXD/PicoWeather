#include "temperature_sensor.h"

void temperature_sensor_init()
{
    adc_set_temp_sensor_enabled(true);
}

float temperature_sensor_read()
{
    adc_select_input(4);
    const float conversionFactor = 3.3f / (1 << 12);
    const float adc = adc_read() * conversionFactor;
    return 27.0f - (adc - 0.706f) / 0.001721f;
}