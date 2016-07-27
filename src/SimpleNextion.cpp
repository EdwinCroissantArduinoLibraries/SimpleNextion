/*
 * Nextion display library for the Arduino microcontroller.
 *  ----------PROOF OF CONCEPT STAGE--------------
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
#include "Arduino.h"
#include "SimpleNextion.h"

/**
 * Couples the Nextion to a serial and passes the default baud rate
 * of the Nextion, if the default baud rate is omitted 9600 baud is used
 */
SimpleNextion::SimpleNextion(HardwareSerial &output, uint32_t defaultBaud) {
	_serial = &output;
	_defaultBaud = defaultBaud;
	_baud = 0;
	_rxState = rxIdle;
	_rxTimeout = 200;
	callbackInt = NULL;
	callbackTxt = NULL;
}

/**
 * Start the communication with the Nextion and sets the baud rate
 * valid baud rates are: 2400, 4800, 9600, 19200, 38400,57600 and 115200
 */
bool SimpleNextion::begin(uint32_t baud) {
	_baud = baud;
	/*
	 * In case of an Arduino reset, the Nextion is already running
	 * at the wanted baud rate so we first try to get a response
	 * from the Nextion.
	 */
	_serial->begin(_baud);
	if (_ping()) return true; // We are ready to go.
	/*
	 * No response so we check at the default baud rate
	 */
	_serial->end();
	_serial->begin(_defaultBaud);
	/*
	 * Empty the buffer of the Nextion as the previous ping left unwanted
	 * characters in it's buffer. "code_c" should clear the buffer
	 * but doesn't do that in editor version 0.36.(neither 0x03 or "_c")
	 * So we sent the triple 0xFF signal and pray that the garbage
	 * in the buffer from the _ping() command send with
	 * the wrong baud rate didn't awake Cthulhu.
	 */
	_tripple0xFF();
	 if (!_ping()) return false; // No response either, must be dead.
	/*
	 * It's alive, set it at the required baud rate
	 */
	_serial->print(F("baud=")); // Forward the baud rate to the Nextion
	_serial->print(_baud);
	_tripple0xFF();
	_serial->end();
	_serial->begin(_baud);	// Set the serial to the wanted baud rate
	delay(100);	// Give the Nextion some time to adapt
	return _ping();	// Just to be sure
}

/*
 * Set a specific Nextion variable to an integer value
 */
void SimpleNextion::setInt(uint8_t id, int32_t value) {
	_varName(id);
	_serial->print(F(".val="));
	_serial->print(value);
	_tripple0xFF();
}

/*
 * Set a specific Nextion variable to a text value
 */
void SimpleNextion::setTxt(uint8_t id, char* txt) {
	_varName(id);
	_serial->print(F(".txt="));
	_serial->print('"');
	_serial->print(txt);
	_serial->print('"');
	_tripple0xFF();
}

/**
 * Kick's the local timer 0 that updates the Nextion
 */
void SimpleNextion::update() {
	_serial->print(F("tm0.en=1"));
	_tripple0xFF();
}

/*
 * To be placed in the main loop. Reads the serial buffer
 * and breaks down the messages and calls the appropriate callback functions
 */
void SimpleNextion::listen() {
	uint8_t c;
	static uint8_t id;
	static bool gotId;
	static uint8_t charCounter;
	static uint32_t timestamp;
	union {
		int32_t int32;
		struct {
			uint8_t bytes[4];
		};
	} value;

	switch (_rxState) {
	case rxIdle:
		/*
		 * read incoming bytes until a valid identifier is found
		 * change the _rxState accordingly and set the timestamp.
		 */
		if (_serial->available() > 0) {
			c = _serial->read();
			switch (c) {
			case 'I':
				_rxState = rxInt;
				timestamp = millis();
				break;
			case 'T':
				_rxState = rxTxt;
				timestamp = millis();
			}
		}
		return;
	case rxInt:
		/*
		 * react if message is at least 6 bytes (id, int32 and null)
		 * decode message and if 6th byte is null initiate the
		 * callback if assigned. After that goto cleanup
		 */
		if (_serial->available() >= 6) {
			id = _serial->read();
			for (uint8_t i = 0; i < 4; ++i) {
				value.bytes[i] = _serial->read();
			}

			if (_serial->read() == 0 && callbackInt)
				callbackInt(id, value.int32);
			goto cleanup;
		}
		break;
	case rxTxt:
		/*
		 * retrieve the id first, then read the characters into the
		 * buffer, a 0 marks the end of the string an initiate the call back
		 * if assigned. After that goto cleanup. If the end of the buffer
		 * is reached before a 0 is reached goto cleanup
		 */
		if (_serial->available()) {
			if (!gotId) {
				id = _serial->read();
				gotId = true;
			} else {
				_txtBuffer[charCounter] = _serial->read();
				if (_txtBuffer[charCounter] == 0 && callbackTxt) {
					callbackTxt(id, _txtBuffer);
					goto cleanup;
				} else {
					if (charCounter < sizeof _txtBuffer - 1)
						charCounter++;
					else
						goto cleanup; // prevent buffer overflow
				}
			}
		}
	} // end of switch statement

	/*
	 * if decoded within the timeout return
	 * otherwise cleanup
	 */
	if ((millis() - timestamp) < _rxTimeout)
		return;

	/*
	 * cleanup resets gotId and the charCounter and set
	 * the state to rxIdle
	 */
	cleanup:
	gotId = false;
	charCounter = 0;
	_rxState = rxIdle;
}

/*
 * The default timeout for decoding a message is 200 ms
 * Adjust at your discretion
 */
void SimpleNextion::setRxTimeout(uint16_t rxTimeout) {
	_rxTimeout = rxTimeout;
}

/* private declarations */


/**
 * A helper method to write var.va#.
 */
void SimpleNextion::_varName(uint8_t id) {
	_serial->print(F("var.va"));
	_serial->print(id);
}

/**
 * A helper method to write the end of a Nextion transmission.
 */
void SimpleNextion::_tripple0xFF() {
	_serial->write(0xFF);
	_serial->write(0xFF);
	_serial->write(0xFF);
}

/**
 * A helper method to get a response from the Nextion.
 */
bool SimpleNextion::_ping() {
	uint32_t timestamp;
	_serial->print(F("printh AA"));
	_tripple0xFF();
	_serial->flush();
	timestamp = millis();
	while (true) {
		if (_serial->available())
			if (_serial->read() == 0xAA)
				return true;
		if ((millis() - timestamp) > _rxTimeout)
			return false;
	}
}
