/*
CCM Interaction Test Sketch for Arduino Mega 2560 by Nomake Wan
Copyright (c) 2021-2025 Nomake Wan <nomake_wan@yahoo.co.jp>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include <wiring_private.h>

#define ECM_DYNAMIC_CTS 0
#define CCM_YEAR 1995

#if CCM_YEAR < 1992
byte window[3];
#if ECM_DYNAMIC_CTS > 0
byte output[15] = {0x41, 0x61, 0x00, 0xEC, 0x00, 0x00, 0x39, 0x01, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x39, 0x4B};
#else
byte output[15] = {0x41, 0x61, 0x00, 0xEC, 0x00, 0xCD, 0x39, 0x01, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x39, 0x7E};
#endif
#elif CCM_YEAR < 1994
byte window[5];
#if ECM_DYNAMIC_CTS > 0
byte output[18] = {0x41, 0x64, 0x01, 0xF3, 0x00, 0x00, 0x60, 0x01, 0x00, 0x6F, 0x0F, 0xD6, 0x83, 0x00, 0x51, 0xFF, 0xFF, 0xE0};
#else
byte output[18] = {0x41, 0x64, 0x01, 0xF3, 0x00, 0xCD, 0x60, 0x01, 0x00, 0x6F, 0x0F, 0xD6, 0x83, 0x00, 0x51, 0xFF, 0xFF, 0x13};
#endif
#else
byte window[5];
#if ECM_DYNAMIC_CTS > 0
byte output[21] = {0x41, 0x67, 0x02, 0xF5, 0x00, 0x00, 0x87, 0x01, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x48, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x0D};
#else
byte output[21] = {0x41, 0x67, 0x02, 0xF5, 0x00, 0xCD, 0x87, 0x01, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x48, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x40};
#endif
#endif

void setup() {
  analogReference(DEFAULT); // Switch to default reference explicitly
  pinMode(A0, INPUT); // Make sure A0 is an input explicitly
  bitSet (DIDR0, ADC0D); // Disable digital buffer on A0
  bitSet (DIDR0, ADC1D); // Disable digital buffer on A1
  bitSet (DIDR0, ADC2D); // Disable digital buffer on A2
  bitSet (DIDR0, ADC3D); // Disable digital buffer on A3
  bitSet (DIDR0, ADC4D); // Disable digital buffer on A4
  bitSet (DIDR0, ADC5D); // Disable digital buffer on A5
  bitSet (DIDR0, ADC6D); // Disable digital buffer on A6
  bitSet (DIDR0, ADC7D); // Disable digital buffer on A7
  bitSet (DIDR1, AIN0D); // Disable digital buffer on AIN0
  bitSet (DIDR1, AIN1D); // Disable digital buffer on AIN1
  bitSet (DIDR2, ADC8D); // Disable digital buffer on A8
  bitSet (DIDR2, ADC9D); // Disable digital buffer on A9
  bitSet (DIDR2, ADC10D); // Disable digital buffer on A10
  bitSet (DIDR2, ADC11D); // Disable digital buffer on A11
  bitSet (DIDR2, ADC12D); // Disable digital buffer on A12
  bitSet (DIDR2, ADC13D); // Disable digital buffer on A13
  bitSet (DIDR2, ADC14D); // Disable digital buffer on A14
  bitSet (DIDR2, ADC15D); // Disable digital buffer on A15

  analogRead(0); // Burn an analog reading on A0
  Serial1.begin(8192); // Open UART1 at 8192 baud
  UBRR1H = (uint8_t)(121>>8); // Switch to 8192 baud at 1x
  UBRR1L = (uint8_t)121; // Switch to 8192 baud at 1x
  cbi(UCSR1A, U2X0); // disable 2x mode
  cbi(UCSR1A, MPCM0); // disable multi-processor mode
  cbi(UCSR1B, TXEN1); // disable transmitter for now
  cbi(UCSR1B, TXCIE1); // disable transmit interrupts for now
  pinMode(18, INPUT); // tri-state TX1
}

void loop() {
  if(Serial1.available()) {
#if CCM_YEAR > 1991
    // Slide the 5-byte window
    for(uint8_t i = 0; i < 4; i++) {
      window[i] = window[i + 1];
    }

    // Add new bytes as they come in
    window[4] = Serial1.read();

    // Check the first two bytes for a match
    if(window[0] == 0x40 && window[1] == 0x57) {
      // Calculate the checksum byte
      byte cs = 0;

      for(uint8_t i = 0; i < 4; i++) {
        cs += window[i];
      }

      cs = 0xFF - cs;
      cs += 0x01;

      // If checksum byte matches, send diagnostic data
      if(cs == window[4]) {
        cbi(UCSR1B, RXEN1); // disable receiver
        cbi(UCSR1B, RXCIE1); // disable receive interrupts
        window[0] = 0x00; // poison the sliding window
        delay(2); // delay to allow ALDL line to settle
        pinMode(18, OUTPUT); // reenable TX1 as output
        sbi(UCSR1B, TXEN1); // enable transmitter
        sbi(UCSR1B, TXCIE1); // enable transmit interrupts
        Serial1.write(output, sizeof(output)); // write the PCM diagnostic message
        Serial1.flush(); // wait until transmit completes
        cbi(UCSR1B, TXEN1); // disable transmitter
        cbi(UCSR1B, TXCIE1); // disable transmit interrupts
        pinMode(18, INPUT); // tri-state TX1
        sbi(UCSR1B, RXEN1); // reenable receiver
        sbi(UCSR1B, RXCIE1); // reenable receive interrupts
      }
    }
  }
#else
    // Slide the 3-byte window
    for(uint8_t i = 0; i < 2; i++) {
      window[i] = window[i + 1];
    }

    // Add new bytes as they come in
    window[2] = Serial1.read();

    // Check the window for a match
    if(window[0] == 0x40 && window[1] == 0x55 && window[2] == 0x6B) {
      // If checksum byte matches, send diagnostic data
      cbi(UCSR1B, RXEN1); // disable receiver
      cbi(UCSR1B, RXCIE1); // disable receive interrupts
      window[0] = 0x00; // poison the sliding window
      delay(2); // delay to allow ALDL line to settle
      pinMode(18, OUTPUT); // reenable TX1 as output
      sbi(UCSR1B, TXEN1); // enable transmitter
      sbi(UCSR1B, TXCIE1); // enable transmit interrupts
      Serial1.write(output, sizeof(output)); // write the PCM diagnostic message
      Serial1.flush(); // wait until transmit completes
      cbi(UCSR1B, TXEN1); // disable transmitter
      cbi(UCSR1B, TXCIE1); // disable transmit interrupts
      pinMode(18, INPUT); // tri-state TX1
      sbi(UCSR1B, RXEN1); // reenable receiver
      sbi(UCSR1B, RXCIE1); // reenable receive interrupts
    }
  }
#endif

#if ECM_DYNAMIC_CTS > 0
  // Read A0 to check status of potentiometer, save to cts byte
  output[5] = analogRead(0)>>2;

  // Calculate new checksum and save to datastream checksum byte
  byte checksum = 0;

#if CCM_YEAR < 1992
  for (uint8_t i = 0; i < 13; i++) {
    checksum += output[i];
  }

  checksum = 0xFF - checksum;
  checksum += 0x01;
  output[14] = checksum;
#elif CCM_YEAR < 1994
  for (uint8_t i = 0; i < 16; i++) {
    checksum += output[i];
  }

  checksum = 0xFF - checksum;
  checksum += 0x01;
  output[17] = checksum;
#else
  for (uint8_t i = 0; i < 20; i++) {
    checksum += output[i];
  }

  checksum = 0xFF - checksum;
  checksum += 0x01;
  output[20] = checksum;
#endif
#endif
}
