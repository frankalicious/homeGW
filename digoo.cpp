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
#include <digoo.h>

String digoo::error_str = "";

digoo::digoo() {
	packet_size = 36;
    END_PACKET = 3000; 
	MIN_PACKET = 650;
}

uint8_t digoo::getChannel(uint64_t packet) {
  uint8_t channel = (packet >> 24) && 0x3;
  return channel;
}

float digoo::getTemperature(uint64_t packet) {
  float temperature = float((packet >> 12) & 0xFFF) / 10;
  return temperature;
}

uint8_t digoo::getHumidity(uint64_t packet) {
  uint8_t humidity = packet & 0x0000F;
  return humidity;
}

uint8_t digoo::isValidWeather(uint64_t ppacket) {
  uint8_t humidity = getHumidity(ppacket);
  //Specs http://www.amazon.co.uk/gp/product/B00327G0MA/ref=oh_details_o00_s00_i00
  if (humidity > 100) { //sanity check according to specs
    error_str = String(humidity);
    return INVALID_HUMIDITY;
  }
  float temperature = getTemperature(ppacket);
  if (temperature < -20.0 || temperature > 50.0) { //sanity check according to specs
    error_str = String((int) temperature) + "." + String(((int) (temperature * 10)) % 10);
    return INVALID_TEMPERATURE;
  }
  return OK;
}

void digoo::processPacket() {
	packet = 0;
	for(unsigned i=1; i< bitsRead; i++) {
		unsigned duration = timings[i];
		if(duration > digoo::ONE) {
	      packet = packet << 1;
          bitSet(packet, 0);
        } else if(duration > digoo::ZERO) {
          packet = packet << 1;
          bitClear(packet, 0);
        }

	}
	#ifdef DEBUG
    Serial.print("~0x");
    Serial.println((long) packet, HEX);
	#endif
}

String digoo::getError() {
  switch(error) {
    case INVALID_SYNC:
      return "Invalid checksum: " + error_str;
    case INVALID_TEMPERATURE:
      return "Invalid temperature: " + error_str;
    case INVALID_HUMIDITY:
      return "Invalid humidity: " + error_str;
    default:
      return "unknown error";
  }
}