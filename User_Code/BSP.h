#ifndef BSP_H_
#define BSP_H_

#include "stm32g4xx_hal.h"
#include "stdint.h"


#define 	TRUE		1
#define 	FALSE		0



int uart_transmit_dma(uint8_t *pBuffer, uint16_t buffer_size);

float get_temperature();
int set_dac_voltage(float voltage);
float get_voltage();
float get_current();

// LED functions
void led_all_on();
void led_all_off();
void led_pc_on();
void led_pc_off();
void led_run_on();
void led_run_off();
void led_current_indication(uint8_t index);

#endif /* BSP_H_ */
