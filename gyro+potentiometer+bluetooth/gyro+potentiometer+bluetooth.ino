/*void setup() {
	// initialize serial communication at 9600 bits per second:
	Serial.begin(9600);
	Serial.println("START - move slider");
}

// the loop routine runs over and over again forever:
void loop() {
	// read the input on analog pin 0:
	int sensorValue = analogRead(A0);
	// print out the value you read:
	double light_strength = (1 - (double)sensorValue / 1023) * 255;
	analogWrite(9, light_strength);
}*/

#include <SoftwareSerial.h>
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 mpu;

#define SAMPLE_SIZE 5
#define OUT_OF_BOUNDS -1000
int MAX_ANGLE = 80;
int MIN_ANGLE = -80;
int ANGLE_RANGE = 160;
int CENTER_OFFSET = 0;
bool CALIBRATION_FLAG = true;
#define MARGEN_PERCENTAGE 0.05
#define STEERING_MARGEN 0.1


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
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
	mpuInterrupt = true;
}

SoftwareSerial blSerial(7,6); //TX,RX

void setup() {

	// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
	TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
	Fastwire::setup(400, true);
#endif

	// initialize serial communication
	// (115200 chosen because it is required for Teapot Demo output, but it's
	// really up to you depending on your project)
	blSerial.begin(9600);
	Serial.begin(115200);
	while (!Serial); // wait for Leonardo enumeration, others continue immediately

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


/*String getDirection(int direction, int max_angle, int min_angle, int center_offset) {
	int max = max_angle + center_offset;
	int min = min_angle + center_offset;
	if (direction > max)
		direction = max;
	if (direction < min)
		direction = min;

	String output = "";
	if (direction - center_offset > STEERING_MARGEN * (max - min)) {

		output = "right ";
		output.concat((double)direction / (max - center_offset));
	}
	else if (direction - center_offset < -STEERING_MARGEN * (max - min)) {
		output += "left ";
		output.concat((double)direction / (center_offset - min));
	}
	else {
		output += "straight ";
	}

	return output;

}*/

String getDirection(int direction) {


	String output = "";
	if (direction  >  CENTER_OFFSET + (STEERING_MARGEN * ANGLE_RANGE)) {

		output = "right ";
		output.concat((double)(direction - CENTER_OFFSET) / (ANGLE_RANGE/2));
	}
	else if (direction < CENTER_OFFSET - (STEERING_MARGEN * ANGLE_RANGE)) {
		output += "left ";
		output.concat((double) (direction - CENTER_OFFSET) / (ANGLE_RANGE / 2));
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



		// put your main code here, to run repeatedly:
		mpu.dmpGetQuaternion(&q, fifoBuffer);
		mpu.dmpGetGravity(&gravity, &q);
		mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

		ypr[0] = ypr[0] * 180 / M_PI;
		ypr[1] = ypr[1] * 180 / M_PI;
		ypr[2] = ypr[2] * 180 / M_PI;

		lastsamples[Insertindex] = ypr[1];
		Insertindex = (Insertindex + 1) % SAMPLE_SIZE;
		int max = (lastsamples[0] > lastsamples[1]) ? lastsamples[0] : lastsamples[1];
		int min = (lastsamples[0] < lastsamples[1]) ? lastsamples[0] : lastsamples[1];

		for (int i = 2; i < SAMPLE_SIZE; i++) {
			if (lastsamples[i] > max)
				max = lastsamples[i];
			if (lastsamples[i] < min)
				min = lastsamples[i];
		}

		Serial.print("pitch		");
		Serial.print(ypr[1]);
		Serial.print("		");
		if (abs(max - min) < MARGEN_PERCENTAGE*(MAX_ANGLE - MIN_ANGLE)) {
			//Serial.print("        current angle      ");
			//Serial.print(lastsamples[SAMPLE_SIZE - 1]);
			if (abs(lastdirection - lastsamples[SAMPLE_SIZE - 1]) > STEERING_MARGEN*(MAX_ANGLE - MIN_ANGLE)) {
				lastdirection = lastsamples[SAMPLE_SIZE - 1];
				Serial.print("      direction     ");
				Serial.println(lastdirection);
			}
			else {
				//	Serial.println("");
			}


		}
		else {
			//Serial.println("");
		}

		Serial.print(getDirection(lastdirection));
		blSerial.print(getDirection(lastdirection));

	}
	// slider part start
	// read the input on analog pin 0:
	int sensorValue = analogRead(A0);
	// print out the value you read:
	double power_percentage = (1 - (double)sensorValue / 1023);
	double light_strength = power_percentage * 255;
	analogWrite(9, light_strength);
	Serial.println("	Throutle - speed : " + String(power_percentage*100));
	blSerial.println("	Throutle - speed : " + String(power_percentage*100));
	// slider part end

}

