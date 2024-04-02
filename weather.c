#include <stdio.h>
#include <hardware/rtc.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include "storage/storage.h"
#include "https_client/https_client.h"
#include "jsmn/json.h"
#include "min_max/min_max.h"
#include "drivers/ili9341.h"
#include "drivers/ft6x36.h"
#include "drivers/bme280.h"
#include "drivers/ccs811.h"
#include "ui/ui.h"

struct settings
{
    char ap_name[32 + 1];
    char password[64 + 1];
    char key[64 + 1];
    char query[64 + 1];
} settings = {
    .ap_name = "",
    .password = "",
    .key = "",
    .query = "Berlin",
};

void world_time_api_handler(const char *json, const jsmntok_t *t, const size_t count, struct json_data *data)
{
    datetime_t *time = data->data;
    if (is_json_key(json, t, "datetime"))
    {
        int year, month, day, hour, minute, second;
        sscanf(json + (t + 1)->start, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
        time->year = year;
        time->month = month;
        time->day = day;
        time->hour = hour;
        time->min = minute;
        time->sec = second;
    }
    else if (is_json_key(json, t, "day_of_week"))
    {
        time->dotw = strtol(json + (t + 1)->start, NULL, 10);
    }
}

struct weather_hour_data
{
    char time[16 + 1];
    float temp_c;
    float feelslike_c;
    float wind_kph;
    int pressure_mb;
    int humidity;
    int chance_of_rain;
    int chance_of_snow;
};

struct weather_day_data
{
    char sunrise[8 + 1];
    char sunset[8 + 1];
    struct weather_hour_data hour[24];
};

struct weather_forecast
{
    bool valid;
    struct weather_day_data day[3];
};

bool is_forecast_day = false;
bool is_hour = false;
int i = 0;
void weather_api_handler(const char *json, const jsmntok_t *t, const size_t count, struct json_data *data)
{
    if (data->array_depth < 1)
    {
        is_forecast_day = false;
        is_hour = false;
    }
    else if (data->array_depth < 2)
    {
        is_hour = false;
    }

    if ((t + 1)->type == JSMN_ARRAY)
    {
        if (is_json_key(json, t, "forecastday"))
        {
            is_forecast_day = true;
            return;
        }
        else if (is_json_key(json, t, "hour"))
        {
            is_hour = true;
            i = data->array_index;
            return;
        }
    }

    struct weather_forecast *forecast = data->data;
    if (is_forecast_day)
    {
        forecast->valid = true;
        if (!is_hour)
        {
            if (is_json_key(json, t, "sunrise"))
            {
                strlcpy(forecast->day[data->array_index].sunrise, json + (t + 1)->start, sizeof(forecast->day[data->array_index].sunrise));
            }
            else if (is_json_key(json, t, "sunset"))
            {
                strlcpy(forecast->day[data->array_index].sunset, json + (t + 1)->start, sizeof(forecast->day[data->array_index].sunset));
            }
        }
        else
        {
            if (is_json_key(json, t, "time"))
            {
                strlcpy(forecast->day[i].hour[data->array_index].time, json + (t + 1)->start, sizeof(forecast->day[i].hour[data->array_index].time));
            }
            else if (is_json_key(json, t, "temp_c"))
            {
                forecast->day[i].hour[data->array_index].temp_c = strtof(json + (t + 1)->start, NULL);
            }
            else if (is_json_key(json, t, "feelslike_c"))
            {
                forecast->day[i].hour[data->array_index].feelslike_c = strtof(json + (t + 1)->start, NULL);
            }
            else if (is_json_key(json, t, "wind_kph"))
            {
                forecast->day[i].hour[data->array_index].wind_kph = strtof(json + (t + 1)->start, NULL);
            }
            else if (is_json_key(json, t, "pressure_mb"))
            {
                forecast->day[i].hour[data->array_index].pressure_mb = strtol(json + (t + 1)->start, NULL, 10);
            }
            else if (is_json_key(json, t, "humidity"))
            {
                forecast->day[i].hour[data->array_index].humidity = strtol(json + (t + 1)->start, NULL, 10);
            }
            else if (is_json_key(json, t, "chance_of_rain"))
            {
                forecast->day[i].hour[data->array_index].chance_of_rain = strtol(json + (t + 1)->start, NULL, 10);
            }
            else if (is_json_key(json, t, "chance_of_snow"))
            {
                forecast->day[i].hour[data->array_index].chance_of_snow = strtol(json + (t + 1)->start, NULL, 10);
            }
        }
    }
}

struct ili9341_display display;
struct ft6x36_touch touch;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    ili9341_draw_window(&display, area->x1, area->y1, area->x2, area->y2);
    const uint32_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    lv_draw_sw_rgb565_swap(px_map, len);
    ili9341_draw(&display, px_map, len * 2);
    lv_disp_flush_ready(disp);
}

