#include <Servo.h>

// Mouth Servo configuration
const int pinMouthServo = 11;
const int mouthclosedUS = 1000;
const int mouthopenUS = 400;
Servo mouthServo;

// PIN Assignments
const int pinSpot = 3;
const int pinSound1 = 4;
const int pinSound2 = 5;
const int pinCrank = 6;
const int pinLidOpenValve = 7;
const int pinLidCloseValve = 8;
const int pinHeadRaiseValve = 10;
const int pinHeadLowerValve = 9;
const int pinTrigger = 3;
const int pinLidClosed = 2;
const int pinLidOpen = 1;
const int pinHeadDown = 0;
const int pinHeadUp = 0;

// Analog ranges for position sensors.  
// Zero = no position reading - not near mag reed
// Important! Precision must be less than half of difference between high and low thresholds.
const int AnalogTrigger = 767;
const int AnalogTriggerPrecision = 10;
const int AnalogHeadDown = 696;
const int AnalogHeadUp = 733;
const int AnalogHeadPrecision = 10;
const int AnalogLidOpen = 0;
const int AnalogLidClosed = 200;
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
int state = 130; //don't start in idle state - make sure trigger is false
const int diag = 1; //set to 1 to enable diagnostic mode
const int ELKms = 200; // ELK-120 only needs a momentary contact
const int Sound1ms = 3528; // Duration of Sound #1

void setup()
{
  SetupProp();  
  if(diag == 1)
  {
    Serial.begin(9600);
    DisplayStatus();
  }
  else
  {
    RecoverProp();
  }
}

void loop()
{
  if(state == idle && isPropTriggered() == true){StartProp();}
  if(state == idle && isPropTriggered() == false && diag == 1){DisplayStatus();delay(500);}
  if(state == triggered){PlayMusic();}
  if(state == musicfinished){OpenLid();}
  if(state == lidopen){RaiseHead();}
  if(state == headraised){StartLaugh();}
  if(state == laughingfinished){LowerHead();}
  if(state == headlowered){CloseLid();}
  
  // don't go back to idle state until trigger turned off, otherwise will loop continuously.
  if(state == lidclosed && isPropTriggered() == false){EndProp();}
}

void StartLaugh()
{
  ChangeStatus(laughing);
  digitalWrite(pinSound2, HIGH);  
  delay(ELKms);
  digitalWrite(pinSound2, LOW);
    
  delay(700); //initial delay for sound to start playing
  
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(334);
  mouthServo.writeMicroseconds(mouthclosedUS);
  
  delay(349);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(338);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(262);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(154);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(182);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(466);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(326);
  mouthServo.writeMicroseconds(mouthopenUS);
  delay(499);
  mouthServo.writeMicroseconds(mouthclosedUS);

  delay(3000); //final delay to keep head up 

  ChangeStatus(laughingfinished);  
}

void StartProp()
{
  digitalWrite(pinSpot, HIGH);
  ChangeStatus(triggered);
}

void EndProp()
{
  digitalWrite(pinSpot, LOW);
  ChangeStatus(idle);
}

void LowerHead()
{
  ChangeStatus(headlowering);
  digitalWrite(pinHeadRaiseValve, LOW); 
  digitalWrite(pinHeadLowerValve, HIGH); 
  while(isHeadDown() == false){delay(10);} 
  digitalWrite(pinHeadLowerValve, LOW); 
  ChangeStatus(headlowered);
}

void RaiseHead()
{
  ChangeStatus(headrising);
  digitalWrite(pinHeadRaiseValve, HIGH); 
  while(isHeadUp() == false){delay(10);}
  ChangeStatus(headraised);
}

void CloseLid()
{
  ChangeStatus(lidclosing);
  digitalWrite(pinLidOpenValve, LOW);
  digitalWrite(pinLidCloseValve, HIGH);
  while(isLidClosed() == false){delay(10);}
  digitalWrite(pinLidCloseValve, LOW);
  
  delay(5000); // delay for tank recharging before next trigger
  
  ChangeStatus(lidclosed);
}

