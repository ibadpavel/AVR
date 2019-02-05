#define F_CPU				4000000UL			// CPU clock frequency



//#include <avr/iom8.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <compat/deprecated.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <float.h>

#include	"LC-meter-mega8.h"

#include	"lc-lcd.c"

//----------------------------------------------------------------------

inline	static void	port_init(void);
inline	static void	check_battery(void);

void	start_measure(void);
void	display_data(void);
void	process_key(void);
void	tune_zero(void);

//----------------------------------------------------------------------

//uint8_t	StringCAP[] = "Capacitor";
//uint8_t	StringIND[] = "Inductance";
//uint8_t StringRES[] = "Resistance";

uint8_t	StringCAP[] = "C";
uint8_t	StringIND[] = "L";
uint8_t StringRES[] = "R";

//uint8_t	s_OK[] = "OK";
//uint8_t	s_NO[] = "NO";
uint8_t	StringLowBattery[] = "LOW";

uint8_t	ErrorString[] = "Error";

uint32_t	MeasuredTicks;
uint16_t	MeasuredPeriods;
//volatile static uint16_t	PeriodCounter, TimeStamp;
//volatile uint8_t	first;
//uint16_t	CParam = 820;
uint16_t	CParam = (40<<8);
uint16_t	CConst = (10<<8);
uint16_t	LParam;
uint16_t	RParam;
uint16_t		TParamInt;
uint16_t 	TParamFract;

uint32_t	Tmp;
uint32_t	Tmp2;
//uint64_t	Tmp;
//uint64_t	Tmp2;


uint8_t		PressMode, PressZero;
uint8_t		Mode, ChangedMode;

uint8_t		FlagReg, Error;
uint8_t		Sym;

volatile uint8_t		Timer0, Timer1;

//--------------------------------------------------------------------**


int	main(void)
{
uint16_t	Mask;
int8_t		Point, Shift;
unsigned char	buffer[20];
uint8_t		TmpLocal, BigCap;

	Error = 0;
	port_init();

	lcd_init();

//	TIMER1_INIT; 
//	TIMER1_ZERO;
    TIMSK |= (1<<TOIE1);											// прерывание по переполнению T/C1 разрешено
    TCCR1A=(0<<COM1A1)|(0<<COM1A0)|(0<<WGM11)|(0<<WGM10); 			// выключить Ў»ћ
    TCCR1B=(1<<ICES1);												// ICR1 - по нарастающему (ICES1=1) фронту сигнала
    TCNT1 = 0;														// счЄтный регистр в 0

	TIMER0_INIT;													// прерывание по переполнению T/C0 разрешено

	check_battery();

	lcd_puts("Start..."); _delay_ms(1000);

	PressMode = PressZero = 0;
	Mode = MODE_MEASURE_CAP;
	ChangedMode = 1;
	FlagReg = 0;

	sei();
	while(1)
	{
		if ( Mode == MODE_MEASURE_CAP && ChangedMode )
		{
			DISABLE_LR;
			ENABLE_CAP;
			lcd_clear();
			lcd_gotoxy(0,0);
			lcd_puts(StringCAP);
			ChangedMode = 0;
		}
		else if ( ChangedMode ) 
		{
			DISABLE_CAP;
			ENABLE_LR;
			lcd_clear();
			lcd_gotoxy(0,0);
			if ( Mode == MODE_MEASURE_IND )
				lcd_puts(StringIND);
			else if ( Mode == MODE_MEASURE_RES )
				lcd_puts(StringRES);

			ChangedMode = 0;
		}
//		_delay_ms(50);
		start_measure();
/*
		while ( !FlagReg )
		{
			if ( IS_TIMER0_STOP )
			{
				process_key();
			}
		}
*/

		switch ( FlagReg )
		{
		case FLAG_MEASURE_READY:
			if ( MeasuredPeriods <=1 )
			{
				Error = 1;
				break;
			}
			Tmp = ((uint32_t)MeasuredTicks << 12 ) / MeasuredPeriods;
//			Tmp = ((uint64_t)MeasuredTicks << 12 ) / MeasuredPeriods;
			if ( Tmp < 0x01000000UL )
			{
				Tmp2 = (Tmp << 8);
				BigCap = 0;
			}
			else
			{
				Tmp2 = Tmp;
				BigCap = 1;
			}
	
	ltoa(Tmp2,buffer,10);

			Tmp = Tmp2 / CParam;
			if ( BigCap )
				Tmp <<= 8;
			Tmp2 = Tmp * CConst;
			Tmp = Tmp2 - ((uint32_t)CConst << 12);
//			Tmp = Tmp2 - ((uint64_t)CConst << 12);


	ltoa(Tmp,buffer,10);

			Tmp2 = Tmp>>20;

			if ( Tmp2 == 0 )
			{
				Tmp2 = ( ( Tmp >> 11 ) * 1000UL ) >> 9;
				Sym = 'p';
				TParamInt = Tmp2;
				TParamFract = 0;
			}
			else if ( Tmp2 < 1000 )
			{
				Sym = 'n';
				TParamInt = Tmp2;
				TParamFract = 0;
			}
			else if ( Tmp2 < 1000000UL )
			{
				Sym = 'u';
				TParamInt = Tmp2 / 1000;
				TParamFract = Tmp2 % 1000;
			}

//			FlagReg = ~(FLAG_MEASURE_READY);
			FlagReg = 0;
			break;

		case FLAG_PRESS_ZERO:
		case ( FLAG_PRESS_ZERO | FLAG_MEASURE_READY ):
			tune_zero();
//			FlagReg &= ~((FLAG_PRESS_ZERO|FLAG_MEASURE_READY));
			FlagReg = 0;
			break;

		case FLAG_PRESS_MODE:
		case ( FLAG_PRESS_MODE | FLAG_MEASURE_READY ):
			if ( Mode == MODE_MEASURE_CAP )
				Mode = MODE_MEASURE_IND;
			else if ( Mode == MODE_MEASURE_IND )
				Mode = MODE_MEASURE_RES;
			else
				Mode = MODE_MEASURE_CAP;
			ChangedMode = 1;
//			FlagReg &= ~((FLAG_PRESS_MODE|FLAG_MEASURE_READY));
			FlagReg = 0;
		break;

		default:
			break;
		}

		display_data();
	}

}
//--------------------------------------------------------------------**

