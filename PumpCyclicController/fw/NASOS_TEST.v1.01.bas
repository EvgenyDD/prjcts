$regfile = "m48def.dat"                                     '���������� ����������
$crystal = 8000000                                          '���������� ���������
'$lib "mcsbyte.lbx"                                          '���������� ���������� �������

Ddrb = &B00100000
Ddrc = &B0111100                                            '��������� ����� � �����
Ddrd = &B11111111                                           '������� �������������
Portb = &B00011110
Portc = &B1000000
Portd = &B00000000

'A Alias Portd.7                                             '��������� ����������� LED
'B Alias Portd.5                                             '
'C Alias Portd.3                                             '
'D Alias Portd.1                                             '
'E Alias Portd.0                                             '
'F Alias Portd.6                                             '
'G Alias Portd.4                                             '
Point Alias Portd.2

'Da Alias Portc.3                                            '������ �����������
'Db Alias Portc.4                                            '������ �����������
'Dc Alias Portc.5                                            '������ �����������

Plus Alias Pinb.1                                           '
Minus Alias Pinb.2                                          '
Modes Alias Pinb.3                                          '
Off_led Alias Portb.0                                       '
On_led Alias Portc.2
Rele Alias Portb.5                                          '����� �� ���� ������
Water Alias Pinb.4                                          '������ ������������ ����

'A F B G C H D E
'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
Dim W As Byte                                               '������������� ���������
Dim Q As Byte                                               '������������� ���������

Dim M As Byte                                               '��������� ���������/���������� ������
Dim Position As Byte                                        '������� ������� ��� ��������� �������
Dim B As Bit

'--------------------���������� � �������---------------------------------------
Dim N As Integer                                            '����� ������������ ������� === ��������� �������

Dim Uph As Byte                                             '����� �� ������ ������ ��������
Dim Upl As Byte                                             '����� �� ������ ������
Dim Dnh As Byte                                             '����� �� ��������
Dim Dnl As Byte                                             '����� �� ��������

Dim Duph As Byte                                            '������� ������� �� �����
Dim Dupl As Byte                                            '������� �������
Dim Ddnh As Byte                                            '������ �������
Dim Ddnl As Byte                                            '������ �������
'-------------------------------------------------------------------------------
'Config Watchdog = 2048                                      '�������� �� ��������� ������ 2 �������
'Start Watchdog
Config Timer0 = Timer , Prescale = 8                        '����������� ������
On Timer0 Int_0                                             '������ �� ������� 0
Enable Interrupts
Enable Timer0                                               '��������� ������ ������� 0

'Gosub Eeprom                                                '��� ������ �� EEPROM
Uph = 0
Upl = 49
Dnh = 4
Dnl = 37
M = 1
Position = 0
'�������������������������������������������������������������������������������
Const T_displ = 2                                           '�����(ms) ������� �� next �����
Const H_water = &B01111001                                  '����� H ��� ������������ ����
Const Deb = 20                                              'debounce (ms)
Const Time_n = 4086                                         '��������� ������� - 1��� [3906.25]
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
 Dupl = Upl
 Duph = Uph
 Ddnl = Dnl
 Ddnh = Dnh
Do
S:
If Water = 0 Then                                           '���� �������� ������ ����
Portd = H_water                                             '�� ���������� HHHHHH
For Q = 0 To 5
Gosub Displ_main
Next Q
Goto S
End If

If Modes = 0 Then
   Disable Interrupts
   Incr Position
   If Position = 7 Then
     Position = 0
   End If
End If
While Position <> 0

Wend

'Reset Watchdog
Gosub Displ_out

Loop
End
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
Int_0:                                                      'interrupts � ���������/���������� ������
Incr N
'===============================================================================
If M = 0 And N > Time_n Then
N = 0
 Decr Ddnl
 Off_led = 1
 On_led = 0

   If Ddnl = 0 And Ddnh = 0 Then
       M = 1
       Rele = 1

       Goto K
   End If

 If Ddnl = 255 Then
  Decr Ddnh
  Ddnl = 59
 End If
