int motor1Pin1 = 4; // pin 2 on L293D
int motor1Pin2 = 3; // pin 7 on L293D
int enablePin = 13; // pin 1 on L293D
int FlipFlop_C =0;

void setup() {
    // set all the other pins you're using as outputs:
    pinMode(motor1Pin1, OUTPUT);
    pinMode(motor1Pin2, OUTPUT);
    pinMode(enablePin, OUTPUT);

    // set enablePin high so that motor can turn on:
    digitalWrite(enablePin, HIGH);
}

void loop() {
  
    if (FlipFlop_C%2==0) {
        digitalWrite(motor1Pin1, LOW); // set pin 2 on L293D low
        digitalWrite(motor1Pin2, HIGH); // set pin 7 on L293D high
        delay(5000);
    }
    else {
        digitalWrite(motor1Pin1, HIGH); // set pin 2 on L293D high
        digitalWrite(motor1Pin2, LOW); // set pin 7 on L293D low
        delay(5000);
    }
    FlipFlop_C++;
}
