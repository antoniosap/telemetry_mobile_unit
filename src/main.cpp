/*! @mainpage
//-------------------------------------------------------------------------------
*/
#include <Arduino.h>

//-- DEBUG ----------------------------------------------------------------------
#define DEBUG_PIN               false
#define DEBUG_VALUE             true

#if DEBUG_PIN
#define TEST_PIN(gpio_nr)          { Serial.print("TEST_PIN BEGIN:"); \
                                     Serial.println(gpio_nr); \
                                     for (uint8_t i = 0; i < 10; i++) { \
                                        Serial.println("TEST_PIN RUNNING"); \
                                        pinMode(gpio_nr, OUTPUT); \
                                        digitalWrite(gpio_nr, HIGH); \
                                        delay(1000); \
                                        digitalWrite(gpio_nr, LOW); \
                                        delay(1000); \
                                     } \
                                     Serial.println("TEST_PIN END"); \
                                   }
#else
#define TEST_PIN(gpio_nr)          {}           
#endif

#if DEBUG_VALUE
#define PR(msg, value)          { Serial.print(msg); Serial.println(value); }
#else
#define PR(msg, value)          {}           
#endif

#define PR_VALUE(msg, value)    { Serial.print(F(msg)); Serial.println(value); }

/*
 * Uncomment to enable debug output.
 * Warning: Debug output will slow down the whole system significantly.
 *          Also, it will result in larger compiled binary.
 * Levels: debug - only main info
 *         verbose - full transcript of all SPI/UART communication
 */

// #define RADIOLIB_DEBUG
// #define RADIOLIB_VERBOSE


//-- CONFIGURATIONS -------------------------------------------------------------
#define UART_ECHO               (0)
#define UART_BAUDRATE           (19200)

//------------------------------------------------------------------------------
template <typename T> T serialPrintBinary(T x, bool usePrefix = true)
{
  if (usePrefix) Serial.print("0b");
  for (uint8_t i = 0; i < 8 * sizeof(x); i++) {
    Serial.print(bitRead(x, sizeof(x) * 8 - i - 1));
  }
  Serial.println();
  return x;
}

//-- 433 MHz RADIO --------------------------------------------------------------
#include <RadioLib.h>
#include <RadioDefs.h>

// https://www.electrodragon.com/w/Si4432
// Si4432 has the following connections:
#define RADIO_nSEL      10
#define RADIO_nIRQ      2
// BUG: https://github.com/jgromes/RadioLib/issues/305
#define RADIO_SDN       5

Module* module = new Module(RADIO_nSEL, RADIO_nIRQ, RADIO_SDN);
Si4432 radio = module;

float radioFreq = 434.0;
float radioBitRateKbSec = 48.0;
float radioFreqDev = 50.0;
float radioRxBw = 181.1;
int8_t radioPower = 10;
uint8_t radioPreambleLen = 40; 

//-- MSG PACK -------------------------------------------------------------------
// RX PROTOCOL
float rxAnalogPan;
float rxAnalogTilt;
uint8_t rxBTNBlkValue = HIGH;    // unpressed
uint8_t rxBTNRedValue = HIGH;    // unpressed
#define PACKET_RX_SIZE (sizeof(float) * 2 + sizeof(uint8_t) * 2)
// TX PROTOCOL
float analogA0;
float analogA1;
float analogA2;
float analogA3;
// #define MSGPACK_DEBUGLOG_ENABLE
#include <MsgPack.h>

MsgPack::Packer packer;
MsgPack::Unpacker unpacker;

//--- CONSOLE MENU ---------------------------------------------------------------
// https://github.com/neu-rah/ArduinoMenu/wiki/Menu-definition
#include <menu.h>
#include <menuIO/serialOut.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#define MAX_DEPTH   1

result menuSave();
result menuInfo();
result menuSetChannel();
result menuSetPower();
result menuLoopbackTest();
result menuRadioStatus();
result menuDumpRadioRegisters();


MENU(radioMenu,"radio",doNothing,noEvent,wrapStyle
  ,OP("set channel",menuSetChannel,enterEvent)
  ,OP("set power",menuSetPower,enterEvent)
  ,EXIT("<Back")
);

MENU(mainMenu,"system config",doNothing,noEvent,wrapStyle
  ,OP("save",menuSave,enterEvent)
  ,SUBMENU(radioMenu)
  ,OP("nav info",menuInfo,enterEvent)
  ,OP("loopback test",menuLoopbackTest,enterEvent)
  ,OP("radio status",menuRadioStatus,enterEvent)
  ,OP("dump radio registers",menuDumpRadioRegisters,enterEvent)
  ,EXIT("<Back")
);

serialIn serial(Serial);
MENU_INPUTS(in,&serial);

MENU_OUTPUTS(out,MAX_DEPTH
  ,SERIAL_OUT(Serial)
  ,NONE//must have 2 items at least
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);


//-------------------------------------------------------------------------------
#include <TaskScheduler.h>

void sensorReading();
Task sensorTask(5000, TASK_FOREVER, &sensorReading);
Scheduler runner;

