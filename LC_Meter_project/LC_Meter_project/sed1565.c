#include "sed1565.h"
#include "font5x7.h"
#include <stdio.h>
//=======================================================

// virtual screen
static char v_display[8][19];

// display initialize
void init_display()
{
	LCD_DDR = 0xff; //configure C port as output
	LCD_PORT = ((1<<LCD_CS)|   // set CS = 1 deactivate controller
				(0<<LCD_SCL)|  // clear serial clock
				(0<<LCD_SDA)|  // clear serial data
				(0<<LCD_CD)|   // clear Control (0)/display data (1) flag
				(0<<LCD_RST)); // clear reset
	//_NOP();
	_delay_ms(100);
	
	LCD_PORT |= (1<<LCD_RST); // set bit in 1
	_delay_ms(100);
	
	sinit_display();
	lcd_clear_vd();
}

void sinit_display()
{
	uint8_t intab[12] = {LCD_NORMAL_DISPLAY,		// Normal display mode;
						 LCD_ALL_POINTS_NORMAL,		// Normal points;
						 LCD_BIAS_1_7,				// LCD bias set 1/7
						 LCD_ADC_SELECT_REVERSE,	// ADC select: reverse
						 LCD_COM_DIRECTION_NORMAL,	// COM normal direction
						 LCD_RESISTOR_RATIO,		// Rb/Ra resistor Ratio
						 LCD_ELECTRONIC_VOL_MODE,	// Electronic volume mode set
						 LCD_CONTROL_POWER_1,		// Power control set
						 LCD_CONTROL_POWER_2,		// Power control set
						 LCD_NOP,
						 LCD_DISPLAY_START_LINE,	// Display start line set
						 LCD_DISPLAY_ON};			// Display ON!!! :)
						 
	LCD_PORT &= ~(1<<LCD_CS); // set LCD_CS bit in 0
	
	for(int i = 0; i < 12; i++)
		lcd_putbyte(intab[i]);	
}

// send byte to display controller
void lcd_putbyte(uint8_t ch)
{
	LCD_PORT &= ~(1<<LCD_CS); // set LCD_CS bit in 0
	int i = 7;
	while(i >= 0)
	{
		LCD_PORT &= ~(1<<LCD_SDA);
		LCD_PORT |= ( (1 & (ch >> i))<<LCD_SDA );
		LCD_PORT |= (1<<LCD_SCL);
		LCD_PORT &= ~(1<<LCD_SCL);
		i--;
	}

	LCD_PORT |= (1<<LCD_CS); // set LCD_CS bit in 1
}

// transfer virtual screen to the GD50 (Nokia 7110) display
void lcd_swap()
{
	short y = LCD_FIRST_ROW;
	short byteCounter;  		// 19 = length line  
	
	//write 8 pages to LCD display (0..64 rows)
	while(y < LCD_END_ROW)
	{
		LCD_PORT &= ~(1<<LCD_CD); // command
			
		lcd_putbyte((uint8_t)y);
		lcd_putbyte((uint8_t)LCD_X_BIAS);
		//lcd_putbyte((uint8_t)0x02);// what the command???
			
		LCD_PORT |= (1<<LCD_CD); // data
		lcd_putbyte(0);
		lcd_putbyte(0);
		lcd_putbyte(0);
		
		byteCounter = 19;
		do
		{
			char ch = v_display[y - LCD_FIRST_ROW][19 - byteCounter];

			if (ch < 0x20) ch = 0x20;
			else if (ch > 0x7f) ch = 0x20;

			for(int i = (ch - 0x20) * 5; i < (ch - 0x20) * 5 + 5; i++)
			{
				lcd_putbyte(pgm_read_byte(&Char_Table[i]));
			}			
		} while(byteCounter-- > 0);
		y++;
	}
	
	LCD_PORT &= ~(1<<LCD_CD); // command		
	lcd_putbyte((uint8_t)LCD_END_ROW);
	lcd_putbyte((uint8_t)LCD_X_BIAS);	
	LCD_PORT |= (1<<LCD_CD); // data
	for(int i = 0; i < 19 * 5; i++)
	{
		lcd_putbyte(0);	
	}
}

// clear virtual display
void lcd_clear_vd()
{
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < 19; j++)
			v_display[i][j] = 0x20;
}

// clear display
void lcd_clear()
{
	lcd_clear_vd();
	lcd_swap();
}

// print char
// row 0..7
// pos 0..18 for font 5x7
void print_ch(char ch, uint8_t row, uint8_t pos)
{
	if((row >= 0) && (row < 9) && (pos >= 0) && (pos < 19))
		v_display[row][pos] = ch;
	lcd_swap();
}

// print string
// row 0..7
// pos 0..18 for font 5x7
void print_st(char* str, uint8_t length, uint8_t row, uint8_t pos)
{
	if((row >= 0) && (row < 9) && (pos >= 0) && ((pos + length - 1) < 19)) {
		for(int i = 0; i < length; i++) {
			v_display[row][pos + i] = str[i];
		}
	}
	lcd_swap();
}

// print double number
void print_double(float value, const uint8_t row, const uint8_t pos)
{
	char temp[6] = {' '};

	dtostrf(value, 6, 2, temp);
	print_st(temp, 6, row, pos);
}

// print integer number max 32767) and min value < -32767
void print_int(int value, const uint8_t row, const uint8_t pos)
{
	char temp[6] = {' '};

	sprintf(temp,"%d",value);
	print_st(temp, 6, row, pos);
}

// print hexadecimal number
void print_hex(uint8_t value, const uint8_t row, const uint8_t pos)
{
	char temp[6] = {' '};
	temp[0] = '0';
	temp[1] = 'x';

	sprintf(temp + 2,"%x",value);
	print_st(temp, 6, row, pos);
}

// invert pixels from selected string
void select_row(short number)
{
	lcd_swap();
	
	if( number < 0) return;
	else if ( number > 7 ) number = 7;
	
	LCD_PORT &= ~(1<<LCD_CD); // command
	
	lcd_putbyte((uint8_t)number + LCD_FIRST_ROW);
	lcd_putbyte((uint8_t)LCD_X_BIAS);
	//lcd_putbyte((uint8_t)0x02); what the command???
	
	LCD_PORT |= (1<<LCD_CD); // data
	
	short byteCounter = 19;
	do
	{
		char ch = v_display[number][19 - byteCounter];
		
		if (ch < 0x20) ch = 0x20;
		else if (ch > 0x7f) ch = 0x20;

		for(int i = (ch - 0x20) * 5; i < (ch - 0x20) * 5 + 5; i++)
		{
			lcd_putbyte(~(pgm_read_byte(&Char_Table[i])));
		}
	}while(byteCounter-- > 0);
	lcd_putbyte(0);			//clear end(96) pixels column
}