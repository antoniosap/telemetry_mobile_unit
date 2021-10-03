# telemetry_mobile_unit

## schematic

https://www.electrodragon.com/w/Si4432
Si4432: 433mHZ radio
PIN 01  GND
PIN 02  
PIN 03
PIN 04
PIN 05  3V3
PIN 06  SDO     <-- D12 MISO
PIN 07  SDI     <-- D11 MOSI
PIN 08  SCLK    <-- D13 SCK
PIN 09  NSEL    <-- D10 CS
PIN 10  NIRQ    <-- D2
PIN 11  SDN     <-- D5
PIN 12
...
PIN 13  ANTENNA
PIN 14

Arduino pro8MHzatmega328 3V3:
PIN 31  TXD     per il USB converter    / serial sensor
PIN 30  RXD     per il USB converter    / serial sensor

RAW 5V0 <-- input from BEC
GND GND common
VCC 3V3 --> output to radio
D13 SCK --> SLCK radio
D12 MISO <- SDI radio
D11 MOSI -> SDO radio
D10 CS  --> NSEL radio
D2      --> NIRQ
D5      --> SDN

USCITE SERVO:
D9  SERVO TILT
D8  SERVO PAN

USCITE DIGITAL:
D3
D4

INGRESSI ANALOGICI 0->3V3:
A0  <-- current ramo #1 ACS712 30A con divisore 5V -> 3V3
A1  <-- current ramo #2 ACS712 30A con divisore 5V -> 3V3
A2  <-- temperatura VTX LM35
A3

- inserire 2 partitori 5V -> 3V3 per i sensori corrente ACS712 30A da 66mV / A con 2.5 V a 0A in +/-
  https://damien.douxchamps.net/elec/resdiv/
  -30A = 0.52V, +30A = 4.48V  partitore / 0.66  510 ohm / 1k ohm
