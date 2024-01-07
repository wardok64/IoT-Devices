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

## Prototyping for proof of concept:

### End devices.
![image](https://github.com/wardok64/IoT-Devices/assets/104173190/77b0d2c0-e6b1-4cc6-af93-a55e68671fe8)  ![image](https://github.com/wardok64/IoT-Devices/assets/104173190/0318fc70-7a25-49d8-ba08-94b6c47788b8) ![image](https://github.com/wardok64/IoT-Devices/assets/104173190/7d737711-b486-4d09-b6ca-d749a7225861)

### Coordinador conected to XCTU.

Variable value:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/2ab42511-804e-4935-a7c3-a0c29e4a3575)

Battery value:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/2d944525-1e2b-46e3-a4f5-5788897028c0)


### Coordinador with Arduino UNO.
<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/222c9a21-9e37-4ad4-9aec-bed3837befa3" width="502" height="350">

By running the serial monitor of our Arduino UNO we can observe the composition of our frame, both Zigbee reception and MQTT sending.

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/e73249fa-6220-46bb-810d-850c36b7beed)
![image](https://github.com/wardok64/IoT-Devices/assets/104173190/2421a7bb-8b02-4246-afaa-4fc2e06ceda3)

## Gateway Zigbee To MQTT:

### Prototyping (Proof of concept).

As mentioned before, the main function of our gateway is to establish a MQTT connection with the Broker server via WIFI (maybe in the future an Ethernet connection will be considered, to guarantee stability), receive the data from the end devices, with the help of our library "xbee. h" library, it identifies the ID of the end device that sent the message, then it processes the message and it starts to send a structured string of characters, which will correspond to the specific topic of the device, finally it publishes the topic and the corresponding payload (a status LED was added to indicate each of the operations).

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/f2e1a936-f53c-4976-96a7-fd00725b65f3" width="250" height="400">

Diagram:

For the electronic design we considered, among other things, a selector and connectors that allow us on the one hand to reconfigure the operating parameters of our Xbee, and on the other hand a firmware update of the MCU ESP8266 (via SERIAL).

![gateway](https://github.com/wardok64/IoT-Devices/assets/104173190/78f2d5cb-1877-40af-b68a-0e12af38da1b)

Code:
Local IP/DNS of Broker MQTT (RaspberryPi).
```
const char* mqtt_server = "nodered.local"; 
```
Connect with Broker MQTT, port 1883 (local/insecure).
```
client.setServer(mqtt_server, 1883);
```
Get payload from Zigbee.
```
for (int i = 9; i < rx.getDataLength(); i++) 
    {
      payload += (char)rx.getData(i);
    }
```
Get remote address, store in strings.
```
SH = String(rx.getRemoteAddress64().getMsb(),HEX);
SL = String(rx.getRemoteAddress64().getLsb(),HEX);
```

Define MQTT topic.
```
String topic = "/line/area/equipment/";
```
Concatenate new topic.
```
topic_ = topic + elemento + slash + variable + slash + SL;
```
Convert String to Char array.
```
char _SL[39];
topic_.toCharArray(_SL,39);
```
Publish to MQTT.
```
client.publish(_SL, _payload);
```

## Node-RED

If we power our Gateway, and connect our nodes, we must first energize and connect our Raspberry pi to the common modem, to run, first the MQTT broker, then a NodeRED client that runs inside the same Raspberry Pi, now through our PC, we can access NodeRED and also open our MQTT.fx application to view the topics and payload that are being sent to the Broker, as shown in the following images:

MQTT.fx

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/0a0a539d-29ef-4eb6-9605-cf75d23a9fd7)


Node-RED.

Initially the MQTTin nodes subscribed to each of the corresponding topicals for each end device were individually configured, the corresponding payload was filtered from the value of the field variable and the battery charge value of each device.

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/38a57e61-35bb-4b4c-8a8e-b4c51995dfda)


Node-RED/Dashboard.

A fairly basic dashboard design was developed to display the values (Front-End).

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/79fc3360-a930-4e4f-99c0-2186b03c9158)


## KICAD and PCB.

End devices.

Once some proofs of concept, testing, debugging and dimensioning adjustments were done, we proceeded to the design in KiCAD of the final devices, keeping in mind the modularity so that each of the final devices is compatible with different types of sensors.

The electronic design was done in KiCAD working with two layers.

![Screenshot from 2024-01-07 12-05-52](https://github.com/wardok64/IoT-Devices/assets/104173190/124173be-f823-49bb-aca9-e34ea6fabb8f)
![Screenshot from 2024-01-07 12-06-19](https://github.com/wardok64/IoT-Devices/assets/104173190/e1aa69ae-888d-4b94-b54c-fee33c53413d)
![Screenshot from 2024-01-07 12-06-52](https://github.com/wardok64/IoT-Devices/assets/104173190/4ee25a6e-862c-4595-9065-da13e49dbdb1)


The manufacturing of the circuits was sent to China.

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/ff0d48a1-b469-4ded-bb23-2a973b2353cf" width="300" height="400">

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/f591c869-7605-4833-9f21-145545e303a9" width="300" height="400">

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/6561d210-ac8d-41ed-a250-e3ec48794e7d" width="270" height="400">

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/10998e8f-462b-47c8-b821-2b68b61277ce" width="300" height="400">



Gateway.

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/d68a1345-7497-4b2a-af43-c2407443b533" width="300" height="400">

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/00f1efd3-5998-4ddf-a8b5-5bfd42e75b55" width="300" height="400">

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/cd6242bd-0cb1-4812-9660-76c664f34462" width="300" height="400">

<img src="https://github.com/wardok64/IoT-Devices/assets/104173190/ca186386-ddd9-4794-83f2-9818a182c67d" width="300" height="400">


## Final Architecture:

![image](https://github.com/wardok64/IoT-Devices/assets/104173190/092d3d08-3e82-4774-8d5a-1680adb35025)

As a conclusion in the development of this project I can mention that the devices are working functionally in a semi-industrial environment, each element of the network was protected with an enclosure of suitable grade (IP67). Any part of the project (software and hardware) is 100% improvable, to mention a few; 

- Use of ethernet instead of WiFi.
- Migration to DIGI mesh.
- Migration to ESP32.
- Bare-metal firmware programming to avoid library dependencies as much as possible (optimizing energy consumption).
- Among others.

## Next steps:
- [ ] Migrate XbeePROS2B to S3B or DigiMesh.
- [ ] Low-Cost Field Network version with ESP-NOW.
- [ ] Gateway ESP-NOW to MQTT with ESP32.
- [ ] Dashboard remote from internet.
- [ ] Enable Low-Power.
- [ ] Interchangeable modular sensors.
- [ ] Add more field sensors.
- [ ] Add more connectors.

