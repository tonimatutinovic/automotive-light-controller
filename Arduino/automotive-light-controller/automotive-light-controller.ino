// Pins
const int POT_PIN = A0;
const int DASH_LDR_PIN = A1;
const int BACK_LDR_PIN = A2;

const int LED_RED_PIN = 11;
const int LED_GREEN_PIN = 10;
const int LED_BLUE_PIN = 9;

const int HEADLIGHT_PIN = 2;

// Sensor Values
int potVal = 0;
int dashLDRval = 0;
int backLDRval = 0;

// Threshold
const int Threshold = 650;

// Delay time between readings
const unsigned int dt = 250;

void setup() {
  Serial.begin(115200);
}

void loop() {
  
}
