/*
 * this is the code I used when I took my bare 3-2-3 frame to R2LA.  
 * My goal here was to make as simple a system as possible with enough safeguards 
 * as to not function, rather than faceplant when something goes wrong. 
 * Only 4 limit switches, up and down for both tilt and center foot.  If it is not in one of those 2 positions , nothing works. 
 * 
 * triggering was accomplished by using the spring loaded joystick  on an rc radio for up and down, with a toggle switch on the radio as a master switch. 
 * 
 * This was written for a Pro Mini. But that was because I had one.  Any arduino will do, 
 */




#include <USBSabertooth.h>


USBSabertoothSerial C;             
USBSabertooth       ST(C, 128);              // Use address 128.



// ======================================================================Timings===================                          
const int LoopTime = 1000;
const int ReadRCInterval= 101;
const int DisplayInterval = 1000;
const int StanceInterval = 100;
const int ShowTimeInterval = 100;
unsigned long currentMillis = 0;      // stores the value of millis() in each iteration of loop()
unsigned long PreviousReadRCMillis = 0;   // 
unsigned long PreviousDisplayMillis = 0; 
unsigned long PreviousStanceMillis = 0;
unsigned long PreviousShowTimeMillis = 0; 
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long ShowTime = 1;


const byte Aux1Pin = A2;    //Input pin from the RC reciever
const byte Aux2Pin = A3;    //Input pin from the RC reciever
const byte TiltUpPin = 6;   //Limit switch input pin, Grounded when closed
const byte TiltDnPin = 7;   //Limit switch input pin, Grounded when closed
const byte LegUpPin = 8;    //Limit switch input pin, Grounded when closed
const byte LegDnPin = 9;    //Limit switch input pin, Grounded when closed

const int ThrNumReadings = 4;         //these 5 lines are smoothing for the RC inputs
int ThrReadings[ThrNumReadings];      // the readings from the analog input
int ThrReadIndex = 0;              // the index of the current reading
int ThrTotal = 0;                  // the running total
int ThrAverage = 0;                // the average
int Aux1 =1500;
int Aux2 =1500;
int TiltUp;
int TiltDn;
int LegUp;
int LegDn;
int Stance;
int StanceTarget;
int LegHappy;
int TiltHappy;
//-----------------------------------------------------------------Setup----------------------------------
void setup(){
pinMode(A2, INPUT);
pinMode(A3, INPUT);
pinMode(6, INPUT_PULLUP);
pinMode(7, INPUT_PULLUP);
pinMode(8, INPUT_PULLUP);
pinMode(9, INPUT_PULLUP);  

  SabertoothTXPinSerial.begin(9600); // 9600 is the default baud rate for Sabertooth Packet Serial.
  Serial.begin(9600);

}

//---------------------------------------------------------Read RC--------------------
void ReadRC() {     //this reads our 6 inputs
     Aux1 = pulseIn(Aux1Pin, HIGH);
 if (Aux1 < 500) {Aux1 = 1500;}
 
   Aux2 = pulseIn(Aux2Pin, HIGH);
 if (Aux2 < 500) {Aux2 = 1500;}

   TiltUp = digitalRead(TiltUpPin);
   TiltDn = digitalRead(TiltDnPin);
   LegUp = digitalRead(LegUpPin);
   LegDn = digitalRead(LegDnPin);

//-----------------------------------------------------------RC Radio master switch
//a toggle switch input from the rc reciever is 1000 when off, and 2000 when on. The following line says that if
// the toggle switch is on (aux1)  AND the joystick (aux 2) is near the ends of the travel will anything be triggered. 
// just safer than a stick that can be bumped
// And even then it is not a command as much as a wish. 

    if ((Aux1 >= 1800)&& (Aux2 <= 1100)){
    StanceTarget = 2;
  }
   if ((Aux1 >= 1800)&& (Aux2 >= 1800)){
    StanceTarget = 1;
   
  }
 

}

//------------------------------------------------Display Values-----------------------
// a bunch of serial display info for debugging
void Display(){
  
  Serial.print("  Aux1  ");
  Serial.print(Aux1);
  Serial.print("  Aux2  ");
  Serial.print(Aux2);
  Serial.print("  Tilt Up  ");
  Serial.print(TiltUp);
  Serial.print("  Tilt Down  ");
  Serial.print(TiltDn); 
   Serial.print("  Leg Up  ");
  Serial.print(LegUp);
  Serial.print("  Leg Down  ");
  Serial.println(LegDn); 
 Serial.print("  Stance  ");
  Serial.print(Stance); 
  Serial.print("  Stance Target  ");
  Serial.print(StanceTarget); 
   Serial.print(" Leg Happy  ");
  Serial.print(LegHappy); 
  Serial.print(" Tilt Happy  ");
  Serial.print(TiltHappy); 
  Serial.print("  Show Time  ");
  Serial.println(ShowTime); 
}
/*
 * Actual movement commands are here,  when we send the command to move leg down, first it checks the leg down limit switch, if it is closed it 
 * stops the motor, sets a flag (happy) and then exits the loop, if it is open the down motor is triggered. 
 * all 4 work the same way
 */
