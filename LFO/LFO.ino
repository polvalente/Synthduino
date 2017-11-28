//LFO - 0Hz to 20Hz
#include "Timer.h"

#define MAX_FREQUENCY 60

int f1, f2, vc1, vc2, lfo1_id, lfo2_id;

int lfo1 = 2;
int lfo2 = 3;

Timer timer1, timer2;

void setup() {
  // put your setup code here, to run once:
  
  pinMode(lfo1, OUTPUT);
  pinMode(lfo2, OUTPUT);

 
  f1 = 1;
  f2 = 1;
  
  lfo1_id = timer1.oscillate(lfo1,1000/(f1),LOW);
  lfo2_id = timer2.oscillate(lfo2,1000/(f2),LOW);

  timer1.every(1000, setFrequency1);
  timer2.every(1000, setFrequency2);

  Serial.begin(115200);
}

void setFrequency1() {
  vc1 = analogRead(0);

  if(vc1 < 15) vc1 = 0;
  else if(vc1 > 1000) vc1 = 1000;
  
  f1 = map(vc1, 0, 1000, 0, MAX_FREQUENCY); 
  
  timer1.stop(lfo1_id);
  lfo1_id = timer1.oscillate(lfo1,1000/(f1),LOW);
  Serial.println(f1);
}

void setFrequency2() {
  vc2 = analogRead(1);

  if(vc2 < 15) vc2 = 0;
  else if(vc2 > 1000) vc2 = 1000;
  
  f2 = map(vc2, 0, 1000, 0, MAX_FREQUENCY); 
  
  timer2.stop(lfo2_id);
  lfo2_id = timer2.oscillate(lfo2,1000/(f2),LOW);
  Serial.println(f2);
}




void loop() {
  timer1.update();
  timer2.update();
}
