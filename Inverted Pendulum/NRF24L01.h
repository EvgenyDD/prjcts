/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NRF24L01_H
#define NRF24L01_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "hw_config.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Memory Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17

/* Bit Mnemonics */
#define MASK_RX_DR  6
#define MASK_TX_DS  5
#define MASK_MAX_RT 4
#define EN_CRC      3
#define CRCO        2
#define PWR_UP      1
#define PRIM_RX     0
#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0
#define AW          0
#define ARD         4
#define ARC         0
#define PLL_LOCK    4
#define RF_DR       3
#define RF_PWR      1
#define LNA_HCURR   0        
#define RX_DR       6
#define TX_DS       5
#define MAX_RT      4
#define RX_P_NO     1
#define TX_FULL     0
#define PLOS_CNT    4
#define ARC_CNT     0
#define TX_REUSE    6
#define FIFO_FULL   5
#define TX_EMPTY    4
#define RX_FULL     1
#define RX_EMPTY    0

//RF setup register
#define NRF24L01_PLL_LOCK		4
#define NRF24L01_RF_DR_LOW		5
#define NRF24L01_RF_DR_HIGH		3
#define NRF24L01_RF_DR			3
#define NRF24L01_RF_PWR			1 //2 bits   

/* Instruction Mnemonics */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define NOP           0xFF

/* NRF24L01 Settings */
#define NRF24L01_CONFIG     ( (1<<MASK_RX_DR) | (1<<EN_CRC) | (0<<CRCO) )

typedef enum {
	NRF24L01_Transmit_Status_Lost = 0,		//Message is lost, reached max retransmissions
	NRF24L01_Transmit_Status_Ok = 1,		//Message sent successfully
	NRF24L01_Transmit_Status_Sending = 0xFF	//Message is still sending
} NRF24L01_Transmit_Status_t;

typedef enum {
	NRF24L01_DataRate_2M,		// 2Mbps
	NRF24L01_DataRate_1M,		// 1Mbps
	NRF24L01_DataRate_250k		// 250kbps
} NRF24L01_DataRate_t;

typedef enum {
	NRF24L01_OutputPower_M18dBm,// -18dBm
	NRF24L01_OutputPower_M12dBm,// -12dBm
	NRF24L01_OutputPower_M6dBm,	// -6dBm
	NRF24L01_OutputPower_0dBm	// 0dBm
} NRF24L01_OutputPower_t;



/* Exported macro ------------------------------------------------------------*/
#define NRF24L01_CSN_H	GPIO_SetBits(GPIOA, GPIO_Pin_15)
#define NRF24L01_CSN_L	GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define NRF24L01_CE_H   GPIO_SetBits(GPIOB, GPIO_Pin_3)
#define NRF24L01_CE_L   GPIO_ResetBits(GPIOB, GPIO_Pin_3)


/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void NRF24L01_Init();
void NRF24L01_Config(uint8_t, uint8_t);
void NRF24L01_SetRF(NRF24L01_DataRate_t, NRF24L01_OutputPower_t);

void NRF24L01_SetRxAdr(uint8_t*);
void NRF24L01_SetTxAdr(uint8_t*);

uint8_t NRF24L01_ReadRegister(uint8_t);
void NRF24L01_WriteRegisters(uint8_t, uint8_t*, uint8_t);
void NRF24L01_WriteRegister(uint8_t, uint8_t);


void NRF24L01_PowerUpRx();
void NRF24L01_PowerUpTx();

void NRF24L01_Send(uint8_t*, uint8_t);
NRF24L01_Transmit_Status_t NRF24L01_GetTransmissionStatus();

uint8_t NRF24L01_IsDataReady();
void NRF24L01_GetData(uint8_t*);


void SPI_WrRdMass(uint8_t*, uint8_t*, uint8_t);
void SPI_WriteMass(uint8_t*, uint8_t);
uint8_t SPI_Send(uint8_t);

#endif //NRF24L01_H
