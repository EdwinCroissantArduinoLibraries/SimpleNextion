#SimpleNextion library

<b> 2016-07-27 Proof of concept stage -- things will change :)

For Nextion editer version 0.36 </b>

This is a bare bone library for the Nextion displays.

To make this as simple as possible there is a strict seperation between the Arduino and the Nextion:
The Arduino can only set variables and recieve only Id-Value pairs.

##Requirements:
* The variables must situated on a page called var.

* Only variables with a name like var# (# is a value between 0 and 254) can be set.

* On each visible page that must be updated when a variable is changed a timer called tm0 must be pressent set at the shortest time possible and in the off state.

* The timer event contains the instructions to update the active page and tm0.en=0.

* In the Preinitialize Event of each page that must be updated when a variable is changed the instruction tm0.en=1 must be present.

* Instructions to the Arduino must be in the format:

  * For integer values:
    ```
    print "I"
    printh <commandID> // (00-FF)
    print <integer value>
    printh 00
    ``` 

  * For text values:

    printh "T"
    printh <commandID> // (00-FF) 
    print <text>
    printh 00


##Usage
* SimpleNextion(Serial, optional default baudrate of the Nection)

  Couples the Nextion to a serial and passes the default baud rate of the Nextion, if the default baud rate is omitted 9600 baud is used.
 
* begin(serial, baudrate)

  Start the communication with the Nextion and sets the baud rate valid baud rates are: 2400, 4800, 9600, 19200, 38400,57600 and 115200.

* setInt(VarId, Value)

  Set a specific Nextion variable to an integer value

* setTxt(VarId, *char)

  Set a specific Nextion variable to a text value
  
* update()

  Kick's the local timer 0 that updates the Nextion  
 
* listen()

  To be placed in the main loop. Reads the serial buffer and breaks down the messages and calls the appropriate callback functions:

* CallbackInt

  Assign a function like `callbackInt(uint8_t id, int32_t value)` to this variable.

* CallbackTxt

  Assign a function like `callbackTxt(uint8_t id, char *txt)` to this variable.



See the example how to use this library.

 
