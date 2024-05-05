/*
   Navinaut-AIS
   Code:  https://github.com/andrejanowicz/Navinaut-AIS
   Shop:  https://navinaut-ais.de/

   Author:  Andre Janowicz, <aj@main-void.de>
   License: CC BY-SA Creative Commons Attribution-ShareAlike
            https://creativecommons.org/licenses/by-sa/4.0/

*/


#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "defs.h"
#include "wireless.h"
#include "LED.h"
#include "BLE.h"

#define ANDROID_WEB_BLE_FIX

bool BLE_device_connected = false;
BLECharacteristic *pCharacteristic;

void BLE_setup(void)
{

  BLEDevice::init(friendly_name);

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN , ESP_PWR_LVL_P9);

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  //
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);


  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();

  DEBUG.print("BLE advertising started.\t");
}
void BLE_send(char* message) {

  if (BLE_device_connected) {

    pCharacteristic->setValue(message);
    pCharacteristic->notify(); // Send value
  }
}

void BLE_send_NMEA(char* message) {

  const uint8_t BLE_MTU = 20;
  uint8_t nmea_len = 0;
  uint8_t packets = 0;
  uint8_t rem = 0;
  char fragment[BLE_MTU + 1];

  nmea_len = strlen(message);
  packets = div(nmea_len, BLE_MTU).quot;
  rem = div(nmea_len, BLE_MTU).rem;

#ifdef ANDROID_WEB_BLE_FIX
  // chop up payload as workaround for current Android browser BLE packet size limitation

  for (int i = 0; i < packets; i++) { // send remaining parts (if any) in 20 Byte chunks
    snprintf(fragment, BLE_MTU + 1, "%s", message + i * BLE_MTU);
    BLE_send(fragment);
  }

  if (rem) {
    snprintf(fragment, rem + 1, "%s", message + packets * BLE_MTU);
    BLE_send(fragment);
  }

#elif
  BLE_send(message);
  
#endif
}
