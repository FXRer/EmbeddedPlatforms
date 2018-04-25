/* Copyright (c) 2015 Daniel Thiele, Axel Wachtler
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/*
 * Example sketch, chatting to Radiofaros by keypress + LED toggling
 *
 * Radiofaro LED1 used as keypress indicator, toggling each press
 * Button connected between d8 and d10, plugged into female header
 */

#include "HardwareRadio.h"

static const uint8_t pinLed = LED1_BUILTIN;
static const uint8_t pinButton = 8;
static const uint8_t pinButtonGnd = 10;

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButtonGnd, OUTPUT);
  digitalWrite(pinButtonGnd, 0);
  pinMode(pinButton, INPUT_PULLUP);
  Radio.begin(17, STATE_RX);
}

void loop() {
  static uint8_t ledtgl=0;
  static uint8_t buttonstate=0;
  static uint8_t lastbuttonstate=0;
  static uint16_t debounce_tmr=0;

  buttonstate = digitalRead(pinButton);
  if((millis() > debounce_tmr) && (buttonstate != lastbuttonstate)){
    switch(buttonstate){
      case LOW:
        ledtgl=1-ledtgl;
        Radio.write('A');
        Radio.flush();
        break;
      case HIGH:
        // do nothing
        break;
      default:
        break;
    }
    lastbuttonstate = buttonstate;
    debounce_tmr = millis()+30; // some button debouncing
  }

  if (Radio.available() > 0)
  {
      Radio.read();
      ledtgl=1-ledtgl;
      debounce_tmr = millis()+30; // some button debouncing
  }

  digitalWrite(pinLed, ledtgl);
}
