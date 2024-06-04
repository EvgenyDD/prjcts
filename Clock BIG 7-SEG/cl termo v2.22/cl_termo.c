#include "clock.h"
// v2.22 вывод термостата выкл. в спящем режиме, также выкл. при отсутствии датчика.
// v2.21 #define kolvo_zon	5  
// V2.2 //можно выбирать кол-во отсрочек сигнала

// Если Вам понравились эти часы и они приносят Вам пользу, то вы можете отблагодарить меня, за то что я  так долго сними парился:)

//--------------------------------------
static void port_init(void)
{
DDRLCD=0xFF;
DDRSERIAL|=PIN_SERIAL|sig_min_00;
DDR_OUT|=PIN_calls1 | PIN_calls_osn | PIN_T_STAT;// | PIN_timer;
PORT_POWER_DETECT|=_power_;
}
//**************************************
static void timer0_init(void){
	TCCR0=0x01;
}
//**************************************
static void timer2_init(void)
{
 TCCR2 = 0x00; 		  //stop
 ASSR  = (1<<AS2);	  //set async mode
 TCCR2 = 0x01; 		  //start
}
//**************************************
void timer1_init(void)
{
 TCCR1B = 0; //stop
 TCCR1A	= 0;
 OCR1AH = ocr1[ocr1ahi];
 OCR1AL = ocr1[ocr1alow];
 DDR_OC1A  &=~ OC1A;	  //OC1A 
}
//**************************************
void timer1_init_in_pwm(void)
{
	DDR_OC1A |= OC1A;		//OC1A
	TCCR1B = 0; 			//stop
	TCCR1A = (1<<COM1A0);	//Toggle OC1A/OC1B on Compare Match //TCCR1B WGM12=1 CTC top-OCR1A
}
//**************************************	
void shim_true(void){
	timer1_init_in_pwm();
	TCCR1B=(1<<CS10)|(1<<WGM12);
}
//**************************************

void init_devices(void)
{
 cli(); //disable all interrupts
 port_init();
 timer0_init();
 timer1_init();
 MCUCR = 0x00;
 GICR  = 0x00;
 ACSR=_BV(ACD);
 TIMSK =(1<<TOIE2)|(1<<TOIE0); //(1<<OCIE2)| timer interrupt sources |(1<<OCIE1A)
 sei(); //re-enable interrupts
}

// ***********************************************************
//Пример кода на Си
unsigned char EEPROM_READ(unsigned int uiAddress)
{
	/* Ожидание  завершения предыдущей записи*/
	while(EECR & (1<<EEWE));
	/* Установка адресного регистра */
	EEAR = uiAddress;
	/* Разрешение чтения из ЭППЗУ путем установки EERE */
	EECR |= (1<<EERE);
	/* Возврат данных из регистра данных ЭСППЗУ*/
	return EEDR;
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	if ( EEPROM_READ(uiAddress)!=ucData){
		/* Ожидаем окончание предыдущей записи */
		while(EECR & (1<<EEWE));
		cli();
		/* Указание адреса и данных */
		EEAR = uiAddress;
		EEDR = ucData;
		/* Запись лог. 1 в  EEMWE */
		EECR |= (1<<EEMWE);
		/* Запуск записи в ЭСППЗУ путем установки EEWE */
		EECR |= (1<<EEWE);
		sei();
		}
}
//*********************************************
void read_data_EEPROM(void)
{
unsigned char *a,i;

		a=&calls[0][0];
		for(i=EEPROM_calls; i < 12*3+EEPROM_calls ; i++, a++){
				*a=EEPROM_READ(i);
				}
		for(i=EEPROM_regim_state_time; i<Nreg_st+EEPROM_regim_state_time ; i++)
			regim_state_time[i-EEPROM_regim_state_time]=EEPROM_READ(i);
		
		a=(uint8_t *)&termostat;
	 	for(i=EEPROM_termostat; i<EEPROM_termostat+sizeof(termostat) ; i++)
			*a++=EEPROM_READ(i);
		
		temper_termostat=termostat.temperature[0];

		otsroshka_num=EEPROM_READ(EEPROM_otsrochka_num);
		//if (i) time_flag|=_BV(fl_otsrochka_true);
		signal_bud=EEPROM_READ(EEPROM_sig_pin_or_bip);
		
		if ( EEPROM_READ(EEPROM_verifu_but)==0xaa){
			for(i=EEPROM_ocr1a; i<4+EEPROM_ocr1a ; i++)
				ocr1[i-EEPROM_ocr1a]=EEPROM_READ(i);
				}
				
		if ( (i=EEPROM_READ(EEPROM_light))<sizeof(light_swith) )
		{
		day_light=i;//EEPROM_READ(EEPROM_light);
		night_light=EEPROM_READ(EEPROM_light+1);
		tim_c_light=EEPROM_READ(EEPROM_light+2);
		tim_do_light=EEPROM_READ(EEPROM_light+3);
		}

		if ( (i=EEPROM_READ(EEPROM_correction)) < sizeof(corr_in_lcd)/2 )
		{
		correction=i;
		corr_flag=EEPROM_READ(EEPROM_corr_flag);
		}
		
		vid_t=EEPROM_READ(EEPROM_vid_t);
		
		lcd_light=day_light;
}

//*************** 1 Wire DALLAS ************************************************
void pullup_off(void){
	DDR_1wire&=~pwire0;
	DDR_1wire&=~pwire1;
	PORT_1wire&=~pwire0;
	PORT_1wire&=~pwire1;
}
//***********************************************
unsigned char readbit(void){
unsigned char i;
	cli();
	DDR_1wire|=out_1wire;//line down

//	asm("nop");
	asm("nop");
	asm("nop");//1.25us
	DDR_1wire&=~out_1wire;//line up through 1.25us
	_delay_us(12);//11
	i=PIN_1wire;
	sei();
	_delay_us(47);//47
	return i;
}
//***********************************************
void readbyte(uint8_t *pw) {  
unsigned char i, r, read_byte[2]={0,0}, pinwire;

pullup_off();
for(i=0, r=1; i<8; i++){
	pinwire=readbit();
	if ( pinwire & pwire0 ) read_byte[0]|=r;
	if ( pinwire & pwire1 ) read_byte[1]|=r;
	r<<=1;
	}
PORT_1wire|=power_suplly_1w;
DDR_1wire|=power_suplly_1w;
pw[0]=read_byte[0];	
pw[1]=read_byte[1];
}
//***********************************************
void writebit0(void){
	cli();
	DDR_1wire|=out_1wire;
	_delay_us(60);//60
	DDR_1wire&=~out_1wire;
	sei();
}
//***********************************************
void writebit1(void){
	cli();
	DDR_1wire|=out_1wire;
	_delay_us(3);
	DDR_1wire&=~out_1wire;
	sei();
	_delay_us(57);//57
}
//***********************************************
void writebyte(unsigned char byte){
unsigned char i;
pullup_off();
	for(i=0; i<8; i++){
		if (byte&_BV(0)) 
			writebit1();
			else
			writebit0();
		byte>>=1;
		}
	PORT_1wire|=power_suplly_1w;
	DDR_1wire|=power_suplly_1w;
}
//****************************************

void detectPresence(void){
unsigned char i;
	
	pullup_off();
	out_1wire=pwire0|pwire1;
	
	DDR_1wire|=pwire0;
	DDR_1wire|=pwire1;
	_delay_ms(0.48);//0.48
	
	cli();
	DDR_1wire&=~pwire0;
	DDR_1wire&=~pwire1;
	_delay_us(60);//60
	i=PIN_1wire;
	sei();
	
	if ( i & pwire0 ) {out_1wire&=~pwire0; PORT_OUT &= ~PIN_T_STAT;}// нет датчика - вывод термостата в ноль
	if ( i & pwire1 ) out_1wire&=~pwire1;

	if ( out_1wire == 0 ){
		count_com_1w=0; 
		return;
		}
	
	_delay_ms(0.42);//0.42
	i=PIN_1wire;
	if (!( i & pwire0 )) {out_1wire&=~pwire0; PORT_OUT &= ~PIN_T_STAT;}// нет датчика - вывод термостата в ноль
	if (!( i & pwire1 )) out_1wire&=~pwire1;
	
	if ( out_1wire == 0 ){
		count_com_1w=0; 
		return;
		}
			
	power_suplly_1w &= out_1wire;
	PORT_1wire|=power_suplly_1w;
	DDR_1wire|=power_suplly_1w;	
}

