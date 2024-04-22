# PicoWeather

PicoWeather is a weather station project built using Raspberry Pi Pico microcontroller, featuring a display with the ILI9341 driver and the FT6X36 touch sensor. It incorporates environmental sensors such as BME280 for measuring temperature, humidity, and air pressure, along with CCS811 for monitoring CO2 and VOC levels.

## Features

- **Real-time Weather Data**: Provides accurate real-time weather information including temperature, humidity, air pressure, CO2, and VOC levels.
- **Display**: Presents weather data in a clear and visually appealing manner on the ILI9341 display.
- **User Interaction**: Utilizes touch sensor for user interaction and control, allowing for easy navigation and customization.
- **Forecast Data**: Allows fetching forecast data from https://www.weatherapi.com/
- **Raspberry Pi Pico**: Powered by Raspberry Pi Pico microcontroller, ensuring reliable performance and versatility.

## Hardware Requirements

- Raspberry Pi Pico microcontroller
- ILI9341 display with FT6X36 touch sensor
- BME280 sensor for temperature, humidity, and air pressure
- CCS811 sensor for CO2 and VOC measurements

## Installation
### Flashing the Raspberry Pi Pico W

1. Download the `weather.uf2` file from the [Releases](https://github.com/DarkIceXD/PicoWeather/releases) section.
2. Connect your Raspberry Pi Pico W to your computer via a micro USB cable while holding the Bootsel button.
3. Flash the `weather.uf2` file onto the Raspberry Pi Pico W by dragging and dropping it.

### Connecting the Display and Touch sensor

- `VCC` -> `3V3(OUT)`
- `GND` -> `GND`

#### Display
- `LCD_CS` -> `GP22`
- `LCD_RST` -> `GP26`
- `LCD_RS` -> `GP27`
- `SDI(MOSI)` -> `GP19`
- `SCK` -> `GP18`

#### Touch sensor
- `CTP_SCL` -> `GP21`
- `CTP_RST` -> `3V3(OUT)`
- `CTP_SDA` -> `GP20`

### Connecting the BME280 sensor

- `VCC` -> `3V3(OUT)`
- `GND` -> `GND`
- `SCL` -> `GP21`
- `SDA` -> `GP20`
- `CSB` -> `3V3(OUT)`
- `SDO` -> `GND`

### Connecting the CCS811 sensor

- `VCC` -> `3V3(OUT)`
- `GND` -> `GND`
- `SCL` -> `GP21`
- `SDA` -> `GP20`
- `WAK` -> `GND`
