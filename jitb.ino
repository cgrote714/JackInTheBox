#include <Servo.h>

// Mouth Servo configuration
const int pinMouthServo = 11;
const int mouthclosedUS = 1000;
const int mouthopenUS = 400;
Servo mouthServo;

// Output pin assignments
const int pinSpot = 4;
const int pinSound1 = 3;
const int pinSound2 = 5;
const int pinCrank = 6;
const int pinLidOpenValve = 9;
const int pinLidCloseValve = 7;
const int pinHeadRaiseValve = 10;
const int pinHeadLowerValve = 8;

// Input pin assignments
// Note: Down and Up for same cylinder should be different pins to prevent driftover triggering
const int pinTrigger = 3;
const int pinLidClosed = 2;
const int pinLidOpen = 1;
const int pinHeadDown = 0; 
const int pinHeadUp = 0; 

// Analog ranges for position sensors.  
// Zero = no position reading - not near mag reed
// Important! Precision must be less than half of difference between high and 
// low thresholds if down and up are read from same pin.
const int AnalogTriggerOff = 0;
const int AnalogTriggerOn = 767;
const int AnalogTriggerPrecision = 10;
const int AnalogHeadDown = 696;
const int AnalogHeadUp = 733;
const int AnalogHeadPrecision = 10;
const int AnalogLidOpen = 510;
const int AnalogLidClosed = 608;
const int AnalogLidPrecision = 5;

// when to turn UV spots off
const int beforeHeadDown = 1; //might be best so visitors don't see prop while it resets
const int afterLidClosed = 10;
const int afterTriggerOff = 100;
const int lightsoff = beforeHeadDown;

// Status values
const int startup = 0;
const int idle = 5;
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
int state = startup; //don't start in idle state - make sure trigger is false
int diag = 0; //set to 1 to enable diagnostic mode
const int ELKms = 500; // ELK-120 only needs a momentary contact
const int Sound1ms = 3528; // Duration of Sound #1

void setup()
{
  SetupProp();  
  
  delay(6000); //don't rush power up
  // this allows insteon to be turned on for linking with remotes
  // diag mode can be enabled by leaving insteon in ON position while powering up prop.
  if(isTriggerOn()==true){diag=1;}
  
  if(diag == 1)
  {
    Serial.begin(9600);
    DisplayStatus(); 
  }

  RecoverProp();

}

void loop()
{
  proploop();
}

void proploop()
{
  if(state == idle && isTriggerOn() == true){StartProp();}
  if(diag == 1){DisplayStatus();delay(100);}
  if(state == triggered){PlayMusic();}
  if(state == musicfinished){OpenLid();}
  if(state == lidopen){RaiseHead();}
  if(state == headraised){StartLaugh();}
  if(state == laughingfinished){LowerHead();}
  if(state == headlowered){CloseLid();}
  
  // don't go back to idle state until trigger turned off, otherwise will loop continuously.
  if(state == lidclosed && isTriggerOff() == true){EndProp();} 
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
  
  ChangeStatus(triggered);
}

void EndProp()
{
  digitalWrite(pinSpot, LOW); // end prop always turns off the light.
  ChangeStatus(idle);
}

void LowerHead()
{
  if(lightsoff == beforeHeadDown){digitalWrite(pinSpot, LOW);}
  ChangeStatus(headlowering);
  digitalWrite(pinHeadRaiseValve, LOW); 
  digitalWrite(pinHeadLowerValve, HIGH); 
  while(isHeadDown() == false){DisplayStatus();} 
  digitalWrite(pinHeadLowerValve, LOW); 
  ChangeStatus(headlowered);
}

void RaiseHead()
{
  ChangeStatus(headrising);
  digitalWrite(pinHeadRaiseValve, HIGH); 
  while(isHeadUp() == false){DisplayStatus();}
  ChangeStatus(headraised);
}

void CloseLid()
{
  ChangeStatus(lidclosing);
  digitalWrite(pinLidOpenValve, LOW);
  digitalWrite(pinLidCloseValve, HIGH);
  while(isLidClosed() == false){DisplayStatus();}
  delay(200); 
  digitalWrite(pinLidCloseValve, LOW);
  if(lightsoff == afterLidClosed){digitalWrite(pinSpot, LOW);}
  
  delay(5000); // delay for tank recharging before next trigger
  
  ChangeStatus(lidclosed);
}

void OpenLid()
{
  ChangeStatus(lidopening);
  
  //pulse lid valve to absorb momentum and prevent harsh flapping
  digitalWrite(pinLidOpenValve, HIGH);
  delay(700);
  digitalWrite(pinLidOpenValve, LOW);
  delay(300);
  digitalWrite(pinLidOpenValve,HIGH);

  while(isLidOpen() == false){DisplayStatus();}
  delay(1000);
  ChangeStatus(lidopen);
}

