#include <stdio.h>
#include <stdlib.h>

using namespace std;

int motor1Pin1 = 4; // pin 7 on L293D 1
int motor1Pin2 = 3; // pin 2 on L293D  1
int enablePin1 = 10; // pin 1 on L293D 1
int motor2Pin1 = 7; // pin 10 on L293D 1
int motor2Pin2 = 6; // pin 15 on L293D 1
int enablePin2 = 11; // pin 9 on L293D 1
int motor3Pin1 = 13; // pin 7 on L293D 2
int motor3Pin2 = 12; // pin 2 on L293D 2
int enablePin3 = 5; // pin 1 on L293D 2
int motor4Pin1 = 8; // pin 10 on L293D 2
int motor4Pin2 = 2; // pin 15 on L293D 2
int enablePin4 = 9; // pin 9 on L293D 2
//double Speed = 0; // value 0 to 1 representing duty cycle of the motor
//int Direction = 0; // 0 = forward , 1 = backwards
//int factor = -10;

String message = "";// serial input

int motor_controls[4][3] = { 4, 3, 10, 7, 6, 11, 13, 12, 5, 8, 2, 9 };

using namespace std;
typedef enum {
	LEFT, RIGHT, STRAIGHT, FORWARD, BACKWARD, NODIRECTION
} Direction_t;
typedef enum {
	MOVEMENT, PAUSE, UNPAUSE, NOMODE
} Mode_t;

class Command {
private:
	//TODO turn to private and make proper functions
	Direction_t direction;
	Direction_t mvmnt_mode;
	int speed;
	int intensity;
	Mode_t mode;
public:

	Command() :
		direction(NODIRECTION), mvmnt_mode(NODIRECTION), speed(0), intensity(
			0), mode(NOMODE) {
	}

	void set_direction(const Direction_t& c) {
		direction = c;
	}
	void set_speed(int s) {
		speed = s;
	}
	void set_intensity(int i) {
		intensity = i;
	}
	void set_mode(Mode_t md) {
		mode = md;
	}
	void set_mvmnt_mode(Direction_t md) {
		mvmnt_mode = md;
	}
	Direction_t get_direction() {
		return direction;
	}
	int get_speed() {
		return speed;
	}
	int get_intense() {
		return intensity;
	}
	Mode_t get_mode() {
		return mode;
	}
	Direction_t get_mvmnt_mode() {
		return mvmnt_mode;
	}
	void print() {
		printf("direction = %d\n", direction);
		printf("monement mode = %d\n", mvmnt_mode);
		printf("mode = %d\n", mode);
		printf("speed = %d\n", speed);
		printf("intensity = %d\n", intensity);
	}

};

void move_motor(int motor_index,int mode_flage, int speed) {
	if (speed < 0) {
		speed = (-speed);
		mode_flage = 1 - mode_flage;
	}
	if (mode_flage == 0) {//forward
		digitalWrite(motor_controls[motor_index][0], LOW); // set pin 2 on L293D low
		digitalWrite(motor_controls[motor_index][1], HIGH); // set pin 7 on L293D high
		analogWrite(motor_controls[motor_index][3], (speed / 100.0) * 255);
	}
	else if (mode_flage == 1) {
		digitalWrite(motor_controls[motor_index][0], HIGH); // set pin 2 on L293D low
		digitalWrite(motor_controls[motor_index][1], LOW); // set pin 7 on L293D high
		analogWrite(motor_controls[motor_index][3], (speed / 100.0) * 255);
	}


}

Command parsing_command(string input) {
	Command cmd;
	Mode_t mode;
	int angle, speed;
	string param1;
	if (input.find(",") != std::string::npos) {
		string int1 = input.substr(1, input.find(",", 0) - 1);
		angle = atoi(int1.c_str());
		string int2 = input.substr(input.find(",", 0) + 1,
			input.find(">", 0) - input.find(",", 0) - 1);
		speed = atoi(int2.c_str());
		mode = (MOVEMENT);
	}
	else {
		param1 = input.substr(1, input.find(">", 0) - 1);
		mode = PAUSE;
	}

	if (mode == MOVEMENT && cmd.get_mode() == PAUSE)
		return cmd;
	if (mode == MOVEMENT) {
		cmd.set_mode(MOVEMENT);
		if (angle == 0) {
			cmd.set_direction(STRAIGHT);
		}
		else if (angle > 0) {
			cmd.set_direction(RIGHT);
			cmd.set_intensity(angle);
		}
		else {
			cmd.set_direction(LEFT);
			cmd.set_intensity(-angle);
		}

		if (speed < 0) {
			cmd.set_mvmnt_mode(BACKWARD);
			cmd.set_speed(-speed);
		}
		else {
			cmd.set_mvmnt_mode(FORWARD);
			cmd.set_speed(speed);
		}

		return cmd;
	}
	cmd.set_mode(("P" == param1) ? PAUSE : UNPAUSE);
	return cmd;

}


