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


#ifndef DEFS_H
#define DEFS_H

#include <Arduino.h>

#define RADIO_GPIO_0      14
#define RADIO_GPIO_1      27
#define RADIO_GPIO_2      25
#define RADIO_GPIO_3      33 
#define RADIO_NIRQ        26  
#define RADIO_SDN         32  
#define RADIO_SEL         VSPI_CS0  //  SPI CS
#define RADIO_CTS         digitalRead(RADIO_GPIO_1)
#define RADIO_READY       RADIO_CTS
#define RADIO_CCA         digitalRead(RADIO_NIRQ)
#define RADIO_SIGNAL      (1 & RADIO_CCA)

#define PH_DATA_CLK_PIN   RADIO_GPIO_2
#define PH_DATA_PIN       RADIO_GPIO_3
 
#define VSPI_CLK          18
#define VSPI_CS0          5
#define VSPI_MISO         19
#define VSPI_MOSI         23

#define PWR_LED           21
#define CONNECTION_LED    4

#define DEBUG             Serial2
#define NMEA_0183_HS      Serial

#define BAUD_DEBUG          38400
#define BAUD_NMEA_0183_HS   38400

#define RXD2 16 // NC
#define TXD2 17

#endif /* DEFS_H_ */
