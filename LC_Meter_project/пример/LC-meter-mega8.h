#ifndef	LC_METER_H
#define LC_METER_H

//------------------------------------------------------------

// ---- buttons input ---------------------
#define B_MODE	PD6
#define B_ZERO	PB1
#define	B_START	PC2

#define	P_B_MODE	PORTD
#define	P_B_ZERO	PORTB
#define	P_B_START	PORTC

#define	I_B_MODE	PIND
#define	I_B_ZERO	PINB
#define	I_B_START	PINC

#define	D_B_MODE	DDRD
#define	D_B_ZERO	DDRB
#define	D_B_START	DDRC

// ---- clock input -----------------------
#define	T0		PD4
#define	P_T0	PORTD
#define	I_T0	PIND
#define	D_T0	DDRD

// ---- battery measure input ---------------
#define	BATTERY		PC3
#define	BAT_CHANNEL	3

// ---- calibrate output ------------------
#define TEST	PD7
#define	P_TEST	PORTD
#define	D_TEST	DDRD

// ---- power on output -------------------
#define POWER_OUT	PD5
#define	P_POWER_OUT	PORTD
#define	D_POWER_OUT	DDRD

//----- display ------------------------

#define	D_RS	PC4
#define	D_E		PC5
#define	P_D_RS	PORTC
#define	P_D_E	PORTC
#define	D_D_RS	DDRC
#define	D_D_E	DDRC
#define	DC_PORT	PORTC
#define	DC_DDR	DDRC

#define	D_D4	PD0
#define	D_D5	PD1
#define	D_D6	PD2
#define	D_D7	PD3

#define	P_D_D4	PORTD
#define	P_D_D5	PORTD
#define	P_D_D6	PORTD
#define	P_D_D7	PORTD

#define	D_D_D4	DDRD
#define	D_D_D5	DDRD
#define	D_D_D6	DDRD
#define	D_D_D7	DDRD

#define	D_DATA_MASK	((1<<D_D4)|(1<<D_D5)|(1<<D_D6)|(1<<D_D7))
#define	D_PORT	PORTD
#define	D_DDR	DDRD

#define	D_E_UP		sbi(P_D_E,D_E)
#define	D_E_DN		cbi(P_D_E,D_E)

#define	D_RS_UP		sbi(P_D_RS,D_RS)
#define	D_RS_DN		cbi(P_D_RS,D_RS)

// ------ common operations --------------------------

#define	IS_PRESS_START	bit_is_clear(I_B_START,B_START)
#define	IS_PRESS_MODE	bit_is_clear(I_B_MODE,B_MODE)
#define	IS_PRESS_ZERO	bit_is_clear(I_B_ZERO,B_ZERO)

#define	POWER_UP		sbi(P_POWER_OUT,POWER_OUT)
#define	POWER_DOWN		cbi(P_POWER_OUT,POWER_OUT)

#define	POWER_OFF		{while(1){cbi(P_POWER_OUT,POWER_OUT);}}

#define	TEST_ON			sbi(P_TEST,TEST)
#define	TEST_OFF		cbi(P_TEST,TEST)

// ------ Analog to Digital Comnvertor (ADC) ----

#define	ADC_INIT	(ADMUX = (1<<REFS1) | (1<<REFS0) | BAT_CHANNEL )

//----- настройка таймера 0 ---------------------------------------------------------------

#define	TIMER0_INIT		(TIMSK|=(1<<TOIE0))									// прерывание по переполнению T/C0 разрешено
#define	TIMER0_ZERO		(TCNT0=0x00)
#define	TIMER0_START_1		(TCCR0|=(0<<CS02)|(0<<CS01)|(1<<CS00))
// prescaler 1
#define	TIMER0_START_8		(TCCR0|=(0<<CS02)|(1<<CS01)|(0<<CS00))
// prescaler 8
#define	TIMER0_START_64		(TCCR0|=(0<<CS02)|(1<<CS01)|(1<<CS00))
// prescaler 64
#define	TIMER0_START_256	(TCCR0|=(1<<CS02)|(0<<CS01)|(0<<CS00))
// prescaler 256
#define	TIMER0_START_1024	(TCCR0|=(1<<CS02)|(0<<CS01)|(1<<CS00))
// prescaler 1024
#define	TIMER0_SET(x)		(TCNT0=(x))

//#define	TIMER0_START_EXT	(TCCR0|=(1<<CS02)|(1<<CS01)|(0<<CS00))
#define	TIMER0_START_EXT	(TCCR0=(1<<CS02)|(1<<CS01)|(0<<CS00))			// T/C0 –режим "счётчик".  активный фронт сигнала - ниспадающий
// external source - falling edge

//#define	TIMER0_STOP		(TCCR0&=~((1<<CS02)|(1<<CS01)|(1<<CS00)))
#define	TIMER0_STOP		(TCCR0=0)
#define	IS_TIMER0_STOP	(!(TCCR0 & 0x07))

//----- настройка таймера 1 ------------------------------------------
 
#define	TIMER1_INIT		{TIMSK|=(1<<TOIE1);TCCR1A=(0<<COM1A1)|(0<<COM1A0)|(0<<WGM11)|(0<<WGM10);}
#define	TIMER1_DISABLE	TIMSK&=~(1<<TOIE1)

#define	TIMER1_ZERO		(TCNT1=0)
#define	TIMER1_START_8	(TCCR1B|=(0<<CS12)|(1<<CS11)|(0<<CS10))
// prescaler 8
#define	TIMER1_START	(TCCR1B=(0<<CS12)|(0<<CS11)|(1<<CS10))			// T/C1 –Активен режим "Таймер". Такт таймера равен такту микроконтроллера
// prescaler 1
#define	TIMER1_STOP		(TCCR1B=0)

//#define	TIMER1_SET(x)		{TCNT1H=(x)>>8;TCNT1L=(x)&0x00FFL;}
//#define	IS_TIMER1_STOP	(!(TCCR1B & ((1<<CS12)|(1<<CS11)|(1<<CS10)) ) )

//----- настройка таймера 2 ---------------------------------------------------------------

#define	TIMER2_INIT		{TIMSK|=(1<<TOIE2);TCCR2=0;}
#define	TIMER2_ZERO		(TCNT2=0x00)
#define	TIMER2_START	(TCCR2=(1<<CS22)|(1<<CS21)|(1<<CS20))
// prescaler 1024
#define	TIMER2_SET(x)	(TCNT2=(x))
#define	TIMER2_STOP		(TCCR2=0)
#define	IS_TIMER2_STOP	(!(TCCR2 & 0x07))


//----- режимы измерений ---------------------------------------------------------------------

#define	MODE_IDLE			0

#define	MODE_MEASURE_CAP	1
#define	MODE_MEASURE_IND	2

//----- флажки ---------------------------------------------------------------------

#define		PRESS_START				1
#define		PRESS_ZERO				2

//--------------------------------------------------------------------------

#define		ERR_OVERLOAD	1
#define		ERR_NO_INDUCT	2
#define		ERR_CALIBRATE	3
#define		ERR_UNKNOWN		4

//--------------------------------------------------------------------------

#define		KEY_START		1
#define		KEY_ZERO		2

//--------------------------------------------------------------------------


#endif