//****************************************
void scratchpad_save(void){

		detectPresence();//если датчик присутствует
		writebyte(0xCC);//SKIP ROM [CCh]
		writebyte(0x4E);//WRITE SCRATCHPAD [4Eh]
		writebyte(5);//	TH
		writebyte(5);//	TL
		writebyte(0x7f);//	Statys 12bit
		
		detectPresence();// сохроняем в EEPROM
		writebyte(0xCC);//SKIP ROM [CCh]
		writebyte(0x48);//COPY SCRATCHPAD [48h]
		PORT_1wire|=power_suplly_1w;
		DDR_1wire|=power_suplly_1w;
}
//****************************************

void format_t(unsigned char p){
int t;
	t=((unsigned int)(tempir_msb[p]<<8))|tempir_lsb[p];//msb
	//t|=tempir_lsb[p];//lsb;
	temperatura[p]=(t*10)>>4;
}	
//**************************************************
void izmerenie_t(void){

	//if (regim) return;//когда нужно видеть т тогда она и измеряется debug

	if ( !count_com_1w ){//сброс
		count_com_1w++;
power_suplly_1w=0;
		detectPresence();//если датчик присуствует
		return;
		}

	if ( count_com_1w == 1 ){
		count_com_1w++;

		writebyte(0xCC);//SKIP ROM [CCh]
		writebyte(0xB4);//READ POWER SUPPLY [B4h]
pullup_off();
		power_suplly_1w=readbit();//если низкий уровень то паразитное питание
		power_suplly_1w=~power_suplly_1w;//где был низкий вкл высокий, парразитное питание
		power_suplly_1w &= out_1wire;//pwire0|pwire1;
PORT_1wire|=power_suplly_1w;
DDR_1wire|=power_suplly_1w;
		return;
		}

	if ( count_com_1w == 2 ){	//старт преобразования
		count_com_1w++;
		detectPresence();//сброс
		return;
		}

	if ( count_com_1w == 3 ){
		count_com_1w++;
		writebyte(0xCC);//SKIP ROM [CCh]
		writebyte(0x44);//CONVERT T [44h]
		time[wire1]=120;//1sek
		return;
		}

	if ( count_com_1w == 4 ){
		if (time[wire1]<-8) count_com_1w++;
		return;
		}

	if ( count_com_1w == 5 ){//чтение темпeратуры
		count_com_1w++;
		detectPresence();//сброс
		return;
		}

	if ( count_com_1w == 6 ){
		count_com_1w++;
		writebyte(0xCC);//SKIP ROM [CCh]
		writebyte(0xBE);//READ SCRATCHPAD [BEh]
		return;
		}

	if ( count_com_1w == 7 ){
		count_com_1w=0;
		readbyte(tempir_lsb);//читаем темпиратуру
		readbyte(tempir_msb);
		format_t(0);
		format_t(1);
		#define reg_termostat	3
		if ( !(regim==reg_termostat+2)){
			//--------------------------------------------------------
			if (temperatura[ds0] <= temper_termostat*10-termostat.gisterezis)	PORT_OUT |= PIN_T_STAT;//если (тек_темпер) меньше чем (порог-гистерезис)  вкл
			else
			if (temperatura[ds0] >= temper_termostat*10)	PORT_OUT &= ~PIN_T_STAT;
			//--------------------------------------------------------
		}
		}

}
//*************** END 1 Wire *********************
//***********************************************/
static void change_light_var(void)
{
		if ( time_flag & _BV(fl_plus) )
		   {
		   if (++change_light>=lcd_light) time_flag &= ~_BV(fl_plus);// уст бит обратного счета
		   }
		   else
		   {
		   
		   if (change_light>1) change_light--; else time_flag|=_BV(fl_plus);// счет прямой
		   }
}
//**************************************
static uint8_t opred_light_current_num(void)
{
  char l;
  unsigned int a=0x100;
 	
	l=lcd_light;
	if (mask)
	 	{
		a>>=znakomesto;
		if ( mask&a ) l=change_light;
		}
		else	
		{
		time_flag&=~_BV(fl_plus);
		change_light=lcd_light;
		}
	return l;
}

//******************74hc595***********************************
void data_shift(unsigned char data)
{
	for(uint8_t i=8; i ; i--){// данные  8 бит
	if (data & _BV(7) )  PORTSERIAL|=_BV(DATA);
					else   PORTSERIAL&=~_BV(DATA); 
	data<<=1;
	PORTSERIAL|=_BV(SHIFT);
	PORTSERIAL&=~_BV(SHIFT);
	} 
}

void display(void)
{
	PORTSERIAL|=_BV(STORAGE);//выкл разряды
#ifdef katod 
	data_shift(~(1<<(znakomesto)));
#else
	data_shift(1<<(znakomesto));
#endif
	
	PORTSERIAL&=~_BV(STORAGE);
	PORTSERIAL|=_BV(STORAGE);
	PORTSERIAL&=~_BV(STORAGE);
	
	if (znakomesto==7){
	PORTSERIAL|=_BV(SHIFT);
	PORTSERIAL&=~_BV(SHIFT);
	}
	if (znakomesto==8)   
	#ifdef katod
		data_shift(0x0);
	#else
		data_shift(0xFF);
	#endif
	PORTSERIAL&=~(_BV(DATA)|_BV(SHIFT)|_BV(STORAGE));  
}

//*************************************************************
void skankey (void){
	unsigned char key_lokal=17;
	
	key=key_lokal;
	
	// PORTID|=pin_id;
	PORTSERIAL|=_BV(STORAGE);

	DDRLCD =0xF0;
	PORTLCD=0x0F;
	_delay_us(10);
	
	 if (!str0)	    	key_lokal=0;
	 else if (!str4)	key_lokal=4;
	 else if (!str8)	key_lokal=8;
	 else if (!str12)	key_lokal=12;
	 else key_lokal=17;//goto vool;
/*	
	 DDRLCD =0x0F;
	 PORTLCD=0xF0;
	 delay(15);
	
	 if (!stolb1)		key_lokal+=1;
	 else if (!stolb2)	key_lokal+=2;
	 else if (!stolb3)	key_lokal+=3;
	 else if (!stolb4)	key_lokal+=4;
	 else
vool:	 key_lokal=17;
*/	
	 if ( key_lokal!=17 )
	 {
	   if (++anti_drebezg[0]==35)//35)
	   	  {
		  key=key_lokal;
		  if ( ( ocr1[ocr1ahi_but] )||( ocr1[ocr1alow_but] ) ){
			OCR1AH  = ocr1[ocr1ahi_but];//0x04;
			OCR1AL  = ocr1[ocr1alow_but];//0x00;
			shim_true();//
			time_flag|=_BV(fl_sig_butt);
			time[note_but]=7;
			}
		  }
	   if (anti_drebezg[0]==anti_drebezg[2]){	//anti_drebezg[0] сам антидребезг
	   	   anti_drebezg[0]=0;					//anti_drebezg[1] кол-во цифр перед убыстрением
		   anti_drebezg[1]++;					//anti_drebezg[2] задержка после опред цифры
		   if (anti_drebezg[1]==3) anti_drebezg[2]=45;//55;
		   }	
	   time_in_regim=25;
	 }
	 else
	 {
	   anti_drebezg[0]=0;
	   anti_drebezg[1]=0;
	   anti_drebezg[2]=200;
	 }
	 #ifdef katod
	 PORTLCD=0x00;
	 #else
	 PORTLCD=0xFF;
	 #endif
	 DDRLCD =0xFF;
}

//**************************************
void inc_dec_var_bit(unsigned char *var,unsigned char bit_n){
		if (PLUS)	 (*var)|= _BV(bit_n);
		if (MINUS) 	 (*var)&=~_BV(bit_n);
}

//**************************************
void inc_dec_var(unsigned char *var, unsigned char limit){
		if (PLUS){
			if ((*var)<limit)	(*var)++; 
			else *var=0;
			}
		if (MINUS){
			if (*var) (*var)--;
			else *var=limit;
			}
}
//**************************************
void mask_next_regim(unsigned int m, unsigned char next_F,unsigned char SET)
{
 	 if (!anti_drebezg[0])  mask=m; 
	 if (F_)   regim=next_F;
	 if (SET_) regim=SET;
	 key=17;
}

void next_regim(void){
	if (PLUS) {
		if (regim<218) 
		regim++;
		}
	if (MINUS) {
		if (regim>210)
		regim--;
		}
}

void reg_see_init(void){
	time_view_buf=3;
	regim_state_time[Nreg_st]=0;
}

