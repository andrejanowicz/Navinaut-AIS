/*
 * Navinaut-AIS
 * Code:  https://github.com/andrejanowicz/Navinaut-AIS
 * Shop:  https://navinaut-ais.de/
 * 
 * Author:  Andre Janowicz, <aj@main-void.de>
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
  
  // send NMEA samples until actual data is received. Helps to setup system at home.
  // Long:      6.97928333333333
  // Lat:       51.376695
  // Location:  Essen, Germany
  
  char NMEA_sample[] = "!AIVDM,1,1,,A,B39E<ih0087w;I7FGR7Q3wTUoP06,0*62\r\n";
  static uint32_t timestamp_latest_sample = 0;
  const uint8_t TEST_SIGNAL_DELTA = 10; // seconds
  
  if ((millis() - timestamp_latest_sample < TEST_SIGNAL_DELTA * 1000 )){
    return;
  }

  DEBUG.print("Sending sample NMEA string:\t");
  DEBUG.print(NMEA_sample);

  NMEA_0183_HS.print(NMEA_sample);
  UDP_send_NMEA(NMEA_sample);
  BLE_send_NMEA(NMEA_sample);
  TCP_IP_send_NMEA(NMEA_sample);
    
  timestamp_latest_sample = millis();
  
  AIS_flash_pending = true;
}

void setup() {

  NMEA_0183_HS.begin(BAUD_NMEA_0183_HS);
  DEBUG.begin(BAUD_DEBUG, SERIAL_8N1, RXD2, TXD2);
  set_friendly_name();

  DEBUG.println("####### RESTART #######");
  DEBUG.print(unique_friendly_name);
  DEBUG.print(" build date: ");
  DEBUG.println(__DATE__);

  LED_setup();
  LED_power_cycle();

  DEBUG.print("WiFi setup:\t");
  WiFi_setup();
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
  
  if (!received_NMEA_success){ 
    // only send samples if we haven´t received actual data
    send_NMEA_sample_data();
  }
  
  packet_handle();
  nmea_process_packet();
  fifo_remove_packet();
  WiFi_handle();
  LED_handle();

}
