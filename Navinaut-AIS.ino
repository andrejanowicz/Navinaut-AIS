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
#include "radio.h"
#include "defs.h"
#include "packet_handler.h"
#include "nmea.h"
#include "fifo.h"
#include "LED.h"
#include "BLE.h"
#include "wireless.h"

void send_NMEA_sample_data() {
  
  // send NMEA test string after startup. Helps users to setup system at home. Stop when receiving actual data

  char NMEA_sample[] = "!AIVDM,1,1,,A,B39E<ih0087w;I7FGR7Q3wTUoP06,0*62";
  static int16_t test_duration = 180; // first 180 seconds after startup a test signal is sent on all available connections
  static uint32_t latest_sample_sent = 0;
  const uint8_t TEST_SIGNAL_DELTA = 10;
  
  if (received_NMEA_package || (test_duration < 0)  ||  millis() - latest_sample_sent < TEST_SIGNAL_DELTA * 1000){
    return;
  }

  DEBUG.print("Sending sample NMEA string:\t");
  DEBUG.println(NMEA_sample);

  NMEA_0183_HS.println(NMEA_sample);
  UDP_send_NMEA(NMEA_sample);
  BLE_send_NMEA(NMEA_sample);
  TCP_IP_send_NMEA(NMEA_sample);
    
  test_duration -= TEST_SIGNAL_DELTA;
  latest_sample_sent = millis();
  AIS_flash_pending = true;
}

void setup() {

  NMEA_0183_HS.begin(BAUD_NMEA_0183_HS);
  DEBUG.begin(BAUD_DEBUG, SERIAL_8N1, RXD2, TXD2);
  set_friendly_name();

  DEBUG.println("####### RESTART #######");
  DEBUG.print(friendly_name);
  DEBUG.print(" build date: ");
  DEBUG.println(__DATE__);

  LED_setup();
  LED_power_cycle();

  DEBUG.print("WiFi setup:\t");
  wifi_setup();
  DEBUG.println("done.");

  DEBUG.print("BLE setup:\t");
  BLE_setup();
  DEBUG.println("done.");

  // setup and configure radio
  DEBUG.print("radio setup:\t");
  radio_setup();
  DEBUG.println("done.");

  DEBUG.print("radio configuration:\t");
  radio_configure();
  DEBUG.println("done.");

  // verify that radio configuration was successful
  DEBUG.print("radio read chip status:\t");
  radio_get_chip_status(0);
  DEBUG.println("done.");

  if (radio_buffer.chip_status.chip_status & RADIO_CMD_ERROR) { // check for command error
    DEBUG.println("radio command error");
    ESP.restart();
  }

  DEBUG.print("packet handler setup:\t");
  ph_setup();
  DEBUG.println("done.");
  DEBUG.print("packet handler start\t");
  ph_start();
  DEBUG.println("done.\nSetup complete, starting main loop()");
}

void loop() {
  // loops every ~50us / 20kHz

  send_NMEA_sample_data();
  
  packet_handle();
  nmea_process_packet();
  fifo_remove_packet();
  wifi_handle();
  LED_handle();

}
