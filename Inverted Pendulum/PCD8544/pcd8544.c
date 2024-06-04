#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"
#include "misc.h"

#include "string.h"

#include "pcd8544.h"
#include "font6x8.h"

//DC==0 -> command send
//DC==1 -> data send

uint8_t LCDBuffer[1000]; // ����� ������� 65*102/8


extern const char font5x8[];

// ���������� �� ��������� �������� ������, ������������ � ���������� NVIC � ����������� ���������
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
/*void SPI1_DMA_TX_IRQHandler()
{
	if (DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4))
	{
		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);

		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
		DMA_Cmd(DMA1_Stream4, DISABLE);
		LCD_CS1;
	}
}*/

unsigned char flip(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

// �������� ������\������ �� �������
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_data(uint8_t data)
{
#ifdef LCD_SPI_HARD
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, data);
#else
	uint8_t i;
	for(i=0; i<8; i++) {
		if (data & 0x80) LCD_MOSI1;
		            else LCD_MOSI0;
		data = data<<1;
		LCD_SCK0;
		LCD_SCK1;
	}
#endif
}



// ������� �������
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Refresh()
{
	LCD_CS0;
	LCD_DC0;

	LCD_data(0x40); // ��������� ������� � ������� Y=0; X=0
	LCD_data(0x80);

#ifdef LCD_SPI_HARD
	while(SPI1->SR & SPI_SR_BSY);
#endif

	LCD_DC1;

#ifndef LCD_SPI_DMA_HARD
	for(uint8_t y=0;y<8;y++)
		for(uint8_t x=0;x<102;x++)
			LCD_data(flip(LCDBuffer[(7-y)*102 + (101-x)]));

	#ifdef LCD_SPI_HARD
		 while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	#endif

	LCD_CS1;
#else
	DMA_Cmd(DMA1_Stream4, ENABLE);
#endif
}




/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Init()
{
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

#ifdef LCD_SPI_HARD
	//SPI_InitTypeDef  SPI_InitStructure;
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

   /* GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);   // SPI2_CLK  (Pin_13)
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);   // SPI2_MOSI (Pin_15)*/

	/*GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin 			= GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType 			= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  			= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);*/



#ifdef LCD_SPI_DMA_HARD // ������������� DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Stream0);

	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(SPI1->DR);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&LCDBuffer[0];
	DMA_InitStructure.DMA_BufferSize = 504;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);

 	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
 	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);

 	NVIC_InitTypeDef NVIC_InitStructure;
 	NVIC_InitStructure.NVIC_IRQChannel 						= DMA1_Stream4_IRQn;
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 0;
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 1;
 	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
 	NVIC_Init(&NVIC_InitStructure);

#endif

	//GPIO_InitStructure.GPIO_Pin  			=(1<<LCD_CSE_PIN) | (1<<LCD_A0_PIN) | (1<<LCD_RST_PIN);

#else //  ������������ ��� ���� SPI

#ifndef	LCD_SOFT_RESET
    GPIO_InitStructure.GPIO_Pin  			=(1<<LCD_CSE_PIN) | (1<<LCD_A0_PIN) | (1<<LCD_SCK_PIN) | (1<<LCD_RST_PIN) | (1<<LCD_SDA_PIN);
#else
    GPIO_InitStructure.GPIO_Pin  			=(1<<LCD_CSE_PIN) | (1<<LCD_A0_PIN) | (1<<LCD_SCK_PIN) | (1<<LCD_SDA_PIN);
#endif

    LCD_SCK0;
#endif

   /* GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_100MHz;
	GPIO_Init(LCD_GPIO, &GPIO_InitStructure);*/

	LCD_CS0;
	LCD_DC0;

	LCD_RST0;
    /*asm("nop");*/
	LCD_RST1;
	/*asm("nop");*/

	/*
	LCD_data(0x21);      // ������� � ����������� �����
	LCD_data(0xC1);

	LCD_data(0x06);		// ������������� �����������, �� 4 �� 7

	LCD_data(0x13);		// Bias 0b0001 0xxx - �������� ��� �������������

	LCD_data(0x20); 	// ������� � ������� �����
	LCD_data(0b1100);	// 0b1100 - normal mode
		                        // 0b1101 - invert mode
		                        // 0b1001 - ��������� ����������� �����
		                        // 0b1000 - ������ �����
		                          */
	LCD_data(0x10);		// Bias 0b0001 0xxx - �������� ��� �������������

	LCD_data(0x21);		// PowerON, ExtCommandSet
	LCD_data(0x09);		// Internal HV-gen x3		09
	LCD_data(0xE0);		// Set Vop
	LCD_data(0x15);		// Bias n=2    //15 //16
	LCD_data(0x06);		// Temperature coeff 2
	LCD_data(0x20);		// StandartCommandSet
	LCD_data(0x0C);		// normal mode, display non-inverted