//**************************************
void key_action(void)
{//mask_next_regim( maska regima, if (key F_), if (key SET_) )

	mask=0;

	if ( signal ){
		mask=0xFFFF;
		if (key!=17)
		if (!( (regim>=52)&&(regim<=54) )) 
			{ 
			time_flag|= _BV(fl_signal_cls);
			time_flag&=~_BV(fl_signal_out_enable);
			if (F_) time_flag&= ~_BV(fl_sig_otsr);// если сигнал с отсрочкой то убрать флаг отсрочки,  
			regim=0;
			return;
			}
		}

	if (!regim) {
		if (F_) {
			regim=7;
			}
			else
		if (SET_){
		   regim=100;
		   }
		   else
		if (MINUS) regim=31;
			else
		if (PLUS) {
			reg_see_init();
			regim=91;
			}
		return;
		}
		
	if (regim==7) { mask_next_regim(0xFFFF,8,31); return;}//calls	
	if (regim==8) { mask_next_regim(0xFFFF,9,reg_termostat); return;}//timer
	if (regim==9) { mask_next_regim(0xFFFF,0,210);  return;}//setup

	if (regim==100) {//при нажатии set в режиме ноль вкл режим 100
		if ( (MINUS)||(PLUS) ) {
			time_flag&=~_BV(fl_sig_otsr);
			regim=0;
			}
		mask_next_regim(0,7,0);
		return;
		}
	//#define reg_termostat	3 // объявлена в ф. izmerenie_t();
	if (regim==reg_termostat) {
		mask_next_regim(0,0,reg_termostat+1);//t00-00-00
		return;
		}
	if (regim==reg_termostat+1) { 
	 	inc_dec_var(&temper_termostat, 124);
		mask_next_regim(0x1C,0,reg_termostat+2);
		return;
	 	}
	if (regim==reg_termostat+2) { 
		if (PLUS) PORT_OUT|=PIN_T_STAT;
		if (MINUS) PORT_OUT&=~PIN_T_STAT;
		mask_next_regim(0x01,0,reg_termostat);
		return;
	 	}
	/*/---------------------timer-------------------------
	 if (regim==3) {
		mask_next_regim(0,0,80);//t00-00-00
		return;
		}
	 if (regim==80) { //true chet timer
	 	inc_dec_var_bit(&timer[3], fl_timer_chet);
		if (PLUS) regim=3;
		mask_next_regim(0b100000000,3,81);
		return;
	 	}
	 if (regim==81) {//set hour timer
	 	inc_dec_var(&timer[2],99);
	 	mask_next_regim(0b11000000,3,82);
		return;
		}
	 if (regim==82) {//set min timer
	 	inc_dec_var(&timer[1],59);
	 	mask_next_regim(0b11000,3,83);
		return;
		}
	 if (regim==83) {//set sek timer
	 	inc_dec_var(&timer[0],59);
	 	mask_next_regim(0b11,3,80);
		return;
		}	
	//----------------------------------------------*/
	 //CALLS//****************************
	 if (regim==31) {//set number calss
	 	inc_dec_var(&budilnik_nomer,11);
	 	mask_next_regim(0b100000000,0,32);
		return;
	 	}
	 if (regim==32) {//on off calls
	 	inc_dec_var_bit(&calls[ budilnik_nomer ][2], 7 );
	 	mask_next_regim(0b11000000,31,33);
		return;
	 	}
	 if (regim==33) {//set hour calss
	 	inc_dec_var(&calls[ budilnik_nomer ][c_hour],23);
	 	mask_next_regim(0b11000,31,34);
		return;
	 	}
	 if (regim==34) {//set min calls
		uint8_t a;
	 	inc_dec_var(&calls[ budilnik_nomer ] [c_min],59);
		if (budilnik_nomer<10) a=35; else a=45;
	 	mask_next_regim(0b11,31,a);
		return;// 
	 	}

	for(uint8_t i=35,a=0; i<42 ;i++,a++){
	  if (regim==i) {//set day calss
		inc_dec_var_bit( &calls[ budilnik_nomer ][2], a );
	 	mask_next_regim(_BV(6-a),31,i+1);
		break;
	 	}
	}
	if (regim==42) {//save current budilnik in eeprom
		for(uint8_t a=EEPROM_calls+budilnik_nomer*3, i=0 ;  i<3  ;a++, i++) 
			EEPROM_write( a, calls[budilnik_nomer][i]);
		regim=31;
		return;
		}
	 if (regim==45) {
		inc_dec_var(&calls_pin[budilnik_nomer-10][h_hour],17); //c_hour
		mask_next_regim(0xC0,31,46);
		return;
	 	}
	 if (regim==46) {// set min
	 	inc_dec_var(&calls_pin[budilnik_nomer-10][h_min],59);//c_min
		mask_next_regim(0b11000,31,47);
		return;
	 	}
		//debug
	 if (regim==47) {// set min
	 	inc_dec_var(&calls_pin[budilnik_nomer-10][h_sek],59);//c_sek
		mask_next_regim(0b11,31,35);
		//if (regim==35){// сохранить в еепром
		//	
		//	}
		return;
	 	}
	//---------------- end calls -------------------	

	//-----------------PORT-------------------------
	 if (regim==211) {//120
		next_regim();
		mask_next_regim(0xFFFF,0,71);//Port  C	
		return;
		}
	 if (regim==71) {//_pin-0_ _c								   
	 	if (PLUS) PORT_OUT|=PIN_calls1;
		if (MINUS) PORT_OUT&=~PIN_calls1;
		mask_next_regim(0b1000,212,212);
		return;
		}
	//--------------------END PORT-----------------
	
	//--------------------t-stat-------------------
	if (regim==212) {//120
		next_regim();
		mask_next_regim(0xFFFF,0,80);//Port  C	
		return;
		}
	if (regim==80) {
		uint8_t i;
	 	inc_dec_var(&p_index,kolvo_zon);//3
		if ( p_index == kolvo_zon ) i=84; else i=81;
	 	mask_next_regim(0x100,0,i);
		return;
		}
	if (regim==81) {
	 	inc_dec_var(&termostat.time[p_index],23);
	 	mask_next_regim(0xC0,85,82);
		return;
		}
	if (regim==82) {
		uint8_t j;
		if ( p_index < kolvo_zon-1 ) j=p_index+1; else j=0;
	 	inc_dec_var(&termostat.time[j],23);
	 	mask_next_regim(0x18,85,83);
		return;
		}
	if (regim==83) {
	 	inc_dec_var(&termostat.temperature[p_index],124);
	 	mask_next_regim(0x07,85,85);
		if (regim==85) p_index++;
		return;
		}
	if (regim==84) {
	 	inc_dec_var(&termostat.gisterezis,99);
	 	mask_next_regim(0x03,85,85);
		return;
		}
	if (regim==85) {
		uint8_t *p;
		p=(uint8_t *)&termostat;
	 	for(uint8_t i=EEPROM_termostat; i<EEPROM_termostat+sizeof(termostat) ; i++)
			EEPROM_write( i, *(p++));
		regim=80;
		return;
		}
	//--------------------END t-stat---------------

	//--------------------CLOCK--------------------
	 if (regim==210) {//121
		next_regim();
		mask_next_regim(0xFFFF,0,21); 
		return; 
		}//clock 4ACb|
 	 if (regim==21) {//set min hour
	 	inc_dec_var(&hour[h_min],59);
	 	mask_next_regim(0b11,0,22);
		return;
		}
	 if (regim==22) {//set hour
	 	inc_dec_var(&hour[h_hour],23);
	 	mask_next_regim(0b11000,0,23);
		return;
		}
	 if (regim==23) {//set day hour
	 	inc_dec_var(&hour[h_day],7);
		if (!hour[h_day]) hour[h_day]=1;
	 	mask_next_regim(0b1000000,0,24);
		return;
		}
	 if (regim==24) {//set num hour
	 	inc_dec_var(&hour[h_num],31);
		if (!hour[h_num]) hour[h_num]=1;
	 	mask_next_regim(0b110000000,0,25);
		return;
		}
	 if (regim==25) {// set month hour
	 	inc_dec_var(&hour[h_month],12);
		if (!hour[h_month]) hour[h_month]=1;//nnonth_0_
	 	mask_next_regim(0b11,0,26);
		return;
		}
	 if (regim==26) {//set hour sek
	 	if (MINUS){
		   if (hour[h_sek]>31) hour[h_min]++;
		   hour[h_sek]=0;
		   clock_=0;
		   }
	 	mask_next_regim(0b11,0,27);
		return;
		}
	 if (regim==27) {//set corection znak
	 	inc_dec_var_bit(&corr_flag, fl_corr_znak_pl);
	 	mask_next_regim(0x10,0,28);
		return;
		}
	 if (regim==28) {//set hour
	 	inc_dec_var(&correction,sizeof(corr_in_lcd)/2-1);
	 	mask_next_regim(0x0F,0,211);
		if (regim==211) {
			EEPROM_write(EEPROM_correction, correction);
			EEPROM_write(EEPROM_corr_flag,   corr_flag);
			}
		return;
		}
	//-----------------end cklock-----------------------
	
	//-------------SEE----------------
	if (regim==213){//122
		reg_see_init();
		next_regim();
		mask_next_regim(0xFFFF,0,91);//_УСt_SEE_
		return;
		}
	if (regim==91) {//F1 r.1-000 
	 	inc_dec_var( &regim_state_time[Nreg_st], Nreg_st/2-1);
	 	mask_next_regim(0b10000000,213,92);//BIT(8)
		return;
		}	
	if (regim==92) {//F1 r.1-000 
	 	inc_dec_var(&regim_state_time[regim_state_time[Nreg_st]*2],11);
	 	mask_next_regim(0b10000,91,93);
		return;
		}	
	 if (regim==93) {//r.1-000 F1
	 	inc_dec_var(&regim_state_time[regim_state_time[Nreg_st]*2+1],255);
	 	mask_next_regim(0b111,94,94);
		return;
		}

	if (regim==94) {
		for(uint8_t i=EEPROM_regim_state_time; i<Nreg_st+EEPROM_regim_state_time ; i++)
			EEPROM_write( i, regim_state_time[i-EEPROM_regim_state_time]);

		regim=91;
		return;
		}
	//-------------END SEE----------------
	
	//-------------yct bud----------------
	if (regim==214) {//123
		next_regim();
		mask_next_regim(0xFFFF,0,75);//yct bud	
		return;
		}
	if (regim==75) {//CAL. bIP C
	 	inc_dec_var(&signal_bud, 2);
		mask_next_regim(0b000011100,214,76);
		return;
		}
	if (regim==76) {//otsroch. on
		time_flag&=~_BV(fl_sig_otsr);//если отсрочка была а fl_otsrochka_true=0 потом 1 сработает сигнал, а так нет
		inc_dec_var(&otsroshka_num, 30);
		mask_next_regim(0b000000011,214,214);
		if (regim==214) {//сигнал шим или вкл вывода
			EEPROM_write(EEPROM_sig_pin_or_bip, signal_bud );
			EEPROM_write(EEPROM_otsrochka_num, otsroshka_num );
			}
		return;			  	
		}
	//-------------------end  yct bud--------------------

	//-------------------light--------------------------
	if (regim==215) {//125
		next_regim();
		mask_next_regim(0xFFFF,0,11); 
		return; 
		}//light
	if (regim==11) {
	 	inc_dec_var(&day_light,sizeof(light_swith)-1);
		if (!day_light) day_light=1;
	 	mask_next_regim(0x100,215,14);
		return;
		}
	if (regim==14) {
	 	inc_dec_var(&night_light,sizeof(light_swith)-1);
		if (!night_light) night_light=1;
	 	mask_next_regim(0b10,215,15);//noch L.5
		if (regim==15) {//night_light
			EEPROM_write(EEPROM_light, day_light);
			EEPROM_write(EEPROM_light+1, night_light);
			//regim=215;
			}
		return;
		}
	if (regim==15) {
		inc_dec_var(&tim_c_light,23);
		mask_next_regim(0b1100000,215,16);
		return;
		}
	if (regim==16) {
		inc_dec_var(&tim_do_light,23);
		mask_next_regim(0b11,215,215);
		if (regim==215) {//night_light
			EEPROM_write(EEPROM_light+2, tim_c_light);
			EEPROM_write(EEPROM_light+3, tim_do_light);
			}
		return;
		}
	//---------------end__light--------------------
	
	
	//------------SIGHAL---------------------------
	if (regim==216){//124
		next_regim();
		mask_next_regim(0xFFFF,0,52);//SIGHAL
		return;
		}
	if (regim==52){
		inc_dec_var(&ocr1[4],1);
		mask_next_regim(0b011111100,216,53);//53budil 54button 
		time[note]=3;
		time_flag|= _BV(fl_signal_cls);
		return;
		}
	if (regim==53) {//H000 L000
	 	inc_dec_var(&ocr1[ocr1[4]*2],0xFF);//ocr1ahi
	 	mask_next_regim(0b011100000,55,54);
		return;
		}
	if (regim==54) {//H000 L000 
	 	inc_dec_var(&ocr1[ocr1[4]*2+1],0xFF);//ocr1ahi
	 	mask_next_regim(0b000000111,55,53);
		return;
		}
	if (regim==55) {
	 	EEPROM_write(EEPROM_verifu_but, 0xaa);
		for(uint8_t i=EEPROM_ocr1a; i<4+EEPROM_ocr1a ; i++)
			EEPROM_write(i, ocr1[i-EEPROM_ocr1a]);
		regim=52;
		return;
		}
	//-------------end__signal--------------
	
	//-------------ds18b20------------------
	if (regim==217) {
		next_regim();
		mask_next_regim(0xFFFF,0,111);
		return; 
		}
	if (regim==111) {
		inc_dec_var(&vid_t,1);//ocr1ahi
		mask_next_regim(0x20,217,112); 
		if ( regim==112 ) EEPROM_write(EEPROM_vid_t,vid_t);
		return;
		}
	if (regim==112) {
		mask_next_regim(0,217,217); 
		if (regim==217) {
			scratchpad_save();		 
			}
		return;
		}
	//-----------end=ds18b20----------------
	
	//-----------СБРОС-----------------------
	if (regim==218) {
	 	if (SET_){
			viewstate=0;
			for(uint8_t i=EEPROM_calls; i < 12*3+EEPROM_calls ; i++){
		  		EEPROM_write( i, 0 ); //
				}
			for(uint8_t i=EEPROM_regim_state_time; i<Nreg_st+EEPROM_regim_state_time ; i++)
				EEPROM_write( i, 0 );
			for(uint8_t i=EEPROM_ocr1a; i<4+EEPROM_ocr1a ; i++)
				EEPROM_write( i, 4 );
			for(uint8_t i=EEPROM_termostat; i<EEPROM_termostat+sizeof(termostat) ; i++)
				EEPROM_write( i, 0);

			EEPROM_write( EEPROM_light,   5);
			EEPROM_write( EEPROM_light+1, 2);
			EEPROM_write( EEPROM_light+2, tim_c_light=23);
			EEPROM_write( EEPROM_light+3, tim_do_light=6);

			EEPROM_write( EEPROM_sig_pin_or_bip, 0);
			EEPROM_write( EEPROM_otsrochka_num, 7);
			EEPROM_write( EEPROM_vid_t, 0);
				
			read_data_EEPROM();
		   }
		next_regim();
	 	mask_next_regim(0xFFFF,0,0);
		return;
		}
	 //_______________________________________________
}

