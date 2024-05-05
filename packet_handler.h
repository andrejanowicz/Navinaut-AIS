/*
 * AIS packet handler for MSP430 + Si4362
 * Author: Adrian Studer
 * License: CC BY-SA Creative Commons Attribution-ShareAlike
 *          https://creativecommons.org/licenses/by-sa/4.0/
 *          Please contact the author if you want to use this work in a closed-source project
 *          
 *  Modified and ported to ESP32 Arduino by Andre Janowicz - <aj@main-void.de>
 *  github.com/andrejanowicz/Navinaut-AIS
 */
 
 
#ifndef PACKET_HANDLER_H_
#define PACKET_HANDLER_H_

void IRAM_ATTR modemdata_handle(void);

// functions to manage packet handler operation
void ph_setup(void);        // setup packet handler, e.g. configuring input pins
void ph_start(void);        // start receiving packages
void ph_stop(void);         // stop receiving packages
void IRAM_ATTR packet_handle();

// packet handler states
enum PH_STATE {
  PH_STATE_OFF = 0,
  PH_STATE_RESET,         // reset/restart packet handler
  PH_STATE_WAIT_FOR_SYNC,     // wait for preamble (010101..) and start flag (0x7e)
  PH_STATE_PREFETCH,        // receive first 8 bits of packet
  PH_STATE_RECEIVE_PACKET     // receive packet payload
};

// packet handler errors
enum PH_ERROR {
  PH_ERROR_NONE = 0,
  PH_ERROR_STUFFBIT,    // invalid stuff-bit
  PH_ERROR_NOEND,     // no end flag after more than 1020 bits, message too long
  PH_ERROR_CRC,     // CRC error
  PH_ERROR_RSSI_DROP    // signal strength fell below threshold
};

uint8_t ph_get_state(void);     // get current state of packet handler
uint8_t ph_get_last_error(void);  // get last packet handler error, will clear error
uint8_t ph_get_radio_channel(void); // get current radio channel
int16_t ph_get_radio_rssi(void);  // get RSSI in dBm at last sync
uint8_t ph_get_message_type(void);  // get last AIS message type
extern volatile uint8_t newData;


#endif /* PACKET_HANDLER_H_ */