void PlayMusic()
{
  ChangeStatus(musicplaying);
  digitalWrite(pinSound1, HIGH);
  delay(ELKms); 
  digitalWrite(pinSound1, LOW);
  
  //give sound a chance to start before turning on light and starting crank
  digitalWrite(pinSpot, HIGH);
  digitalWrite(pinCrank, HIGH); 
  
  //wait for sound to finish playing
  delay(Sound1ms-ELKms); 
  
  digitalWrite(pinCrank, LOW);
  ChangeStatus(musicfinished);
  
  if(isTriggerOff() == true){ChangeStatus(lidclosed);} //early abort
}

void ChangeStatus(int newstatus)
{
  state = newstatus;
  
  //Update LCD line 1
  //if(state == idle){Serial.print("Idle");}
  //etc.
  
  //Clear LCD line 2
  
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
  //int pa = aaread(pinHeadUp,5,10); //pin average
  //if (pa < AnalogHeadUp + AnalogHeadPrecision && pa > AnalogHeadUp - AnalogHeadPrecision)
  if(StableRead(pinHeadUp, AnalogHeadUp - AnalogHeadPrecision, AnalogHeadUp + AnalogHeadPrecision,
    10, 5))
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
  //int pa = aaread(pinHeadDown,5,10); //pin average
  //if (pa < AnalogHeadDown + AnalogHeadPrecision && pa > AnalogHeadDown - AnalogHeadPrecision)
  if(StableRead(pinHeadDown, AnalogHeadDown - AnalogHeadPrecision, AnalogHeadDown + AnalogHeadPrecision,
    10, 5))
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
  //int pa = aaread(pinLidClosed,10,10); //pin average
  //if (pa < AnalogLidClosed + AnalogLidPrecision && pa > AnalogLidClosed - AnalogLidPrecision)
  if(StableRead(pinLidClosed, AnalogLidClosed - AnalogLidPrecision, AnalogLidClosed + AnalogLidPrecision,
    10, 5))
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
  //int pa = aaread(pinLidOpen,5,10); //pin average
  //if (pa < AnalogLidOpen + AnalogLidPrecision && pa > AnalogLidOpen - AnalogLidPrecision)
  if(StableRead(pinLidOpen, AnalogLidOpen - AnalogLidPrecision, AnalogLidOpen + AnalogLidPrecision,
    10, 5))
  {
    return true; 
  }
  else
  {
    return false;
  }
}

boolean isTriggerOn()
{ 
  //int pa = aaread(pinTrigger,5,5); //pin average
  //if(pa < AnalogTriggerOn + AnalogTriggerPrecision && pa > AnalogTriggerOn - AnalogTriggerPrecision)
  if(StableRead(pinTrigger, AnalogTriggerOn - AnalogTriggerPrecision, AnalogTriggerOn + AnalogTriggerPrecision,
    5, 5))
  {
    return true;
  }
  else
  {
    return false;
  }
}

boolean isTriggerOff()
{ 
  //int pa = aaread(pinTrigger,5,5); //pin average
  //if(pa < AnalogTriggerOff + AnalogTriggerPrecision && pa > AnalogTriggerOff - AnalogTriggerPrecision)
  if(StableRead(pinTrigger, AnalogTriggerOff - AnalogTriggerPrecision, AnalogTriggerOff + AnalogTriggerPrecision,
    10, 5))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Wait(int msDelay)
{
  int time_start = millis();
  // Update LCD Line 2
  // Delay XXXXX ms
  // 1234567890123456
  while(millis() < time_start + msDelay)
  {
    // Update LCD line 2, position 7-11 with countdown?
    delay(10); //LCD refresh rate 
  }  
}

void DisplayStatus()
{
  if(diag == 1)
  {
    Serial.print("State: ");Serial.print(state);
    Serial.print(" Head Down: ");Serial.print(analogRead(pinHeadDown));
    Serial.print(" Head Up: ");Serial.print(analogRead(pinHeadUp));
    Serial.print(" Lid Closed: ");Serial.print(analogRead(pinLidClosed));
    Serial.print(" Lid Open: ");Serial.print(analogRead(pinLidOpen));
    Serial.print(" Trigger: ");Serial.println(analogRead(pinTrigger));
    //delay(100);
  }
}

int aaread(int aPin, int nTimes, int msDelay)
{
  //returns analog pin average reading
  //reads pin nTimes with msDelay between each reading
  
  int pa = analogRead(aPin); 
  delay(msDelay);
  
  for (int i=2; i <= nTimes; i++)
  {
    pa += analogRead(aPin);
    delay(msDelay);
  }

  return pa / nTimes;
}

boolean StableRead(int aPin, int iFrom, int iTo, int nTimes, int msDelay)
{
  //ensures sensor reading is consistent nTimes with msDelay retest delay
  
  // update LCD line 2 with what read is looking for 
  // Wait XXX<YYY<ZZZ
  // 1234567890123456
  int pa = 0; 
  if(analogRead(aPin) <= iTo && analogRead(aPin) >= iFrom){pa++;}
  delay(msDelay);
  
  for (int i=2; i <= nTimes; i++)
  {
    if(analogRead(aPin) <= iTo && analogRead(aPin) >= iFrom){pa++;}
    // update LCD line 2
    delay(msDelay);
  }

  if(pa == nTimes)
  {
    return true;
  }
  else
  {
    return false;
  }
}