//**************************************
void result_in_bcd(int result)
{//0000 0000 0000 (1)0000  // (2)0000 (3)0000 (4)0000 (5)0000 hi2 hi3 low4 low5
    unsigned char dig_out=0;
	bcd_num1=0;
	bcd_num2=0;
	bcd_num3=0;
	bcd_num4=0;
	while (result >= 10000) {
	 result -= 10000;
 	 dig_out ++;
	}
	bcd_num0=dig_out;
    dig_out = 0;
	while (result >= 1000) {
	 result -= 1000;
 	 dig_out ++;
	}
	bcd_num1=dig_out;
    dig_out = 0;
	while (result >= 100)  {
 	 result -= 100;
	 dig_out ++;
	 }
	bcd_num2=dig_out;
	dig_out = 0;
	while (result >= 10)   {
	 result -= 10;
	 dig_out ++;
	 }
	bcd_num3=dig_out;
	bcd_num4= result;
}
//**************************************
void cls_lcd_buffer(void){
unsigned char i;
 for(i=0;i<9;i++){
	lcd_buffer[i]=0;
	}
}
/**************************************
void load_str_in_lcdbuffer(const unsigned char *str_lcd){
unsigned char b,e,length;//str_lcd[0] = 0000 1111-смещение, 1111 0000 - длина строки
	length=str_lcd[0];
	for( b=0x0F&(length), e=1 ; b<(0x0F&(length>>4)) ; b++, e++){
		lcd_buffer[b]=str_lcd[e];
		}
}
*/
void load_str_in_lcdbuffer(prog_int8_t* str_lcd){
unsigned char b,e,length;//str_lcd[0] = 0000 1111-смещение, 1111 0000 - длина строки
	length=pgm_read_byte_near(&str_lcd[0]);
	for( b=0x0F&(length), e=1 ; b<(0x0F&(length>>4)) ; b++, e++){
		lcd_buffer[b]=pgm_read_byte_near(&str_lcd[e]);
		}
}
//**************************************
void num2_bcd_in_lcd(unsigned char znak_mesto, unsigned char *param_out)
{
 	result_in_bcd(*param_out);
	lcd_buffer[znak_mesto]=font[bcd_num3];
	lcd_buffer[znak_mesto+1]  =font[bcd_num4];
}
void num3_th_in_lcd_buffer(unsigned char m){
	lcd_buffer[m]	=font[bcd_num2];
	lcd_buffer[m+1]	=font[bcd_num3];
	lcd_buffer[m+2]	=font[bcd_num4]^tochka;
}
//**************************************
void blank_nul(unsigned char mesto){
 if (lcd_buffer[mesto]==font[n0]) 
	lcd_buffer[mesto]=0;
}

