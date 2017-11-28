/*

    This sketch implements an 8-bit synthesizer with the help of the following libraries:
    
    Arduino MIDI library v4.2 
    (https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2)
  
    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users
  
    Paulo Valente, 2017
*/

/* 
 * HARDWARE CONNECTIONS
 * RX1, TX1, GND, +5V == RX, TX, GND, +5V MIDI SHIELD
 * PORT C - AUDIO OUTPUT
 * 
 */

// ---------------------------------------
// INCLUDES AND CLASSES
// ---------------------------------------

#include <MIDI.h>

#include <MozziGuts.h>

#include <Oscil.h> // oscillator template
#include <tables/saw2048_int8.h> // sawtooth table for oscillator
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/triangle2048_int8.h> // triangle table for oscillator
#include <tables/square_no_alias_2048_int8.h> // square table for oscillator

#include <mozzi_midi.h>

#define USE_ADSR 1
// use #define for CONTROL_RATE, not a constant

#ifndef USE_ADSR
#define MAX_NOTES 8
#define CONTROL_RATE 256 // powers of 2 please
#else
#define MAX_NOTES 4
#define CONTROL_RATE 256 // powers of 2 please
#endif

class Note
{
  public:
  
  byte n;
  byte ch;
  bool state; // false: off; true: on
  byte v;

  Note(){
    state = false;
    n = 0;
    v = 0;
  }
  
  void on(byte ch, byte n, byte v){
    state = true;
    this->ch = ch;
    this->n = n;
    this->v = v;
  }

  void off(){
    state = false;
    #ifndef USE_ADSR
    ch = 0;
    n = 0;
    v = 0;
    #endif
  }
};

// ---------------------------------------
// GLOBAL VARIABLES
// ---------------------------------------

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, sMIDI);

// 0 - sin
// 1 - saw
// 2 - tri
// 3 - square

// audio oscillators
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc0(WAVE_DATA);
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc1(WAVE_DATA);
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc2(WAVE_DATA);
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc3(WAVE_DATA);

#ifndef USE_ADSR
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc4(WAVE_DATA);
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc5(WAVE_DATA);
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc6(WAVE_DATA);
Oscil <WAVE_NUM_CELLS, AUDIO_RATE> osc7(WAVE_DATA);
#endif

Note notes[MAX_NOTES];

// envelope generator
#ifdef USE_ADSR
#include <ADSR.h>
ADSR <CONTROL_RATE, AUDIO_RATE> envelope[4];
#endif

// ---------------------------------------
// MIDI HANDLERS
// ---------------------------------------
byte on_notes = 0;

