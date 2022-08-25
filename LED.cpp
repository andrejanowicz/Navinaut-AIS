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


#include <Arduino.h>
#include "defs.h"
#include "LED.h"
#include "BLE.h"
#include "wireless.h"

uint32_t last_tick = 0;
bool AIS_flash_pending = false;
uint32_t AIS_LED_start = 0;
uint8_t LED_pulse_length = 50;

void LED_setup(void) {

  pinMode(PWR_LED, OUTPUT);
  pinMode(CONNECTION_LED, OUTPUT);
  digitalWrite(PWR_LED, LOW);
  digitalWrite(CONNECTION_LED, LOW);
}

void LED_PWR_error_flash(void) {

  digitalWrite(PWR_LED, HIGH);
  delay(LED_pulse_length);
  digitalWrite(PWR_LED, LOW);
  delay(LED_pulse_length);
}


void LED_power_cycle(void) {

  digitalWrite(PWR_LED, HIGH);
  digitalWrite(CONNECTION_LED, HIGH);
  delay(LED_pulse_length);
  digitalWrite(PWR_LED, LOW);
  digitalWrite(CONNECTION_LED, LOW);
  delay(LED_pulse_length);
}

void LED_handle(void) {

  uint32_t now = millis();

  if (AIS_flash_pending) {
    digitalWrite(PWR_LED, LOW);
    AIS_flash_pending = false;
    AIS_LED_start = now;
  }

  if (now - AIS_LED_start >  LED_pulse_length) {
    digitalWrite(PWR_LED, HIGH);
  }

  digitalWrite(CONNECTION_LED, BLE_device_connected | wireless_has_clients);
}