//**************************************
void clock_in_lcd_gl(unsigned char znakomesto)
{
 	num2_bcd_in_lcd(znakomesto,&hour[h_hour]);
	blank_nul(znakomesto);
	#ifdef miganie
 	if (time_flag&_BV(fl_sek_1_switch))
	#endif
		lcd_buffer[znakomesto+2]=font[f_seg_g];
	num2_bcd_in_lcd(znakomesto+3,&hour[h_min]);
}
//**************************************
void clock_in_lcd0(void)
{
 	num2_bcd_in_lcd(0,&hour[h_num]);
	lcd_buffer[1]^=tochka;
	result_in_bcd(hour[h_day]);
	lcd_buffer[2]=font[bcd_num4];
	clock_in_lcd_gl(4);
}
//**************************************
void day_str(void){
	lcd_buffer[7]=pgm_read_byte_near(&h_day_str[(hour[h_day]-1)*2]);
	lcd_buffer[8]=pgm_read_byte_near(&h_day_str[(hour[h_day]-1)*2+1]);
}
//**************************************
void clock_day_str(void){
	clock_in_lcd_gl(1);
	day_str();
}
//**************************************
void lcd_hh_mm(unsigned char *param){
	num2_bcd_in_lcd(4,param);
	lcd_buffer[6]=font[f_seg_g];
	num2_bcd_in_lcd(7,param-1);
}
//**************************************
void hh_mm_ss(uint8_t *param)
{
	num2_bcd_in_lcd(1,param+2);
	blank_nul(1);
	lcd_buffer[3]=font[f_seg_g];
	lcd_hh_mm(param+1);
}
//**************************************
void num_month_day(void){//iln
	num2_bcd_in_lcd(1, &hour[h_num]);
	lcd_buffer[3]=font[f_seg_d];
	num2_bcd_in_lcd(4, &hour[h_month]);
	lcd_buffer[6]=font[f_seg_d];
	day_str();
}
//**************************************
void tempir_in_bcd(unsigned char t, unsigned char mesto, unsigned char pin_wire, unsigned char point){
unsigned char znak=0, n1=0,n2=0;

	if ( !(pin_wire & out_1wire) ){
	 	lcd_buffer[mesto]	=font[f_seg_g];
		lcd_buffer[mesto+1]	=font[f_seg_g];
		lcd_buffer[mesto+2]	=font[f_seg_g];
		}
		else
		{
	int te;
	te=temperatura[t];

		if ( te<0 ){
			te*=-1;
			znak=1;
			}

		result_in_bcd(te);// 15.7 104.3 -55.2 5.2 -5.3
		
		if (vid_t==0)
		{
			if ( bcd_num1 ) n1=font[bcd_num1];
			else 
			if (znak) 	n1=font[f_seg_g];
			n2=font[bcd_num2];
		} 
		else
		{
			if ( bcd_num1 ) {
			n1=font[bcd_num1];
			n2=font[bcd_num2];
			}
			else
			{
			if ( bcd_num2 ){ 
			n2=font[bcd_num2]; 
			if ( znak ) n1=font[f_seg_g];
			}
			else
			if ( znak ) n2=font[f_seg_g];
			}
		}
		
		lcd_buffer[mesto]=n1;
		lcd_buffer[mesto+1]=n2;
		lcd_buffer[mesto+2]=font[bcd_num3];
		lcd_buffer[mesto+3]=font[bcd_num4];
		
		if (point) lcd_buffer[mesto+2]^=tochka;
			else if (bcd_num4>5) lcd_buffer[mesto+2]^=tochka;
		
		}
}

//**************************************
void tempir__clock_in_lcd(void){
	lcd_buffer[3]=font[f_gr];
	clock_in_lcd_gl(4);
}
void tempir_1_clock_in_lcd(void){
	tempir_in_bcd(ds0,0, pwire0, false );
	tempir__clock_in_lcd();
}
void tempir_2_clock_in_lcd(void){
	tempir_in_bcd(ds1,0,pwire1, false);
	tempir__clock_in_lcd();
}
//**************************************/
void tempir_2_in_lcd(unsigned char point){
	tempir_in_bcd(ds0,0,pwire0, point);
	tempir_in_bcd(ds1,5,pwire1, point);
}
//**************************************/
void tempir_2o_in_lcd(void){//temp o temp
	tempir_2_in_lcd(true);
	lcd_buffer[4]=font[f_gr];
}

void tempir_2oo_in_lcd(void){//temp o temp
	tempir_2_in_lcd(false);
	lcd_buffer[3]=font[f_gr];
	lcd_buffer[8]=font[f_gr];
}
//**************************************/
void grad_celsia(void){
	lcd_buffer[7]=font[f_gr];
	lcd_buffer[8]=font[f_C];
}
//**************************************/
void one_tempir_0_in_lsd(void){
	tempir_in_bcd(ds0,2,pwire0,true);//012345678 //inl
	grad_celsia();
	lcd_buffer[0]=font[f_d]^tochka;
}
//**************************************/
void one_tempir_1_in_lsd(void){
	tempir_in_bcd(ds1,2,pwire1,true);//0-00.0 *c //inl
	grad_celsia();
	lcd_buffer[0]=font[f_y]^tochka;
}
//**************************************/
//void clock_in_lcd1(void)
//{
//	num2_bcd_in_lcd(1,&hour[h_hour]);
//	blank_nul(1);
//	lcd_buffer[3]=font[f_seg_g];
//	lcd_hh_mm(&hour[h_min]);
//}

/**************************************
void timer_in_lcd(void)
{
    if (timer[3]&_BV(fl_timer_chet)) lcd_buffer[0]=font[f_t];
		else lcd_buffer[0]=font[f_seg_d];
	num2_bcd_in_lcd(1,&timer[h_hour]);
	lcd_buffer[3]=font[f_seg_g];
	lcd_hh_mm(&timer[h_min]);
}
**************************************/
void on__(unsigned char zn, unsigned char tr_fl){
unsigned char a,b;
	if (  tr_fl )
		{
		a=font[f_o];		
		b=font[f_n]^tochka;
		}
		else
		{
		a=font[f_seg_d];
		b=font[f_seg_d];
		}
	lcd_buffer[zn]=a;
	lcd_buffer[zn+1]=b;
}



//*************************************
void power_in_lcd(unsigned char pin_wire, unsigned char mesto){
unsigned char a,b;
	if ( !(pin_wire & out_1wire) ){
		a=font[f_seg_g];
		b=font[f_seg_g];
		}
		else
		if (power_suplly_1w & pin_wire )
		{
		a=font[f_P];
		b=font[f_A];	
		}
		else{
		a=font[f_o];
		b=font[f_n];
		}
	lcd_buffer[mesto]=a;
	lcd_buffer[mesto+1]=b;
}