//--------------------------------------------------------------------Move Leg Down---------------------------
void MoveLegDn(){
   LegDn = digitalRead(LegDnPin);
 if(LegDn == 0){
  ST.motor(1, 0);     // Stop. 
  LegHappy = 0;
return;}

 if(LegDn == 1){
  ST.motor(1, 2047);  // Go forward at full power. 
}

} 

//--------------------------------------------------------------------Move Leg Up---------------------------
void MoveLegUp(){
   LegUp = digitalRead(LegUpPin);
 if(LegUp == 0){
  ST.motor(1, 0);     // Stop. 
   LegHappy = 0;
return;}

 if(LegUp == 1){
  ST.motor(1, -2047);  // Go forward at full power. 
}

} 
//--------------------------------------------------------------------Move Tilt down---------------------------
void MoveTiltDn(){
  TiltDn = digitalRead(TiltDnPin);
 if(TiltDn == 0){
  ST.motor(2, 0);     // Stop. 
  TiltHappy = 0;
return;}

 if(TiltDn == 1){
  ST.motor(2, 2047);  // Go forward at full power. 
}

} 
//--------------------------------------------------------------------Move Tilt Up---------------------------
void MoveTiltUp(){
  TiltUp = digitalRead(TiltUpPin);
 if(TiltUp == 0){
  ST.motor(2, 0);     // Stop. 
  TiltHappy = 0;
return;}

 if(TiltUp   == 1){
  ST.motor(2, -2047);  // Go forward at full power. 
}

} 
//----------------------------------------------------------------Two To Three -------------------------------
/*
 * this command to go from two to three, ended up being a combo of tilt down and leg down 
 * with a last second chech each loop on the limit switches
 * timing worked out great, by the time the tilt down needed a center foot, it was there.
 */
void TwoToThree(){
   TiltDn = digitalRead(TiltDnPin);
   LegDn = digitalRead(LegDnPin);

    
Serial.print("  Three Legs  ");
if(LegDn == 0){
  ST.motor(1, 0);
  LegHappy = 0;
  }
if(LegDn == 1){
  ST.motor(1, 2047);  // Go forward at full power. 
}
if(TiltDn == 0){
  ST.motor(2, 0);
  TiltHappy = 0;
  }
if(TiltDn == 1){
  ST.motor(2, 2047);  // Go forward at full power. 
}
}

//----------------------------------------------------------------Three To Two -------------------------------
/*
 * going from three legs to two needed a slight adjustment. I start a timer, called show time, and use it to delay the 
 * center foot from retracting.
 */
void ThreeToTwo(){
   TiltUp = digitalRead(TiltUpPin);
   LegUp = digitalRead(LegUpPin);
   TiltDn = digitalRead(TiltDnPin);
//  First if the center leg is up, do nothing. 
Serial.print("  Two Legs  ");
if(LegUp == 0){
 ST.motor(1, 0);
 LegHappy = 0; 
}
//  If leg up is open AND the timer is in the first 20 steps then lift the center leg at 25 percent speed
if(LegUp == 1 &&  ShowTime >= 1 && ShowTime <= 20){
 ST.motor(1, -500); 
}
//  If leg up is open AND the timer is over 21 steps then lift the center leg at full speed
if(LegUp == 1 && ShowTime >= 21){
 ST.motor(1, -2047); 
}
// at the same time, tilt up till the switch is closed
if(TiltUp == 0){
 ST.motor(2, 0);
 TiltHappy = 0; 
}
if(TiltUp == 1 ){
 ST.motor(2, -2047);  // Go forward at full power. 
}

}

//--------------------------------------------------------------------Check Stance-----------------------
// This is simply taking all of the possibilities of the switch positions and giving them a number. 
//and this loop is only allowerd to run if both my happy flags have been triggered. 
// at any time, including power up, the droid can run a check and come up with a number as to how he is standing. 

