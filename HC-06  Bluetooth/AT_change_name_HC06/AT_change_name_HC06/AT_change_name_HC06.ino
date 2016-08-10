#include <SoftwareSerial.h>
SoftwareSerial mySerial(0, 1);
//for AT mode connect TX -> 4 and RX -> 2
//for nomarl mode connect RX-> 4 and TX -> 2
//if it does not work try switch the wires after you upload the sketch
//important - set no line endinging and not NL & CR in serial

String command = ""; // Stores response of the HC-06 Bluetooth device


void setup() {
  // Open serial communications:
  Serial.begin(9600);
  Serial.println("Type AT commands!");
  
  // The HC-06 defaults to 9600 according to the datasheet.
  mySerial.begin(38400);
}

void loop() {

  // Read device output if available.
  if (mySerial.available()) {
    while(mySerial.available()) { // While there is more to be read, keep reading.
      command += (char)mySerial.read();
    }
    
    Serial.println(command);
    command = ""; // No repeats
  }

  // Read user input if available.
  if (Serial.available()) {
	  delay(10); // The delay is necessary to get this working!
	  mySerial.write(Serial.read());
  }
  
}

