#ifndef LCD_LIB_H
#define LCD_LIB_H

#include <ioavr.h>
#include <intrinsics.h>

//���� � �������� ���������� ���� ������ ���
#define PORT_DATA PORTA
#define PIN_DATA  PINA
#define DDRX_DATA DDRA

//���� � �������� ���������� ����������� ������ ���
#define PORT_SIG PORTA
#define PIN_SIG  PINA
#define DDRX_SIG DDRA

//������ ������� � ������� ���������� ����������� ������ ��� 
#define RS 0
#define RW 2
#define EN 1

#define F_CPU 20000000

#define CHECK_FLAG_BF
#define BUS_4BIT
//******************************************************************************
//�������




//��������� �������
void LCD_Init(void);//������������� ������ � ���
void LCD_WriteData(unsigned char data); //������� ���� ������ �� ���
void LCD_WriteCom(unsigned char data); //�������� ������� ���
void LCD_SendStringFlash(unsigned char __flash *str);
void LCD_SendString(char *str);
void BCD_2IntLcd(unsigned int value);
void BCD_4IntLcd(unsigned int value);
void LCD_Goto(char x, char y);

void LCD_SetInitDisplay();
void ds1307_GetALL();
void LCD_DataSent();
void EpromCopy();
void Buttons();
void SetAwakePWM(unsigned int, unsigned int);
void TimeComp();
#endif