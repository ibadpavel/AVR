#include "lc-meter.h"
//------------------------------------------------------------------------------------
inline	static void	port_init(void);		//
inline	static void	check_battery(void);	//
//void	start_measure(void);				//
//void	display_data(void);					//
//void	process_key(void);					//
//void	tune_zero(void);					//
//------------------------------------------------------------------------------------

int	main(void)
{
	port_init();							// ����������� �����
	while(1)
	{
		;
	}
}
//------------------------------------------------------------------------------------
inline static void port_init(void)
{
	D_DDR = 0;								// ��������� ���� ��� ���� 
	D_PORT = 0;								// ��������� ����� Hi-Z (�����������������)
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
//------------------------------------------------------------------------------------