#include "stdio.h"
#include "lc-meter.h"
#include "fixPointNum.h"
#include "sed1565.h"
#include <string.h>
//------------------------------------------------------------------------------------
inline	static void	port_init(void);		//
static void	check_battery(void);	//
void calibration(void);
void	measure(void);				//
void	display_menu(void);					//
//void	display_data(void);					//
//void	process_key(void);					//
//void	tune_zero(void);					//
//------------------------------------------------------------------------------------
struct fPointval
{
	uint32_t value;
	uint8_t fPoint;
};

fixPointReal calibration_coeff;
volatile uint8_t		Timer0, Timer1;
volatile uint16_t		count0, count1;
volatile uint32_t		base_freq, measure_freq;
volatile uint32_t		time_m;
//menu
volatile uint8_t currentMenuIndex = 1;
volatile uint8_t acceptedMenuIndex = 0;
// bit 1 - button flag
// bit 2 - need calibration
volatile uint8_t flag = 0b00000001; 
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
	
	init_display();
	lcd_swap();
	
	sei();
	
	Timer0 = 0;
	Timer1 = 0;
	
	usart_send_str("UART ready!\r\n");
	print_st("Hello!", 7, 0, 0);
	print_st("Dispay is work!", 16, 1, 0);
	//calibration();
	//-------------------------------
	char esc = 13;
	while(1)
	{
		usart_send_str(&esc);
		_delay_ms(100);
		check_battery();
		
		if ((PIND & (1<<PD2)) == 0) {
			_delay_ms(10);
			if ((PIND & (1<<PD2)) == 0 && (flag & 0x1) != 0) {
				flag &= ~0x1;
				currentMenuIndex--;
				if (currentMenuIndex < 1)
					currentMenuIndex = 1;
			}
		}
		if ((PIND & (1<<PD3)) == 0) {
			_delay_ms(10);
			if ((PIND & (1<<PD3)) == 0) {
				flag &= ~0x1;
				acceptedMenuIndex = currentMenuIndex;
			}
		}
		if ((PIND & (1<<PD4)) == 0) {
			_delay_ms(10);
			if ((PIND & (1<<PD4)) == 0 && (flag & 0x1) != 0) {
				flag &= ~0x1;
				currentMenuIndex++;
				if (currentMenuIndex > 3)
				currentMenuIndex = 3;
			}
		}
		
		if ((PIND & (1<<PD2)) != 0 &&
			(PIND & (1<<PD3)) != 0 &&
			(PIND & (1<<PD4)) != 0) {
			  flag |= 0x1;
		  }
		
		display_menu();
		
		if (acceptedMenuIndex == 3) {
			calibration();
			acceptedMenuIndex = 1;
		}
		
		measure();
		//time_m = count0 + 255 * Timer0;
		//Timer0 = 0;
		measure_freq = (Timer1 * 65535 + count1) * 4;
		Timer1 = 0;
		
		char str[] = {"Freq: ________"};
		char v_str[8];
		sprintf(v_str, "%lu", measure_freq);
		memcpy(str + 6, v_str, 7);
		print_st(str, 15, 4, 0);
		/*
		fixPointReal coeff_freq = fprDivide(base_freq, measure_freq);
		coeff_freq = fprMultiple(coeff_freq, coeff_freq);
		uint32_t measure_cap = abs(fprGetTotal(coeff_freq * IN_CAPACITY) - IN_CAPACITY);
		
		{
			char fred_str[15];
			sprintf(fred_str, "Measure: %lu.%03lu pF", fprGetTotal(calibration_coeff * measure_cap), fprGetFract(calibration_coeff * measure_cap));
			usart_send_str(fred_str);
			usart_send_str("\r\n");
		}*/
	}
}
//------------------------------------------------------------------------------------
inline static void port_init(void)
{
	// Настройка входа Таймера\Счётчика1
	DDRD &= ~(1<<PD5);						// PD5 пин настроить как вход
	PORTD &= ~(1<<PD5);						// PD5 установить вход Hi-Z (высокоимпедансный)
	// Настройка портов кнопки Down
	DDRD &= ~(1<<PD2);						// DDRD2 вход
	PORTD |= (1<<PD2);						// Кнопка с подтяжкой
	// Настройка портов кнопки Ok
	DDRD &= ~(1<<PD3);						// DDRD3 вход
	PORTD |= (1<<PD3);						// Кнопка с подтяжкой
	// Настройка портов кнопки Up
	DDRD &= ~(1<<PD4);						// DDRD4 вход
	PORTD |= (1<<PD4);						// Кнопка с подтяжкой
	
	// Выход на реле измерения С
	DDRB |= (1<<PB4);
	PORTB &= ~(1<<PB4);
	// Выход на реле измерения калибровки
	DDRB |= (1<<PB5);
	PORTB &= ~(1<<PB5);
}
//------------------------------------------------------------------------------------
inline static void check_battery(void)
{
	ADC_INIT;
	ADC_START;

	while(ADC_BUSY);					// Ждем пока завершится измерение
	
	int value = ADCH<<2;				// Дополняем двумя незначащими битами вместо чтения ADCL
	ADC_STOP;
	
	// 5.7 коэффициент делителя напряжения
	struct fPointval v_bat;
	v_bat.value = (uint32_t)value * 5 * 57 / 1023;
	v_bat.fPoint = 1;	
	
	char str[] = {"Vbat: __._V\r\n"};
	char v_str[3];
	sprintf(v_str, "%lu", v_bat.value);
	memcpy(str + 6, v_str, 2);
	memcpy(str + 9, v_str + 2, 1);
	usart_send_str(str);
	
	print_st(str, 12, 3, 0);
}
//------------------------------------------------------------------------------------
void calibration(void)
{
	// Read calibration values
	measure();
	base_freq = (Timer1 * 65535 + count1) * 4;
	Timer1 = 0;
	
	C_RELAY_ON;
	CALIB_RELAY_ON;
	_delay_ms(20);			// Ждём пока замкнётся реле и антидребезг
	measure();
	CALIB_RELAY_OFF;
	C_RELAY_OFF;
	measure_freq = (Timer1 * 65535 + count1) * 4;
	Timer1 = 0;
	
	
	fixPointReal coeff_freq = fprDivide(base_freq, measure_freq);
	coeff_freq = fprMultiple(coeff_freq, coeff_freq);
	uint32_t measure_cap = fprGetTotal(coeff_freq * IN_CAPACITY) - IN_CAPACITY;
	calibration_coeff = fprDivide(CALIBRATE_CAPACITY, measure_cap);
	// Debug
	{
		char fred_str[15];
		sprintf(fred_str, "Calibration measure: %lu.%03lu pF", fprGetTotal(calibration_coeff * measure_cap), fprGetFract(calibration_coeff * measure_cap));
		usart_send_str(fred_str);
		usart_send_str("\r\n");
	}
}
//------------------------------------------------------------------------------------
void measure(void)
{
	//TIMER0_INIT;
	TIMER1_INIT;
	//TIMER0_START;
	TIMER1_START;

	_delay_ms(250);
	
	//TIMER0_STOP;
	TIMER1_STOP;
	//count0 = TCNT0;
	count1 = TCNT1L;
	count1 = count1 | (TCNT1H << 8);
}
//------------------------------------------------------------------------------------
void display_menu(void)
{
	// 1 - cap
	// 2 - ind
	// 3 - calib
	
	if (acceptedMenuIndex == 1) {
		C_RELAY_ON;
		usart_send_str(">>");
	}
	else if (currentMenuIndex == 1) {
		C_RELAY_OFF;
		usart_send_str(">");
	} else
		C_RELAY_OFF;
	usart_send_str("Cap\r\n");
	
	if (acceptedMenuIndex == 2) {
		C_RELAY_OFF;
		usart_send_str(">>");
	}
	else if (currentMenuIndex == 2)
		usart_send_str(">");
	usart_send_str("Ind\r\n");
	
	if (acceptedMenuIndex == 3) {
		usart_send_str(">>");
	}
	else if (currentMenuIndex == 3)
		usart_send_str(">");
	usart_send_str("Calib\r\n");
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------