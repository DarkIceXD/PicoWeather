#ifndef _MIN_MAX_H_
#define _MIN_MAX_H_
#include <pico/stdlib.h>

float min_float(const float a, const float b);
float min_float_array(const float *data, const size_t object_size, const size_t array_size);
float max_float(const float a, const float b);
float max_float_array(const float *data, const size_t object_size, const size_t array_size);
int min_int(const int a, const int b);
int min_int_array(const int *data, const size_t object_size, const size_t array_size);
int max_int(const int a, const int b);
int max_int_array(const int *data, const size_t object_size, const size_t array_size);

#endif