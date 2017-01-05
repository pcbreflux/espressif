EESchema Schematic File Version 2
LIBS:module
LIBS:ESP32-adapter-cache
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:ESP32-HELP-Button-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
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
L ES-WROOM-32 U1
U 1 1 586CCFFB
P 5050 2750
F 0 "U1" H 5025 4096 60  0000 C CNN
F 1 "ES-WROOM-32" H 5025 3986 60  0000 C CNN
F 2 "module:ESP-WROOM-32-short" H 5500 2450 60  0001 C CNN
F 3 "http://espressif.com/sites/default/files/documentation/esp_wroom_32_datasheet_en.pdf" H 5500 2450 60  0001 C CNN
F 4 "espressif" H 5050 2750 60  0001 C CNN "MFG Name"
F 5 "esp-wroom-32" H 5050 2750 60  0001 C CNN "MFG Part Num"
F 6 "ESP-32S" H 5050 2750 60  0001 C CNN "1st Distrib PN"
F 7 "http://www.analoglamb.com/product/esp3212-wifi-bluetooth-combo-module/" H 5050 2750 60  0001 C CNN "1st Distrib Link"
F 8 "ESP-32S-ALB" H 5050 2750 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.analoglamb.com/product/esp-32s-alb/" H 5050 2750 60  0001 C CNN "2nd Distrib Link"
	1    5050 2750
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X05 P1
U 1 1 586CD0A1
P 7150 2500
F 0 "P1" H 7231 2588 50  0000 L CNN
F 1 "CONN_01X05" H 7231 2495 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x05_Pitch2.54mm" H 7231 2402 50  0000 L CNN
F 3 "http://www.molex.com/webdocs/datasheets/pdf/en-us//0022285051_PCB_HEADERS.pdf" H 7150 2500 50  0001 C CNN
F 4 "Molex" H 7150 2500 60  0001 C CNN "MFG Name"
F 5 "22-28-4012" H 7150 2500 60  0001 C CNN "MFG Part Num"
F 6 "pin header 40 pin male 2.54mm" H 7150 2500 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=pin+header+40+pin+male+2.54mm" H 7150 2500 60  0001 C CNN "1st Distrib Link"
F 8 "538-22-28-5051" H 7150 2500 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Molex/22-28-5051/?qs=sGAEpiMZZMs%252bGHln7q6pm8uos17B4BQA0VgWHqJKoxo%3d" H 7150 2500 60  0001 C CNN "2nd Distrib Link"
	1    7150 2500
	1    0    0    -1  
$EndComp
$Comp
L Battery BT1
U 1 1 586CD133
P 2350 2650
F 0 "BT1" H 2461 2743 50  0000 L CNN
F 1 "Battery" H 2461 2650 50  0000 L CNN
F 2 "module:cr2032_bat_holder" H 2461 2557 50  0000 L CNN
F 3 "http://www.mouser.com/ds/2/238/bat-hld-001-220194.pdf" V 2350 2710 50  0000 C CNN
F 4 "Linx Technologies" H 2350 2650 60  0001 C CNN "MFG Name"
F 5 "BAT-HLD-001" H 2350 2650 60  0001 C CNN "MFG Part Num"
F 6 "712-BAT-HLD-001" H 2350 2650 60  0001 C CNN "1st Distrib PN"
F 7 "http://www.mouser.de/ProductDetail/Linx-Technologies/BAT-HLD-001/?qs=sGAEpiMZZMtz8P%2feuiupSRWPoTl3m4cbQyLs9uuV7KM%3d" H 2350 2650 60  0001 C CNN "1st Distrib Link"
F 8 "BAT-HLD-001-THM-ND" H 2350 2650 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.digikey.com/product-detail/en/linx-technologies-inc/BAT-HLD-001-THM/BAT-HLD-001-THM-ND/3044009" H 2350 2650 60  0001 C CNN "2nd Distrib Link"
	1    2350 2650
	1    0    0    -1  