void HandleNoteOn(byte channel, byte note, byte velocity) { 
  byte i = 9;
  
  on_notes++;
  
  for(i = 0; i < MAX_NOTES; i++){
    if(!notes[i].state){
      notes[i].on(channel, note, velocity);
      break;
    }
  }

  #ifdef USE_ADSR
  byte level = 255*velocity/127;
  #endif
  switch(i){
    case 0:
      osc0.setFreq(mtof(float(note)));
      #ifdef USE_ADSR
      envelope[0].setADLevels(level+1,level);
      envelope[0].noteOn();
      #endif
      break;
    case 1:
      osc1.setFreq(mtof(float(note)));
      #ifdef USE_ADSR
      envelope[1].setADLevels(level+1,level);
      envelope[1].noteOn();
      #endif
      break;
    case 2:
      osc2.setFreq(mtof(float(note)));
      #ifdef USE_ADSR
      envelope[2].setADLevels(level+1,level);
      envelope[2].noteOn();
      #endif
      break;
    case 3:
      osc3.setFreq(mtof(float(note)));
      #ifdef USE_ADSR
      envelope[3].setADLevels(level+1,level);
      envelope[3].noteOn();
      #endif
      break;
    #ifndef USE_ADSR
    case 4:
      osc4.setFreq(mtof(float(note)));
      break;
    case 5:
      osc5.setFreq(mtof(float(note)));
      break;
    case 6:
      osc6.setFreq(mtof(float(note)));

      break;
    case 7:
      osc7.setFreq(mtof(float(note)));
      break;
    #endif
    default:
      break;
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity) { 
  for(byte i=0; i < MAX_NOTES; i++){
    if(notes[i].n == note){
      notes[i].off();
      
      #ifdef USE_ADSR
      envelope[i].noteOff();
      #endif
      break;
    }
  }
}

// ---------------------------------------
// SETUP
// ---------------------------------------
int attackLevel, releaseLevel;
void setup() {

  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  
  attackLevel = analogRead(0);
  if (attackLevel < 20)
    attackLevel = 0;
  else if (attackLevel > 1000)
    attackLevel = 1000;
  attackLevel = map(attackLevel, 0, 1000,20,4000);
  
  releaseLevel = analogRead(1);
  if (releaseLevel < 20)
    releaseLevel = 0;
  else if (releaseLevel > 1000)
    releaseLevel = 1000;
  releaseLevel = map(releaseLevel, 0, 1000,20,4000);

  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  sMIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  sMIDI.setHandleNoteOff(HandleNoteOff);  // Put only the name of the function
  // Initiate MIDI communications, listen to all channels (not needed with Teensy usbMIDI)
  sMIDI.begin(MIDI_CHANNEL_OMNI);  

  #ifdef USE_ADSR
  for(byte i = 0; i < MAX_NOTES; i++){
    envelope[i].setADLevels(255,200);
    envelope[i].setTimes(attackLevel,20,100000,releaseLevel); // 10000 is so the note will sustain 10 seconds unless a noteOff comes
  }
  
  #ifdef DEBUG
  Serial.println(attackLevel);
  Serial.println(releaseLevel);
  #endif//debug
  #endif//use_adsr

  /*Select oscillator type*/
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  byte s1 = digitalRead(2);
  byte s2 = digitalRead(3);
  byte type = 0;  

  if (s1 == HIGH && s2 == HIGH){
    type = 3;//SQUARE
    osc0.setTable(SQUARE_NO_ALIAS_2048_DATA);
    osc1.setTable(SQUARE_NO_ALIAS_2048_DATA);
    osc2.setTable(SQUARE_NO_ALIAS_2048_DATA);
    osc3.setTable(SQUARE_NO_ALIAS_2048_DATA);
    #ifndef USE_ADSR
    osc4.setTable(SQUARE_NO_ALIAS_2048_DATA);
    osc5.setTable(SQUARE_NO_ALIAS_2048_DATA);
    osc6.setTable(SQUARE_NO_ALIAS_2048_DATA);
    osc7.setTable(SQUARE_NO_ALIAS_2048_DATA);
    #endif
  }
  else if (s1 == HIGH && s2 == LOW){
    type = 2;//TRI
    osc0.setTable(TRIANGLE2048_DATA);
    osc1.setTable(TRIANGLE2048_DATA);
    osc2.setTable(TRIANGLE2048_DATA);
    osc3.setTable(TRIANGLE2048_DATA);
    #ifndef USE_ADSR
    osc4.setTable(TRIANGLE2048_DATA);
    osc5.setTable(TRIANGLE2048_DATA);
    osc6.setTable(TRIANGLE2048_DATA);
    osc7.setTable(TRIANGLE2048_DATA);
    #endif
  }
  else if (s1 == LOW && s2 == HIGH){
    type = 1;//SAW
    osc0.setTable(SAW2048_DATA);
    osc1.setTable(SAW2048_DATA);
    osc2.setTable(SAW2048_DATA);
    osc3.setTable(SAW2048_DATA);
    #ifndef USE_ADSR
    osc4.setTable(SAW2048_DATA);
    osc5.setTable(SAW2048_DATA);
    osc6.setTable(SAW2048_DATA);
    osc7.setTable(SAW2048_DATA);
    #endif
  }
  else{
    type = 0;//SINE
    osc0.setTable(SIN2048_DATA);
    osc1.setTable(SIN2048_DATA);
    osc2.setTable(SIN2048_DATA);
    osc3.setTable(SIN2048_DATA);
    #ifndef USE_ADSR
    osc4.setTable(SIN2048_DATA);
    osc5.setTable(SIN2048_DATA);
    osc6.setTable(SIN2048_DATA);
    osc7.setTable(SIN2048_DATA);
    #endif
  }

 
  osc0.setFreq(440); // default frequency
  osc1.setFreq(440); // default frequency
  osc2.setFreq(440); // default frequency
  osc3.setFreq(440); // default frequency
  #ifndef USE_ADSR
  osc4.setFreq(440); // default frequency
  osc5.setFreq(440); // default frequency
  osc6.setFreq(440); // default frequency
  osc7.setFreq(440); // default frequency
  #endif

  startMozzi(CONTROL_RATE); 
}

// ---------------------------------------
// MAIN AUDIO CONTROLS
// ---------------------------------------

void updateControl(){
  sMIDI.read();

  #ifdef USE_ADSR
  envelope[0].update();
  envelope[1].update();
  envelope[2].update();
  envelope[3].update();
  #endif
}


int updateAudio(){
  int s = 0;

  #ifdef USE_ADSR
  s += (envelope[0].next() * osc0.next())>>9;
  s += (envelope[1].next() * osc1.next())>>9; 
  s += (envelope[2].next() * osc2.next())>>9;
  s += (envelope[3].next() * osc3.next())>>9;
  #else 
  //need to add back velocity
  s += ((notes[0].state) /* notes[0].v */* osc0.next())/(8);
  s += ((notes[1].state) /* notes[1].v */* osc1.next())/(8);
  s += ((notes[2].state) /* notes[2].v */* osc2.next())/(8);
  s += ((notes[3].state) /* notes[3].v */* osc3.next())/(8);
  s += ((notes[4].state) /* notes[4].v */* osc4.next())/(8);
  s += ((notes[5].state) /* notes[5].v */* osc5.next())/(8);
  s += ((notes[6].state) /* notes[6].v */* osc6.next())/(8);
  s += ((notes[7].state) /* notes[7].v */* osc7.next())/(8);
  #endif

  return s;
}

// ---------------------------------------
// MAIN LOOP
// ---------------------------------------

void loop() {
  audioHook(); // required here
} 



