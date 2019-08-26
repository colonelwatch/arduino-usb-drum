void setup() {
  // put your setup code here, to run once:
  pinMode(A0,INPUT);
  pinMode(A2,INPUT);
  Serial.begin(115200);
}

void loop() {
  // How to calibrate: (1) Record the value after it settles with no pressure, (2) hold the plates down with modest pressure, then (3) record the new value
  //                   The difference is what should be entered for DRUM_###_MAX
  Serial.print(analogRead(A0));   // Drum one
  Serial.print(' ');
  Serial.println(analogRead(A2)); // Drum two
}
