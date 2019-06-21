#include "stdio.h"
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
volatile uint16_t		count, count1;
volatile uint32_t		freq;
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
	usart_init(57600);						// Настраиваем USART
	sei();
	
	Timer0 = 0;
	Timer1 = 0;
	
	usart_send_str("UART ready!\r");
	while(1)
	{
		_delay_ms(1000);
		check_battery();
		
		measure();
		count += 255 * Timer0;
		Timer0 = 0;
		freq = (Timer1 * 65535 + count1) * 4;
		Timer1 = 0;
		char v_str[8];
		sprintf(v_str, "%06lu", freq);
		usart_send_str(v_str);
		usart_send_str("\r\n");
	}
}
//------------------------------------------------------------------------------------
inline static void port_init(void)
{
	// Настройка входа Таймера\Счётчика1
	DDRD &= ~(1<<PD5);						// Пин PD5 настроить как вход
	PORTD &= ~(1<<PD5);						// Установить вход Hi-Z (высокоимпедансный)
	DDRD &= ~(1<<PD2);						// DDRD2 вход
	PORTD |= (1<<PD2);						// Кнопка с подтяжкой
	// Настройка портов дисплея
	//D_DDR = 0;								// Настроить порт как вход 
	//D_PORT = 0;								// Выставить режим Hi-Z (высокоимпедансный)
	
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
	/*
	TIMER0_INIT;
	TIMER0_START;
	_delay_ms(25);
	TIMER0_STOP;
	count = TCNT0;
	*/
	//---------------------------------------
	TIMER1_INIT;
	TIMER1_START;

	_delay_ms(250);
	
	TIMER1_STOP;
	count1 = TCNT1L;
	count1 = count1 | (TCNT1H<<8);
}
//------------------------------------------------------------------------------------