$EndComp
$Comp
L Q_NMOS_GDS Q1
U 1 1 586CD292
P 3250 4350
F 0 "Q1" H 3456 4443 50  0000 L CNN
F 1 "Q_NMOS_GDS" H 3456 4350 50  0000 L CNN
F 2 "w_smd_trans:sot23" H 3456 4257 50  0000 L CNN
F 3 "http://www.aosmd.com/pdfs/datasheet/AO3406.pdf" H 3250 4350 50  0000 C CNN
F 4 "Alpha & Omega Semiconductor Inc." H 3250 4350 60  0001 C CNN "MFG Name"
F 5 "AO3406" H 3250 4350 60  0001 C CNN "MFG Part Num"
F 6 "ao3406 100pcs" H 3250 4350 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=ao3406+100pcs" H 3250 4350 60  0001 C CNN "1st Distrib Link"
F 8 "785-1005-1-ND" H 3250 4350 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.digikey.com/product-detail/en/alpha-omega-semiconductor-inc/AO3406/785-1005-1-ND/1855947" H 3250 4350 60  0001 C CNN "2nd Distrib Link"
	1    3250 4350
	1    0    0    -1  
$EndComp
$Comp
L SW_PUSH SW1
U 1 1 586CD64A
P 2950 3700
F 0 "SW1" H 2950 4053 50  0000 C CNN
F 1 "SW_PUSH" H 2950 3960 50  0000 C CNN
F 2 "module:SMD_Reed_Switch" H 2950 3867 50  0000 C CNN
F 3 "http://www.mouser.com/ds/2/240/Littelfuse_Reed_Switches_MDSM_4_Datasheet.pdf-938384.pdf" H 2950 3700 50  0000 C CNN
F 4 "Hamlin / Littelfuse" H 2950 3700 60  0001 C CNN "MFG Name"
F 5 "MDSM-4R-7-13" H 2950 3700 60  0001 C CNN "MFG Part Num"
F 6 "934-MDSM-4R-7-13" H 2950 3700 60  0001 C CNN "1st Distrib PN"
F 7 "http://www.mouser.de/ProductDetail/Hamlin-Littelfuse/MDSM-4R-7-13/?qs=sGAEpiMZZMvFdY0L2HfHxpd8me%252bqUBsA6gYFFzMsi64%3d" H 2950 3700 60  0001 C CNN "1st Distrib Link"
F 8 "F7839CT-ND" H 2950 3700 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.digikey.com/product-detail/en/littelfuse-inc/MDSM-4R-7-13/F7839CT-ND/6217915" H 2950 3700 60  0001 C CNN "2nd Distrib Link"
	1    2950 3700
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 586CD7F1
P 3700 2450
F 0 "R1" H 3770 2543 50  0000 L CNN
F 1 "10K" H 3770 2450 50  0000 L CNN
F 2 "w_smd_resistors:r_0603" H 3770 2357 50  0000 L CNN
F 3 "" H 3700 2450 50  0000 C CNN
F 4 "arbitrary" H 3700 2450 60  0001 C CNN "MFG Name"
F 5 "10k 0603 resistor 10%" H 3700 2450 60  0001 C CNN "MFG Part Num"
F 6 "10k 0603 resistor 1% 1000pcs" H 3700 2450 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=10k+0603+resistor+1%25+1000pcs" H 3700 2450 60  0001 C CNN "1st Distrib Link"
F 8 "603-RC0603JR-0710KL" H 3700 2450 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Yageo/RC0603JR-0710KL/?qs=sGAEpiMZZMvdGkrng054txdMlaQGj9fi5a0YMwYlB2s%3d" H 3700 2450 60  0001 C CNN "2nd Distrib Link"
	1    3700 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 2450 2350 2050
Wire Wire Line
	1850 2050 4150 2050
Wire Wire Line
	3350 1950 4150 1950
