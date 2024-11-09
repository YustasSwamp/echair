# echair
RC chair

# How to build

## Install ESP32 Board support bundle
* esp32 by Espressif Systems, 3.0.7

## Install Libraries
* Async TCP by Me-No-Dev, 3.2.12
* ESP Async WebServer by Me-No-Dev, 3.3.21
* ESP32-TWAI-CAN by sorek.uk, 1.0.1
* PID by Brett Beauregard, 1.2.0
* WebSerial by Ayush Sharma, 2.0.8

## Select a board
* esp32 -> DOIT ESP32 DEVKIT V1

## Modify a code
* Add password.h file with the following context, and modify a password accordingly.
```
const char* password = "password";
```
* Select a chair type 2wd or 4wd by commenting or uncommenting `#define ECHAIR_4x4` line in echair.ino

## Build the project


