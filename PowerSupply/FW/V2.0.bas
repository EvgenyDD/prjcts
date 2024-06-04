$regfile = "m16def.dat"
$crystal = 20000000
                        '$lib "mcsbyte.lbx"
$baud = 19200

Ddra = &B00000000
Ddrb = &B11110000
Ddrc = &B11000011
Ddrd = &B01110000

Porta = &B00000000
Portb = &B00001100
Portc = &B00001100
Portd = &B00001111
'-------------------------------------------------------------------------------
Const Disp_time = 500                                       '���
'-------------------------------------------------------------------------------
Key Alias Portc.7                                           '���� ���������� ������
Rele Alias Portc.6                                          '���� ������������ ���������� (15 ��� 26 �����)
U_pwm Alias Pwm1b                                           '��� ����������
Fan_pwm Alias Pwm1a                                         '����������� �����������
'Buzzer Alias Pwm2
Button Alias Pind.1                                         '������ - �����
'-------------------------------------------------------------------------------
Dim Mode_n As Byte
Dim Disp_n As Byte
Dim Enc As Byte

Dim Disp As Byte
Dim Digit As Byte

Dim Temp As Single

Dim U_real As Word
Dim I_12 As Word                                            '�������� ���� � ��       ����� 12 �����      (2��-1.999�)
Dim I_5 As Word                                             '�������� ���� � ��       ����� 5 �����       (2��-1.999�)
Dim I_800 As Word                                           '�������� ���� � ��       ������������ �����  (1��-0.800�)
Dim I_99 As Word                                            '�������� ���� � 100��    ������������ �����  (0.10�-9.90�)

Dim Um_real As Single
Dim Im_12 As Single                                         '�������� ���� � ��       ����� 12 �����      (2��-1.999�)
Dim Im_5 As Single                                          '�������� ���� � ��       ����� 5 �����       (2��-1.999�)
Dim Im_800 As Single                                        '�������� ���� � ��       ������������ �����  (1��-0.800�)
Dim Im_99 As Single                                         '�������� ���� � 100��    ������������ �����  (0.10�-9.90�)

Dim Ic_12 As Word                                           '�������� ��������        ����� 12 �����      (2��-1.999�)
Dim Ic_5 As Word                                            '�������� ��������        ����� 5 �����       (2��-1.999�)
Dim Ic_800 As Word                                          '�������� ��������        ������������ �����  (1��-0.800�)
Dim Ic_99 As Word                                           '�������� ��������        ������������ �����  (0.10�-9.90�)

Dim U_temp As Word
Dim I_set As Word
Dim I_measure As Word

Dim I_string As Byte                                        '������� ���� (1-6)

Dim Blu_led As Bit                                          '����� LED � ������
Dim Overload As Bit                                         '������� LED ���������
Dim Btn As Bit

Dim Cycle_fan As Word
Dim Cycle As Word
Dim X As Byte
Dim Y As Word
Dim Z As Word
Dim Uart As Word

'-------------------------------------------------------------------------------
Config Int0 = Falling : Config Int1 = Falling : Config Int2 = Falling
Config Adc = Single , Prescaler = 128 , Reference = Aref    'ADC
'Config Timer0 = Timer , Prescale = 64
Config Timer1 = Pwm , Pwm = 10 , Compare A Pwm = Clear Down , Compare B Pwm = Clear Down , Noise Cancel = 1 , Prescale = 1
Config Timer2 = Pwm , Prescale = 1024 , Pwm = On , Compare Pwm = Clear Up

'On Timer0 Int_time
On Int0 Encoder_button
On Int1 Encoder0
On Int2 On_off

'Start Timer0
Start Timer1 : Start Timer2
Start Adc

Enable Int0 : Enable Int1 : Enable Int2
'Enable Timer0
Enable Interrupts

$include "DISPLAY.BAS"

U_pwm = 450

Disp_n = 0
I_string = 3
Cycle_fan = 0 : Cycle1 = 0
Rele = 0
Key = 1
Overload = 0
Btn = 0

Gosub Fan
If Button = 0 Then : Gosub Calibrate : End If

'//////////////////////MAIN/////////////////////////////////////////////////////
'###############################################################################
'###############################################################################
Do
   If Button = 0 Then                                       '���������� �� ������ � LED
 A:
   If Button = 0 Then : Goto A : End If
      Incr Disp_n : Waitms 50
      If Disp_n = 3 Then : Disp_n = 0 : End If              'limitting
   End If

Gosub Measuring

On Disp_n Gosub Display_main , Display_sub5 , Display_sub12 '��� ������������ ������
Gosub Display_2                                             '������������ ����� � ��������� ����������

Gosub Fan                                                   '��������� ����������� � ��������� �����������
   'Sound  Portd.1 , 100 , 500

'/////////////////////////////////UART//////////////////////////////////////////
'Incr Uart
If Uart = 200 Then
   Print Ic_12 ; "  =Ic_12-  " ; Ic_5 ; "  =Ic_5-  " ; Ic_800 ; "  =Ic_800-  " ; Ic_99 ; "  =Ic_99"
   Print Im_12 ; "  =I_12 -  " ; Im_5 ; "  =I_5 -  " ; Im_800 ; "  =I_800 -  " ; Im_99 ; "  =I_99"
   Print " "
   Uart = 0