//**************************************
void view_in_indikator(unsigned char view){
/*//тест на сборс датчика, 85,0
if 	(view==0)	{
	tempir_in_bcd(lsb1,5,pwire1, 1);
	
	static uint8_t err=0, byt=0;
	if (lcd_buffer[6]==font[8]) {
		err++;
		byt=tempir[lsb1];
		tempir[2]=0;
		tempir[3]=0;
		}
	num2_bcd_in_lcd(0, &err);
	//result_in_bcd(tempir[lsb1]);
	//num3_th_in_lcd_buffer(0);
	lcd_buffer[3]=font[lcd_light];//count_com_1w
	}
//*/

	//*
	if 			(view==0)	clock_in_lcd0();//num,day, hour-min
	else 	if 	(view==1)	hh_mm_ss(&hour[h_sek]);//hour-min-sek
	else 	if	(view==2)	clock_in_lcd_gl(2);//clock_in_lcd2();//hour-min
	else 	if	(view==3)	clock_day_str();//hour-min day
	else 	if	(view==4)	num_month_day();//num_month day
	else	if	(view==5)	tempir_2oo_in_lcd();
	else	if	(view==6)	one_tempir_0_in_lsd();// _ -000 *C 1датчик
	else	if	(view==7)	one_tempir_1_in_lsd();// _ -000 *C 2датчик
	else	if	(view==8)	tempir_1_clock_in_lcd();// tempir0 hour-min
	else	if	(view==9)	tempir_2_clock_in_lcd();// tempir1 hour-min
	else 	if	(view==10)	tempir_2o_in_lcd();// 00.0 o 00.0
	else	//if	(view==11)	
	tempir_2_in_lcd(true);// -00 -00//
//	else	if	(view==12)	adc_kanal(0);
//	else	adc_kanal(1);
//*/
}
//--------------------------------------
void indikator_manager(void){
unsigned char i;

 i=regim_state_time[Nreg_st];
 if ( ++i>=(Nreg_st/2) ) i=0; // если предыдущий был последний режим то сначало.

	while( regim_state_time[i*2+1] == 0 ){
		if ( i == regim_state_time[Nreg_st]) {
			time_view_buf=1;
			return;
			}
		if ( ++i>=(Nreg_st/2) ) i=0;
		}
	regim_state_time[Nreg_st]=i;
	time_view_buf=regim_state_time[i*2+1];
	viewstate=regim_state_time[i*2];
}


