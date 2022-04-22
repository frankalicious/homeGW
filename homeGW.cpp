/*
  Written by Diogo Gomes, diogogomes@gmail.com

  Copyright (c) 2014 Diogo Gomes.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <homeGW.h>
#include <RingBufCPP.h>

#if defined(ESP8266) || defined(ESP32)
#define ISR_PREFIX IRAM_ATTR
#else
#define ISR_PREFIX
#endif

// MUST BE AN INTTERUPT COMPATIBLE PIN
#define EVENT_PIN 5
#define MAX_NUM_ELEMENTS 10

struct Event
{
  int index;
  unsigned char pinState;
  unsigned long timestamp;
};

// Declare as volatile, since modified in ISR
volatile unsigned int _index = 0;

// Stack allocate the buffer to hold event structs
RingBufCPP<struct Event, MAX_NUM_ELEMENTS> buf;


Plugin **HomeGW::plugin;
uint8_t HomeGW::MAX_PLUGINS;

HomeGW::HomeGW(uint8_t max_plugins) {
  MAX_PLUGINS = max_plugins;
  plugin = new Plugin*[MAX_PLUGINS];
  for (int i = 0; i < MAX_PLUGINS; i++)
    plugin[i] = NULL;
}

HomeGW::~HomeGW() {
  delete HomeGW::plugin;
}


void HomeGW::registerPlugin(Plugin *p) {
  for (int i = 0; i < MAX_PLUGINS; i++) {
    if (plugin[i] == NULL) {
      plugin[i] = p;
      return;
    }
  }

}

bool HomeGW::setup(uint8_t pin) {
  bool isESP = false;

#if defined(ESP8266)
  isESP = true;
#endif

  if (!isESP && (!(pin == 3 || pin == 2)))
  {
    return false;
  }

  HomeGW::pin = pin;

  pinMode(pin, INPUT);
  digitalWrite(pin, LOW);
  uint8_t interuptPin;

  if (!isESP)
  {
    interuptPin = pin - 2;
  }
  else
  {
    interuptPin = pin;
  }

  attachInterrupt(interuptPin, HomeGW::handleInterrupt, CHANGE); // 1 = PIN3

  return true;
}

void ISR_PREFIX HomeGW::handleInterrupt() {
  struct Event e;
  e.index = _index++;
  e.pinState = digitalRead(EVENT_PIN);
  e.timestamp = micros();
  // Add it to the buffer
  buf.add(e);
}

void HomeGW::handleDeferredInterrupt() {
  static unsigned long lastTime;
  struct Event e;

  // Keep looping until pull() returns NULL
  while (buf.pull(&e)) {
    unsigned int duration = e.timestamp - lastTime;

    for (int i = 0; i < MAX_PLUGINS; i++) {
      if (plugin != NULL) {
        if (plugin[i] != NULL)
          plugin[i]->detectPacket(duration, e.pinState, plugin[i]);
      }
    }
    lastTime = e.timestamp;
  }
}