inline	void	port_init(void)
{

	D_PORT = 0;
	D_DDR = 0;	// Z-state input фшЄ€ыхщэvщ €ю®™
	
	DDRB |= (1<<EN_C)|(1<<EN_L);	// ¬ыходы

	PORTC |= (1<<B_MODE)|(1<<B_ZERO);	//  нопки на подт€жке
	DDRC &= ~(1<<B_MODE)|(1<<B_ZERO);
	//OFF_RESET;


}
//--------------------------------------------------------------------**

inline	static	void	check_battery(void)
{
/*
	AC_INIT;
	AC_START;
	_delay_ms(10);
	if ( IS_AC_EVENT )
	{
		lcd_gotoxy(0,0);
		lcd_clear();
		lcd_puts(StringLowBattery);
		_delay_ms(500);
		OFF_SET;
	}
*/
}
//--------------------------------------------------------------------**

void	start_measure(void)
{
uint16_t	Start1, Stimer1, Stimer0;
uint16_t	End1, Etimer1, Etimer0;
uint8_t		Start0, End0;

	cli();
	Timer0 = Timer1 = 0;
//	PeriodCounter = 0;
	TIMER0_START_EXT;					// счёт тактов на ноге Т1
	TIMER1_START;						// таймер по тактам микроконтроллера
	TIFR |= (1<<ICF1);
	sei();
	while( bit_is_clear(TIFR,ICF1) )
		continue;
	cli();
	Start1 = ICR1;
	Start0 = TCNT0;
	Stimer0 = Timer0;
	Stimer1 = Timer1;
	sei();

	_delay_ms(25);
	TIFR |= (1<<ICF1);
	while( bit_is_clear(TIFR,ICF1) )
		continue;

	cli();
	End1 = ICR1;
	End0 = TCNT0;
	Etimer0 = Timer0;
	Etimer1 = Timer1;
	TIMER1_STOP;
	TIMER0_STOP;
	sei();

	MeasuredTicks = ( (uint32_t)(Etimer1-Stimer1)<<16 ) + End1 - Start1;
	MeasuredPeriods = ( (uint16_t)(Etimer0-Stimer0) << 8 ) + End0 - Start0;

	FlagReg |= FLAG_MEASURE_READY;
}
//--------------------------------------------------------------------**

void	display_data(void)
{
char	buffer[12], *s;

	lcd_gotoxy(0,1);
	if ( Error )
	{
		lcd_puts(ErrorString);
		return;
	}
	ltoa(TParamInt,buffer,10);
	lcd_puts((uint8_t*)buffer);
	lcd_putchar('.');
	TParamFract += 100;
	s = buffer+1;
	ltoa(TParamFract,buffer,10);
	TParamFract -= 100;
	lcd_puts((uint8_t*)s);

	lcd_putchar(Sym);
}
//--------------------------------------------------------------------**

// Timer1 overflow
ISR(SIG_OVERFLOW1)
{
	Timer1++;
}
//--------------------------------------------------------------------**

void	process_key(void)
{
	if ( bit_is_clear(I_B_MODE,B_MODE) )
		PressMode = 1;
	if ( bit_is_clear(I_B_ZERO,B_ZERO) )
		PressZero = 1;
	if ( PressZero || PressMode )
	{
		TIMER0_ZERO;
		TIMER0_INIT;
		TIMER0_START_256;
	}
}
//--------------------------------------------------------------------**

// Timer0 overflow
ISR(SIG_OVERFLOW0)
{
	Timer0++;
}
//--------------------------------------------------------------------**

void	tune_zero(void)
{
	Tmp = ((uint32_t)MeasuredTicks << 8 ) / MeasuredPeriods;
	if ( Mode == MODE_MEASURE_CAP )
		CParam = (uint16_t)Tmp;
	else if ( Mode == MODE_MEASURE_IND )
		LParam = (uint16_t)Tmp;
	if ( Mode == MODE_MEASURE_RES )
		RParam = (uint16_t)Tmp;
}
//--------------------------------------------------------------------**