//**************************************/
void data_lcd(void)
{
 	cls_lcd_buffer();
	
	if (regim){
		viewstate=0;
		time_view_buf = 3;
		}
	
	if (regim==0){
		view_in_indikator(viewstate);
		if (time_flag & _BV(fl_sig_otsr))
			if (!(time_flag&_BV(fl_sek_1_switch))) 
				lcd_buffer[8]^=tochka;
		return;
	   }
	
	if (regim==100){// вкл при нулевом режиме если нажата сет
		clock_in_lcd0();//тоесть показ число день время
		if (time_flag & _BV(fl_sig_otsr))	
			lcd_buffer[3]=font[f_o];//if (time_flag & _BV(fl_sig_otsr)) 
		return;
		}
	
	if (regim==7){//0, 0, f_C, f_A, f_L, f_L, f_S, 0, 0, //2,5
		load_str_in_lcdbuffer((prog_uint8_t *)str_calls);//load_str_in_lcdbuffer(&str_[0]);
		return;
	    }
	if  (regim==9){//0, 0, S, E, t, U, P, 0, 0 //2,5
		load_str_in_lcdbuffer((prog_uint8_t *)str_setup);
		return;
		}//*/
	if  (regim==8){
		load_str_in_lcdbuffer((prog_uint8_t *)str_t_stat);
		return;
		}
	if  (regim==215){//0, 0, L, I, G, H, t, 0, 0 //2,5
		load_str_in_lcdbuffer((prog_uint8_t *)str_light);
		return;
		}
		
	if  ((regim>=reg_termostat)&&(regim<=reg_termostat+3)){
		tempir_in_bcd(ds0,0,pwire0,false);
		lcd_buffer[3]=font[f_gr];
		result_in_bcd( temper_termostat );
		num3_th_in_lcd_buffer(4);
		if (!bcd_num2)
			lcd_buffer[4]	=0;
		
		lcd_buffer[7]	=font[f_P];
		if ( PIN_OUT & PIN_T_STAT ) lcd_buffer[8]=font[n1];
		   						   else lcd_buffer[8]=font[n0]; 
		return;
		}
		
	//light	
	if  (regim==11){
		lcd_buffer[0]=font[f_L];
	    for(uint8_t i=0; i<day_light; i++)
			lcd_buffer[i+1]=font[f_o];
		lcd_light=day_light;
		return;
		}
	if  (regim==14){//H, O, 4, b, 0, L., -, 0, 0 //0,6 
		load_str_in_lcdbuffer((prog_uint8_t *)str_light_noch);
		lcd_buffer[7]=font[night_light];
		lcd_light=night_light;
		return;
		}// end light
	if  ( (regim>14)&&(regim<17) ){//H, O, 4, b, 0, L., -, 0, 0 //0,6 
		lcd_buffer[1]=font[f_c];
		num2_bcd_in_lcd(2,&tim_c_light);
		lcd_buffer[5]=font[f_d];
		lcd_buffer[6]=font[f_o];
		num2_bcd_in_lcd(7,&tim_do_light);
		return;
		}// end light
	
	
	//----------ЧАСЫ------------------
	if (regim==210){
		load_str_in_lcdbuffer((prog_uint8_t *)str_clock);
		return;
	    }
	if ((regim>20)&&(regim<25)){
	   	clock_in_lcd0();
		return;
	    }
	if ( regim==25 ){//(regim==25) {//n, n, o, n, t, h, 0, 0, 0,
		load_str_in_lcdbuffer((prog_uint8_t *)str_monht);
		num2_bcd_in_lcd(7,&hour[5]);
		return;
	   	}
		
	if (regim==26){
		hh_mm_ss(&hour[h_sek]);//hour-min-sek корекция часов
		return;
		}
	if ((regim>26)&&(regim<29)){//C, o, r, r., //0x39, 0x9C, 0x14, 0x54,
		load_str_in_lcdbuffer((prog_uint8_t *)str_corr);
		if ( corr_flag&_BV(fl_corr_znak_pl) ) lcd_buffer[4]=font[f_seg_d]; 
	   	  else 							   	 lcd_buffer[4]=font[f_seg_g];   
		lcd_buffer[5]=font[n0]^tochka; 
		result_in_bcd( pgm_read_word_near(&corr_in_lcd[correction]) );
		lcd_buffer[6]	=font[bcd_num2];
		lcd_buffer[7]	=font[bcd_num3];
		lcd_buffer[8]	=font[bcd_num4];
		return;
		}	

	if ((regim>30)&&(regim<35)){
	   	lcd_buffer[0]=font[budilnik_nomer];
		lcd_hh_mm(&calls[ budilnik_nomer ] [c_hour]);
		on__(1, ((calls[ budilnik_nomer ] [2]) & (_BV(7))) );
		return;
		}	
	//debug
	if ((regim>44)&&(regim<48)){
		hh_mm_ss(&calls_pin[budilnik_nomer-10][h_sek]);
		return;
		}	
	if ((regim>34)&&(regim<42)){
	    lcd_buffer[0]=font[f_d]^tochka;
		//-----------
		for (uint8_t i=0, a=1; i<7; i++, a<<=1){
			if ((calls[ budilnik_nomer ] [2])&a)
			lcd_buffer[i+2]=font[i+1]; else lcd_buffer[i+2]=font[f_seg_d];
			}
		return;
		}
	//--SIGNAL--
	if (regim==216){//0, S, I, G, H, A, L, 0, 0 //1,6
		load_str_in_lcdbuffer((prog_uint8_t *)str_signal);
		return;
	    }
	if (regim==52){//0, b, u, d, I, I., 0, 0, S //1,8 
		if (ocr1[4]==0)	load_str_in_lcdbuffer((prog_uint8_t *)str_budil); 
		else				load_str_in_lcdbuffer((prog_uint8_t *)str_button);	
		lcd_buffer[8]=font[f_S];
		return;
		}
	if ((regim>=53)&&(regim<=54)){
		if (ocr1[4]==0){
			signal=10;
			time_signal=10;
			time_flag|= _BV(fl_signal_out_enable);
			}
		lcd_buffer[0]=font[f_H];
		result_in_bcd(ocr1[ocr1[4]*2]);
		num3_th_in_lcd_buffer(1);
		lcd_buffer[5]=font[f_L];
		result_in_bcd(ocr1[ocr1[4]*2+1]);
		num3_th_in_lcd_buffer(6);
		return;
		}
	//--END--SIGNAL--

	   
	if (regim==218){//d, E, F, A, U, L, t, 0, S, //0,9
		load_str_in_lcdbuffer((prog_uint8_t *)str_default);
		return;
	    }
	if (regim==211){//0, 0, P, o, r, t, 0, 0, 0, //2,4
		load_str_in_lcdbuffer((prog_uint8_t *)str_port);
		return;
	    }
	if ( regim==71 ){//_pin-0_ _c	//0, P, i, n, -, 0000 1,4
		load_str_in_lcdbuffer((prog_uint8_t *)str_pin_c);
		if ( PIN_OUT & PIN_calls1 ) lcd_buffer[5]=font[n1];
		   						   else lcd_buffer[5]=font[n0]; 
		lcd_buffer[8]=font[f_C]^tochka;
		return;
	   	}
	//===============================
	//-----------------t-stat-----------------------
	if (regim==212){
		load_str_in_lcdbuffer((prog_uint8_t *)str_t_stat);
		return;
	    }
	if ((regim>=80)&&(regim<=84)) {
		lcd_buffer[0]=font[p_index+1]^tochka;
		if (p_index<kolvo_zon){
			num2_bcd_in_lcd(1,&termostat.time[p_index]);
			lcd_buffer[3]=font[f_seg_g];
			uint8_t j;
			if (p_index<kolvo_zon-1) j=p_index+1; else j=0;
			num2_bcd_in_lcd(4,&termostat.time[j]);
			
			num2_bcd_in_lcd(7,&termostat.temperature[p_index]);
			if (bcd_num2) { lcd_buffer[5]^=tochka; lcd_buffer[6]=font[bcd_num2]; }
			}else{
			load_str_in_lcdbuffer((prog_uint8_t *)str_gisterezis);
			num2_bcd_in_lcd(7,&termostat.gisterezis);
			lcd_buffer[7]^=tochka;
			}
		return;
		}
	//----------------------------------------------
	if ( regim==214 ){//  _yct_bud_ //0, y, C, t, 0, 6, u, d., 0 //1,7 
		load_str_in_lcdbuffer((prog_uint8_t *)str_yst_bud);
		return;
	    }
	if (regim==75){ //CAL. bIP C //C, A, L., //0x39, 0xB7, 0x59, 
		load_str_in_lcdbuffer((prog_uint8_t *)str_cal);
		uint8_t i,a,b;
		if (signal_bud==0){
		   a=font[f_b];
		   b=font[n1];
		   i=font[f_P];
		   }
		   else
		   if (signal_bud==1){
  		   a=font[f_P];
		   b=font[f_seg_e];
		   i=font[f_n];
		   }
		   else{
		   a=font[f_O];
		   b=font[f_b];
		   i=font[f_A];
		   }
		   lcd_buffer[4]=a;
		   lcd_buffer[5]=b;
		   lcd_buffer[6]=i;
		lcd_buffer[8]=font[f_C]^tochka;
		return;
	    }
	if (regim==76){//O, t, S, R, O, 4., 0, 0, 0 //0,6
		load_str_in_lcdbuffer((prog_uint8_t *)str_otsro4);
		//on__(7, time_flag&_BV(fl_otsrochka_true) );
		num2_bcd_in_lcd(7,&otsroshka_num);
		return;
		}
		
	//if 	((regim>79)&&(regim<84)) {timer_in_lcd(); return;}
	
	if (regim==213){//усt see	//{}//0, Y, C, t, 0, S, E, E, 0 //1,7 
	    load_str_in_lcdbuffer((prog_uint8_t *)str_yst_see);
		return;
	    }
	if ((regim>90)&&(regim<94)) {
		lcd_buffer[0]=font[f_P];
		lcd_buffer[1]=font[regim_state_time[Nreg_st]+1];
		lcd_buffer[3]=font[f_r]^tochka;
		lcd_buffer[4]=font[regim_state_time[regim_state_time[Nreg_st]*2] ];
		lcd_buffer[5]=font[f_seg_g];
		result_in_bcd(regim_state_time[regim_state_time[Nreg_st]*2+1]);
		lcd_buffer[6]=font[bcd_num2];
		lcd_buffer[7]=font[bcd_num3];
		lcd_buffer[8]=font[bcd_num4];
		return;
		}
	
	if (regim==217){//d, S, 1, 8, b, 2, 0, 0, F // 0,9
		load_str_in_lcdbuffer((prog_uint8_t *)str_ds18b20);
		return;
		}
	if (regim==111){
		if (vid_t==0) 
			load_str_in_lcdbuffer((prog_uint8_t *)str_vid1_t);
			else
			load_str_in_lcdbuffer((prog_uint8_t *)str_vid2_t);
			
		return;
		}
	if (regim==112){
		load_str_in_lcdbuffer((prog_uint8_t *)str_pit);
		power_in_lcd(pwire0, 4);
		power_in_lcd(pwire1, 7);
		return;
		}
	
	if (regim==150){//бегущая строка str_bud
		lcd_buffer[0]=font[budilnik_nomer];
		lcd_hh_mm(&hour[h_min]);
	/*
	if (time[beg_str]<=0){
			time[beg_str]=45;
			simvol++;
			if (simvol==str_length+9) simvol=0;
			}
		for (a=0, i=simvol; a<9 ; i++, a++){
				if ( (i>8)&&(i<str_length+9) )
					lcd_buffer[a]=*(ukaz_na_str+i-9);//str[i-9];
			}
	*/
		}
}
/**************************************
void str_init(void){
unsigned char h;
	//0-5; 6-11; 12-17; 18-23;
	
	h=hour[h_hour];
	if (h<6)	ukaz_na_str=&str_noch[0];	else
	if (h<12)	ukaz_na_str=&str_utro[0];	else
	if (h<18)	ukaz_na_str=&str_day[0];	else
				ukaz_na_str=&str_vecher[0];

	*(ukaz_na_str+5)=font[budilnik_nomer];
	result_in_bcd(hour[h_hour]);
	*(ukaz_na_str+7)=font[bcd_num3];
	*(ukaz_na_str+8)=font[bcd_num4];
	result_in_bcd(hour[h_min]);
	*(ukaz_na_str+10)=font[bcd_num3];
	*(ukaz_na_str+11)=font[bcd_num4];
}
*/
//**************************************
void init_signal(void){
//	str_init();
	count_note = 0;
	time[note]=3;
	signal=1;
	time_in_regim = time_signal = 59;
	time_flag &=~  _BV(fl_signal_cls); // флаг отключения сигналов
	time_flag |=   _BV(fl_signal_out_enable);// флаг выдачи сигнала
}

void shim(void){
unsigned char tmp;
	if (time[note]<=0)
		{
		tmp=uprav_TCCR1B[count_note];
		if (tmp&0x01)	timer1_init_in_pwm();
			else		timer1_init();
		time[note]=length_note[count_note];
		TCCR1B=tmp;
		if ( ++count_note == 4 ) count_note=0;
		}
}

void budilnik_out(void){
if (!signal_bud){
	shim();
	}
	else 
	if (signal_bud==1){
		PORT_OUT|=PIN_calls_osn;	
		}
	else{
	shim();
	PORT_OUT|=PIN_calls_osn;
	}
}

void signal_off(void)
{
// решение по времени---------------------------
	if ( time_signal==0 )
	{	
		if (signal==0) // если нет сигнала
		{
		if ( time_flag & _BV(fl_sig_otsr) ) // если есть отсрочка
			{
			if ( budilnik_play_num++==otsroshka_num ) {// если отср. наступала X раз, //отключить
				time_flag &= ~_BV(fl_sig_otsr);
				}
				else{// если нет сигнала, если есть отсрочка, то вкл сигнал
				init_signal();
				regim=150;
				signal=1;//sig_on_otsr;
				return;// выйти из функции
				}
			}
		}
		else
		time_flag |= _BV(fl_signal_cls);// конец сигнала по времени
	}
// выключение сигнала
	if ( !( time_flag & _BV(fl_sig_butt) ) )
	if ( time_flag & _BV(fl_signal_cls) )// выключение сигнала
		{
		time_flag &=~  (_BV(fl_signal_cls)|_BV(fl_signal_out_enable));
			
		PORT_OUT &=~PIN_calls_osn;
		//PORT_OUT &=~  PIN_timer;
		timer1_init();
		
		time_signal=0xFF;	
	
		signal=0;
		}
}
//*************************************/

