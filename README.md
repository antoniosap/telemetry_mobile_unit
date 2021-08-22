# telemetry_mobile_unit

# schematic

Si4432: 433mHZ radio
PIN 01  GND
PIN 02  
PIN 03
PIN 04
PIN 05  3V3
PIN 06  SDO     <-- D11 MOSI
PIN 07  SDI     <-- D12 MISO
PIN 08  SCLK    <-- D13 SCK
PIN 09  NSEL    <-- D10 CS
PIN 10  NIRQ    <-- D2
PIN 11
PIN 12
PIN 13  ANTENNA
PIN 14

Arduino pro8MHzatmega328 3V3:
PIN 31  TXD     per il USB converter    / serial sensor
PIN 30  RXD     per il USB converter    / serial sensor


RAW 5V0 <-- input from BEC
GND GND common
VCC 3V3 --> output to radio
D13 SCK --> SLCK radio
D12 MISO -> SDI radio
D11 MOSI <- SDO radio
D10 CS  --> NSEL radio
D2      --> NIRQ

USCITE SERVO:
D9  SERVO TILT
D8  SERVO PAN
...
USCITE DIGITAL:
...
INGRESSI ANALOGICI 0->3V3:
...
