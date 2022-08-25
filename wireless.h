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


#ifndef WIRELESS_H_
#define WIRELESS_H_

#include <Arduino.h>
#include <stdint.h>

extern uint64_t chipid;  
extern char friendly_name[22];
extern uint8_t wireless_has_clients;
void set_friendly_name();
void UDP_send_NMEA(char* message);
void TCP_IP_send_NMEA(char* message);

void WiFi_setup();
void WiFi_handle();

#endif /* WIRELESS_H_ */