End If

'///////////////////////////////////////////////////////////////////////////////

'Fan_pwm = 1023
'Toggle Portb.3

Loop
'###############################################################################
'###############################################################################







'/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/INTs/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
Encoder0:                                                   '�������
 Waitus 800
 If Pind.0 = 0 Then
   If U_pwm <= 1019 Then : U_pwm = U_pwm + 4 : End If       'Enc = 1
 Else
   If U_pwm >= 4 Then : U_pwm = U_pwm - 4 : End If          'Enc = 2
 End If
 Gifr = 128
Return

'*******************************************************************************
On_off:                                                     '������ ��������� ��������
 If Key = 0 Then
   Key = 1 : Blu_led = 1
 Else
   Key = 0 : Blu_led = 0
 End If
 Gifr = 32
Return

'*******************************************************************************
Encoder_button:                                             '������ ��������
   Incr Mode_n
   If Mode_n = 3 Then : Mode_n = 0 : End If

   If Mode_n <> 1 Then : U_temp = U_pwm
   Else : I_set = 7500
   End If

 Gifr = 64
Return

'*******************************************************************************
Int_time:                                                   '��������� ���� � ������

   If Mode_n <> 1 Then
      If I_measure > I_set Then : Overload = 1 : Key = 0 : End If
   End If

   If Mode_n = 1 Then
      If I_measure > I_set Then
         Decr U_pwm
      End If
   Else
      If U_pwm < U_temp Then
         Incr U_pwm
      End If
   End If
Return
'/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/







'//////////////////////SUBs/////////////////////////////////////////////////////

'0000000000000000000000000000000000000000000000000000000000000000000000000000000
'0000000000000000000000000000000000000000000000000000000000000000000000000000000
'0000000000000000000000000000000000000000000000000000000000000000000000000000000
Measuring:                                                  '��������� ���� � ������

 Incr Cycle1
 If Cycle1 = 21 Then
   Um_real = Um_real * 0.2711                               '��������� ����������
   Um_real = Um_real + 1.3205
   U_real = Um_real

'////////
   Im_5 = Im_5 - Ic_5                                       '2.198*x
'   Im_5 = Im_5 * 2.198                                     '50.9269
   If Im_5 < 1 Then : I_5 = 0 : Else : I_5 = Im_5 : End If
   '////////
   Im_12 = Im_12 - Ic_12                                    '2.0889*x
 '  Im_12 = Im_12 * 2.0889                                   '34.5523
   If Im_12 < 1 Then : I_12 = 0 : Else : I_12 = Im_12 : End If
   '////////
   Im_800 = Im_800 - Ic_800                                 '0.9*x
 '  Im_800 = Im_800 * 0.9                                    '11.4781
   If Im_800 < 1 Then : I_800 = 0 : Else : I_800 = Im_800 : End If
   '////////
   Im_99 = Im_99 - Ic_99                                    '9.054*x
'   Im_99 = Im_99 * 9.054                                     '184.8688
   If Im_99 < 1 Then : I_99 = 0 : Else : I_99 = Im_99 : End If

   Cycle1 = 0
   Im_99 = 0 : Im_800 = 0 : Im_12 = 0 : Im_5 = 0 : Um_real = 0
 Else
   Gosub Step_meter
 End If

Return

'0000000000000000000000000000000000000000000000000000000000000000000000000000000
Step_meter:

   Y = Getadc(5)
   Temp = Y / 20
   Um_real = Um_real + Temp

   Y = Getadc(1)
   Temp = Y / 20
   Im_5 = Im_5 + Temp

   Y = Getadc(3)
   Temp = Y / 20
   Im_12 = Im_12 + Temp

   Y = Getadc(2)
   Temp = Y / 20
   Im_800 = Im_800 + Temp

   Y = Getadc(0)
   Temp = Y / 20
   Im_99 = Im_99 + Temp

Return

'0000000000000000000000000000000000000000000000000000000000000000000000000000000
Fan:                                                        '��������� ����������� � ��������� �����������

 Incr Cycle_fan

 If Cycle_fan = 1000 Then
   Y = Getadc(4)                                            '������������������ �� ���������

   Select Case Y
      Case 0 To 470 : Fan_pwm = 0
      Case 471 To 484 : Fan_pwm = 60
      Case 485 To 499 : Fan_pwm = 100
      Case 500 To 519 : Fan_pwm = 120
      Case 520 To 539 : Fan_pwm = 150
      Case 540 To 559 : Fan_pwm = 170
      Case 560 To 579 : Fan_pwm = 200
      Case 580 To 598 : Fan_pwm = 220
      Case 600 To 1023 : Fan_pwm = 1023
   End Select

   Cycle_fan = 0
 End If

Return

'0000000000000000000000000000000000000000000000000000000000000000000000000000000
Button_check:

 If Pinc.2 = 0 Then : I_string = 2
 End If

 If Pinc.3 = 0 Then : I_string = 4
 End If

Return
'###############################################################################





'0000000000000000000000000000000000000000000000000000000000000000000000000000000
Calibrate:
 For X = 1 To 20
   Gosub Step_meter
 Next X
   Ic_5 = Im_5
   Ic_12 = Im_12
   Ic_800 = Im_800
   Ic_99 = Im_99

Return