#ifdef LCD_SPI_HARD
	while(SPI1->SR & SPI_SR_BSY);
#endif
	LCD_CS1;
}



/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Clear()
{
	for(uint8_t y=0; y<8; y++)
		for(uint8_t x=0; x<102; x++)
			LCDBuffer[y*102 + x] = 0;
}



/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Pixel(uint8_t x, uint8_t y, uint8_t mode)
{
	if(x>102 || y>65) return;

	uint16_t adr = (y>>3)*102 + x;
	LCDBuffer[adr] = (mode) ? LCDBuffer[adr] | (1<<(y&0x07)) : LCDBuffer[adr] & ~(1<<(y&0x07));
}



// ��������� ��������� �����
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode)
{
	uint8_t x = x1, y = y1;

	uint8_t	dX = (x2-x1 >= 0)?(x2-x1):(x1-x2), dY = (y2-y1 >= 0)?(y2-y1):(y1-y2);
	signed char	signX = (x2-x1 >= 0)?1:-1, signY = (y2-y1 >= 0)?1:-1;

	for(uint8_t i=(dX >= dY)?dX:dY; i>0; i--)
	{
		if(dX >= dY)
		{
			LCD_Pixel(x, y1 + signX*dY*(x-x1)/dX, mode);
			x += signX;
		}
		else
		{
			LCD_Pixel(x1 + signY*(dX*(y-y1))/dY, y, mode);
			y += signY;
		}
	}

	LCD_Pixel(x2, y2, mode);
}



// ��������� �������������� (�� ������������)
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode)
{
	LCD_Line(x1,y1, x2,y1, mode);
	LCD_Line(x1,y2, x2,y2, mode);
	LCD_Line(x1,y1, x1,y2, mode);
	LCD_Line(x2,y1, x2,y2, mode);
}



// ������ ����������� �����
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Int(uint8_t x, uint8_t y, int val, uint8_t mode)
{
	char s[15];
	itoa_(val, s);
	LCD_String(x, y, s, mode);
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_IntWString(uint8_t x, uint8_t y, int val, char s[], uint8_t mode)
{
	char out[strlen(s)+12];
	out[0] = '\0';
	strcat_(out, s);
	char ss[15];
	itoa_(val, ss);
	strcat_(out, ss);
	LCD_String(x, y, out, mode);
}



// ����� ������� �� ����� �� �����������
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_Char(uint8_t px, uint8_t py, uint8_t ch, uint8_t mode)
{
    uint8_t lcd_YP = 7 - (py & 0x07);    // ������� ������� ������� � �����
    uint8_t lcd_YC = (py & 0xF8)>>3; 	// �������� ������� ������� �� ������

	for(uint8_t x=0; x<5; x++)
	{
		uint8_t temp = font5x8[5*(ch-32) +x];

		if(mode)
		{
			temp = ~temp;
			if(py) LCD_Pixel(px, py-1, 1);	// ���� ������ � ������ �������� - ������ �������� �����
		}

		LCDBuffer[lcd_YC*102+px] |= (temp<<(7-lcd_YP)); 	// ������ ������� ����� �������

	    if(lcd_YP < 7) LCDBuffer[(lcd_YC+1)*102+px] |= (temp>>(lcd_YP+1)); 	// ������ ������ ����� �������
		px++;
		if(px>102) return;
	}
}


// ����� ������
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LCD_String(uint8_t x, uint8_t y, char *str, uint8_t mode)
{


	for(;*str != 0; str++, x += 6)
	{
		LCD_Char(x, y, *str, mode);
		if(mode && x) LCD_Line(x-1, y-1, x-1, y+6, 1);
	}

}
