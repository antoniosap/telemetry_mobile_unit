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
#define RADIO_SDN       1

Si4432 radio = new Module(RADIO_nSEL, RADIO_nIRQ, RADIO_SDN);


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
}

void loop() {
  byte payload[8 + 1];
  int state = radio.receive(payload, 8);

  if (state == ERR_NONE) {
    // packet was successfully received
    Serial.println(F("I:Si4432:success!"));
    payload[8] = 0;

    // print the data of the packet
    Serial.print(F("I:Si4432:Data:\t\t"));
    Serial.println((char*)payload);

  } else if (state == ERR_RX_TIMEOUT) {
    // timeout occurred while waiting for a packet
    Serial.println(F("I:Si4432:timeout!"));

  } else if (state == ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("I:Si4432:CRC error!"));

  } else {
    // some other error occurred
    Serial.print(F("I:Si4432:failed, code "));
    Serial.println(state);

  }
}