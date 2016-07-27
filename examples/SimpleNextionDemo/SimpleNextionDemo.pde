/*
 * Nextion display library example for the Arduino microcontroller.
 *
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

#include "Arduino.h"
#include <SimpleNextion.h>

/*
 * If Serial is used the Nextion must be disconnected before
 * uploading this sketch
 */

SimpleNextion myNextion(Serial);
uint32_t timestamp;
uint32_t refreshTime = 250;
char circularText[] = "0123456789";

enum varNames {
	varRefreshTime = 0,
	varCirculairText = 1,
	varMillis = 2,
};

enum rxNames {
	rxSkipTime = 0,
	rxRotateLeft = 1,
	rxRotateRight = 2
};

enum pins {
	led = 13
};

void setup() {
	pinMode(led, OUTPUT);
	if (!myNextion.begin(115200))
		blinkForever();
	myNextion.callbackInt = callbackInt;
	myNextion.callbackTxt = callbackTxt;
	myNextion.setInt(varRefreshTime, refreshTime);
	myNextion.setTxt(varCirculairText, circularText);
	myNextion.setInt(varMillis, millis());
	myNextion.update();
	timestamp = millis();
}

void loop() {
	if (millis() - timestamp >= refreshTime) {
		timestamp = millis();
		myNextion.setInt(varMillis, millis());
		myNextion.update();
	}
	myNextion.listen();
}


void callbackInt(uint8_t id, int32_t value) {
	switch (id) {
	case 0:
		refreshTime = value;
		myNextion.setInt(varRefreshTime, refreshTime);
		myNextion.update();
		break;
	}

}

void callbackTxt(uint8_t id, char *txt) {
	switch (id) {
	case rxRotateLeft: // rotate left
		rotateLeft(txt);
		myNextion.setTxt(varCirculairText, txt);
		myNextion.update();
		break;
	case rxRotateRight: // rotate right
		rotateRight(txt);
		myNextion.setTxt(varCirculairText, txt);
		myNextion.update();
		break;
	}
}

/*
 * Rotate all the characters in a zero terminated string anti clockwise
 */
void rotateLeft(char *txt) {
	uint8_t i, tempChar, lastChar;
	lastChar = strlen(txt) - 1;
	tempChar = txt[0];
	for (i = 0; i < lastChar; i++)
		txt[i] = txt[i + 1];
	txt[i] = tempChar;
}

/*
 * Rotate all the characters in a zero terminated string clockwise
 */
void rotateRight(char *txt) {
	uint8_t i, tempChar, lastChar;
	lastChar = strlen(txt) - 1;
	tempChar = txt[lastChar];
	for (i = lastChar; i > 0; i--)
		txt[i] = txt[i - 1];
	txt[0] = tempChar;
}

/*
 * Blink the led until reset or powered down.
 */
void blinkForever() {
	while (true) {
	  digitalWrite(led, HIGH);
	  delay(500);
	  digitalWrite(led, LOW);
	  delay(500);
	}
}

