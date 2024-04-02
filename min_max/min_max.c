#include "min_max.h"

float min_float(const float a, const float b)
{
    return a < b ? a : b;
}

float min_float_array(const float *data, const size_t object_size, const size_t array_size)
{
    float min = *data;
    for (size_t i = 1; i < array_size; i++)
    {
        const float value = *((float *)((uint8_t *)data + object_size * i));
        if (min > value)
            min = value;
    }
    return min;
}

float max_float(const float a, const float b)
{
    return a > b ? a : b;
}

float max_float_array(const float *data, const size_t object_size, const size_t array_size)
{
    float max = *data;
    for (size_t i = 1; i < array_size; i++)
    {
        const float value = *((float *)((uint8_t *)data + object_size * i));
        if (max < value)
            max = value;
    }
    return max;
}

int min_int(const int a, const int b)
{
    return a < b ? a : b;
}

int min_int_array(const int *data, const size_t object_size, const size_t array_size)
{
    int min = *data;
    for (size_t i = 1; i < array_size; i++)
    {
        const int value = *((int *)((uint8_t *)data + object_size * i));
        if (min > value)
            min = value;
    }
    return min;
}

int max_int(const int a, const int b)
{
    return a > b ? a : b;
}

int max_int_array(const int *data, const size_t object_size, const size_t array_size)
{
    int max = *data;
    for (size_t i = 1; i < array_size; i++)
    {
        const int value = *((int *)((uint8_t *)data + object_size * i));
        if (max < value)
            max = value;
    }
    return max;
}