/*
PIN� - �������� ������� ���������� �������
DDRx - ���������� ���� / �����
	 DDRxy=0 ����� �������� ��� ����
	 DDRxy=1 ����� �������� �� �����
PORTx - ����� ���������� ���������� ������
	���� DDRx �����, �� �� �������� ���������������� ���� � �������� PORTx ���������� ��������� ������
	���� DDRx ����, �� 
		PORTxy=0, �� Hi-Z
		PORTxy=1, �� PullUp

���� Hi-Z � ����� ������������������� �����. ������������� ����� ����� ������, ����� ������� ����.
���� PullUp � ���� � ���������.
*/
#ifndef LC_METER_H_
#define LC_METER_H_

#define F_CPU				12000000UL			// CPU clock frequency

#include <avr/io.h>
#include <avr/interrupt.h>

//--DISPLAY-1602------------------------
#define	D_RS_PORT	PORTD
#define	D_E_PORT	PORTD
#define	DC_PORT		PORTD
#define	DC_DDR		DDRD
#define	D_RS		PD6						// Command / Data bit
#define	D_E			PD7						// Clock
#define	D_D4		PC3
#define	D_D5		PC2
#define	D_D6		PC1
#define	D_D7		PC0

#define	D_DATA_MASK	((1<<D_D4)|(1<<D_D5)|(1<<D_D6)|(1<<D_D7))
#define	D_PORT	PORTC
#define	D_DDR	DDRC

#define	D_E_UP				sbi(D_E_PORT, D_E)
#define	D_E_DOWN			cbi(D_E_PORT, D_E)
#define	D_DATA_MODE			sbi(D_RS_PORT, D_RS)
#define	D_COMMAND_MODE		cbi(D_RS_PORT, D_RS)
//--------------------------------------

#endif LC_METER_H_