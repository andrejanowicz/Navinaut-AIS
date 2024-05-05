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


#ifndef LED_H_
#define LED_H_

#include <Arduino.h>
#include "defs.h"

extern uint32_t last_tick;
extern bool AIS_flash_pending;
extern uint8_t LED_pulse_length;

void LED_setup(void);
void LED_power_cycle(void);
void LED_PWR_error_flash(void);
void LED_handle();

#endif /* LED_H_ */
