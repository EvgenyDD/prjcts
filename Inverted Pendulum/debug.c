/* Includes ------------------------------------------------------------------*/
#include "debug.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t DebugDecrease;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : DebugInit
* Description    : Initialize debug (via USART1)
*******************************************************************************/
void DebugInit()
{
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE );

	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = /*USART_Mode_Rx | */USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	//USART RX
	/*NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);*/

	USART_Cmd(USART1, ENABLE);

}


/*******************************************************************************
* Function Name  : DebugSendString
* Description    : Sends debug information (via USART1)
* Input          : pointer to text massive
*******************************************************************************/
void DebugSendString(char *pText)
{
	for(; *pText != '\0'; pText++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
		USART_SendData(USART1, *pText);
	}
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
	USART_SendData(USART1, '\n');

}


/*******************************************************************************
* Function Name  : DebugSendChar
* Description    : Sends debug information (via USART1)
* Input          : char to send
*******************************************************************************/
void DebugSendChar(char charTx)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, charTx);
}

void DebugSysTimeStart()
{
	DebugDecrease = 0;
}
/*******************************************************************************
* Function Name  : DebugSysTimeEnd
* Description    : send time from DebugSysTimeStart()
*******************************************************************************/
/*void DebugSysTimeEnd()
{
	char f[15];
	//if(Debug)
	ftoa_(DebugDecrease, f);
	DebugSendString(f);
}*/



void DebugSendNum(uint16_t Num)
{
	char str[50];
	itoa_(Num, str);
	DebugSendString(str);
}

void DebugSendNumWDesc(char *string, uint16_t Num)
{
	char str[50]={'\0'}, number[20];
	strcat_(str, string);
	itoa_(Num, number);
	strcat_(str, number);
	DebugSendString(str);
}

void DebugSendPID(int16_t pos, int16_t speed)
{
	char out[10] = "A";
	char conv[5];
	itoa_(pos, conv);
	strcat_(out, conv);
	strcat_(out, "B");
	itoa_(speed, conv);
	strcat_(out, conv);
	strcat_(out, "C");
	DebugSendString(out);
}