Wire Wire Line
	2650 3700 2350 3700
Wire Wire Line
	2350 2850 2350 4750
Wire Wire Line
	3700 2300 3700 2150
Wire Wire Line
	3700 2150 4150 2150
Wire Wire Line
	3700 2600 3700 2750
Wire Wire Line
	3700 2750 3600 2750
Wire Wire Line
	3600 2750 3600 2050
Connection ~ 3600 2050
Wire Wire Line
	4000 1950 4000 1250
Wire Wire Line
	4000 1250 6250 1250
Wire Wire Line
	6250 1250 6250 2300
Wire Wire Line
	6250 1950 5900 1950
Connection ~ 4000 1950
Wire Wire Line
	6250 2300 6950 2300
Connection ~ 6250 1950
Wire Wire Line
	3900 2150 3900 1150
Wire Wire Line
	3900 1150 6450 1150
Wire Wire Line
	6450 1150 6450 2400
Wire Wire Line
	6450 2400 6950 2400
Connection ~ 3900 2150
Wire Wire Line
	5900 2250 6150 2250
Wire Wire Line
	6150 2250 6150 2500
Wire Wire Line
	6150 2500 6950 2500
Wire Wire Line
	5900 2350 6050 2350
Wire Wire Line
	6050 2350 6050 2600
Wire Wire Line
	6050 2600 6950 2600
Wire Wire Line
	5900 3250 6200 3250
Wire Wire Line
	6200 3250 6200 2700
Wire Wire Line
	6200 2700 6950 2700
Wire Wire Line
	4150 2850 2500 2850
Wire Wire Line
	3350 1450 3350 4150
Wire Wire Line
	3350 4750 3350 4550
Wire Wire Line
	1850 4750 3350 4750
Connection ~ 2350 3700
$Comp
L R R2
U 1 1 586CDBB0
P 4050 4450
F 0 "R2" H 4120 4543 50  0000 L CNN
F 1 "330R" H 4120 4450 50  0000 L CNN
F 2 "w_smd_resistors:r_0603" H 4120 4357 50  0000 L CNN
F 3 "" H 4050 4450 50  0000 C CNN
F 4 "arbitrary" H 4050 4450 60  0001 C CNN "MFG Name"
F 5 "330R 0603 resistor 10%" H 4050 4450 60  0001 C CNN "MFG Part Num"
F 6 "330R 0603 resistor 10% 1000pcs" H 4050 4450 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=330R+0603+resistor+10%25+1000pcs" H 4050 4450 60  0001 C CNN "1st Distrib Link"
F 8 "603-RC0603JR-07330RL" H 4050 4450 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Yageo/RC0603JR-07330RL/?qs=sGAEpiMZZMvdGkrng054t1u4NFGr8RGrZwnaMsEkwfc%3d" H 4050 4450 60  0001 C CNN "2nd Distrib Link"
	1    4050 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2850 2500 4350
Wire Wire Line
	3250 3700 3850 3700
Connection ~ 3350 3700
Wire Wire Line
	2500 4350 3050 4350
$Comp
L LED D1
U 1 1 586CE503
P 4050 4050
F 0 "D1" V 4135 3929 50  0000 R CNN
F 1 "LED" V 4042 3929 50  0000 R CNN
F 2 "LEDs:LED_0603" V 3949 3929 50  0000 R CNN
F 3 "" H 4050 4050 50  0000 C CNN
F 4 "arbitrary" H 4050 4050 60  0001 C CNN "MFG Name"
F 5 "led green 0603" H 4050 4050 60  0001 C CNN "MFG Part Num"
F 6 "led green 0603 100pcs" H 4050 4050 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=led+green+0603+100pcs" H 4050 4050 60  0001 C CNN "1st Distrib Link"
F 8 "859-LTST-C190GKT" H 4050 4050 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Lite-On/LTST-C190GKT/?qs=sGAEpiMZZMseGfSY3csMkXFyfJS0jLI%2f1FiRwkUeSzk%3d" H 4050 4050 60  0001 C CNN "2nd Distrib Link"
	1    4050 4050
	0    -1   -1   0   