//**************************************
void calls_verify(void)
{
unsigned char i;
	for(i=0;i<12;i++)
	{
	if ( _BV(7) & calls[i][c_day] )				//проверка вкл буд
	 if ( _BV(hour[h_day]-1) & calls[i][c_day] )	//этот день
	  if (calls[i][c_hour]==hour[h_hour])			//час
	   if (calls[i][c_min]==hour[h_min])			//мин
		{
		if (i<10){
			budilnik_nomer=i;
			//signal=1;
			if ( otsroshka_num ) time_flag |= _BV(fl_sig_otsr);
			budilnik_play_num=0;//что бы было одинаковое число отсрочек при наступившем звонке при действуйщей отсрочке
			init_signal();
			regim=150;
			} 
			else{
			time_bud_pin=calls_pin[i-10][h_hour]*3600+calls_pin[i-10][h_min]*60+calls_pin[i-10][h_sek];
			if (time_bud_pin){
				fl_calls_pin|=_BV(fl_calls_pin_A_on);
				PORT_OUT|= PIN_calls1 ;
				}
			}
		}
	}
}
//**************************************
void calls_pin_verify(void){

	if (fl_calls_pin&_BV(fl_calls_pin_A_on))
	{
	if (--time_bud_pin==0xFFFF)
		{
		PORT_OUT&=~ PIN_calls1 ;
		fl_calls_pin&=~_BV(fl_calls_pin_A_on);
		}
	}
}

/**************************************
void timer_verify(void){
	if (timer[3]&_BV(fl_timer_chet))
	{
	if		(!timer[h_hour])
	 if 	(!timer[h_min])
	  if	(!timer[h_sek])	
		{
		//signal=2;
		regim=80;
		timer[3]&=~_BV(fl_timer_chet);
		init_signal();	
		return;
		}
	if ( --timer[h_sek]<0 ){
	  	timer[h_sek]=59;
		timer[h_min]--;
		if ( timer[h_min]<0 ){
		 	timer[h_min]=59;
			timer[h_hour]--;
			}
		}
	}
}
**************************************/

void set_temper_termostat(uint8_t j){
if ( hour[h_hour] == termostat.time[j]) temper_termostat=termostat.temperature[j];
}

//**************************************
void clock(void){

if ( isr_1sek_ )//прошла секунда
  {
	correct=0;
	PORT_sig_min&=~sig_min_00;
	
	hour[h_sek]++;
	if (hour[h_sek]==60)
	{
   	hour[h_sek] =0;
	hour[h_min]++;
	if (hour[h_min]==60)
		{
		if ( corr_flag & _BV(fl_corr_znak_pl) )//корекция часов
			correct=correction;//для убыстрения
			else
			correct= 0x00-correction;
	
		hour[h_min] =0;
		hour[h_hour]++;
		
		if (hour[h_hour]==24)
			{
			hour[h_hour]=0;
			hour[h_day]++;
			hour[h_num]++;
			if (hour[h_day]>7)	hour[h_day]=1;
			if (hour[h_num]>num_in_month[ hour[h_month]-1 ] )
				{
				hour[h_num]=1;
				hour[h_month]++;
				if (hour[h_month]>12) 	{hour[h_month]=1;}
				}
			}
		
		if ( hour[h_hour] == tim_c_light)	lcd_light=night_light;
		if ( hour[h_hour] == tim_do_light)	lcd_light=day_light;
		
		for(uint8_t i=0; i<kolvo_zon; i++)
			set_temper_termostat(i);
		if (!pin_power_)
			if (lcd_light==day_light)
				PORT_sig_min|=sig_min_00;
		}
	calls_verify();
	}
  isr_1sek_=0;
  }
 		
}
//****************************************
void time_count(void){
static unsigned char t, sek;
unsigned char i;
	if ( t != clock_ )
		{//  0,0078125 сек.
		t=clock_;
		if (t>=64) time_flag|=_BV(fl_sek_1_switch); else time_flag&=~_BV(fl_sek_1_switch);
		for (i=0; i<4; i++) time[i]--;
		
		if ( time_flag & _BV(fl_sig_butt) )// прекращение сигнала от кнопок
			if (time[note_but]<0)
			{
			time_flag&= ~ _BV(fl_sig_butt);
			timer1_init();
			}
		
		if (sek!=hour[h_sek]){//прошла секунда
			sek=hour[h_sek];

			if (--time_view_buf == 0 ) indikator_manager();
			if (--time_in_regim == 0 )
						if (regim>=4) regim=0;
			time_signal--;
			calls_pin_verify();// будильник упр нагрузки
			}
		// вывод сигнала
		signal_off();
		if ( time_flag & _BV(fl_signal_out_enable) ) budilnik_out();// здесь выполняется вся работа шим или пин
		if ( time_flag & _BV(fl_power_init_device) )//при питание от батарей
			skankey();
			
		
		if (regim>=4)//if (mask)
			{
			static uint8_t time_mig=0;
			if (++time_mig>=light_time_mig[lcd_light])//(10-lcd_light)
				{
				time_mig=0;
				change_light_var();
				}
			}
		}
}//*/
//*/**************************************
void init_devices_save_mode(void)
{
	TIMSK = _BV(TOIE2);//|(1<<OCIE1A)
	PORTLCD=0;
	PORTSERIAL|=_BV(STORAGE);//все разряды выкл
	#ifdef katod
	data_shift(0xFF);
	#else
	data_shift(0);
	#endif
	PORT_OUT &= ~PIN_T_STAT;// вывод термостата в ноль
}//*/
//**************************************
int main(void)
{
	
	cli();
	read_data_EEPROM();
	timer2_init();
	init_devices();


while(1)
 {//while
 clock();
   if (!pin_power_)
	{//begin power on
	 	if (time_flag&_BV(fl_power_init_device))
			{
			time_flag &= ~( _BV(fl_power_init_device) | _BV(fl_sig_otsr) );
			//time_flag &= ~_BV(fl_sig_otsr);// что бы при появлении пит. не возникало отложенного сигнала
			init_devices();
			time_view_buf=2;
			time_in_regim=2;
			}
		/*рабочий цикл*/   
		if (lcd_time_switch>=30)/*прошло 2msek*/ 
	  	   	{
			lcd_time_switch=0x00;
			znakomesto++;
//DDRB|=1;
//PORTB|=1;
  	   		if  (znakomesto<9) {
				skankey(); // чего то там понажимали:)
				key_action();// среагировали новым состоянием
				light=light_swith[opred_light_current_num()];//определили яркость конкретной цифры
				display();// включили цифру
//if  (znakomesto==8) PORTB&=~1;
				}
				else	
				{
				PORTSERIAL|=_BV(STORAGE);//все разряды выкл
				#ifdef katod
				PORTLCD=0x00;
				#else
				PORTLCD=0xFF;
				#endif
				
				lcd_time_switch=25;
				#ifdef katod
				data_shift(0xFF);
				#else
				data_shift(0);
				#endif
				if (regim){
					if ( hour[h_hour] >= tim_do_light )	lcd_light=day_light; 	else	lcd_light=night_light;
					if ( hour[h_hour] >= tim_c_light )		lcd_light=night_light;
					}
				data_lcd();
				znakomesto=0xFF;
				//izmerenie_t();	/*измерение темпиратуры*/
				}
	  		}

		time_count();
		if (lcd_time_switch<5) izmerenie_t();	/*измерение темпиратуры*/

	}//end power on
	 else
	{
	if (!(time_flag&_BV(fl_power_init_device)))
		{
		time_flag|=_BV(fl_power_init_device);
		init_devices_save_mode();
		}
	
	if ( signal || ( time_flag & _BV(fl_sig_butt) ) )// считаем временные отрезки при вкл сигнале и сигнале от нажатия
		{
		time_count();
		if (key!=17){
			time_flag |=  _BV(fl_signal_cls);
			time_flag &= ~_BV(fl_signal_out_enable);
			}
		}
		else
		{//  не уснет пока идет сигнал от кнопки
		MCUCR = BIT(SM1)|BIT(SM0)|BIT(SE);
		asm("sleep");
		MCUCR=0;
		}

	}
 }//while

}

//**************************************
ISR(SIG_OVERFLOW0){//0,000064
 	lcd_time_switch++;
	if (lcd_time_switch==light)
		#ifdef katod 
		PORTLCD=lcd_buffer[znakomesto];
		#else
		PORTLCD=~lcd_buffer[znakomesto];
		#endif
}
//**************************************
ISR(SIG_OVERFLOW2)
{
 clock_++;
 if (clock_==128)
	{	   	 		  	   	  				   	
	clock_=correct;
	isr_1sek_=1;
  	}
}
