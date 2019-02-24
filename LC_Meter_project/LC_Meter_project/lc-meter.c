#include "lc-meter.h"
#include <string.h>
//------------------------------------------------------------------------------------
inline	static void	port_init(void);		//
static void	check_battery(void);	//
void	measure(void);				//
//void	display_data(void);					//
//void	process_key(void);					//
//void	tune_zero(void);					//
//------------------------------------------------------------------------------------
volatile uint8_t		Timer0, Timer1;
volatile uint16_t		count;
//------------------------------------------------------------------------------------
// USART прерывание принятия пакета
ISR (USART_RXC_vect)
{
	
}
// Timer0 прерывание переполнения
ISR (TIMER0_OVF_vect)
{
	Timer0++;
}
// Timer1 прерывание переполнения
ISR (TIMER1_OVF_vect)
{
	Timer1++;
}

int	main(void)
{
	port_init();							// Настраиваем порты
	usart_init(57600);
	sei();
	
	Timer0 = 0;
	Timer1 = 0;
	
	usart_send_str("UART ready!\r");
	while(1)
	{
		_delay_ms(1000);
		check_battery();
		
		measure();
		char str[6] = {"______\n\r"};
		char v_str[6];
		itoa(count, v_str, 10);
		memcpy(str, v_str, 6);
		usart_send_str(str);
	}
}
//------------------------------------------------------------------------------------
inline static void port_init(void)
{
	D_DDR = 0;								// Настроить порт как вход 
	D_PORT = 0;								// Выставить режим Hi-Z (высокоимпедансный)
	/*
	DDRB |= (1<<EN_C)|(1<<EN_L);	// ¬ыходы

	PORTC |= (1<<B_MODE)|(1<<B_ZERO);	//  нопки на подт¤жке
	DDRC &= ~(1<<B_MODE)|(1<<B_ZERO);
	//OFF_RESET;
	*/

}
//------------------------------------------------------------------------------------
inline static void check_battery(void)
{

	ADC_INIT;
	ADC_START;
	_delay_ms(100);
	
	while(ADC_BUSY);					// Ждем пока завершится измерение
	
	int value = ADCH<<2;				// Дополняем двумя незначащими битами вместо чтения ADCL
	ADC_STOP;
	
	double v = 5.7 * value * (5.0 / 1023);		// 5.7 коэффициент делителя напряжения
	
	char str[] = {"Vbat: ___V "};
	char v_str[3];
	dtostrf(v, 2, 1, v_str);
	memcpy(str + 6, v_str, 3);
	usart_send_str(str);
}
//------------------------------------------------------------------------------------
void measure(void)
{
	TIMER0_INIT;
	TIMER0_START;
	
	_delay_ms(25);
	
	TIMER0_STOP;

	count = TCNT0;
}
//------------------------------------------------------------------------------------