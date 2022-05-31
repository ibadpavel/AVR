//==========================================================================================
#include "usart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
//----------------------------------------------------------------------------------------------
uint8_t USART::bufIndex;
char USART::buffer[RX_MAX];
//----------------------------------------------------------------------------------------------
ISR (USART_RXC_vect)
{
	USART::usart_push_char(UDR);
}

USART::USART()
{
	bufIndex = 0;
}

void USART::usart_init(uint16_t speed)
{  
	// Baud rate
  uint16_t ubrr = (F_CPU / 4 / speed - 1) / 2;

	// set baud rate
	UBRRH = ubrr >> 8;            // MS byte
	UBRRL = ubrr;                 // LS byte
	UCSRA = 1 << U2X;							// Double speed
   
	UCSRB |= (1 << RXEN);					// Enable receiver
	UCSRB |= (1 << TXEN);					// Enable transmitter
	UCSRB |= (1 << RXCIE);				// Enable recive interrupt
  UCSRB &= ~(1 << UDRIE);
	// Setting sending format: 8 data bits, 1 stop bit
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void USART::usart_send_str(char *s)
{
	if(*s == 0)	{
		while ( (UCSRA & (1 << UDRE)) == 0 );	// White until UDR is not empty
		UDR = *s;
		return;
	}
	
	while (*s != 0)	{
    UCSRC = (UCSRC & ((1 << U2X) | (1 << TXC)));
		while ( (UCSRA & (1 << UDRE)) == 0 );	// White until UDR is not empty
		UDR = *s++;
	}
}

bool USART::data_available()
{
	if (USART::bufIndex == 0)
		return false;
	else
		return true;
}

void USART::usart_push_char(char s)
{
	USART::buffer[USART::bufIndex] = s;
    USART::bufIndex++;
    
	if (USART::bufIndex == RX_MAX)
		USART::bufIndex = 0;
}

void USART::usart_read_bytes(char *s, uint8_t max_size)
{
	uint8_t count = 0;
	while ((count < max_size) && (count < USART::bufIndex)) {
		s[count] = buffer[count];
   count++;
	}
	
	USART::bufIndex = 0;
}
