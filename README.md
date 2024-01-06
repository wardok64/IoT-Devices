# Basic IoT devices to measure different field variables (based on Zigbee and MQTT)

This document details the final and functional design of a network of wireless sensors, established as End Devices and Coordinator within a Zigbee network, which, through its gateway consisting of the coordinator and an ESP12 communicated via UART data is sent to a MQTT Broker, and from there, it is possible to digitally display the data through NodeRED Dashboard.

The data sent from the end devices pertains to the value of the associated field sensor (it can be temperature, humidity or current), as well as the charge value of the battery that is powering it, in order to monitor the battery life and to protect the end device from low voltage.

## Initial requirements:

- Scalable software level via Zigbee and Node-RED.
- Battery powered field devices.
- Use MCU or development board with minimal elements to maximise power savings.
- Develop firmware with power saving option: "LowPower.h"/"RTCZero.h".
- Standard power supply for all field devices (3.3VDC).
- Scalability at hardware level.
- In the case of sensors with analogue signal, consider an ADC resolution between 12 and 16-bits.
- Low voltage protection in main power supply. (consider alarm via Zigbee).

## Sketch idea:
![image](https://github.com/wardok64/IoT-Devices/assets/104173190/0a244fde-c015-43fd-9832-4da00ae3cd0e)

## Core materials HW:

- Seeeduino XIAO (ARM Cortex-M0+ CPU(SAMD21G18) running at up to 48MHz)
- Xbee XBP24 module Digi (XbeePROS2B).
- ESP8266/ESP-12 package.
- Raspberry Pi Zero W.

## Core Arduino libraries:

- Xbee.h
- PubSubClient.h
- RTCZero.h

## Software requirements:
  
- ArduinoIDE/VSC.
- XCTU (Digi).
- Node-RED/Dashboard.
- Mosquitto Broker.
- KiCAD.

## Sensors:

### Current:
- SCT-019-000 Current sensor/ Input current: 0-200A/ Output: 0-33mA (It needs burden resistor to convert to voltage).
- SCT-013-030 Current sensor/ Input current: 0-30A/ Output: 0-1Vrms. (incorporates its burden resistor)
- SCT-013-000 Current sensor/ Input current: 0-100A/ Output: 0-1Vrms. (incorporates its burden resistor)
  - Libraries:
    - emonlib.h (with some modifications.)

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/c67e8c14-4ecd-4708-8bcd-dcbf921f0cd5)

### Temperature:
- DS18B20.
  - Libraries:
    - OneWire.h
    - DallasTemperature.h
- RTD PT100 or PT1000 with MAX31865 Amplifer.
  - Libraries:
    - Adafruit_MAX31865.h
- Type K Thermocouple with MAX6675 controller.
  - Libraries:
    - max6675.h

### Relative Humidity and Temperature:
- SHT11.
  - Libraries:
    - SHT1x.h

## Summary:

The following is a basic project focused on the measurement, processing and monitoring of field variables in a semi-industrial process, in a rural area without direct access to internet or cellular networks, initially thought of solutions based on LoRa technology, SigFox or WiFi, but finally opted to use Zigbee using XbeePROS2B modules (apparently discontinued but currently low cost), The SAMD21G18 Cortex-M0 microcontroller, which is responsible for receiving and processing the signal from the field sensors and then send its value through Zigbee, was used Seeeduino-XIAO development board that incorporates the SAMD21 microcontroller for several reasons:
- [x] Compatible with Arduino IDE.
- [x] Multiple development interfaces and 11 digital/analog pins (ADC 10/12-bit).
- [x] Power Supply 3.3V/5V DC.
- [x] USB type-C Power supply and downloading interface.
- [x] Small size (20x17.5mm).
- [x] SWD Interface.
- [x] Easy project operation, Breadboard-friendly.

It was evaluated to use the development board Seeeduino XIAO, this development board unlike an Arduino nano, does not have an FTDI chip, as this is programmed directly with a USB chip, which allows us to save energy while the device is not being programmed, ie the FTDI circuit in the Arduino nano, is draining current, even if it is not being used at that time, another alternative was to use the Arduino mini, but it is officially discontinued and is only available in clone.

We should also mention that this development board has a 3.3V linear output voltage regulator, the XC6206 which allows us to perfectly power our Xbee.

## Hardware modifications:

Along with the use of the RTC library that fits perfectly to put the MCU in power saving mode, here is a table with current consumptions considering the user LED (PA17) embedded on the board, as well as the power LED:

