/*
   Library to control Silicon Laboratories SI4362 radio
   Author: Adrian Studer
   License: CC BY-SA Creative Commons Attribution-ShareAlike
            https://creativecommons.org/licenses/by-sa/4.0/
            Please contact the author if you want to use this work in a closed-source project

 *  Modified and ported to ESP32 Arduino by Andre Janowicz - <aj@main-void.de>
 *  github.com/andrejanowicz/Navinaut-AIS
 */


#include <Arduino.h>
#include <SPI.h>
#include "radio.h"
#include "radio_config.h"
#include "defs.h"
#include "LED.h"

#define CMD_NOP                   0x00
#define CMD_PART_INFO             0x01
#define CMD_POWER_UP              0x02
#define CMD_FUNC_INFO             0x10
#define CMD_SET_PROPERTY          0x11
#define CMD_GET_PROPERTY          0x12
#define CMD_GPIO_PIN_CONFIG       0x13
#define CMD_GET_ADC_READING       0x14
#define CMD_FIFO_INFO             0x15
#define CMD_PACKET_INFO           0x16
#define CMD_IRCAL                 0x17
#define CMD_PROTOCOL_CFG          0x18
#define CMD_GET_INT_STATUS        0x20
#define CMD_GET_PH_STATUS         0x21
#define CMD_GET_MODEM_STATUS      0x22
#define CMD_GET_CHIP_STATUS       0x23
#define CMD_START_RX              0x32
#define CMD_REQUEST_DEVICE_STATE  0x33
#define CMD_CHANGE_STATE          0x34
#define CMD_RX_HOP                0x36
#define CMD_READ_CMD_BUFF         0x44
#define CMD_FRR_A_READ            0x50
#define CMD_READ_RX_FIFO          0x77

#define SPI_ON    digitalWrite(RADIO_SEL, LOW);
#define SPI_OFF   digitalWrite(RADIO_SEL, HIGH);

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;// functions to test NMEA operation

#ifdef TEST
uint8_t test_nmea_verify_packet(const char* message); // verify if packet in FIFO is identical with in NMEA encoded message
#endif

/* dbm things*/
int16_t dbm = 0;
const uint8_t m = 25;
int array[m+1], pos = 0, location = 0;

// radio configuration as generated by WDS and stored in radio_config.h
const uint8_t radio_configuration[] = RADIO_CONFIGURATION_DATA_ARRAY;

// radio image rejection calibration sequence (as per datasheet)
const uint8_t radio_ircal_sequence_coarse[] = { 0x56, 0x10, 0xCA, 0xF0 };
const uint8_t radio_ircal_sequence_fine[] = { 0x13, 0x10, 0xCA, 0xF0 };

union radio_buffer_u radio_buffer;

static void send_command(uint8_t cmd, const uint8_t *send_buffer, uint8_t send_length, uint8_t response_length);
static int receive_result(uint8_t length);

// configure I/O pins used for radio
void radio_setup(void)
{

  //initialise instance of the SPIClass attached to VSPI
  vspi = new SPIClass(VSPI);

  //initialise vspi w default pins
  vspi->begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_CS0);

  //set up slave select pins as outputs as the Arduino API
  pinMode(RADIO_SEL, OUTPUT);           // VSPI SS doesn't handle automatically pulling SS low
  pinMode(RADIO_GPIO_1, INPUT_PULLUP);  // initialize CTS pin as input
  pinMode(RADIO_NIRQ, INPUT);           // initialize CTS pin as input
  pinMode(RADIO_SDN, OUTPUT);           // initialize shutdown pin as output
  return;
}

inline uint8_t spi_transfer(const uint8_t data)
{
  vspi->beginTransaction(SPISettings(VSPI_CLK, MSBFIRST, SPI_MODE0));
  SPI_ON;
  vspi->transfer(data);
  vspi->endTransaction();
  SPI_OFF;

  return 1;
}

// shut down radio, call radio_configure to restart
void radio_shutdown(void)
{
  digitalWrite(RADIO_SDN, HIGH); // SDN high = turn off radio
}

