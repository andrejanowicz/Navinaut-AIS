/*
 * NMEA library. Encodes raw AIS data from FIFO and sends through UART
 * Author: Adrian Studer
 * License: CC BY-SA Creative Commons Attribution-ShareAlike
 *          https://creativecommons.org/licenses/by-sa/4.0/
 *          Please contact the author if you want to use this work in a closed-source project
 *          
 *  Modified and ported to ESP32 Arduino by Andre Janowicz - <andre.janowicz@gmail.com>
 *  github.com/andrejanowicz/Navinaut-AIS
 */


#ifndef NMEA_H_
#define NMEA_H_

void nmea_process_packet(void);     // create nmea sentences from current message in FIFO
extern bool received_NMEA_package;
// functions to test NMEA operation
#ifdef TEST
uint8_t test_nmea_verify_packet(const char* message); // verify if packet in FIFO is identical with in NMEA encoded message
#endif

#endif /* NMEA_H_ */