End If
'&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
If M = 1 And N > Time_n Then
N = 0
 Decr Dupl
 Off_led = 0
 On_led = 1

   If Dupl = 0 And Duph = 0 Then
       M = 0
       Rele = 0

       Goto K
   End If

 If Dupl = 255 Then
  Decr Duph
  Dupl = 59
 End If
End If
'&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
 Goto L
K:                                                          '����� ������ - ��������� � default
 Dupl = Upl
 Duph = Uph
 Ddnl = Dnl
 Ddnh = Dnh
L:
Return
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Change:

Enable Interrupts
Return
'###############################################################################
'###############################################################################
'###############################################################################
'###############################################################################
Up_down:                                                    '��������� ������ ��� ������� �� ������ � ������ ������ �����

On Position Goto End_on , C_1 , C_2 , C_3 , C_4 , C_5 , C_6 '����� ���������� �����
'End If

C_1:
W = Uph                                                     '+/- �����
Gosub T1
Uph = W
Goto End_on

C_2:
Q = 10                                                      '��� ������� (+) ������� ������ +/- �� Q
W = Upl
Gosub T2
Upl = W
Goto End_on

C_3:
Q = 1                                                       '��� ������� (+) ������� ������ +/- �� Q
W = Upl
Gosub T2
Upl = W
Goto End_on

C_4:
W = Dnh                                                     '+/- �����
Gosub T1
Dnh = W
Goto End_on

C_5:
Q = 10                                                      '��� ������� (+) ������� ������ +/- �� Q
W = Dnl
Gosub T2
Dnl = W
Goto End_on

C_6:
Q = 1                                                       '��� ������� (+) ������� ������ +/- �� Q
W = Dnl
Gosub T2
Dnl = W
Goto End_on
'8888888888888888888888888888888888888888888888888888888888888888888888888888888
T1:                                                         '���������� ����������� ����� �� ����
If B = 1 Then
Incr W
Else
Decr W
End If
'==========
If W > 9 Or W < 0 Then
W = 0
End If
Return

T2:                                                         '���������� ����������� ����� �� ����
If B = 1 Then
W = W + Q
Else
W = W - Q
End If
'==========
If W < 0 Or W > 59 Then
W = 0
End If
Return

End_on:
Return
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Displ_out:                                                  '����� ������ �� �������
W = Duph                                                    '���� ������� �����
Portd = Lookup(w , Segments)
Set Point
Q = 0
Gosub Displ_main

W = Dupl / 10                                               '���� ������� ������
Portd = Lookup(w , Segments)
Q = 1
Gosub Displ_main

W = Dupl Mod 10                                             '���� ��������� {������} ����� �����
Portd = Lookup(w , Segments)
Q = 2
Gosub Displ_main
'=====
W = Ddnh                                                    '���� ������� �����
Portd = Lookup(w , Segments)
Set Point
Q = 3
Gosub Displ_main

W = Ddnl / 10                                               '���� ������� ������
Portd = Lookup(w , Segments)
Q = 4
Gosub Displ_main

W = Ddnl Mod 10                                             '���� ��������� {������} ����� �����
Portd = Lookup(w , Segments)
Q = 5
Gosub Displ_main
Return
'------------------
Displ_main:                                                 '���� q - ����� ����������(0-5)+��� T_displ
W = Lookup(q , Positions)
Portc = W Or Pinc
Waitms T_displ
Reset Portc.3
Reset Portc.4
Reset Portc.5
Return
'###############################################################################
'#########################' 'A F B G C H D E '##################################
'###############################################################################
'###############################################################################
Segments:                                                   '������� ��� ������ �� LED
Data &B11101011 , &B00101000 , &B10110011 , &B10111010 , &B01111000 , &B11011010 , &B11011011 , &B10101000 , &B11111011 , &B111111010
Positions:                                                  '������� ��� ������ �� LED
Data &B0010000 , &B0001000 , &B0110000 , &B0011000 , &B0100000 , &B0101000