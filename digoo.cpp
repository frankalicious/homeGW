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

#if defined(ESP8266) || defined(ESP32)
    #define ISR_PREFIX ICACHE_RAM_ATTR
#else
    #define ISR_PREFIX
#endif

digoo::digoo() {
	packet_size = 37;
    END_PACKET = 3000;
	MIN_PACKET = 650;
}


uint8_t digoo::getId(uint64_t packet) {
  uint8_t id = (packet >> 28) & 0xFF;
  return id;
}

uint8_t digoo::getBattery(uint64_t packet) {
  uint8_t batt = (packet >> 24) & 0x8;
  return batt ? 1 : 0;
}

uint8_t digoo::getChannel(uint64_t packet) {
  uint8_t channel = (packet >> 24) & 0x3;
  return channel+1;
}

float digoo::getTemperature(uint64_t packet) {
	int16_t t = packet >> 12 & 0x0FFF;
	t = 0x0800 & t ? 0xF000 | t  : t;
	float temperature = float(t) / 10;
  return temperature;
}

uint8_t digoo::getHumidity(uint64_t packet) {
  uint8_t humidity = packet & 0xFF;
  return humidity;
}

uint8_t digoo::isValidWeather(uint64_t ppacket) {
  uint8_t humidity = getHumidity(ppacket);
  //Specs http://www.amazon.co.uk/gp/product/B00327G0MA/ref=oh_details_o00_s00_i00
  if (humidity > 100) { //sanity check according to specs
    return INVALID_HUMIDITY;
  }
  float temperature = getTemperature(ppacket);
  if (temperature < -20.0 || temperature > 50.0) { //sanity check according to specs
    return INVALID_TEMPERATURE;
  }
  return OK;
}

ISR_PREFIX void digoo::processPacket() {
	uint64_t pkt[2]={0};
	uint8_t skipped=0;

	for(unsigned i=0; i< bitsRead; i++) {
		unsigned duration = timings[i];
		if(duration > digoo::ONE) {
			pkt[1] = (pkt[1] << 1) | (pkt[0] >> 31);
			pkt[0] = (pkt[0] << 1);
			bitSet(pkt[0], 0);
			//Serial.print("1");
		} else if(duration > digoo::ZERO) {
			pkt[1] = (pkt[1] << 1) | (pkt[0] >> 31);
			pkt[0] = (pkt[0] << 1);
			bitClear(pkt[0], 0);
			//Serial.print("0");
		} else skipped++;
		//Serial.print("0x");
		//Serial.print((unsigned long) pkt[1], HEX);
		//Serial.println((unsigned long) pkt[0], HEX);
	}
	packet = (pkt[1]<<32) | pkt[0];
	//Serial.printf("\nR%dS%d\n",bitsRead, skipped);
	#ifdef DEBUG
	if (skipped == 0) {
		Serial.print("~0x");
		Serial.print((unsigned) pkt[1], HEX);
		Serial.println((unsigned) pkt[0], HEX);
		for(unsigned i=0; i < bitsRead; i++)
			Serial.println(timings[i]);
	}
	#endif
}