void ExcecuteCommand(Command cmd) {
	int mode_flag;
	if (cmd.get_mvmnt_mode() == FORWARD) {
		mode_flag = 0;
	}else if (cmd.get_mvmnt_mode() == BACKWARD) {
		mode_flag = 0;
	}else{
		mode_fag = 2;
	}
	if (cmd.get_mode() == PAUSE || cmd.get_mvmnt_mode(0 == NODIRECTION) {
		analogWrite(enablePin1, 0);
		analogWrite(enablePin2, 0);
		analogWrite(enablePin3, 0);
		analogWrite(enablePin4, 0);
		return;
	}
	if (cmd.get_direction() == STRAIGHT) {
		for (int i = 0; i < 4; i++)
			move_motor(i, mode_flage, cmd.get_speed());
		return;
	}
	if (cmd.get_direction() == RIGHT) {
		move_motor(0, mode_flag, cmd.get_speed());
		move_motor(2, mode_flag, cmd.get_speed());
		move_motor(1, mode_flag, cmd.get_speed()-cmd.get_intensity());
		move_motor(3, mode_flag, cmd.get_speed() - cmd.get_intensity());
		return;
	}
	if (cmd.get_direction() == LEFT) {
		move_motor(1, mode_flag, cmd.get_speed());
		move_motor(3, mode_flag, cmd.get_speed());
		move_motor(0, mode_flag, cmd.get_speed() - cmd.get_intensity());
		move_motor(2, mode_flag, cmd.get_speed() - cmd.get_intensity());
		return;
	}
	return;
}

Command command;
void setup() {
	// set all the other pins you're using as outputs:
	pinMode(motor1Pin1, OUTPUT);
	pinMode(motor1Pin2, OUTPUT);
	pinMode(enablePin1, OUTPUT);
	pinMode(motor2Pin1, OUTPUT);
	pinMode(motor2Pin2, OUTPUT);
	pinMode(enablePin2, OUTPUT);
	pinMode(motor3Pin1, OUTPUT);
	pinMode(motor3Pin2, OUTPUT);
	pinMode(enablePin3, OUTPUT);
	pinMode(motor4Pin1, OUTPUT);
	pinMode(motor4Pin2, OUTPUT);
	pinMode(enablePin4, OUTPUT);

	Speed = 100;
}

void loop() {
/*
	if (Direction % 2 == 0) {
		digitalWrite(motor1Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor1Pin2, HIGH); // set pin 7 on L293D high
		digitalWrite(motor2Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor2Pin2, HIGH); // set pin 7 on L293D high
		digitalWrite(motor3Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor3Pin2, HIGH); // set pin 7 on L293D high
		digitalWrite(motor4Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor4Pin2, HIGH); // set pin 7 on L293D high
	}
	else {
		digitalWrite(motor1Pin1, HIGH); // set pin 2 on L293D high
		digitalWrite(motor1Pin2, LOW); // set pin 7 on L293D low
		digitalWrite(motor2Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor2Pin2, HIGH); // set pin 7 on L293D high
		digitalWrite(motor3Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor3Pin2, HIGH); // set pin 7 on L293D high
		digitalWrite(motor4Pin1, LOW); // set pin 2 on L293D low
		digitalWrite(motor4Pin2, HIGH); // set pin 7 on L293D high
	}

	analogWrite(enablePin1, (Speed / 100.0) * 255);
	analogWrite(enablePin2, (Speed / 100.0) * 255);
	analogWrite(enablePin3, (Speed / 100.0) * 255);
	analogWrite(enablePin4, (Speed / 100.0) * 255);
	delay(1000);
	//Speed += factor;
	if (Speed <= 0 || Speed > 100) {
		factor = factor * -1;
		Direction++;
	}
*/


}