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

// Fault detection
const int LDR_MIN = 20; // LDR reading below this value indicates sensor fault
const int LDR_MAX = 1003; // LDR reading above than this value indicates sensor fault
const unsigned long SENSOR_FAULT_DELAY = 5000; // 5 seconds delay for checking sensor fault
unsigned long dashFaultStart = 0;
unsigned long backFaultStart = 0;
bool dashFault = false;
bool backFault = false;

// Error value
String error = "NO";


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

  // CHECK DASH LDR
  if(dashLDRval < LDR_MIN || dashLDRval > LDR_MAX){
    if(dashFaultStart == 0){
      dashFaultStart = millis();  // Start timer
    }
    else if(millis() - dashFaultStart >= SENSOR_FAULT_DELAY){
      dashFault = true; // If time passed from last valid dash LDR reading is longer than 5 seconds, sensor fault is detected
    }
  }
  else{
    dashFault = false;
    dashFaultStart = 0;
  }
  // CHECK BACK LDR
  if(backLDRval < LDR_MIN || backLDRval > LDR_MAX){
      if(backFaultStart == 0) 
        backFaultStart = millis();
      else if(millis() - backFaultStart >= SENSOR_FAULT_DELAY) 
        backFault = true; // If time passed from last valid back LDR reading is longer than 5 seconds, sensor fault is detected
  }else{
      backFaultStart = 0;
      backFault = false;
  }

  // ERROR HANDING
  // Both sensors not active - system not working
  if(dashFault && backFault){
    ldrLightsOn = true; // Turning lights on because of safety reasons
    error = "CRITICAL_FAULT"; // error status for Serial and  VPython
  }
  // One active - system works based on one LDR, shows error message for inactive LDR
  else if(dashFault)
    error = "DASH_FAULT";
  else if(backFault) 
    error = "BACK_FAULT";
  // Both active - system works
  else
    error = "NO";

  bool dashActive = !dashFault && (dashLDRval > Threshold);
  bool backActive = !backFault && (backLDRval > Threshold);

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
  Serial.print(auto_mode);
  Serial.print(" ");
  Serial.println(error);
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
