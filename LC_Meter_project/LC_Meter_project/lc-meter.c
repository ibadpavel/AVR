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
// USART ���������� �������� ������
ISR (USART_RXC_vect)
{
}
// Timer0 ���������� ������������
ISR (TIMER0_OVF_vect)
{
	Timer0++;
}
// Timer1 ���������� ������������
ISR (TIMER1_OVF_vect)
{
	Timer1++;
}

int	main(void)
{
	port_init();							// ����������� �����
	usart_init(57600);						// ����������� USART
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
	// ��������� ����� �������\��������1
	DDRD &= ~(1<<PD5);						// ��� PD5 ��������� ��� ����
	PORTD &= ~(1<<PD5);						// ���������� ���� Hi-Z (�����������������)
	DDRD &= ~(1<<PD2);						// DDRD2 ����
	PORTD |= (1<<PD2);						// ������ � ���������
	// ��������� ������ �������
	//D_DDR = 0;								// ��������� ���� ��� ���� 
	//D_PORT = 0;								// ��������� ����� Hi-Z (�����������������)
	
	/*
	DDRB |= (1<<EN_C)|(1<<EN_L);	// ������

	PORTC |= (1<<B_MODE)|(1<<B_ZERO);	// ������ �� �������
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
	
	while(ADC_BUSY);					// ���� ���� ���������� ���������
	
	int value = ADCH<<2;				// ��������� ����� ����������� ������ ������ ������ ADCL
	ADC_STOP;
	
	double v = 5.7 * value * (5.0 / 1023);		// 5.7 ����������� �������� ����������
	
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