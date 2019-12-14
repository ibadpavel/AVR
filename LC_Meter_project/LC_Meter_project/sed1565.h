#ifndef SED1565_H_
#define SED1565_H_
//=============================================================================
#include <avr/io.h>
#include <util/delay.h>
//-----------------------------------------------------------------------------
#define LCD_DISPLAY_ON 0xAF
#define LCD_DISPLAY_OFF 0xAE
//-----
#define LCD_NORMAL_DISPLAY 0xA6
#define LCD_NEGATIVE_DISPLAY 0xA7
//-----
#define LCD_ALL_POINTS_NORMAL 0xA4
#define LCD_ALL_POINTS_ON 0xA5
//-----
#define LCD_BIAS_1_7 0xA3
#define LCD_BIAS_1_9 0xA2
//-----
#define LCD_ADC_SELECT_NORMAL 0xA0
#define LCD_ADC_SELECT_REVERSE 0xA1
//-----
#define LCD_COM_DIRECTION_NORMAL 0xC0
#define LCD_COM_DIRECTION_REVERSE 0xC8
//-----
#define LCD_RESISTOR_RATIO 0x22 // 000 - small > 111 - large
//-----
#define LCD_ELECTRONIC_VOL_MODE 0x81
//-----
#define LCD_CONTROL_POWER_1 0x2E // Booster circuit: ON; Voltage regulator circuit: ON; Voltage follower circuit: OFF;
#define LCD_CONTROL_POWER_2 0x2F // Booster circuit: ON; Voltage regulator circuit: ON; Voltage follower circuit: ON;
//-----
#define LCD_NOP 0xE3
//-----
#define LCD_DISPLAY_START_LINE 0x40 //000000 adress > 111111 adress
//-----
#define LCD_FIRST_ROW 0xB0
#define LCD_END_ROW 0xB8
#define LCD_X_BIAS 17
//--SET-PORTS-AND-PINS--------------------------------------------------------
#define LCD_DDR DDRC
#define LCD_PORT PORTC
#define LCD_CS PC0
#define LCD_SCL PC1
#define LCD_SDA PC2
#define LCD_CD PC3
#define LCD_RST PC4
//----------------------------------------------------------------------------
extern void init_display();
extern void sinit_display();
extern void lcd_putbyte(uint8_t ch);
extern void lcd_swap();
extern void lcd_clear_vd();
extern void lcd_clear();
extern void print_ch(const char ch, const uint8_t row, const uint8_t pos);
extern void print_st(char* str, const uint8_t length, const uint8_t row, const uint8_t pos);
extern void print_double(float value, const uint8_t row, const uint8_t pos);
extern void print_int(int value, const uint8_t row, const uint8_t pos);
extern void print_hex(uint8_t value, const uint8_t row, const uint8_t pos);
extern void select_row(short number);
//============================================================================
#endif /* SED1565_H_ */