// reset radio and load configuration data
void radio_configure(void)
{
  // reset radio: SDN=1, wait >1us, SDN=0
  digitalWrite(RADIO_SDN, HIGH);
  delayMicroseconds(2);
  digitalWrite(RADIO_SDN, LOW);

  while (!RADIO_READY) {
    LED_PWR_error_flash(); // wait for chip to wake up
  }

  const uint8_t *cfg = radio_configuration; // transfer radio configuration
  while (*cfg)  {                           // configuration array stops with 0

    char count = (*cfg++) - 1;              // 1st byte: number of bytes, incl. command
    char cmd = *cfg++;                      // 2nd byte: command
    send_command(cmd, cfg, count, 0);       // send bytes to chip
    cfg += count;                           // point at next line
    while (!RADIO_READY) {
      LED_PWR_error_flash();        // wait for chip to complete operation
    }
  }
  return;
}

// directly set individual radio property
void radio_set_property(uint8_t prop_group, uint8_t prop_num, uint8_t value)
{
  radio_buffer.data[0] = prop_group;
  radio_buffer.data[1] = 1;
  radio_buffer.data[2] = prop_num;
  radio_buffer.data[3] = value;
  send_command(CMD_SET_PROPERTY, radio_buffer.data, 4, 0);
  return;
}

// invoke radio image rejection self-calibration
void radio_calibrate_ir(void)
{
  // send calibration sequence as per datasheet - doing it wrong, read AN790
  // note: looking at si446x_ircal.c in provided examples, CMD_IRCAL has 3 undocumented return values CAL_STATE, RSSI, DIR_CH
  // note2: don't use this method, rely on WDS to generate proper calibration sequence
  radio_start_rx(0, 0, 0, 0, 0, 0);
  send_command(CMD_IRCAL, radio_ircal_sequence_coarse, sizeof(radio_ircal_sequence_coarse), 0);
  send_command(CMD_IRCAL, radio_ircal_sequence_fine, sizeof(radio_ircal_sequence_fine), 0);

  while (!RADIO_READY) {
    ;;    // wait for calibration to complete
  }

}

// retrieve radio device information (e.g. chip model)
void radio_part_info(void)
{
  send_command(CMD_PART_INFO, 0, 0, sizeof(radio_buffer.part_info));
}

// retrieve radio function revision information
void radio_func_info(void)
{
  send_command(CMD_FUNC_INFO, 0, 0, sizeof(radio_buffer.func_info));
}

// retrieve radio FIFO information
void radio_fifo_info(uint8_t reset_fifo)
{
  radio_buffer.data[0] = reset_fifo;
  send_command(CMD_FIFO_INFO, radio_buffer.data, 1, sizeof(radio_buffer.fifo_info));
}

// retrieve radio interrupt status information
void radio_get_int_status(uint8_t ph_clr_pending, uint8_t modem_clr_pending, uint8_t chip_clr_pending)
{
  radio_buffer.data[0] = ph_clr_pending;
  radio_buffer.data[1] = modem_clr_pending;
  radio_buffer.data[2] = chip_clr_pending;
  send_command(CMD_GET_INT_STATUS, radio_buffer.data, 3, sizeof(radio_buffer.int_status));
}

// retrieve radio packet handler status information
void radio_get_ph_status(uint8_t clr_pending)
{
  radio_buffer.data[0] = clr_pending;
  send_command(CMD_GET_PH_STATUS, radio_buffer.data, 1, sizeof(radio_buffer.ph_status));
}

// retrieve radio chip status information
void radio_get_chip_status(uint8_t clr_pending)
{
  radio_buffer.data[0] = clr_pending;
  send_command(CMD_GET_CHIP_STATUS, radio_buffer.data, 1, sizeof(radio_buffer.chip_status));
}

// retrieve radio modem status information
void radio_get_modem_status(uint8_t clr_pending)
{
  radio_buffer.data[0] = clr_pending;
  send_command(CMD_GET_MODEM_STATUS, radio_buffer.data, 1, sizeof(radio_buffer.modem_status));
}

