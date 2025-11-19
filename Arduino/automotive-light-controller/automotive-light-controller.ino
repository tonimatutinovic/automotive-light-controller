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

// Work mode based on potentiometer value
int OFFstart = 0, OFFend = 341;
int AUTOstart = 342, AUTOend = 682;
int ONstart = 683, ONend = 1023;

// Threshold
const int Threshold = 650;

// Lights mode in AUTO work mode (ON/OFF)
String auto_mode;

// Delay time between readings
const unsigned int dt = 250;

// Debounce (tunnel logic)
unsigned long ldrChangeStart = 0;
bool ldrLightsOn = false;
const unsigned long CHANGE_DELAY = 2000;

// Function for updating LED
void UpdateLED(int r, int g, int b){
  analogWrite(LED_RED_PIN, r);
  analogWrite(LED_GREEN_PIN, g);
  analogWrite(LED_BLUE_PIN, b);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  pinMode(HEADLIGHT_PIN, OUTPUT);

  Serial.println("READY");
}

void modeOff() {
  UpdateLED(30,30,30); // RGB weak white
  digitalWrite(HEADLIGHT_PIN, LOW); // Lights off
  Serial.println("OFF MODE");
}

void modeAuto() {
  // Reading sensor values
  dashLDRval = analogRead(DASH_LDR_PIN);
  backLDRval = analogRead(BACK_LDR_PIN);

  bool dashActive = (dashLDRval > Threshold);
  bool backActive = (backLDRval > Threshold);

  // Threshold logic
  if (dashActive || backActive){
    if(ldrChangeStart == 0){
      ldrChangeStart = millis();
    }
    else if(millis() - ldrChangeStart >= CHANGE_DELAY){
      ldrLightsOn = true;
      auto_mode = "AUTO_ON";
    }
  }
  else{
    ldrLightsOn = false;
    ldrChangeStart = 0;
    auto_mode = "AUTO_OFF";
  }

  // Update Headlight
  digitalWrite(HEADLIGHT_PIN, ldrLightsOn ? HIGH : LOW);

  // Orange RGB
  UpdateLED(255, 120, 0);

  // Serial info
  Serial.print("AUTO ");
  Serial.print("dashLDR:");
  Serial.print(dashLDRval);
  Serial.print(" backLDR:");
  Serial.print(backLDRval);
  Serial.print(" ");
  Serial.println(auto_mode);
}

void modeOn() {
  UpdateLED(0,255,0); // RGB green
  digitalWrite(HEADLIGHT_PIN, HIGH); // Lights on
  Serial.println("ON MODE");
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
