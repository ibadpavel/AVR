
// ----* LCD parameters 

#define lcd_max_x 16 //number of chars in one row
#define lcd_max_y 2 //number of rows

//--------------------------------------------------------------

void	lcd_init(void);
void 	lcd_clear(void);
void 	lcd_putchar(char chr);
void 	lcd_gotoxy(uint8_t x, uint8_t y);
void 	lcd_puts(char *str);
void 	lcd_write_command(char cmd);
void 	lcd_write_data(char data);
void 	lcd_send_data(char data);
//uint8_t lcd_read_BF(void);
//void 	CGRAMUserChar(uint8_t Addr, uint8_t *char_code);
void	lcd_init_write(char);

//---------------------------------------------------------

uint8_t lcd_x_cnt=0;
uint8_t lcd_y_cnt=0;
uint8_t _base_y[4]={0x80, 0xc0, 0, 0};

//---------------------------------------------------------

void lcd_init(void)
{
uint8_t		tmp;

DC_PORT &= ~((1<<D_E)|(1<<D_RS));	// down RS, E 
DC_DDR |= ((1<<D_E)|(1<<D_RS));	// set output

D_PORT &= ~D_DATA_MASK;		// data down
D_DDR &= ~D_DATA_MASK;		// set Z-state inputs

_delay_ms(50);//wait for LCD internal initialization

tmp = D_PORT;

D_DDR |= D_DATA_MASK;	// set output


_base_y[2]=lcd_max_x+0x80;
_base_y[3]=lcd_max_y+0xc0;
lcd_init_write(0x30);
lcd_init_write(0x30);
lcd_init_write(0x30);

lcd_init_write(0x20);//set 4-bit mode

lcd_write_command(0x28);//4 bit mode, 2 strings
lcd_write_command(0x08);// display off
lcd_write_command(0x01);// display clear
lcd_write_command(0x04);//AC=decrement, S=no shift screen

lcd_clear();
}
//--------------------------------------------

// Clear LCD ///
void lcd_clear(void)
{
        lcd_write_command(0x02);
        lcd_write_command(0x0C); 
        lcd_write_command(0x01);
}

//--------------------------------------------
/// Print single char ///
void lcd_putchar(char chr)
{
//	lcd_write_data(chr);

  if (chr==0xA8) chr=0xA2;							//ð
  else if (chr==0xB8) chr=0xB5;							//ñ
//  else if (chr > 0xBF) chr = TransTable [(chr-0xC0)];		//¢á¥ ®áâ «ì­ë¥ ¡ãª¢ë

  lcd_write_data(chr);
  if (++lcd_x_cnt==lcd_max_x)
  {
	lcd_x_cnt=0;
	if(++lcd_y_cnt==lcd_max_y) lcd_y_cnt=0;
	lcd_gotoxy(0,lcd_y_cnt);
  }
}
//--------------------------------------------
/// Set cursor position x y ///
void lcd_gotoxy(uint8_t x, uint8_t y)
{ 
  lcd_write_command(_base_y[y]+x);
  lcd_x_cnt=x;
  lcd_y_cnt=y;
}
//--------------------------------------------

// write the string str located in SRAM to the LCD
void lcd_puts(char *str)
{
uint8_t k;

	k=*str;
	while (k) 
	{
		lcd_putchar(k);
		str++;
		k = *str;
	}
}
//--------------------------------------------
/// Write higher nibble in 8-bit mode on init stage ///

void lcd_init_write(char cmd)
{ 
char	tmp;

//D_E_DN;
//D_RS_DN;//command mode
DC_PORT &= ~((1<<D_E)|(1<<D_RS));

D_DDR |= D_DATA_MASK;
D_PORT &= ~D_DATA_MASK;

_delay_us(4);

//tmp = (cmd & 0xF0);
tmp = cmd >> 4;
D_PORT |= tmp;	// ÊÎÍÊÐÒÍÀß ÏÐÈÂßÇÊÀ ê íîãàì äëÿ äèñïëåÿ!!!
 
D_E_UP;
_delay_us(4);
D_E_DN;

_delay_ms(10);
} 
//--------------------------------------------
/// Send to command register ///
void lcd_write_command(char cmd)
{    
  D_RS_DN;//command mode
  lcd_send_data(cmd);            
} 
//--------------------------------------------
/// Send to data register ///
void lcd_write_data(char data)
{    
  D_RS_UP;//data mode
  lcd_send_data(data);          
}
//--------------------------------------------
/// Send prepared data ///
void lcd_send_data(char data)
{
char	tmp;

D_E_DN;

D_DDR |= D_DATA_MASK;	// set output

_delay_us(4);

//tmp = data & 0xF0;	//out higher nibble
tmp = data >> 4;	//out higher nibble - ïðèâÿçêà!!!
D_PORT &= ~D_DATA_MASK;	// clear
D_PORT |= tmp;

_delay_us(4);

D_E_UP;//latch-up
_delay_us(4);
D_E_DN;
_delay_us(4);

//tmp = data << 4; // out lower nibble - ïðèâÿçêà!!!
tmp = data & 0x0F; // out lower nibble - ïðèâÿçêà!!!
D_PORT &= ~D_DATA_MASK;
D_PORT |= tmp;

D_E_UP;//latch-up
_delay_us(4);
D_E_DN;

//set high impendance/deactivate
//D_PORT &= ~D_DATA_MASK;
//D_DDR &= ~D_DATA_MASK;

_delay_ms(2);	// 0.002 seconds!!! ;   
}
//-------------------------------------------- 


