using namespace std ;

int motor1Pin1 = 4; // pin 2 on L293D
int motor1Pin2 = 3; // pin 7 on L293D
int enablePin = 11; // pin 1 on L293D
double Speed = 0; // value 0 to 1 representing duty cycle of the motor
int Direction =0; // 0 = forward , 1 = backwards
int factor = -10;

String message ="";// serial input


typedef enum {FORWARD , BACKWARD , SPEED , INVALID} Command_type;
typedef enum {LEFT,RIGHT,STRAIGHT,NONE} Command_direction;

class Command {
  public://TODO turn to private and make proper functions
    Command_type c_type ;
    Command_direction c_direction;
    int Speed ;
    int intensity ; 
  
};

Command command ;
void getCommand(String msg){
  // TODO testing stage , only int inpit
  if(msg == ""){
    return;
  }
  Serial.write(" msg is : ");
  Serial.write(msg.c_str());
  Serial.write(" \n");
  command.c_type = SPEED;
  command.Speed = atoi( msg.c_str() );
  Serial.write(" Roger that \n");
  Serial.write(" Speed is : ");
  Serial.write(int(Speed));
  Serial.write(" \n");
  
  return ;
}

void ExecuteCurrentCommand(void){
  
  if(command.c_type == FORWARD){
    Direction = 0;
    command.c_type=INVALID;
    return ;
  }
  if(command.c_type == BACKWARD){
    Direction = 1;
    command.c_type=INVALID;
    return ;
  }
  if(command.c_type == SPEED){
    Speed = command.Speed;
    return ;
  }

}


void setup() {
    // set all the other pins you're using as outputs:
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(enablePin, OUTPUT);

    Serial.begin(9600); 
    /*
    // TODO move to default constructor of Command
    command.c_type = INVALID;
    command.c_direction = NONE;
    command.Speed =0;
    command.intensity = 0;
*/
    Speed = 100;
}

void loop() {
  /*
   // flip flop direction test
    if (Direction%2==0) {
        digitalWrite(motor1Pin1, LOW); // set pin 2 on L293D low
        digitalWrite(motor1Pin2, HIGH); // set pin 7 on L293D high
        delay(5000);
    }
    else {
        digitalWrite(motor1Pin1, HIGH); // set pin 2 on L293D high
        digitalWrite(motor1Pin2, LOW); // set pin 7 on L293D low
        delay(5000);
    }
    Direction++;
  */
  /*
  while(Serial.available())
  {//while there is data available on the serial monitor
    message+=char(Serial.read());//store string from serial command
  }
  if(!Serial.available())
  {
    if(message!="")
    {//if data is available
      message.toUpperCase();
      getCommand(message);
      message=""; //clear the data
    }
  }
  ExecuteCurrentCommand();

  if (Direction%2==0) {
        digitalWrite(motor1Pin1, LOW); // set pin 2 on L293D low
        digitalWrite(motor1Pin2, HIGH); // set pin 7 on L293D high
    }
    else {
        digitalWrite(motor1Pin1, HIGH); // set pin 2 on L293D high
        digitalWrite(motor1Pin2, LOW); // set pin 7 on L293D low
    }
    analogWrite(enablePin,(Speed/100.0)*255);
*/

 if (Direction%2==0) {
        digitalWrite(motor1Pin1, LOW); // set pin 2 on L293D low
        digitalWrite(motor1Pin2, HIGH); // set pin 7 on L293D high
    }
    else {
        digitalWrite(motor1Pin1, HIGH); // set pin 2 on L293D high
        digitalWrite(motor1Pin2, LOW); // set pin 7 on L293D low
    }

    analogWrite(enablePin,(Speed/100.0)*255);
    delay(1000);
    Speed += factor;
    if(Speed <= 0 || Speed > 100 ){
      factor= factor * -1;
      Direction ++;
    }


  
}
