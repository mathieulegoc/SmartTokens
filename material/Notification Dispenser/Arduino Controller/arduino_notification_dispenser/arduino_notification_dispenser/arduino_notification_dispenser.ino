#include <Servo.h>

#define NB_CATEGORIES 3

const int buttonPin = 10;
const int ledPin =  13;

Servo servo[3];
int buttonState = 0;
byte servoAngle = 80;
boolean running = false;


void setup()
{
  Serial.begin(9600);
  //Initialize all the dispensers'servos
  //Here using pin 9, 10, 11
  for (int i = 0; i < NB_CATEGORIES; i++)
  {
    servo[i].attach(9 + i);
    servo[i].write(servoAngle);
  }

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(ledPin, LOW);
}

//routine to drop a token, moves a arm wait a bit and move it back
void dropToken(byte id)
{
  servo[id].write(servoAngle - 20);
  delay(108);
  servo[id].write(servoAngle);
  delay(250);
}

void loop()
{
  byte id = 0;
  if (Serial.available()) {
    //incoming byte indicate which dispenser has to drop a token
    id = Serial.read() - 1;
    if (id < NB_CATEGORIES)
      dropToken(id);
  }
}







