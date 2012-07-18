#include <Servo.h>

// Mouth Servo configuration
const int pinMouthServo = 11;
const int mouthclosedUS = 1000;
const int mouthopenUS = 400;
Servo mouthServo;

// PIN Assignments
const int pinSound1 = 4;
const int pinSound2 = 5;
const int pinCrank = 6;
const int pinLidOpenValve = 7;
const int pinLidCloseValve = 8;
const int pinHeadRaiseValve = 9;
const int pinHeadLowerValve = 10;
const int pinTrigger = 0;
const int pinLidPosition = 1;
const int pinHeadPosition = 2;

// Analog ranges for position sensors.  
// Zero = no position reading - not near mag reed
// Important! Precision must be less than half of difference between high and low thresholds.
const int AnalogHeadDown = 200;
const int AnalogHeadUp = 50;
const int AnalogHeadPrecision = 25;
const int AnalogLidOpen = 200;
const int AnalogLidClosed = 50;
const int AnalogLidPrecision = 20;

// Status values
const int idle = 0;
const int triggered = 10;
const int musicplaying = 20;
const int musicfinished = 30;
const int lidopening = 40;
const int lidopen = 50;
const int headrising = 60;
const int headraised = 70;
const int laughing = 80;
const int laughingfinished = 90;
const int headlowering = 100;
const int headlowered = 110;
const int lidclosing = 120;
const int lidclosed = 130;

// Misc.
int state;
const int diag = 0; //change to 1 to output state to serial with each change
const int ELKms = 200; // ELK-120 only needs a momentary contact

void setup()
{
  if(diag == 1){Serial.begin(9600);}
  SetupProp();
}

void loop()
{
  CheckTrigger();
  if(state == triggered){PlayMusic();}
  if(state == musicfinished){OpenLid();}
  if(state == lidopen){RaiseHead();}
  if(state == headraised){StartLaugh();}
  if(state == laughingfinished){LowerHead();}
  if(state == headlowered){CloseLid();}
  
  // don't go back to idle state until trigger turned off, otherwise will loop continuously.
  if(state == lidclosed && analogRead(pinTrigger) == 0){ChangeStatus(idle);}
  
}

void StartLaugh()
{
  ChangeStatus(laughing);
  digitalWrite(pinSound2, HIGH);  
  delay(ELKms);
  digitalWrite(pinSound2, LOW);
    
  delay(700);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(400);
  mouthServo.writeMicroseconds(mouthclosedUS);
  
  delay(700);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(400);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(700);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(400);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(700);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(400);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(700);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(400);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(3000); //final delay to keep head up 

  ChangeStatus(laughingfinished);  
}

void LowerHead()
{
  ChangeStatus(headlowering);
  digitalWrite(pinHeadRaiseValve, LOW); 
  digitalWrite(pinHeadLowerValve, HIGH); 
  while(analogRead(pinHeadPosition) > AnalogHeadDown + AnalogHeadPrecision || analogRead(pinHeadPosition) < AnalogHeadDown - AnalogHeadPrecision){delay(10);} 
  digitalWrite(pinHeadLowerValve, LOW); 
  ChangeStatus(headlowered);
}

void RaiseHead()
{
  ChangeStatus(headrising);
  digitalWrite(pinHeadRaiseValve, HIGH); 
  while(analogRead(pinHeadPosition) > AnalogHeadUp + AnalogHeadPrecision || analogRead(pinHeadPosition) < AnalogHeadUp - AnalogHeadPrecision){delay(10);}
  ChangeStatus(headraised);
}

void CloseLid()
{
  ChangeStatus(lidclosing);
  digitalWrite(pinLidOpenValve, LOW);
  digitalWrite(pinLidCloseValve, HIGH);
  while(analogRead(pinLidPosition) > AnalogLidClosed + AnalogLidPrecision || analogRead(pinLidPosition) < AnalogLidClosed - AnalogLidPrecision){delay(10);}
  digitalWrite(pinLidCloseValve, LOW);
  
  delay(5000); // delay for tank recharging before next trigger
  
  ChangeStatus(lidclosed);
}

void OpenLid()
{
  ChangeStatus(lidopening);
  digitalWrite(pinLidOpenValve, HIGH);
  while(analogRead(pinLidPosition) > AnalogLidOpen + AnalogLidPrecision || analogRead(pinLidPosition) < AnalogLidOpen - AnalogLidPrecision){delay(10);}
  ChangeStatus(lidopen);
}

void PlayMusic()
{
  ChangeStatus(musicplaying);
  digitalWrite(pinCrank, HIGH);
  digitalWrite(pinSound1, HIGH);
  delay(ELKms); 
  digitalWrite(pinSound1, LOW);
  
  delay(4300); // set to length of sound #1
  
  digitalWrite(pinCrank, LOW);
  ChangeStatus(musicfinished);
}

void CheckTrigger()
{
   if(state == idle && analogRead(pinTrigger) > 128){ChangeStatus(triggered);} 
}

void ChangeStatus(int newstatus)
{
  state = newstatus;
  if(diag == 1){Serial.println("State changed to " + char(state)); } 
}

void SetupProp()
{
  mouthServo.attach(pinMouthServo);
  mouthServo.writeMicroseconds(mouthclosedUS);
  
  pinMode(pinCrank, OUTPUT);
  pinMode(pinSound1, OUTPUT);
  pinMode(pinSound2, OUTPUT);
  pinMode(pinLidOpenValve, OUTPUT);
  pinMode(pinLidCloseValve, OUTPUT);
  pinMode(pinHeadRaiseValve, OUTPUT);
  pinMode(pinHeadLowerValve, OUTPUT); 
  
  digitalWrite(pinCrank, LOW);
  digitalWrite(pinSound1, LOW);
  digitalWrite(pinSound2, LOW);
    
  // recovery procedure if power failed, check current positions and initiate closure
  // if head is not all the way down, open lid and set status to laughingfinished
  
  if(analogRead(pinHeadPosition)>0)
  {
    OpenLid();
    ChangeStatus(laughingfinished);
  }
  else
  {
    digitalWrite(pinLidOpenValve, LOW);
    digitalWrite(pinLidCloseValve, LOW);
    digitalWrite(pinHeadRaiseValve, LOW);
    digitalWrite(pinHeadLowerValve, LOW);
    ChangeStatus(lidclosed);
  }
}


