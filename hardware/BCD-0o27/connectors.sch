EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 8
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:USB_C_Receptacle J?
U 1 1 6143B94C
P 1300 2400
F 0 "J?" H 1407 3667 50  0000 C CNN
F 1 "USB_C_Receptacle" H 1407 3576 50  0000 C CNN
F 2 "" H 1450 2400 50  0001 C CNN
F 3 "https://www.usb.org/sites/default/files/documents/usb_type-c.zip" H 1450 2400 50  0001 C CNN
	1    1300 2400
	1    0    0    -1  
$EndComp
$Comp
L Power_Protection:USBLC6-2SC6 U?
U 1 1 6143B954
P 3350 3500
F 0 "U?" H 3450 3900 50  0000 C CNN
F 1 "USBLC6-2SC6" H 3700 3150 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 3350 3000 50  0001 C CNN
F 3 "https://www.st.com/resource/en/datasheet/usblc6-2.pdf" H 3550 3850 50  0001 C CNN
F 4 " C2687116" H 3350 3500 50  0001 C CNN "#LCSC"
	1    3350 3500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0110
U 1 1 6143B962
P 3350 4100
F 0 "#PWR0110" H 3350 3850 50  0001 C CNN
F 1 "GND" H 3355 3927 50  0000 C CNN
F 2 "" H 3350 4100 50  0001 C CNN
F 3 "" H 3350 4100 50  0001 C CNN
	1    3350 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 1900 2000 1900
Wire Wire Line
	2000 1900 2000 1950
Wire Wire Line
	2000 2000 1900 2000
Connection ~ 2000 1950
Wire Wire Line
	2000 1950 2000 2000
Wire Wire Line
	1900 2100 2000 2100
Wire Wire Line
	2000 2100 2000 2150
Wire Wire Line
	2000 2200 1900 2200
Connection ~ 2000 2150
Wire Wire Line
	2000 2150 2000 2200
Text Label 2050 2150 0    50   ~ 0
USB_CONN_D+
Wire Wire Line
	2000 2150 2600 2150
Wire Wire Line
	2900 3400 2950 3400
Wire Wire Line
	2400 3600 2950 3600
Wire Wire Line
	3750 3600 4350 3600
Wire Wire Line
	3750 3400 3800 3400
Text Label 2400 3600 0    50   ~ 0
USB_CONN_D-
Text Label 3800 3600 0    50   ~ 0
USB_CONN_D+
Wire Wire Line
	1900 1600 2000 1600
Wire Wire Line
	1900 1700 2000 1700
$Comp
L power:GND #PWR0112
U 1 1 61591F6F
P 1300 4100
F 0 "#PWR0112" H 1300 3850 50  0001 C CNN
F 1 "GND" H 1305 3927 50  0000 C CNN
F 2 "" H 1300 4100 50  0001 C CNN
F 3 "" H 1300 4100 50  0001 C CNN
	1    1300 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 4000 1300 4100
NoConn ~ 1000 4000
Wire Wire Line
	2000 1950 2600 1950
Text Label 2050 1950 0    50   ~ 0
USB_CONN_D-
Text HLabel 2900 3400 0    50   Output ~ 0
USB_D-
Text HLabel 3800 3400 2    50   Output ~ 0
USB_D+
Text HLabel 2000 1600 2    50   Output ~ 0
USB_CC1
Text HLabel 2000 1700 2    50   Output ~ 0
USB_CC2
Wire Wire Line
	3350 1400 3350 3100
Wire Wire Line
	3350 3900 3350 4100
NoConn ~ 1900 2400
NoConn ~ 1900 2500
NoConn ~ 1900 2700
NoConn ~ 1900 2800
NoConn ~ 1900 3000
NoConn ~ 1900 3100
NoConn ~ 1900 3300
NoConn ~ 1900 3400
NoConn ~ 1900 3600
NoConn ~ 1900 3700
Wire Wire Line
	1900 1400 2000 1400
Text HLabel 2000 1100 1    50   Output ~ 0
USB_VBUS
Wire Wire Line
	2000 1100 2000 1400
Connection ~ 2000 1400
Wire Wire Line
	2000 1400 3350 1400
$EndSCHEMATC
