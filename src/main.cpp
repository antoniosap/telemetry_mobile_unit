/*! @mainpage
//-------------------------------------------------------------------------------------------------------
*/
#include <Arduino.h>

//-- DEBUG ----------------------------------------------------------------------

//-- CONFIGURATIONS -------------------------------------------------------------
#define UART_ECHO               (0)
#define UART_BAUDRATE           (19400)

//-- 433 MHz RADIO --------------------------------------------------------------
#include <RadioLib.h>

// https://www.electrodragon.com/w/Si4432
// Si4432 has the following connections:
#define RADIO_nSEL      10
#define RADIO_nIRQ      2
#define RADIO_SDN       5

Si4432 radio = new Module(RADIO_nSEL, RADIO_nIRQ, RADIO_SDN);

//-- MSG PACK -------------------------------------------------------------------
// RX PROTOCOL
uint8_t pan;
uint8_t tilt;
uint8_t outd;   // bit 3 = PIN 3, pin 4 = PIN 4
#define PACKET_RX_SIZE (sizeof(uint8_t) * 3)
// TX PROTOCOL
float analogA0;
float analogA1;
float analogA2;
float analogA3;
// #define MSGPACK_DEBUGLOG_ENABLE
#include <MsgPack.h>

MsgPack::Packer packer;
MsgPack::Unpacker unpacker;

//-------------------------------------------------------------------------------
#include <TaskScheduler.h>

void sensorReading();
Task sensorTask(5000, TASK_FOREVER, &sensorReading);
Scheduler runner;

float voltageReading(float value) {
  return ((5*value)/1023)*4.103354632587859;
}

void sensorReading() {
  analogA0 = voltageReading(A0);
  analogA1 = voltageReading(A1);
  analogA2 = voltageReading(A2);
  analogA3 = voltageReading(A3);
  packer.serialize(analogA0, analogA1, analogA2, analogA3);
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

//-------------------------------------------------------------------------------
void setup() {
  Serial.begin(UART_BAUDRATE);
  // needed to keep leonardo/micro from starting too fast!
  while (!Serial) { delay(10); }

  Serial.print(F("I:Si4432:START"));
  int state = radio.begin();
  if (state == ERR_NONE) {
    Serial.println(F("I:Si4432:success!"));
  } else {
    Serial.print(F("I:Si4432:failed, code"));
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
    unpacker.deserialize(pan, tilt, outd);

    // print the data of the packet
    Serial.print(F("I:Si4432:RX:Data:\t\t"));
    Serial.println((char*)payload);

  } else if (state == ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    Serial.println(F("I:Si4432:RX:timeout!"));

  } else if (state == ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("I:Si4432:RX:CRC error!"));

  } else {
    // some other error occurred
    Serial.print(F("I:Si4432:RX:failed, code "));
    Serial.println(state);

  }
  runner.execute();
}