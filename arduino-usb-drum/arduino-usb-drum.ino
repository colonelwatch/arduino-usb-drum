// User configurable values
#define DEAD_RANGE 2        // minimum deviation from zero point, combats noise (increase to raise cutoff)
#define DEBOUNCE_TIME 50000 // in microseconds, minimum time between notes to eliminate notes caused by ringing
#define MIDI_NOTE 60        // MIDI note number, ranges from 0 - 127
// MUST be configured by the user
#define DRUM_ONE_MAX 80     // Maximum height measured by drum one, found using calibration sketch
#define DRUM_TWO_MAX 200    // Maximum height measured by drum two, found using calibration sketch

// Global constants
const int sensitivity1 = DRUM_ONE_MAX / 10.;
const int sensitivity2 = DRUM_TWO_MAX / 10.;

// Global variables
int avg, avg2;                                              // Stores value of DC bias
unsigned long hit_timestamp, hit_timestamp2;                // Stores time of hit for debouncing purposes
int buff[10] = {0}, buff2[10] = {0}, iBuff = 0, iBuff2 = 0; // Circular buffer of past samples and its index

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);         // Opens serial link with computer for Hairless MIDI to grab (Note: set baud rate in Hairless to 115200)
  
  // Initializes peak detector input pins
  pinMode(A0, INPUT);
  pinMode(A2, INPUT);

  // Throws out initial 20 values before finding average
  for(int i = 0; i < 20; i++){
    analogRead(A0);
    analogRead(A2);
  }

  // Finds average value in order to compensate for DC bias later
  int total = 0, total2 = 0, deriv_total2 = 0;
  for(int i = 0; i < 20; i++){
    total += analogRead(A0);
    total2 += analogRead(A2);
  }
  avg = total/20;
  avg2 = total2/20;

  // Fills buffer before first loop
  hit_timestamp = micros();
  hit_timestamp2 = micros();
  buff[iBuff] = analogRead(A0) - avg;
  iBuff++;
  buff2[iBuff2] = analogRead(A2) - avg2;
  iBuff2++;
  delay(10);
}

void loop() {
  // put your main code here, to run repeatedly:
  int pulse_max2 = 0;    // Stores maximum value of pulse
  
  // Reads from each input while compensating for DC bias
  int val = analogRead(A0) - avg;
  if(abs(val) <= DEAD_RANGE) val = 0;
  int val2 = analogRead(A2) - avg2;
  if(abs(val2) <= DEAD_RANGE) val2 = 0;
  
  // Checks if greatest change is greater than trigger value
  bool trigger = false;
  for(int i = 0; i < 10; i++) if(val > buff[i] + sensitivity1) trigger = true;
  bool trigger2 = false;
  for(int i = 0; i < 10; i++) if(val2 > buff2[i] + sensitivity2) trigger2 = true;

  // Finds peak then sends MIDI note
  if(trigger){
    int pulse_max = 0;    // Stores maximum value of pulse
    hit_timestamp = micros();
    while(micros() < hit_timestamp + DEBOUNCE_TIME){   // Holds uC in recording stage until debounce time passes
      val = analogRead(A0) - avg;
      if(abs(val) <= DEAD_RANGE) val = 0;
      if(val > pulse_max) pulse_max = val;    // Raises maximum value if it is exceeded
      
      // Adds reading to buffer 1
      if(iBuff < 10){
        buff[iBuff] = val;
        iBuff++;
      }
      else{
        iBuff = 0;
        buff[iBuff] = val;
      }
    }
    if(pulse_max > DRUM_ONE_MAX) pulse_max = DRUM_ONE_MAX;  // Caps maximum value to prevent overflow

    // MIDI output to Serial
    Serial.write(0b10010000);                             // Sends inital NOTEON byte in MIDI
    Serial.write(MIDI_NOTE);                              // Sends user-defined MIDI note
    Serial.write(map(pulse_max, 0, 80, 20, 127));         // Maps maximum value to note velocity then sends
  }
  else{
    // Adds reading to buffer 1
    if(iBuff < 10){
      buff[iBuff] = val;
      iBuff++;
    }
    else{
      iBuff = 0;
      buff[iBuff] = val;
    }
  }

  // Same as above, but with different variables for drum 2
  if(trigger2){
    int pulse_max = 0;
    hit_timestamp2 = micros();
    while(micros() < hit_timestamp2 + DEBOUNCE_TIME){
      val2 = analogRead(A2) - avg2;
      if(abs(val2) <= DEAD_RANGE) val2 = 0;
      if(val2 > pulse_max) pulse_max = val2;
      
      // Adds reading to buffer 2
      if(iBuff2 < 10){
        buff2[iBuff2] = val2;
        iBuff2++;
      }
      else{
        iBuff2 = 0;
        buff2[iBuff2] = val2;
      }
    }
    if(pulse_max > DRUM_TWO_MAX) pulse_max = DRUM_TWO_MAX;
    Serial.write(0b10010000);
    Serial.write(MIDI_NOTE+1);
    Serial.write(map(pulse_max, 0, 200, 20, 127));
  }
  else{
    // Adds reading to buffer 2
    if(iBuff2 < 10){
      buff2[iBuff2] = val2;
      iBuff2++;
    }
    else{
      iBuff2 = 0;
      buff2[iBuff2] = val2;
    }
  }
}
