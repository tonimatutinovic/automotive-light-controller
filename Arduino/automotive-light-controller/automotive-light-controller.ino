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

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  pinMode(HEADLIGHT_PIN, OUTPUT);

  Serial.println("READY");
}

void modeOff() {
  // Off mode logic
}

void modeAuto() {
  // Auto mode logic
}

void modeOn() {
  // On mode logic
}

void loop() {
  potVal = analogRead(POT_PIN);

  // Basic mode selection (example ranges)
  if (potVal < 341) {
    modeOff();
  } else if (potVal < 682) {
    modeAuto();
  } else {
    modeOn();
  }

  delay(dt);
}
