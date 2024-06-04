$regfile = "m48def.dat"                                     '���������� ����������
$crystal = 8000000                                          '���������� ���������
'$lib "mcsbyte.lbx"                                          '���������� ���������� �������

Ddrb = &B00000000
Ddrc = &B0111000                                            '��������� ����� � �����
Ddrd = &B11111111                                           '������� �������������
Portb = &B00011110
Portc = &B1000000
Portd = &B00000000

A Alias Portd.7                                             '��������� ����������� LED
B Alias Portd.5                                             '
C Alias Portd.3                                             '
D Alias Portd.1                                             '
E Alias Portd.0                                             '
F Alias Portd.6                                             '
G Alias Portd.4                                             '
Point Alias Portd.2                                         '

Da Alias Portc.3                                            '������ �����������
Db Alias Portc.4                                            '������ �����������
Dc Alias Portc.5                                            '������ �����������

Water Alias Pinb.4                                          '������ ������������ ����

'A F B G C H D E
'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
'SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS

Const T_displ = 2                                           '�����(ms) ������� �� next �����
Const H = &B01111001                                        '����� H ��� ������������ ����
Const X = 20                                                'debounce (ms)

Dim W As Byte                                               '������������� ���������
Dim Q As Byte                                               '������������� ���������

Dim D_work_h As Byte                                        '������� �������
Dim D_work_l As Byte                                        '������� �������
Dim D_waiting_h As Byte                                     '������ �������
Dim D_waiting_l As Byte                                     '������ �������

'Config Watchdog = 2048                                      '�������� �� ��������� ������ 2 �������
'Start Watchdog

'Gosub Eeprom                                                '��� ������ �� EEPROM
D_work_l = 27
D_work_h = 1
D_waiting_l = 26
D_waiting_h = 1

'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
Do
S:
If Water = 0 Then                                           '���� �������� ������ ����
Portd = H                                                   '�� ���������� HHHHHH

For Q = 0 To 5
Gosub Displ_main
Next Q
Goto S
End If


Gosub Displ_out
Loop
End
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'�������������������������������������������������������������������������������
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
'###############################################################################
'###############################################################################
'###############################################################################
'###############################################################################
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Displ_out:                                                  '����� ������ �� �������
W = D_work_h                                                '���� ������� �����
Portd = Lookup(w , Segments)
Q = 0
Gosub Displ_main

W = D_work_l / 10                                           '���� ������� ������
Portd = Lookup(w , Segments)
Q = 1
Gosub Displ_main

W = D_work_l Mod 10                                         '���� ��������� {������} ����� �����
Portd = Lookup(w , Segments)
Q = 2
Gosub Displ_main
'=====
W = D_waiting_h                                             '���� ������� �����
Portd = Lookup(w , Segments)
Q = 3
Gosub Displ_main

W = D_waiting_l / 10                                        '���� ������� ������
Portd = Lookup(w , Segments)
Q = 4
Gosub Displ_main

W = D_waiting_l Mod 10                                      '���� ��������� {������} ����� �����
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
Data &B11101011 , &B00101000 , &B10110011 , &B10111010 , &B01111000 , &B11011010 , &B11011011 , &B10101010 , &B11111011 , &B111111010
Positions:                                                  '������� ��� ������ �� LED
Data &B0010000 , &B0001000 , &B0000000 , &B0011000 , &B0100000 , &B0101000
