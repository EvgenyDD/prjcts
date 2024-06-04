$regfile = "m48def.dat"                                     '���������� ����������
$crystal = 8000000                                          '���������� ���������
'$lib "mcsbyte.lbx"                                          '���������� ���������� �������

Ddrb = &B00000000
Ddrc = &B0111000                                            '��������� ����� � �����
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
Indicator Alias Portb.0                                     '
Rele Alias Portb.5                                          '����� �� ���� ������
Water Alias Pinb.4                                          '������ ������������ ����

'A F B G C H D E
'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
Dim W As Byte                                               '������������� ���������
Dim Q As Byte                                               '������������� ���������

Dim M As Byte                                               '��������� ���������/���������� ������
'--------------------���������� � �������---------------------------------------
Dim N As Integer                                            '����� ������������ ������� === ��������� �������
Dim Seconds As Word                                         '��������� �������

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
Uph = 1
Upl = 50
Dnh = 1
Dnl = 20
M = 1
'�������������������������������������������������������������������������������
Const T_displ = 2                                           '�����(ms) ������� �� next �����
Const H_water = &B01111001                                  '����� H ��� ������������ ����
Const Deb = 20                                              'debounce (ms)
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

'Reset Watchdog
Gosub Displ_n

Loop
End
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
Int_0:                                                      'interrupts � ���������/���������� ������
Incr N

If N > 1800 Then                                            '��������� ������� - 1��� [3906.25]
Incr Seconds
N = 0
End If

Return
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
'###############################################################################
'###############################################################################
'###############################################################################
'###############################################################################
Displ_n:                                                    '��������(���������� ������)
If M = 0 Then                                               '���� ����� ��������

  If Ddnl = 0 And Ddnh = 0 Then
       M = 1
       Rele = 1
       Goto K
  End If

If Ddnl = 0 Or Ddnl = 255 Then
     Ddnl = 59
End If

 Dupl = Upl
 Duph = Uph
 W = Seconds Mod 60
 Q = Seconds / 60
 Ddnl = Dnl - W                                             '������ ��������� � ���� �������
 Ddnh = Dnh - Q

 Indicator = 0
End If
'===============================================================================
If M = 1 Then                                               '���� ����� �������

  If Dupl = 0 And Duph = 0 Then
       M = 0
       Rele = 0
       Goto K
  End If

If Dupl = 0 Or Dupl = 255 Then
     Dupl = 59

End If

 W = Seconds Mod 60
 Q = Seconds / 60
 Dupl = Upl - W
 Duph = Uph - Q
 Ddnl = Dnl                                                 '������ ��������� � ���� �������
 Ddnh = Dnh

 Indicator = 1
End If

 Goto L
K:                                                          '����� ������ - ��������� � default
 Seconds = 0
 Dupl = Upl
 Duph = Uph
 Ddnl = Dnl
 Ddnh = Dnh
L:
Gosub Displ_out
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