$EndComp
$Comp
L LED D2
U 1 1 586CE574
P 4400 4050
F 0 "D2" V 4485 3929 50  0000 R CNN
F 1 "LED" V 4392 3929 50  0000 R CNN
F 2 "LEDs:LED_0603" V 4299 3929 50  0000 R CNN
F 3 "" H 4400 4050 50  0000 C CNN
F 4 "arbitrary" H 4400 4050 60  0001 C CNN "MFG Name"
F 5 "led red 0603" H 4400 4050 60  0001 C CNN "MFG Part Num"
F 6 "led red 0603 100pcs" H 4400 4050 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=led+red+0603+100pcs" H 4400 4050 60  0001 C CNN "1st Distrib Link"
F 8 "859-LTST-C190KRKT" H 4400 4050 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Lite-On/LTST-C190KRKT/?qs=sGAEpiMZZMseGfSY3csMkc1s5Vl7SWse2rJHZg0oi4o%3d" H 4400 4050 60  0001 C CNN "2nd Distrib Link"
	1    4400 4050
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3850 3700 3850 4850
Wire Wire Line
	3850 4850 4400 4850
Wire Wire Line
	4050 4850 4050 4600
Wire Wire Line
	4400 4850 4400 4600
Connection ~ 4050 4850
Wire Wire Line
	4050 4300 4050 4200
Wire Wire Line
	4400 4300 4400 4200
Wire Wire Line
	4050 3900 4050 3600
Wire Wire Line
	4050 3600 3800 3600
Wire Wire Line
	3800 3600 3800 2950
Wire Wire Line
	3800 2950 4150 2950
Wire Wire Line
	4400 3900 4400 3750
Wire Wire Line
	4400 3750 4200 3750
Wire Wire Line
	4200 3750 4200 3500
Wire Wire Line
	4200 3500 3900 3500
Wire Wire Line
	3900 3500 3900 3050
Wire Wire Line
	3900 3050 4150 3050
$Comp
L C C2
U 1 1 586E07DC
P 3050 1700
F 0 "C2" H 3165 1793 50  0000 L CNN
F 1 "10uF" H 3165 1700 50  0000 L CNN
F 2 "w_smd_cap:c_1206" H 3165 1607 50  0000 L CNN
F 3 "" H 3050 1700 50  0000 C CNN
F 4 "arbitrary" H 3050 1700 60  0001 C CNN "MFG Name"
F 5 "10uF 1206 capacitor X7R" H 3050 1700 60  0001 C CNN "MFG Part Num"
F 6 "10uF 1206 capacitor X7R 100pcs" H 3050 1700 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=10uF+1206+capacitor+X7R+100pcs" H 3050 1700 60  0001 C CNN "1st Distrib Link"
F 8 "581-1206ZC106KAT2A" H 3050 1700 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/AVX/1206ZC106KAT2A/?qs=sGAEpiMZZMs0AnBnWHyRQPSZir2U04Awt0wjnhgBKAA%3d" H 3050 1700 60  0001 C CNN "2nd Distrib Link"
	1    3050 1700
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 586E0907
P 2800 1700
F 0 "C1" H 2915 1840 50  0000 L CNN
F 1 "100nF" H 2915 1747 50  0000 L CNN
F 2 "w_smd_cap:c_0603" H 2915 1654 50  0000 L CNN
F 3 "" H 2915 1561 50  0000 L CNN
F 4 "arbitrary" H 2800 1700 60  0001 C CNN "MFG Name"
F 5 "100nF 0603 capacitor X7R" H 2800 1700 60  0001 C CNN "MFG Part Num"
F 6 "100nF 0603 capacitor X7R 1000pcs" H 2800 1700 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=100nF+0603+capacitor+X7R+1000pcs" H 2800 1700 60  0001 C CNN "1st Distrib Link"
F 8 "710-885012206020" H 2800 1700 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Wurth-Electronics/885012206020/?qs=sGAEpiMZZMs0AnBnWHyRQEGbLOF2VP1ivD1RfS10zjEe%2fCqv5VAAJA%3d%3d" H 2800 1700 60  0001 C CNN "2nd Distrib Link"
	1    2800 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 1850 2800 2050