void CheckStance(){
  if(LegHappy == 0 && TiltHappy == 0){
 if(LegUp == 0 && LegDn == 1 && TiltUp == 0 && TiltDn == 1 ){
  Stance = 1;
  }
  if(LegUp == 1 && LegDn == 0 && TiltUp == 1 && TiltDn == 0 ){
  Stance = 2;
 }
  if(LegUp == 0 && LegDn == 1 && TiltUp == 1 && TiltDn == 1 ){
  Stance = 3;
 }
  if(LegUp == 1 && LegDn == 1 && TiltUp == 0 && TiltDn == 1 ){
  Stance = 4;
  }
  if(LegUp == 1 && LegDn == 0 && TiltUp == 0 && TiltDn == 1 ){
  Stance = 4;
  }
  if(LegUp == 1 && LegDn == 0 && TiltUp == 1 && TiltDn == 1 ){
  Stance = 5;
  }
  if(LegUp == 1 && LegDn == 1 && TiltUp == 1 && TiltDn == 0 ){
  Stance = 6;
  }
   if(LegUp == 1 && LegDn == 1 && TiltUp == 1 && TiltDn == 1 ){
  Stance = 7;
 
 }}}
//---------------------------------------------------------MOVE-----------------------------------
void Move(){
// there is no stance target 0, so turn off your motors and do nothing. 
if(StanceTarget == 0){
   ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
 }
 // if you are told to go where you are, then do nothing
 if(StanceTarget == Stance){
   ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
 }
 // Stance 7 is bad, all 4 switches open, no idea where anything is.  do nothing. 
 if(Stance == 7){
   ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
 }
 // if you are in three legs and told to go to 2
 if(StanceTarget == 1 && Stance == 2){
   LegHappy = 1;
   TiltHappy = 1;
  ThreeToTwo();
  }
  // This is the first of the slight unknowns, target is two legs,  look up to stance 3, the center leg is up, but the tilt is unknown.
  //You are either standing on two legs, or already in a pile on the ground. Cant hurt to try tilting up. 
 if(StanceTarget == 1 && Stance == 3){
   TiltHappy = 1;
 MoveTiltUp();
  }
  // target two legs, tilt is up, center leg unknown, Can not hurt to try and lift the leg again. 
 if(StanceTarget == 1 && Stance == 4){
   LegHappy = 1;
   MoveLegUp();
  } 
  //Target is two legs, center foot is down, tilt is unknown, too risky do nothing.  
 if(StanceTarget == 1 && Stance == 5){
  ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
  // target is two legs, tilt is down, center leg is unknown,  too risky, do nothing. 
 if(StanceTarget == 1 && Stance == 6){
  ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
  } 
  // target is three legs, stance is two legs, run two to three. 
   if(StanceTarget == 2 && Stance == 1){
   LegHappy = 1;
   TiltHappy = 1;
  TwoToThree();
  } 
  //Target is three legs. center leg is up, tilt is unknown, safer to do nothing, Recover from stance 3 with the up command
   if(StanceTarget == 2 && Stance == 3){
   ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
  }
  // target is three legs, but don't know where the center leg is.   Best to not try this, 
  // recover from stance 4 with the up command, 
  if(StanceTarget == 2 && Stance == 4){
   ST.motor(1, 0);
   ST.motor(2, 0);
   LegHappy = 0;
   TiltHappy = 0;
  return;
  }
  // Target is three legs, the center foot is down, tilt is unknownm. either on 3 legs now, or a smoking mess, 
  // nothing to loose in trying to tilt down again
  if(StanceTarget == 2 && Stance == 5){
  TiltHappy = 1;
  MoveTiltDn();
  }
  // kinda like above, Target is 3 legs, tilt is down, center leg is unknown, ......got nothing to loose. 
    if(StanceTarget == 2 && Stance == 6){
   LegHappy = 1;
   MoveLegDn();
  }
 }
}


void loop(){
  currentMillis = millis();  // this updates the current time each loop
 if(TiltDn == 0){   // when the tilt down switch opens, the timer starts
  ShowTime = 0;
 }
  if (millis() - PreviousReadRCMillis >= ReadRCInterval){
    PreviousReadRCMillis = currentMillis;
  ReadRC();
  }
   if (millis() - PreviousDisplayMillis >= DisplayInterval){
    PreviousDisplayMillis = currentMillis;
  Display();
   }
   if (millis() - PreviousStanceMillis >= StanceInterval){
    PreviousStanceMillis = currentMillis;
 CheckStance();
}
Move();
// the following lines triggers my showtime timer to advance one number every 100ms.
//I find it easier to work with a smaller number, and it is all trial and error anyway. 
 if (millis() - PreviousShowTimeMillis >= ShowTimeInterval){
            PreviousShowTimeMillis = currentMillis;
            ShowTime++;
     }



   }
