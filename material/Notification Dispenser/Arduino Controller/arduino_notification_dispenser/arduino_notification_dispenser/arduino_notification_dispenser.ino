#include <Servo.h>

#define NB_CATEGORIES 3

const int buttonPin = 10;
const int ledPin =  13;

Servo servo[3];
int buttonState = 0;
byte servoAngle[] = {65, 64, 68}; //pour #2 70; avec delta=10 et delay=90
boolean running = false;

void setup()
{
  Serial.begin(9600);
  for (int i = 0; i < NB_CATEGORIES; i++) {
    servo[i].attach(9 + i);
    servo[i].write(servoAngle[i]);
  }
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(ledPin, LOW);
}

void dropToken(byte id) {
  servo[id].write(servoAngle[id] - 8);
  delay(60); //delay(80);
  //  if(id==2){
  //        servo[id].write(servoAngle[id]-8);
  //   delay(60); //delay(80);
  //  }
  //  else{
  //    servo[id].write(servoAngle[id]-8);
  //    delay(60);
  //  }
  servo[id].write(servoAngle[id]);
  delay(250);
}

void loop()
{
  byte id = 0;
  if (Serial.available()) {
    id = Serial.read() - 1;
    if (id < NB_CATEGORIES)
      dropToken(id);
  }
}







