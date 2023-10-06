#ifndef __USART3_H
#define __USART3_H

#include <stdio.h>

#define RX_DATA_SIZE 200

void usart3_init(void);
void usart3_send_byte(uint8_t byte);
void usart3_send_array(uint8_t *array, uint16_t length);
void usart3_send_string(const char *string);
void usart3_send_number(uint32_t number, uint8_t length);
void usart3_printf(char *format, ...);

uint8_t usart3_get_rx_count(void);
const uint8_t* usart3_get_rx_data(void);

#endif
