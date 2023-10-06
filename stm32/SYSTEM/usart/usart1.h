#ifndef __USART1_H
#define __USART1_H

#include <stdio.h>

#define RX_DATA_SIZE 200

void usart1_init(void);
void usart1_send_byte(uint8_t byte);
void usart1_send_array(uint8_t *array, uint16_t length);
void usart1_send_string(char *string);
void usart1_send_number(uint32_t number, uint8_t length);
void usart1_printf(char *format, ...);

void usart1_send_u32(uint32_t data);

uint8_t usart1_get_rx_count(void);
const uint8_t* usart1_get_rx_data(void);

#endif