Connection ~ 2800 2050
Wire Wire Line
	3050 1850 3050 2050
Connection ~ 3050 2050
Wire Wire Line
	3350 1450 3050 1450
Wire Wire Line
	3050 1450 3050 1550
Connection ~ 3350 1950
Wire Wire Line
	2800 1350 2800 1550
Wire Wire Line
	2800 1350 3450 1350
Wire Wire Line
	3450 1350 3450 1950
Connection ~ 3450 1950
$Comp
L Battery BT2
U 1 1 586E0CB8
P 1850 2650
F 0 "BT2" H 1961 2743 50  0000 L CNN
F 1 "Battery" H 1961 2650 50  0000 L CNN
F 2 "module:18650_Battery_Holder_SMT" H 1961 2557 50  0000 L CNN
F 3 "http://www.mouser.com/ds/2/215/042P-744096.pdf" V 1850 2710 50  0001 C CNN
F 4 "Keystone Electronics" H 1850 2650 60  0001 C CNN "MFG Name"
F 5 "1042P" H 1850 2650 60  0001 C CNN "MFG Part Num"
F 6 "18650 Battery Holder Battery Storage Box With Bronze PinsTBH-18650-1C-SMT" H 1850 2650 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/item/5Pcs-lot-18650-Battery-Holder-SMD-18650-Battery-Storage-Box-High-Quality-With-Bronze-Pins-TBH/32542773400.html" H 1850 2650 60  0001 C CNN "1st Distrib Link"
F 8 "534-1042P" H 1850 2650 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Keystone-Electronics/1042P/?qs=sGAEpiMZZMt13fpse6CWDTND2ZLh8pVfdYM5p6OtKurYl9svQbsGHQ%3d%3d" H 1850 2650 60  0001 C CNN "2nd Distrib Link"
	1    1850 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 2050 1850 2450
Connection ~ 2350 2050
Wire Wire Line
	1850 2850 1850 4750
Connection ~ 2350 4750
$Comp
L R R3
U 1 1 586E2561
P 4400 4450
F 0 "R3" H 4470 4543 50  0000 L CNN
F 1 "330R" H 4470 4450 50  0000 L CNN
F 2 "w_smd_resistors:r_0603" H 4470 4357 50  0000 L CNN
F 3 "" H 4400 4450 50  0000 C CNN
F 4 "arbitrary" H 4400 4450 60  0001 C CNN "MFG Name"
F 5 "330R 0603 resistor 10%" H 4400 4450 60  0001 C CNN "MFG Part Num"
F 6 "330R 0603 resistor 10% 1000pcs" H 4400 4450 60  0001 C CNN "1st Distrib PN"
F 7 "https://www.aliexpress.com/wholesale?SearchText=330R+0603+resistor+10%25+1000pcs" H 4400 4450 60  0001 C CNN "1st Distrib Link"
F 8 "603-RC0603JR-07330RL" H 4400 4450 60  0001 C CNN "2nd Distrib PN"
F 9 "http://www.mouser.de/ProductDetail/Yageo/RC0603JR-07330RL/?qs=sGAEpiMZZMvdGkrng054t1u4NFGr8RGrZwnaMsEkwfc%3d" H 4400 4450 60  0001 C CNN "2nd Distrib Link"
	1    4400 4450
	1    0    0    -1  
$EndComp
Text Label 3650 1950 0    60   ~ 0
GND
$EndSCHEMATC
