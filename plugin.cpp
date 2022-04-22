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
#include <plugin.h>

Plugin::Plugin(const unsigned int _packet_size) :  packet_size(_packet_size) {
  packet = 0;
}

Plugin::~Plugin() {

}

void Plugin::detectPacket(unsigned int duration, unsigned char pinState, Plugin *self ) {

  if ((duration > END_PACKET) && (1 == pinState)) {
    //check if we are in the range
    if (bitsRead > packet_size_min
        && bitsRead < packet_size_max) {
#ifdef DEBUG
      Serial.println(bitsRead);
#endif
      //Serial.printf("\nB%d", digitalRead(5));
      self->processPacket();
    } else {
      //Serial.printf("\nB%d", bitsRead);
    }
    bitsRead = 0;
  } else {

    if ((duration > MIN_PACKET) && (1 == pinState)) {
      timings[bitsRead] = duration;
      bitsRead++;//Serial.printf("\n%d#", digitalRead(5));
    } else {
      /*Serial.printf("\n%d@", duration);*/
    }

    if (bitsRead == MAX_CHANGES) {
      bitsRead = 0;
    }
  }
}

String Plugin::getString(uint64_t packet) {
  String s = "0x";
  return s + String((unsigned long long) packet, HEX);
}

uint64_t Plugin::getPacket() {
  uint64_t p = packet;
  packet = 0;
  return p;
}

bool Plugin::available() {
  if (Plugin::packet != 0) {
    return true;
  }
  return false;
}
