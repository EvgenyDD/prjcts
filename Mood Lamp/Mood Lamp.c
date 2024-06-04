float red, green, blue;     //now colors
unsigned int tred, tgreen, tblue;//tagert colors
float delta_red, delta_green, delta_blue;       //�e�����a �p��a��� �a �a�
unsigned int time;  //����e���oc�� �a�a


void generate_tagert_color(void)
{
      tred = rand();
      tgreen = rand();
      tblue = rand();
      tred = tred & 0xFF;       //to max 255
      tgreen = tgreen & 0xFF;
      tblue = tblue & 0xFF;
};

void delay(unsigned int delayt)               //�a�ep��a delay_ms() �a �xo� �p����ae� �o���o �o�c�a��y( 
{
  while (delayt != 0)
  {
    delay_ms(1);
    delayt--;
  };
};

unsigned int temp1, temp2, temp3, temp4;
============    
       do 
       {
        generate_tagert_color();
        temp1 = (abs(tred - tgreen));
        temp2 = (abs(tred - tblue));
        temp3 = (abs(tgreen - tblue));
       }
       while (( temp1 < 150 ) && (temp2 < 150) && (temp3 < 150));  //ec�� �ce ��e�a �y�y� �p��ep�o pa���e �o �y�e� �e��� ��o �e ���epec�o
      
       if (rand() < 13100)                                         //c �epo���oc��� 0,4 (32767) �����ae� o��� ��e�
       {
         do
         {
            temp4 = rand() & 0x000F ;
         }
        while ((temp4 == 0) || (temp4 > 3));
       
        if (temp4 == 1)                                            //�e �o�e��o �pac�o�y
        {                                                          //�ac�� �o 0 - 20% o� �cxo��o�o
          do
           {
             temp4 = rand() & 0x0FFF;
           }
          while ((temp4 == 0) || (temp4 > 3276));
          
          tred = ceil ((float) (temp4 / 32767) * tred);
        };
        
        if (temp4 == 2)
        {
          do
           {
             temp4 = rand() & 0x0FFF;
           }
          while ((temp4 == 0) || (temp4 > 3276));
          
          tgreen = ceil ((float) (temp4 / 32767) * tgreen);
        };
        
        if (temp4 == 3)
        {
          do
           {
             temp4 = rand() & 0x0FFF;
           }
          while ((temp4 == 0) || (temp4 > 3276));
          
          tblue = ceil ((float) (temp4 / 32767) * tblue);
        };
        
       }
       
  //��a�e��e �p��a���. �oxo��� �o �y��o�o ��e�a �a 255 �a�o�
       delta_red = (float) (tred - red) / 255;
       delta_green = (float) (tgreen - green) / 255;
       delta_blue = (float) (tblue - blue) / 255; 
      
       
       do
        {
          temp4 = rand() & 0x00FF;
        }
       while ((temp4 < 50) || (temp4 > 100));   //c�o�poc�� �epexo�a c�y�a��a o� 5 �o 10c
      
       time = ceil( ((float) temp4 / 255) * 10);             //�a�ep��a �o���a ���� �e�o�
       
            
       do 
       {
        if (tred != ceil(red))
         red = red + delta_red;
        if (tgreen != ceil(green))
         green = green + delta_green;
        if (tblue != ceil(blue))
         blue = blue + delta_blue;
       
        OCR1A = ceil(blue);
        OCR1B = ceil(green);
        OCR2 = ceil(red);
      
        delay(time);
       }
       while ((tred != ceil(red)) || (tgreen != ceil(green)) || (tblue != ceil(blue))); 
       
       delay(time * 255);                         //c�pe����c� � ��o�y ��e�y, �a�ep��� e�o
