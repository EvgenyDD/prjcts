/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/

#define uint8_t unsigned char

#define uint16_t unsigned int
    #define uint32_t long
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int strlen(char *);
int strlenNum(char *pText, int begin);
void itoa_(int, char s[]);
void dtoa_(uint32_t n, char s[]);
void ftoa_(float, char str[], char precision);
void reverse(char s[]);
void strcat_(char first[], char second[]);
void strcatNum(char first[], char second[], int begin, int end);

float log10_(int v);
float pow_(float x, float y);
float log10_(int v);
void ftoa_(float num, char str[], char precision);


#endif //STRING_H