// read fast read registers, results in radio_buffer.data[0..3], frr = start register 'A'..'D', count = # of values
void radio_frr_read(uint8_t frr, uint8_t count)
{
  while (!RADIO_READY);         // always wait for radio to be ready (CTS) before sending next command

  // implemented directly as FRR access has no CTS handshake
  SPI_ON;

  spi_transfer(CMD_FRR_A_READ + ((frr - 1) & 0x03)); // transmit command, start register

  uint8_t i = 0;
  while (i < count) {
    radio_buffer.data[i] = vspi->transfer(0);   // receive bytes
    i++;
  }

  SPI_OFF;
}

// put radio in receive state
void radio_start_rx(uint8_t channel, uint8_t start_condition, uint16_t rx_length, uint8_t rx_timeout_state, uint8_t rx_valid_state, uint8_t rx_invalid_state)
{
  radio_buffer.data[0] = channel;
  radio_buffer.data[1] = start_condition;
  radio_buffer.data[2] = rx_length >> 8;
  radio_buffer.data[3] = rx_length & 0xff;
  radio_buffer.data[4] = rx_timeout_state;
  radio_buffer.data[5] = rx_valid_state;
  radio_buffer.data[6] = rx_invalid_state;
  send_command(CMD_START_RX, radio_buffer.data, 7, 0);
}

// wait for radio to complete previous command
void radio_wait_for_CTS(void)
{
  while (!RADIO_READY) {
    ;;      // wait for radio to be ready before returning
  }
}

// read radio state
void radio_request_device_state(void)
{
  send_command(CMD_REQUEST_DEVICE_STATE, 0, 0, sizeof(radio_buffer.device_state));
}

// change radio state, e.g. from receive to ready
void radio_change_state(uint8_t next_state)
{
  radio_buffer.data[0] = next_state;
  send_command(CMD_CHANGE_STATE, radio_buffer.data, 1, 0);
}

// read various radio status and information registers
void radio_debug(void)
{
  radio_get_int_status(0, 0, 0);
  radio_get_chip_status(0);
  radio_get_modem_status(0);
  radio_part_info();
  radio_func_info();
  radio_request_device_state();
}

// send command, including optional parameters if sendBuffer != 0
void send_command(uint8_t cmd, const uint8_t *send_buffer, uint8_t send_length, uint8_t response_length)
{

  while (!RADIO_READY) {
    ;;    // always wait for radio to be ready (CTS) before sending next command
  }

  SPI_ON;

  vspi->transfer(cmd);            // transmit command

  if (send_length && send_buffer) {   // if there are parameters to send, do so
    uint8_t c = 0;
    while (c != send_length) {
      vspi->transfer(send_buffer[c]); // transmit byte
      c++;
    }

  }
  SPI_OFF;

  if (response_length) {
    while (!RADIO_READY) {
      ;; // wait for radio to be ready before retrieving response
    }
    while (receive_result(response_length) == 0) {
      ;;  // wait for valid response
    }
  }
  return;
}

int receive_result(uint8_t length)
{
  SPI_ON;
  vspi->transfer(CMD_READ_CMD_BUFF);    // send 0x44 to receive data/CTS
  uint16_t val = vspi->transfer(0);     //0xff is a dummy

  if ((val) != 0xff)
  { // if CTS is not 0xff
    SPI_OFF;
    return 0;                       // data not ready yet
  }

  uint8_t i = 0;              // data ready, read result into buffer
  while (i < length) {
    radio_buffer.data[i] = vspi->transfer(0);   // receive byte
    if (i == 2) {
      int16_t value = RADIO_RSSI_TO_DBM(radio_buffer.data[i]);
      array[pos++] = value;
      pos %= m;
      for (int i = 0; i < m; i++) {
        if (array[i] > array[i + 1]) {
          location = i;
        }
      }
      dbm = array[location];

    }
    i++;
  }

  SPI_OFF
  return 1;           // data received
}
