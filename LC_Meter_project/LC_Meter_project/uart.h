//==========================================================================================
#ifndef UART_H_
#define UART_H_
//==========================================================================================
#include <avr/io.h>
#include "lc-meter.h"
//------------------------------------------------------------------------------------------
extern void usart_init(unsigned int speed);
extern void usart_send_str(char *s);
// Add
// ISR ( USART_RXC_vect)
// {}
// To your code!!!
#endif /* UART_H_ */
