//==========================================================================================
#ifndef USART_H_
#define USART_H_
//==========================================================================================
#include <avr/io.h>
//------------------------------------------------------------------------------------------
#define RX_MAX 128

class USART {
public:
	USART();
	static void usart_init(uint16_t speed);
	static void usart_send_str(char *s);
  static void usart_bytes_array(char *s, uint16_t size);
	static bool data_available();
	static void usart_push_char(char s);
	static void usart_read_bytes(char *s, uint8_t max_size);
	
private:
	static uint8_t bufIndex;
	static char buffer[RX_MAX];
};

#endif /* USART_H_ */