float voltageReading(uint8_t pin) {
  return analogRead(pin) * (3.3 / 1024.0); // @3V3
}

void sensorReading() {
  analogA0 = voltageReading(A0);
  analogA1 = voltageReading(A1);
  analogA2 = voltageReading(A2);
  analogA3 = voltageReading(A3);
  packer.clear();
  packer.serialize(analogA0, analogA1, analogA2, analogA3);
  PR("\nI:TX:A0:", analogA0);
  PR("I:TX:A1:", analogA1);
  PR("I:TX:A2:", analogA2);
  PR("I:TX:A3:", analogA3);
  PR("I:TX:SIZE:", packer.size());
  int state = radio.transmit((uint8_t *)packer.data(), packer.size());

  if (state == ERR_NONE) {
    // the packet was successfully transmitted
    Serial.println(F("I:Si4432:TX:success!"));

  } else if (state == ERR_PACKET_TOO_LONG) {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("I:Si4432:TX:too long!"));

  } else if (state == ERR_TX_TIMEOUT) {
    // timeout occured while transmitting packet
    Serial.println(F("I:Si4432:TX:timeout!"));

  } else {
    // some other error occurred
    Serial.print(F("I:Si4432:TX:failed, code "));
    Serial.println(state);
  }
}

//-------------------------------------------------------------------------------------------------------

result menuSave() {
  Serial.println("menuSave");
  return proceed;
}

result menuInfo() {
  Serial.println("\nI:CONSOLE");
  Serial.println("I:Use keys [+ up] [- down] [* enter] [/ esc]");
  Serial.println("I:to control the menu navigation");
  return proceed;
}

result menuSetChannel() {
  Serial.println("menuSetChannel");
  return proceed;
}

result menuSetPower() {
  Serial.println("menuSetPower");
  return proceed;
}

result menuLoopbackTest() {
  Serial.println("menuLoopbackTest");
  return proceed;
}

result menuRadioStatus() {
  PR_VALUE("\nradio freq MHz: ", radioFreq);
  PR_VALUE("bit rate kb/s: ", radioBitRateKbSec);
  PR_VALUE("TX power: ", radioPower);
  PR_VALUE("RSSI:", module->SPIgetRegValue(SI443X_REG_RSSI));
  PR_VALUE("RSSI THR:", module->SPIgetRegValue(SI443X_REG_RSSI_CLEAR_CHANNEL_THRESHOLD));
  return proceed;
}

result menuDumpRadioRegisters() {
  Serial.println("\n+dump radio registers");
  for (uint8_t i = 0; i <= 0x7F; i++) {
    Serial.print("I:REG:");
    Serial.print(i, HEX);
    Serial.print(":");
    serialPrintBinary((uint8_t)module->SPIgetRegValue(i));
  }
  return proceed;
}

//-------------------------------------------------------------------------------
void setup() {
  Serial.begin(UART_BAUDRATE);
  // needed to keep leonardo/micro from starting too fast!
  while (!Serial) { delay(10); }

  TEST_PIN(RADIO_nSEL);
  TEST_PIN(RADIO_nIRQ);
  TEST_PIN(RADIO_SDN);
  TEST_PIN(18);
  Serial.println(F("I:Si4432:START"));
  PR("I:MOSI:", MOSI);
  PR("I:MISO:", MISO);
  PR("I:SCK:", SCK);
  PR("I:SS:", SS);
  int state = radio.begin(radioFreq, radioBitRateKbSec, radioFreqDev, radioRxBw, radioPower, radioPreambleLen);
  if (state == ERR_NONE) {
    Serial.println(F("I:Si4432:success!"));
  } else {
    Serial.print(F("I:Si4432:failed code: "));
    Serial.println(state);
    while (true);
  }

  runner.init();
  runner.addTask(sensorTask);
  sensorTask.enable();
}

void loop() {
  byte payload[PACKET_RX_SIZE];
  int state = radio.receive(payload, PACKET_RX_SIZE);

  if (state == ERR_NONE) {
    // packet was successfully received
    Serial.println(F("I:Si4432:RX:success!"));
    unpacker.feed(payload, PACKET_RX_SIZE);
    unpacker.deserialize(rxAnalogPan, rxAnalogTilt, rxBTNBlkValue, rxBTNRedValue);

    // print the data of the packet
    // Serial.print(F("I:Si4432:RX:Data:"));
    // nSerial.println((char*)payload);
    PR_VALUE("\nI:RX:PAN:", rxAnalogPan);
    PR_VALUE("I:RX:TILT:", rxAnalogTilt);
    PR_VALUE("I:RX:BLK:", rxBTNBlkValue);
    PR_VALUE("I:RX:RED:", rxBTNRedValue);

  } else if (state == ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    // Serial.println(F("I:Si4432:RX:timeout!"));
    Serial.print(F("."));

  } else if (state == ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("I:Si4432:RX:CRC error!"));

  } else {
    // some other error occurred
    Serial.print(F("I:Si4432:RX:failed code: "));
    Serial.println(state);

  }
  runner.execute();

  nav.doInput();
  if (nav.changed(0)) {
    nav.doOutput();
  }
}
