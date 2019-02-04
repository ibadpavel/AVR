/*
 * lcd1602.c
 *
 * Created: 03.12.2018 16:04:32
 *  Author: Pavel
 */ 
#include "lc-meter.h"

//-------LCD-parameters-----------------------------------------
#define lcd_max_x 16			//number of chars in one row
#define lcd_max_y 2				//number of rows
//--------------------------------------------------------------
void	lcd_init(void);
void 	lcd_clear(void);
void	lcd_init_write(char);
void 	lcd_write_command(char cmd);
void 	lcd_write_data(char data);
void 	lcd_send_data(char data);
void 	lcd_gotoxy(uint8_t x, uint8_t y);
void 	lcd_putchar(char chr);
void 	lcd_puts(char *str);
//--------------------------------------------------------------
uint8_t lcd_x_cnt = 0;
uint8_t lcd_y_cnt = 0;
uint8_t _base_y[4] = {0x80, 0xc0, 0, 0};
//--------------------------------------------------------------
void lcd_init(void)
{
	// Set input/outputs ports
	DC_PORT &= ~((1<<D_E)|(1<<D_RS));		// down RS, E
	DC_DDR |= ((1<<D_E)|(1<<D_RS));			// set output (1)
	D_PORT &= ~D_DATA_MASK;					// set data down (0)
	D_DDR &= ~D_DATA_MASK;					// set inputs (0)
	_delay_ms(50);							// wait for LCD internal initialization
	D_DDR |= D_DATA_MASK;					// set output (1)

	// Set base display index
	_base_y[2] = lcd_max_x + 0x80;
	_base_y[3] = lcd_max_y + 0xc0;

	lcd_init_write(0x30);
	lcd_init_write(0x30);
	lcd_init_write(0x30);
	lcd_init_write(0x20);					// set 4-bit mode

	lcd_write_command(0x28);				// 4 bit mode, 2 strings
	lcd_write_command(0x08);				// display off
	lcd_write_command(0x01);				// display clear
	lcd_write_command(0x04);				// AC=decrement, S=no shift screen

	lcd_clear();
}
//--------------------------------------------------------------
//--Clear LCD---------------------------------------------------
void lcd_clear(void)
{
	lcd_write_command(0x02);
	lcd_write_command(0x0C);
	lcd_write_command(0x01);
}
//--------------------------------------------------------------
//--Write-higher-nibble-in-8-bit-mode-on-init-stage-------------
void lcd_init_write(char cmd)
{
	D_E_DOWN;						// down clock pin
	D_COMMAND_MODE;					// command mode
	D_DDR |= D_DATA_MASK;			// set output
	D_PORT &= ~D_DATA_MASK;			// clear data pins

	_delay_us(4);
	D_PORT |= (cmd >> 4);			// send to pins display
	
	D_E_UP;							// up clock pin
	_delay_us(4);					// wait
	D_E_DOWN;						// down clock pin

	_delay_ms(10);
}
//--------------------------------------------------------------
//--Send-to-command-register------------------------------------
void lcd_write_command(char cmd)
{
	D_COMMAND_MODE;
	lcd_send_data(cmd);
}
//--------------------------------------------------------------
//--Send-to-data-register---------------------------------------
void lcd_write_data(char data)
{
	D_DATA_MODE;
	lcd_send_data(data);
}
//--------------------------------------------------------------
/// Send prepared data ///
void lcd_send_data(char data)
{
	D_E_DOWN;
	D_DDR |= D_DATA_MASK;			// set output
	_delay_us(4);

	D_PORT &= ~D_DATA_MASK;			// clear port
	D_PORT |= (data >> 4);			// out higher nibble
	_delay_us(4);

	D_E_UP;							// latch-up
	_delay_us(4);
	D_E_DOWN;
	_delay_us(4);

	D_PORT &= ~D_DATA_MASK;
	D_PORT |= (data & 0x0F);		// out lower nibble

	D_E_UP;							// latch-up
	_delay_us(4);
	D_E_DOWN;
	_delay_us(4);
}
//--------------------------------------------------------------
//--Set-cursor-position-(x,y)--------------------------------???
void lcd_gotoxy(uint8_t x, uint8_t y)
{
	lcd_write_command(_base_y[y] + x);
	lcd_x_cnt = x;
	lcd_y_cnt = y;
}
//--------------------------------------------------------------
//--Print-single-char-------------------------------------------
void lcd_putchar(char chr)
{
	//	lcd_write_data(chr);

	if (chr==0xA8) chr=0xA2;							//π
	else if (chr==0xB8) chr=0xB5;							//ρ
	//  else if (chr > 0xBF) chr = TransTable [(chr-0xC0)];		//?α• αβ†€μ?λ• °γ™?λ

	lcd_write_data(chr);
	if (++lcd_x_cnt==lcd_max_x)
	{
		lcd_x_cnt=0;
		if(++lcd_y_cnt==lcd_max_y) lcd_y_cnt=0;
		lcd_gotoxy(0,lcd_y_cnt);
	}
}
//--------------------------------------------------------------
//-write-the-string-str-located-in-SRAM-to-the-LCD--------------
void lcd_puts(char *str)
{
	uint8_t k;
	k=*str++;

	while (k != 10)				// while not end line
	{
		lcd_putchar(k);
		k = *str++;
	}
}
//--------------------------------------------------------------