void OpenLid()
{
  ChangeStatus(lidopening);
  digitalWrite(pinLidOpenValve, HIGH);
  while(isLidOpen() == false){delay(10);}
  ChangeStatus(lidopen);
  
}

void PlayMusic()
{
  ChangeStatus(musicplaying);
  digitalWrite(pinCrank, HIGH);
  digitalWrite(pinSound1, HIGH);
  delay(ELKms); 
  digitalWrite(pinSound1, LOW);
  
  delay(Sound1ms-ELKms); 
  
  digitalWrite(pinCrank, LOW);
  ChangeStatus(musicfinished);
  
  if(isPropTriggered() == false){ChangeStatus(lidclosed);} //early abort
}

void ChangeStatus(int newstatus)
{
  state = newstatus;
  if(diag == 1){Serial.print("State changed to ");Serial.println(state);DisplayStatus();} 

}

void SetupProp()
{
  mouthServo.attach(pinMouthServo);
  mouthServo.writeMicroseconds(mouthclosedUS);
  
  pinMode(pinSpot, OUTPUT);
  pinMode(pinCrank, OUTPUT);
  pinMode(pinSound1, OUTPUT);
  pinMode(pinSound2, OUTPUT);
  pinMode(pinLidOpenValve, OUTPUT);
  pinMode(pinLidCloseValve, OUTPUT);
  pinMode(pinHeadRaiseValve, OUTPUT);
  pinMode(pinHeadLowerValve, OUTPUT); 
  
  digitalWrite(pinSpot, LOW);
  digitalWrite(pinCrank, LOW);
  digitalWrite(pinSound1, LOW);
  digitalWrite(pinSound2, LOW);
}

void RecoverProp()
{
  // recovery procedure if power failed, check current positions and initiate closure
  // if head is not all the way down, set status to laughingfinished
  
  if(isLidClosed() != true)
  {
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

boolean isHeadUp()
{
  if (analogRead(pinHeadUp) < AnalogHeadUp + AnalogHeadPrecision && analogRead(pinHeadUp) > AnalogHeadUp - AnalogHeadPrecision)
  {
    return true;
  }
  else
  {
    return false;
  }
}

boolean isHeadDown()
{
  if (analogRead(pinHeadDown) < AnalogHeadDown + AnalogHeadPrecision && analogRead(pinHeadDown) > AnalogHeadDown - AnalogHeadPrecision)
  {
    return true;
  }
  else
  {
    return false;
  }
}

boolean isLidClosed()
{
  if (analogRead(pinLidClosed) < AnalogLidClosed + AnalogLidPrecision && analogRead(pinLidClosed) > AnalogLidClosed - AnalogLidPrecision)
  {
    return true; 
  }
  else
  {
    return false;
  }
}

boolean isLidOpen()
{
  if (analogRead(pinLidOpen) < AnalogLidOpen + AnalogLidPrecision && analogRead(pinLidOpen) > AnalogLidOpen - AnalogLidPrecision)
  {
    return true; 
  }
  else
  {
    return false;
  }
}

boolean isPropTriggered()
{  
  if(analogRead(pinTrigger) < AnalogTrigger + AnalogTriggerPrecision && analogRead(pinTrigger) > AnalogTrigger - AnalogTriggerPrecision)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void DisplayStatus()
{
    Serial.print("State: ");Serial.print(state);
    Serial.print(" Head Down: ");Serial.print(analogRead(pinHeadDown));
    Serial.print(" Head Up: ");Serial.print(analogRead(pinHeadUp));
    Serial.print(" Lid Closed: ");Serial.print(analogRead(pinLidClosed));
    Serial.print(" Lid Open: ");Serial.print(analogRead(pinLidOpen));
    Serial.print(" Trigger: ");Serial.println(analogRead(pinTrigger));
}

