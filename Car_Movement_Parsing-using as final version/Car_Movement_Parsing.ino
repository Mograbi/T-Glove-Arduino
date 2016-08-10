#include <stdio.h>
#include <stdlib.h>
#include <SoftwareSerial.h>

using namespace std;

int motor1Pin1 = 2; // pin 7 on L293D 1
int motor1Pin2 = 4; // pin 2 on L293D  1
int enablePin1 = 3; // pin 1 on L293D 1
int motor2Pin1 = 7; // pin 10 on L293D 1
int motor2Pin2 = 6; // pin 15 on L293D 1
int enablePin2 = 5; // pin 9 on L293D 1

int LED = 13;
          /*int motor3Pin1 = 2; // pin 7 on L293D 2
          int motor3Pin2 = 4; // pin 2 on L293D 2
          int enablePin3 = 3; // pin 1 on L293D 2
          int motor4Pin1 = 7; // pin 10 on L293D 2
          int motor4Pin2 = 6; // pin 15 on L293D 2
          int enablePin4 = 5; // pin 9 on L293D 2*/
          /*double Speed = 0; // value 0 to 1 representing duty cycle of the motor
          int Direction = 0; // 0 = forward , 1 = backwards
          int factor = -10;
          */
String message = "";// serial input

          //int motor_controls[4][3] = { 2,4,3,7,6,5,2,4,3,7,6,5 };
int motor_controls[2][3] = { 2,4,3,7,6,5 };
typedef enum {LEFT_MOTOR , RIGHT_MOTOR} MOTOR_SIDE_t;

using namespace std;
typedef enum {
  LEFT, RIGHT, STRAIGHT, FORWARD, BACKWARD, NODIRECTION
} Direction_t;
typedef enum {
  MOVEMENT, PAUSE, UNPAUSE, NOMODE
} Mode_t;

class Command {
private:
  Direction_t Direction;
  Direction_t mvmnt_mode;
  int Speed;
  int intensity;
  Mode_t mode;
public:

  Command() :
    Direction(NODIRECTION), mvmnt_mode(NODIRECTION), Speed(0), intensity(
      0), mode(NOMODE) {
  }

  void set_direction(const Direction_t& c) {
    Direction = c;
  }
  void set_speed(int s) {
    Speed = s;
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
    return Direction;
  }
  int get_speed() {
    return Speed;
  }
  int get_intensity() {
    return intensity;
  }
  Mode_t get_mode() {
    return mode;
  }
  Direction_t get_mvmnt_mode() {
    return mvmnt_mode;
  }
  void printcmd() {
    Serial.print("direction = ");
    Serial.println(Direction);
    Serial.print("movement mode = ");
    Serial.println(mvmnt_mode);
    Serial.print("mode = ");
    Serial.println(mode);
    Serial.print("speed = ");
    Serial.println(Speed);
    Serial.print("intensity = ");
    Serial.println(intensity);
  }

};

void move_motor(int motor_index, int mode_flage, int spd) {
  if (spd < 0) {
    spd = (-spd);
    mode_flage = 1 - mode_flage;
  }
  if(spd > 100)
    spd = 100;
  
  if (mode_flage == 0) {//forward
    digitalWrite(motor_controls[motor_index][0], LOW); // set pin 2 on L293D low
    digitalWrite(motor_controls[motor_index][1], HIGH); // set pin 7 on L293D high
    analogWrite(motor_controls[motor_index][2], (spd / 100.0) * 255);
  }
  else if (mode_flage == 1) {
    digitalWrite(motor_controls[motor_index][0], HIGH); // set pin 2 on L293D low
    digitalWrite(motor_controls[motor_index][1], LOW); // set pin 7 on L293D high
    analogWrite(motor_controls[motor_index][2], (spd / 100.0) * 255);
  }


}

Command parsing_command(String input) {
  Command cmd;
  Mode_t mode;
  int angle, spd;
  String param1;
  if (input.indexOf(",") != -1) {
    String int1 = input.substring(input.indexOf("<") + 1, input.indexOf(","));
    angle = atoi(int1.c_str());

       
    String int2 = input.substring(input.indexOf(",") + 1, input.indexOf(">"));
    spd = atoi(int2.c_str());
    /* Serial.print("int1 ");
    Serial.print(int1);
    Serial.print("  ");
    Serial.print("int2 ");
    Serial.println(int2);
    */

    mode = (MOVEMENT);
  }
  else {
    param1 = input.substring(input.indexOf("<") + 1, input.indexOf(">"));
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
    //rabea angle tunning
    /*  angle = angle*1.1; // +10%
     if(angle>100){
       angle =100;
     }*/
    //rabea angle tunning
      cmd.set_intensity(angle);
    }
    else {
      cmd.set_direction(LEFT);
      angle = angle*1.35; // +35%
     if(angle<-100){
       angle =-100;
     }
      cmd.set_intensity(-angle);
    }

    if (spd < 0) {
      cmd.set_mvmnt_mode(BACKWARD);
      cmd.set_speed(-spd);
    }
    else {
      cmd.set_mvmnt_mode(FORWARD);
      cmd.set_speed(spd);
    }

    return cmd;
  }
  cmd.set_mode((String("P") == param1) ? PAUSE : UNPAUSE);
  return cmd;

}


