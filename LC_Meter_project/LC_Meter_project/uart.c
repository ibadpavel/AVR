//==========================================================================================
#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
//------------------------------------------------------------------------------------------
/*
 * usart initialize
 */
void usart_init(unsigned int speed)
{
	// Baud rate
	uint16_t ubrr = F_CPU/16/speed-1;
	//------------------------------------------------------------------------------------------
	// set baud rate
	UBRRH = (unsigned char)(ubrr >> 8);		// MS байт
	UBRRL = (unsigned char) ubrr;			// LS байт
	UCSRA = 0x00;							// Reset all flags
	UCSRB |= (1 << RXEN);					// Enable receiver
	UCSRB |= (1 << TXEN);					// Enable transmitter
	UCSRB |= (1 << RXCIE);					// Enable recive interrupt
	// Setting sending format: 8 data bits, 1 stop bit
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

/*
 * Send string or char* to usart
 */
void usart_send_str(char *s)
{
	if(*s == 0)
	{
		while ( (UCSRA & (1 << UDRE)) == 0 );	// White until UDR is not empty
		UDR = *s;
		return;
	}
	
	while (*s != 0)
	{
		while ( (UCSRA & (1 << UDRE)) == 0 );	// White until UDR is not empty
		UDR = *s++;
	}
}
