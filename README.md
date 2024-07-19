# LoRa Waterlevel Sensor



**A set of three meshed devices, namely two sensing units and a display unit, to monitor the fluid level in two remotely located reservoirs.**


Due to the remote location and other constraints, the sensing units have the following features
1. Battery-powered, lasting 6 month on a single charge
2. fully waterproof enclosure to be protected from rainwater and condensation
3. wireless transfer of data using Long Range 868 MHz frequency
4. remote wakeup call to configure polling rate on demand
5. non-contact fluid level sensing using ultra-sonic and time of flight infrared sensors
6. pattern matching to measure fluid level between obstacles. (see further down)

# The Sensing Units

## Overview



![image](https://github.com/user-attachments/assets/788b529f-041d-4914-918c-e479765e2ead)


The sensing unit consists of a tray, holding the electronics, a waterproof contrainer and a stainless steel tube pertruding from the underside, holding the sensor

## Electronics

BOM:
- ESP32-E on a firebeetle 2 board
- ebyte LoRa E220 TTL Module
- Unit 1: AJ-SR04M Ultrasonic sensing board and sensor
- Unit 2: ST microelectronics VL53L5CX multi zone TOF sensor 
- 3700 mAh 3.7V LiPo battery

This particular firebeelte board also features a charging circuit and a breakable jumper to reduce its power consumption by 500 uA during idle and further to 13 uA during deep sleep. A 300 Ohm resistor was added to the AJ-SR04M low power pads to reduce its sleep power consumption to <40 uA.

### Power Consumption

Device | Normal operation | Sleep
 :-------------------------:|:-------------------------:|:-------------------------:
 Esp32-E | 40mA | 13 uA
 AJ-SR04M | 30 mA | 40 uA
 LoRa E220 | 50 mA | 2 uA
 Active time | 1s | 15 min

![IMG_20240624_133554](https://github.com/user-attachments/assets/0165ff7d-f08b-4859-8541-e6013c86e1cc)  |  ![IMG_20240621_184606](https://github.com/user-attachments/assets/61593344-f5bc-4aee-9b0e-5e6af38d56cd)
:-------------------------:|:-------------------------:
![IMG_20240624_133729](https://github.com/user-attachments/assets/c6443ede-875d-49e2-a2c9-6f09bb93a37b)  |  ![IMG_20240624_133757](https://github.com/user-attachments/assets/a7d6eb42-5ffd-4090-83f1-a5a2d81f6025)


### Schematics

![Fluidlevel_receiver drawio](https://github.com/user-attachments/assets/79652827-876a-4b0b-9009-ad5fe4164200)



## Code

The code mainly does the following things:
1. wake up by WORA signal using the E220 LoRa module
2. perform a measurement
3. transmit a package containing the measurement, sensor id, timestamp and battery level

Sensing unit 1 features a simple ultrasonic sensor to measure the disance from the top of the reservoir to the fluid level.


# Pattern Matching

Sensing unit 2 needed a special solution because the second reservoir features internal walls, separating the reservoir into 3 chambers. As a consistent orientation of the sensor with regards to the internal walls could not be assurred, the sensor would need to "percept" the obstacle.

![image](https://github.com/user-attachments/assets/3e224c99-d6d7-4dd1-84b8-34aea2136521)  |  ![image](https://github.com/user-attachments/assets/9d7ebe67-b23d-4f9b-b2a6-b557012aefda) 
:-------------------------:|:-------------------------:

For this, a pattern matching algorithm was written that takes the 8x8 sensor data matrix, finds the Wall intersection and orientation and then performs 3 measurements, one in each chamber.
A mockup was written in python, which provides the illustrative pictures here. 

In a first step, the data from the LV53L5CX sensor was normalized and thresholded

![image](https://github.com/user-attachments/assets/2b418a5c-5e7d-484b-9c45-8fd1bae13564)

Next an edge map was created using convolution with an edge kernel, after which the pattern could be tested on the image, again using convolution and find its best position (red marker) and orientation. From there the 3 measureing coordinates (blue markers) could be computed using simple trigonometry.

![image](https://github.com/user-attachments/assets/51a12063-0d33-4756-92e0-5d182701ab70)

Using diameter and size of the reservoir, the volume of the containing fluid could be computed and send to the receiver.


# Reveiver
The receiver is an ESP32 based display module, showing the fill status of both reservoirs.
It features:
- color display
- fill status warning
- sensor error handling
- data logging, showing plots for different time frames

## Electronics

BOM:
- Lilygo t-display ESP32-S3 board containing a color tft display
- ebyte LoRa E220 TTL Module




![IMG_20240624_133929](https://github.com/user-attachments/assets/b66dd109-3e1a-4b50-abd6-e5defa47f302)

![IMG_20240705_124411](https://github.com/user-attachments/assets/c7c64790-ad77-4b71-bcc3-45e320926d03)  |  ![IMG_20240705_124420](https://github.com/user-attachments/assets/6230383a-c9d5-47c9-a6ea-e3f7e94d7866)
:-------------------------:|:-------------------------:



