// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v2.0)
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2013-05-08 - added seamless Fastwire support
//                 - added note about gyro calibration
//      2012-06-21 - added note about Arduino 1.0.1 + Leonardo compatibility error
//      2012-06-20 - improved FIFO overflow handling and simplified read process
//      2012-06-19 - completely rearranged DMP initialization code and simplification
//      2012-06-13 - pull gyro and accel data from FIFO packet instead of reading directly
//      2012-06-09 - fix broken FIFO read sequence and change interrupt detection to RISING
//      2012-06-05 - add gravity-compensated initial reference frame acceleration output
//                 - add 3D math helper file to DMP6 example sketch
//                 - add Euler output and Yaw/Pitch/Roll output formats
//      2012-06-04 - remove accel offset clearing for better results (thanks Sungon Lee)
//      2012-06-01 - fixed gyro sensitivity to be 2000 deg/sec instead of 250
//      2012-05-30 - basic DMP initialization working

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include <SoftwareSerial.h>
#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

/* =========================================================================
NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
depends on the MPU-6050's INT pin being connected to the Arduino's
external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
digital I/O pin 2.
* ========================================================================= */

/* =========================================================================
NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
when using Serial.write(buf, len). The Teapot output uses this method.
The solution requires a modification to the Arduino USBAPI.h file, which
is fortunately simple, but annoying. This will be fixed in the next IDE
release. For more info, see these links:

http://arduino.cc/forum/index.php/topic,109987.0.html
http://code.google.com/p/arduino/issues/detail?id=958
* ========================================================================= */



// uncomment "OUTPUT_READABLE_QUATERNION" if you want to see the actual
// quaternion components in a [w, x, y, z] format (not best for parsing
// on a remote host such as Processing or something though)
//#define OUTPUT_READABLE_QUATERNION

// uncomment "OUTPUT_READABLE_EULER" if you want to see Euler angles
// (in degrees) calculated from the quaternions coming from the FIFO.
// Note that Euler angles suffer from gimbal lock (for more info, see
// http://en.wikipedia.org/wiki/Gimbal_lock)
//#define OUTPUT_READABLE_EULER

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
#define OUTPUT_READABLE_YAWPITCHROLL

// uncomment "OUTPUT_READABLE_REALACCEL" if you want to see acceleration
// components with gravity removed. This acceleration reference frame is
// not compensated for orientation, so +X is always +X according to the
// sensor, just without the effects of gravity. If you want acceleration
// compensated for orientation, us OUTPUT_READABLE_WORLDACCEL instead.
//#define OUTPUT_READABLE_REALACCEL

// uncomment "OUTPUT_READABLE_WORLDACCEL" if you want to see acceleration
// components with gravity removed and adjusted for the world frame of
// reference (yaw is relative to initial orientation, since no magnetometer
// is present in this case). Could be quite handy in some cases.
//#define OUTPUT_READABLE_WORLDACCEL

// uncomment "OUTPUT_TEAPOT" if you want output that matches the
// format used for the InvenSense teapot demo
//#define OUTPUT_TEAPOT


#define SAMPLE_SIZE 5
#define OUT_OF_BOUNDS -1000
int MAX_ANGLE = 80;
int MIN_ANGLE = -80;
int ANGLE_RANGE = 160;
int CENTER_OFFSET = 0;
int CALIBRATION_FLAG = 0; // max tries 3
#define MARGEN_PERCENTAGE 0.05
#define STEERING_MARGEN 0.1


SoftwareSerial blSerial(7, 6); //TX,RX


int lastdirection;
int lastsamples[SAMPLE_SIZE];
int Insertindex = 0;

#define YELLOW_R 7
#define YELLOW_L 5
#define RED_M	6
#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;


// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

						// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

						// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };



// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
	mpuInterrupt = true;
}



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
	// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
	TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
	//TWBR = 12;//fifo overflow fix -trying
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
	Fastwire::setup(400, true);
#endif

	// initialize serial communication
	// (115200 chosen because it is required for Teapot Demo output, but it's
	// really up to you depending on your project)
	blSerial.begin(9600);
	Serial.begin(115200);
	while (!Serial); // wait for Leonardo enumeration, others continue immediately

					 // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3v or Ardunio
					 // Pro Mini running at 3.3v, cannot handle this baud rate reliably due to
					 // the baud timing being too misaligned with processor ticks. You must use
					 // 38400 or slower in these cases, or use some kind of external separate
					 // crystal solution for the UART timer.

					 // initialize device
	Serial.println(F("Initializing I2C devices..."));
	mpu.initialize();

	// verify connection
	Serial.println(F("Testing device connections..."));
	Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

	// wait for ready
	Serial.println(F("\nSend any character to begin DMP programming and demo: "));
	while (Serial.available() && Serial.read()); // empty buffer
	while (!Serial.available());                 // wait for data
	while (Serial.available() && Serial.read()); // empty buffer again

												 // load and configure the DMP
	Serial.println(F("Initializing DMP..."));
	devStatus = mpu.dmpInitialize();

	// supply your own gyro offsets here, scaled for min sensitivity
	mpu.setXGyroOffset(220);
	mpu.setYGyroOffset(76);
	mpu.setZGyroOffset(-85);
	mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

							   // make sure it worked (returns 0 if so)
	if (devStatus == 0) {
		// turn on the DMP, now that it's ready
		Serial.println(F("Enabling DMP..."));
		mpu.setDMPEnabled(true);

		// enable Arduino interrupt detection
		Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
		attachInterrupt(0, dmpDataReady, RISING);
		mpuIntStatus = mpu.getIntStatus();

		// set our DMP Ready flag so the main loop() function knows it's okay to use it
		Serial.println(F("DMP ready! Waiting for first interrupt..."));
		dmpReady = true;

		// get expected DMP packet size for later comparison
		packetSize = mpu.dmpGetFIFOPacketSize();
	}
	else {
		// ERROR!
		// 1 = initial memory load failed
		// 2 = DMP configuration updates failed
		// (if it's going to break, usually the code will be 1)
		Serial.print(F("DMP Initialization failed (code "));
		Serial.print(devStatus);
		Serial.println(F(")"));
	}


	// configure LED for output
	pinMode(LED_PIN, OUTPUT);
	pinMode(YELLOW_L, OUTPUT);
	pinMode(RED_M, OUTPUT);
	pinMode(YELLOW_R, OUTPUT);

	for (int i = 0; i < 5; i++)
		lastsamples[i] = OUT_OF_BOUNDS;

	lastdirection = OUT_OF_BOUNDS;
}



// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================






String getDirection(int direction) {

	int straight_range_size = STEERING_MARGEN * ANGLE_RANGE;
	int left_range_size = (ANGLE_RANGE / 2) - straight_range_size;
	int right_range_size = left_range_size;
	String output = "";
	if (direction  >  CENTER_OFFSET + straight_range_size) {

		output = "right ";
		output.concat((double)(direction - CENTER_OFFSET - straight_range_size) / (right_range_size));
	}
	else if (direction < CENTER_OFFSET - straight_range_size) {
		output += "left ";
		output.concat((double)(direction + CENTER_OFFSET - straight_range_size) / (left_range_size));
	}
	else {
		output += "straight ";
	}

	return output;

}







void loop() {
	// if programming failed, don't try to do anything
	if (!dmpReady) return;

	// wait for MPU interrupt or extra packet(s) available
	while (!mpuInterrupt && fifoCount < packetSize) {
		// other program behavior stuff here
		// .
		// .
		// .
		// if you are really paranoid you can frequently test in between other
		// stuff to see if mpuInterrupt is true, and if so, "break;" from the
		// while() loop to immediately process the MPU data
		// .
		// .
		// .
	}

	// reset interrupt flag and get INT_STATUS byte
	mpuInterrupt = false;
	mpuIntStatus = mpu.getIntStatus();

	// get current FIFO count
	fifoCount = mpu.getFIFOCount();

	// check for overflow (this should never happen unless our code is too inefficient)
	if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
		// reset so we can continue cleanly
		mpu.resetFIFO();
		Serial.println(F("FIFO overflow!"));

		// otherwise, check for DMP data ready interrupt (this should happen frequently)
	}
	else if (mpuIntStatus & 0x02) {
		// wait for correct available data length, should be a VERY short wait
		while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

		// read a packet from FIFO
		mpu.getFIFOBytes(fifoBuffer, packetSize);

		// track FIFO count here in case there is > 1 packet available
		// (this lets us immediately read more without waiting for an interrupt)
		fifoCount -= packetSize;

		// auto calibration dmp 20 sseconds - start
		if (CALIBRATION_FLAG == 0) {
			//CALIBRATION_FLAG++;
			Serial.print("calibrating - keep you hand straight for 20 seconds\n");
			delay(20000); // wait 20 seconds fro dmp auto calibration
						  //return;
		}
		// auto calibration dmp 20 sseconds - end

		// display Euler angles in degrees
		mpu.dmpGetQuaternion(&q, fifoBuffer);
		mpu.dmpGetGravity(&gravity, &q);
		mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
		/*Serial.print("ypr\t");
		Serial.print(ypr[0] * 180 / M_PI);
		Serial.print("\t");
		Serial.print(ypr[1] * 180 / M_PI);
		Serial.print("\t");
		Serial.println(ypr[2] * 180 / M_PI);*/
		ypr[0] = ypr[0] * 180 / M_PI;
		ypr[1] = ypr[1] * 180 / M_PI;
		ypr[2] = ypr[2] * 180 / M_PI;

		lastsamples[Insertindex] = ypr[1];
		int max = ypr[1];
		int min = ypr[1];

		for (int i = 2; i < SAMPLE_SIZE; i++) {
			if (lastsamples[i] > max)
				max = lastsamples[i];
			if (lastsamples[i] < min)
				min = lastsamples[i];
		}


		Serial.print("pitch      ");
		Serial.print(ypr[1]);
		Serial.print("	");
		if (abs(max - min) < MARGEN_PERCENTAGE*(ANGLE_RANGE)) {
			
			lastdirection = lastsamples[Insertindex];
		}

		Insertindex = (Insertindex + 1) % SAMPLE_SIZE;

		if (CALIBRATION_FLAG < 3) {
			CALIBRATION_FLAG++;
		}


		//calibration 
		if (CALIBRATION_FLAG == 3) {
			//print + delay
			//Serial.print("calibrating - keep you hand straight for 20 seconds\n");
			//delay(20000); // wait 20 seconds fro dmp auto calibration
			//CENTER_OFFSET = lastsamples[Insertindex - 1];
			CENTER_OFFSET = ypr[1];
			MAX_ANGLE = (ANGLE_RANGE / 2) + CENTER_OFFSET;
			MIN_ANGLE = (-1)*(ANGLE_RANGE / 2) + CENTER_OFFSET;
			Serial.print("CENTER_OFFSET =   ");
			Serial.print(CENTER_OFFSET);
			Serial.println("");
			Serial.print("MAX_ANGLE =   ");
			Serial.print(MAX_ANGLE);
			Serial.println("");
			Serial.print("MIN_ANGLE =   ");
			Serial.print(MIN_ANGLE);
			Serial.println("");
			Serial.println("calibration done - you can start steering the CAR - ENJOY");
			CALIBRATION_FLAG++;
			Serial.println("10 seconds window to read the values");
			delay(10000);
		}

		//

		Serial.print(getDirection(lastdirection));
		//blSerial.print(getDirection(lastdirection));

	}

	// slider part start
	// read the input on analog pin 0:
	int sensorValue = analogRead(A0);
	// print out the value you read:
	double power_percentage = (1 - (double)sensorValue / 1023);
	double light_strength = power_percentage * 255;
	analogWrite(9, light_strength);
	Serial.println("	Throutle - speed : " + String(power_percentage * 100));
	//blSerial.println("	Throutle - speed : " + String(power_percentage * 100)); //causes fifo overflow - slow printing
	// slider part end

}