static void read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (ft6x36_read_data(&touch))
    {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch.data.touch_points[0].x;
        data->point.y = touch.data.touch_points[0].y;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

int main()
{
    stdio_init_all();
    datetime_t time;
    load(&settings, sizeof(settings));
    if (cyw43_arch_init())
    {
        printf("failed to init cyw43\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    int retries = 2;
    int err;
    do
    {
        err = cyw43_arch_wifi_connect_timeout_ms(settings.ap_name, settings.password, CYW43_AUTH_WPA2_AES_PSK, 30000);
    } while ((retries--) && err);
    if (err)
    {
        printf("failed to connect after 2 retries\n");
        return 1;
    }

    rtc_init();
    char *data = https_get_request("worldtimeapi.org", "/api/ip", NULL);
    if (data)
    {
        json_parse(data, world_time_api_handler, &time);
        free(data);
    }
    rtc_set_datetime(&time);

    spi_init(spi0, 62500000);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    i2c_init(i2c0, 400000);
    gpio_set_function(21, GPIO_FUNC_I2C);
    gpio_set_function(20, GPIO_FUNC_I2C);

    ili9341_init(&display, spi0, 22, 26, 27);
    const int rotation = 90;
    ili9341_rotate(&display, rotation, true);

    if (!ft6x36_init(&touch, i2c0, 15, rotation))
        printf("failed to init ft6x36\n");

    struct bme280_sensor bme280;
    if (!bme280_init(&bme280, i2c0))
        printf("failed to init bme280\n");

    struct ccs811_sensor ccs811;
    if (!ccs811_init(&ccs811, i2c0, CCS811_DRIVE_MODE_1SEC, 0, 0))
        printf("failed to init ccs811\n");

    char request[128];
    sniprintf(request, 128, "/v1/forecast.json?key=%s&q=%s&days=3", settings.key, settings.query);
    int8_t last_hour = 24;

    struct ui ui;
    ui_init(&ui, display.width, display.height, flush_cb, read_cb);
    while (true)
    {
        char value[20];
        rtc_get_datetime(&time);
        snprintf(value, sizeof(value), "%02d:%02d", time.hour, time.min);
        lv_label_set_text(ui.time, value);
        snprintf(value, sizeof(value), "%02d.%02d.%d", time.day, time.month, time.year);
        lv_label_set_text(ui.date, value);

        if (bme280_read_data(&bme280))
        {
            const uint16_t ccs811_humidity = bme280.data.humidity >> 1;
            const uint16_t ccs811_temp = bme280.data.temperature > -25 * 100 ? ((uint32_t)(bme280.data.temperature + 25 * 100) << 9) / 100 : 0;
            ccs811_set_env_data(&ccs811, ccs811_humidity, ccs811_temp);

            const float temp = bme280.data.temperature / 100.f;
            lv_bar_set_value(ui.temperature, temp, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%.1f", temp);
            lv_label_set_text(ui.temperature_value, value);

            const float hum = bme280.data.humidity / 1024.f;
            lv_bar_set_value(ui.humidity, hum, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%.1f", hum);
            lv_label_set_text(ui.humidity_value, value);

            const int press = bme280.data.pressure / (256.f * 100);
            lv_bar_set_value(ui.pressure, press, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%d", press);
            lv_label_set_text(ui.pressure_value, value);
        }

        if (ccs811_read_data(&ccs811))
        {
            lv_bar_set_value(ui.co2, ccs811.data.co2, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%d", ccs811.data.co2);
            lv_label_set_text(ui.co2_value, value);

            lv_bar_set_value(ui.voc, ccs811.data.voc, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%d", ccs811.data.voc);
            lv_label_set_text(ui.voc_value, value);
        }

        if (last_hour != time.hour)
        {
            data = https_get_request("api.weatherapi.com", request, NULL);
            if (data)
            {
                last_hour = time.hour;
                struct weather_forecast forecast;
                forecast.valid = false;
                json_parse(data, weather_api_handler, &forecast);
                free(data);
                for (int i = 0; i < 3; i++)
                {
                    lv_chart_set_range(
                        ui.days[i].temp,
                        LV_CHART_AXIS_PRIMARY_Y,
                        min_float(min_float_array(&forecast.day[i].hour[0].temp_c, sizeof(forecast.day[i].hour[0]), 24), min_float_array(&forecast.day[i].hour[0].feelslike_c, sizeof(forecast.day[i].hour[0]), 24)),
                        max_float(max_float_array(&forecast.day[i].hour[0].temp_c, sizeof(forecast.day[i].hour[0]), 24), max_float_array(&forecast.day[i].hour[0].feelslike_c, sizeof(forecast.day[i].hour[0]), 24)));
                    lv_chart_set_range(
                        ui.days[i].wind,
                        LV_CHART_AXIS_PRIMARY_Y,
                        min_float_array(&forecast.day[i].hour[0].wind_kph, sizeof(forecast.day[i].hour[0]), 24),
                        max_float_array(&forecast.day[i].hour[0].wind_kph, sizeof(forecast.day[i].hour[0]), 24));
                    lv_chart_set_range(
                        ui.days[i].pressure,
                        LV_CHART_AXIS_PRIMARY_Y,
                        min_int_array(&forecast.day[i].hour[0].pressure_mb, sizeof(forecast.day[i].hour[0]), 24),
                        max_int_array(&forecast.day[i].hour[0].pressure_mb, sizeof(forecast.day[i].hour[0]), 24));
                    lv_chart_set_range(
                        ui.days[i].humidity,
                        LV_CHART_AXIS_PRIMARY_Y,
                        min_int_array(&forecast.day[i].hour[0].humidity, sizeof(forecast.day[i].hour[0]), 24),
                        max_int_array(&forecast.day[i].hour[0].humidity, sizeof(forecast.day[i].hour[0]), 24));
                    for (int j = 0; j < 24; j++)
                    {
                        ui.days[i].temp_series[0]->y_points[j] = forecast.day[i].hour[j].temp_c;
                        ui.days[i].temp_series[1]->y_points[j] = forecast.day[i].hour[j].feelslike_c;
                        ui.days[i].wind_series[0]->y_points[j] = forecast.day[i].hour[j].wind_kph;
                        ui.days[i].pressure_series[0]->y_points[j] = forecast.day[i].hour[j].pressure_mb;
                        ui.days[i].humidity_series[0]->y_points[j] = forecast.day[i].hour[j].humidity;
                    }
                    lv_chart_refresh(ui.days[i].temp);
                    lv_chart_refresh(ui.days[i].wind);
                    lv_chart_refresh(ui.days[i].pressure);
                    lv_chart_refresh(ui.days[i].humidity);
                }
            }
        }
        lv_timer_handler();
        lv_tick_inc(1);
        sleep_ms(1);
    }
    return 0;
}