void ExcecuteCommand(Command cmd) {
  int mode_flag;
  int delta_range = 70; // max value is 70 - may need to fix the 30 value
  int intensity_delta = 0; //put is as zero to get old behaviour
  if (cmd.get_mvmnt_mode() == FORWARD) {
    mode_flag = 0;
  }
  else if (cmd.get_mvmnt_mode() == BACKWARD) {
    mode_flag = 1;
  }
  else {
    mode_flag = 2;
  }

  int spd = cmd.get_speed();
  if( spd <=10 ){
      spd =0;   
    }else{
      spd = ((spd -10)/90.0)*70 +30 ;
    }
  
  if (cmd.get_mode() == PAUSE || cmd.get_mvmnt_mode() == NODIRECTION || spd ==0) {

    analogWrite(motor_controls[RIGHT_MOTOR][2], 0);
    analogWrite(motor_controls[LEFT_MOTOR][2], 0);
    return;
  }
  if (cmd.get_direction() == STRAIGHT) {
    //Serial.println("going straight");
    for (int i = 0; i < 2; i++)
      move_motor(i, mode_flag, spd);
    return;
  }
  if (cmd.get_direction() == RIGHT) {
    //Serial.println("going right");
    
    int inner_motor_speed = spd-(cmd.get_intensity());
    inner_motor_speed = (inner_motor_speed > 30 )? inner_motor_speed : 30;
	
	

	if(spd - inner_motor_speed < delta_range*((cmd.get_intensity()+intensity_delta)/100.0)){
      spd = (delta_range*((cmd.get_intensity()+intensity_delta)/100.0)) + inner_motor_speed;
    }
	/*if(spd - inner_motor_speed < delta_range*(cmd.get_intensity()/100.0)){
      inner_motor_speed = spd-(delta_range*(cmd.get_intensity()/100.0));
	  inner_motor_speed = (inner_motor_speed > 30 )? inner_motor_speed : 30;
    }*/
    move_motor(RIGHT_MOTOR, mode_flag, inner_motor_speed);
    move_motor(LEFT_MOTOR, mode_flag, spd);

    return;
  }
 
  if (cmd.get_direction() == LEFT) {
    //Serial.println("going left");
    int inner_motor_speed = spd-(cmd.get_intensity());
    inner_motor_speed = (inner_motor_speed > 30 )? inner_motor_speed : 30;
    
  if(spd - inner_motor_speed < delta_range*((cmd.get_intensity()+intensity_delta)/100.0)){
      spd = (delta_range*((cmd.get_intensity()+intensity_delta)/100.0)) + inner_motor_speed;
    }
	
    /*if(spd - inner_motor_speed < delta_range*(cmd.get_intensity()/100.0)){
      inner_motor_speed = spd-(delta_range*(cmd.get_intensity()/100.0));
	  inner_motor_speed = (inner_motor_speed > 30 )? inner_motor_speed : 30;	  
    }*/
    
    move_motor(LEFT_MOTOR, mode_flag, inner_motor_speed);
    move_motor(RIGHT_MOTOR, mode_flag, spd);

    return;
  }
  return;
}

Command command;
SoftwareSerial blSerial(8,9); //TX,RX
String cmd_buffer;
void setup() {
  // set all the other pins you're using as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enablePin1, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enablePin2, OUTPUT);
  pinMode(LED, OUTPUT);
  /* pinMode(motor3Pin1, OUTPUT);
  pinMode(motor3Pin2, OUTPUT);
  pinMode(enablePin3, OUTPUT);
  pinMode(motor4Pin1, OUTPUT);
  pinMode(motor4Pin2, OUTPUT);
  pinMode(enablePin4, OUTPUT);*/

  //Speed = 100;

 // Serial.begin(9600);
  blSerial.begin(9600);
  cmd_buffer = "";
  //Serial.println("Ready to RUMBLE");
  digitalWrite(LED, LOW);
}

void loop() {
  

  // Keep reading from HC-05 and send to Arduino Serial Monitor
   
 if (blSerial.available()) {
    digitalWrite(LED, LOW);
 // Serial.println(" ALL HAIL BELAL 1");
    //Serial.write("###");
    char my_c=blSerial.read();
    cmd_buffer+=my_c;

    //Serial.write(my_c);
   // Serial.write("\n");
    
    if(my_c=='>'){
      //mySerial.flush();
      int val1;
      int val2;
      int res = sscanf(cmd_buffer.c_str(),"<%d,%d>",&val1,&val2);
      if(res != 2){
        //Serial.println("problem in fromat");
        //delay(5000);
        digitalWrite(LED, HIGH);
        delay(50);
      }
      digitalWrite(LED, LOW);

  
  
    command = parsing_command(cmd_buffer);
    command.printcmd();
    ExcecuteCommand(command);
    cmd_buffer = "";
    }
    
  }else{
    digitalWrite(LED, HIGH);
    /*command = Command();
     ExcecuteCommand(command);*/
  }

  // Keep reading from Arduino Serial Monitor and send to HC-05
  //if (Serial.available())
    //mySerial.write(Serial.read());
}