| Conditions    | Results       | Details       |
| ------------- | ------------- | ------------- |
| Input voltage via USB-C port: 4.95V, LED flashes every second | 12,4 mA a 4,95 V (61,38 mW)  | Measured when the flashing LED is on. |
| Input voltage via USB-C port: 4.95V, Sleep mode with RTCZero library| 2,2 mA a 4,95 V (10,89 mW)|The MCU itself uses less than 1mA (the RTC and some other services are active), the rest is used by the power LED and the voltage regulator.|

In our case, we chose to eliminate the power LED, which consumes current all the time, when the XIAO is powered (Corresponds to GREEN PWR), this was extracted from the diagram:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/636222af-dce3-4a3a-b626-f11c065aee3f)

Among the examples of the RTC library, there is a basic one, which places the microcontroller in sleep mode for a programmed time, when the time is over, it wakes up and turns on its LED. The example can be found as: "sleepRTCAlarm.ino" Once we verify that the library works correctly, we load it to our Seeeduino XIAO board, then we measure the current consumption in sleep mode, with a multimeter, giving us as a result of the readings the following values:

| Seeeduino-XIAO ||||
| ------------- | ------------- | ------------- | ------------- |
|Normal mode with power LED on.|Sleep Mode with activated power LED.|Normal mode without power LED.|Sleep Mode without power LED.|
|14.40mA|1.40mA|12.60mA|0.08mA|

> [!NOTE]
> Power LED, as for example on the Arduino UNO board, this LED indicates when the board is powered.

## Configuracion Sleep-Mode en XCTU:

The parameters for our XbeePROS2B to go into sleep mode, but through external control, i.e. to put the Xbee to sleep and wake it up by commands from the SAMD21. The sleep settings are as follows in XCTU:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/e7b2646a-b289-4840-b658-78e08fbad499)

## Code descriptions:

We define a macro for pin D10 (11) pin to manipulate Xbee Sleep-Mode.
```
#define SleepXbee 10 
```
We define a macro for pin D9 (10) pin to identify when our xbee has connected to the zigbee network.
```
#define AssLEDEN   9 
```
This pin corresponds to an association signal from our Xbee, which is indicated in the data sheet:

> ### Associate LED:
> The Associate pin (pin 15) provides an indication of the device's sleep status and diagnostic information. To take advantage of these indications, connect an LED to the Associate pin. To enable the Associate LED functionality, set the D5 command to 1; it is enabled by default. If enabled, the Associate pin is configured as an output. This section describes the behavior of the pin. The Associate pin indicates the synchronization status of a sleep compatible XBee/XBee-PRO DigiMesh 2.4. If a device is not sleep compatible, the pin functions as a power indicator. Use the LT command to override the blink rate of the Associate pin. If you set LT to 0, the device uses the default blink time: 500 ms for a sleep coordinator, 250 ms otherwise. The following table describes the Associate LED functionality.

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/36cd7f3a-deeb-45b7-9cd8-2c2292f10d92)

Source: [DIGI Documentation](https://www.digi.com/resources/documentation/Digidocs/90000991/reference/r_associate_led.htm?TocPath=Work%20with%20networked%20devices%7CTest%20links%20between%20adjacent%20devices%7C_____6)

## Basic connection diagram between Xbee and Seeeduino-XIAO:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/c0e17270-8654-4b8c-8f54-a7f646bc4428)

## Power consumption:

Table of total current consumption Xbee+Seeduino XIAO.
| XIAO SAMD21 + XBee Pro S2B Current with same power supply |-|
| ------------- | ------------- |
|Sleep-Mode Xbee + XIAO|Transmision Mode Zigbee|
|0.09mA.|30-60mA.|

> [!NOTE]
> The maximum average time from the time the End Device wakes up, establishes a connection with the network coordinator, and sends the data frame is 15 seconds.

> [!NOTE]
> It was determined that the best way for our MCU to verify when our Xbee has been associated to a Zigbee network is to use the Assossiate LED pin, which by means of a counter, indicates when the Xbee has successfully connected to the network and is ready to send a data, this pin is digital, so it can work perfectly as an input for the XIAO Seeeduino.

## Final Architecture:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/092d3d08-3e82-4774-8d5a-1680adb35025)


## Next steps:
- [ ] Migrate XbeePROS2B to S3B or DigiMesh.
- [ ] Low-Cost Field Network version with ESP-NOW.
- [ ] Gateway ESP-NOW to MQTT with ESP32.
- [ ] Dashboard remote from internet.
- [ ] Enable Low-Power.
- [ ] Interchangeable modular sensors.
- [ ] Add more field sensors.
- [ ] Add more connectors.

