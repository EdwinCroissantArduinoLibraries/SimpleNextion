/*
 * Nextion display library for the Arduino microcontroller.
 * ----------PROOF OF CONCEPT STAGE--------------
 *
 * Copyright (C) 2016 Edwin Croissant
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the README.md file for additional information.
 */

#ifndef SIMPLENEXTION_H
#define SIMPLENEXTION_H

#include "Arduino.h"


class SimpleNextion {
public:
	SimpleNextion(HardwareSerial &output, uint32_t defaultBaud=9600);
	bool begin(uint32_t baud);
	void setInt(uint8_t id, int32_t value);
	void setTxt(uint8_t id, char* txt);
	void update();
	void listen();
	void setRxTimeout(uint16_t rxTimeout);
	/*
	 * Assign a function like callbackInt(uint8_t id, int32_t value) to this variable
	 */
	void (*callbackInt)(uint8_t id, int32_t value);
	/*
	 * Assign a function like callbackTxt(uint8_t id, char *txt) to this variable
	 */
	void (*callbackTxt)(uint8_t id, char *txt);
private:
	HardwareSerial *_serial;
	uint32_t _defaultBaud;
	uint32_t _baud;
	uint16_t _rxTimeout;
	char _txtBuffer[32];
	enum rxState {
		rxIdle, rxInt, rxTxt
	} _rxState;

	void _varName(uint8_t id);
	void _tripple0xFF();
	bool _ping();
};

#endif // SIMPLENEXTION_H
