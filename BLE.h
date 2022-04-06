/*
 * Navinaut-AIS
 * Code:  https://github.com/andrejanowicz/Navinaut-AIS
 * Shop:  https://navinaut-ais.de/
 * 
 * Author:  Andre Janowicz, <andre.janowicz@gmail.com>
 * License: CC BY-SA Creative Commons Attribution-ShareAlike
 *          https://creativecommons.org/licenses/by-sa/4.0/
 *          
 */


#ifndef BLE_H_
#define BLE_H_

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "defs.h"

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define MTU 28

extern bool BLE_device_connected;
extern BLECharacteristic *pCharacteristic;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {

    BLEDevice::startAdvertising();
    BLE_device_connected = true;
    DEBUG.println("BLE connected.");
  };

  void onDisconnect(BLEServer* pServer) {
    DEBUG.println("BLE disconnected.");
    BLE_device_connected = false;
    BLEDevice::startAdvertising();
    DEBUG.println("BLE advertising started.");
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    /*
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
      for (int i = 0; i < rxValue.length(); i++) {
        DEBUG.print(rxValue[i]);
      }
      */
    return;
  }
};

void BLE_setup(void);

void BLE_send_NMEA(char* message);

#endif /* BLE_H_ */
