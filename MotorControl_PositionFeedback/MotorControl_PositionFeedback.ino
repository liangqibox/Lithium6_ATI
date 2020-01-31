const int motorPin = 2;
const int limmitPin = 3;
const int polarPin = 6;
const int digiPin = 7;
const int analogPin = A0;
const int settingPinL = A2;
const int settingPinR = A3;
const double scale = 0.592;

bool deBug = false;

const int centerPosition = 1500;
const int period = 5000;
const int fullRotationCycle = 80;
const int measureAverage = 10;
const int adjustResolution = 2;
const int measureResolution = 2;
const int feedBackRotationCycle = 2;

int RatationCycle = fullRotationCycle;
int pulseLengthL = 2000;
int pulseLengthR = 1000;
int setPointL = 2890;
int setPointR = 1200;
int lastRotate = 0;

void setup() {
  analogReadResolution(12);
  pinMode(motorPin, OUTPUT);
  pinMode(limmitPin, INPUT);
  pinMode(polarPin, INPUT);
  pinMode(digiPin, INPUT);
  if(Serial){
    Serial.begin(9600);
    deBug = true;
  }
  setPointL = analogRead(settingPinL);
  setPointR = analogRead(settingPinR);
}

void loop() {
  int counter = 0;
  int state = readState(&counter);
  int pulseLength = centerPosition;
  RatationCycle = fullRotationCycle;
  while (state > 0 && counter < RatationCycle) {
    if (state == 1) pulseLength = pulseLengthL;
    else pulseLength = pulseLengthR;
    digitalWrite(motorPin, HIGH);
    delayMicroseconds(pulseLength); 
    digitalWrite(motorPin, LOW);
    delayMicroseconds(period - pulseLength);
    counter++;
    state = readState(&counter);
  }

  for(int i=0; i<feedBackRotationCycle; i++){
    feedBack(lastRotate);
    if (state == 1) pulseLength = pulseLengthL;
    else pulseLength = pulseLengthR;
    digitalWrite(motorPin, HIGH);
    delayMicroseconds(pulseLength);
    digitalWrite(motorPin, LOW);
    delayMicroseconds(period - pulseLength);
  }
  
  while(state==readState(0)){
    adjustment();
    delayMicroseconds(5000);
  }
}

int readState(int *counter) {
  int state = 0;
  bool rotationL = false;
  if (digitalRead(digiPin) == HIGH)rotationL = true;
  if (digitalRead(polarPin) == HIGH)rotationL = !rotationL;
  if (rotationL)state = 1;
  else state = 2;
  if(lastRotate!=state){
    counter = 0;
    RatationCycle = fullRotationCycle;
  }
  lastRotate = state;
  return state;
}

void adjustment(){
  int spL = analogRead(settingPinL);
  int spR = analogRead(settingPinR);
  int ajL = abs(spL-setPointL);
  int ajR = abs(spR-setPointR);
  if(ajL>adjustResolution) setPointL = spL;
  if(ajR>adjustResolution) setPointR = spR;
}

void feedBack(int rotate){
  double measurePoint = 0;
  double setPoint = 0;
  if(rotate==1)setPoint = setPointL;
  else setPoint = setPointR;
  for(int i=0; i<measureAverage; i++){
    measurePoint += analogRead(analogPin);
    delayMicroseconds(500);
  }
  measurePoint = measurePoint/measureAverage;
  double correction = (setPoint - measurePoint)*scale;
  if(rotate==1) pulseLengthL += (int)correction;
  else pulseLengthR += (int)correction;
  if(deBug){
    Serial.print(" Rotate:");
    Serial.print(rotate);
    Serial.print(" Measure:");
    Serial.print(measurePoint);
    Serial.print(" SetPoint:");
    Serial.print(setPoint);
    Serial.print(" L:");
    Serial.print(pulseLengthL);
    Serial.print(" R:");
    Serial.print(pulseLengthR);
    Serial.print(" Correction:");
    Serial.print(correction);
    Serial.print("\n");
  }
}
