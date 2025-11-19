#include <Wire.h>
#include <RTClib.h>

// RTC
RTC_DS3231 rtc;

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

// Time
DateTime now;
DateTime nightTime, sunriseTime;
bool SimulatedTime = false;

// Functions for getting sunset and sunrise time
DateTime getNightTime(DateTime current);
DateTime getSunriseTime(DateTime current);

// Time simulation for testing
DateTime getSimulatedTime();

// Function for updating LED
void UpdateLED(int r, int g, int b){
  analogWrite(LED_RED_PIN, r);
  analogWrite(LED_GREEN_PIN, g);
  analogWrite(LED_BLUE_PIN, b);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting main program...");

  // RTC setup
  if(!rtc.begin()){
    Serial.println("RTC not found!");
    while(1);
  }

  if(rtc.lostPower()){
    Serial.println("RTC lost power, setting to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  pinMode(HEADLIGHT_PIN, OUTPUT);

  if(SimulatedTime)
    now = getSimulatedTime(); // now is simulated time (for testing purposes)
  else
    now = rtc.now();
  // Getting sunset and sunrise time based on time given
  nightTime = getNightTime(now);
  sunriseTime = getSunriseTime(now);

  // Signal indicating that setup process is completed
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
  else {
    if(dashFault)
      error = "DASH_FAULT";
    else if(backFault) 
      error = "BACK_FAULT";
    // Both active - system works
    else
      error = "NO";

    bool dashActive = !dashFault && (dashLDRval > Threshold);
    bool backActive = !backFault && (backLDRval > Threshold);

    // CHECK TIME - night is after sunset until sunrise
    bool isNight = (now >= nightTime || now <= sunriseTime);

    // NIGHT
    if(isNight){
        ldrLightsOn = true; // Lights on
        auto_mode = "AUTO_ON";
    }
    // DAY
    else{
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
    }
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
  // Updating current time
  if(SimulatedTime)
    now = getSimulatedTime();
  else
    now = rtc.now();

  // Reading potentiometer value
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

// Function for getting sunset time for date given
DateTime getNightTime(DateTime current){
  int year = current.year();
  DateTime winterSolstice(year,12,21,16,3,0); // winter solstice sunset
  DateTime summerSolstice(year,6,21,20,39,0); // summer solstice sunset

  // If time given is after winter solstice, winter solstice was year before
  if(current < summerSolstice && current.month() < 6)
    winterSolstice = DateTime(year-1,12,21,16,3,0);

  TimeSpan total = summerSolstice - winterSolstice;
  int totalDays = total.days(); // Number of days between summer and winter solstice

  long sunsetMinutes;
  int winterMinutes = 16*60 + 3;
  int summerMinutes = 20*60 + 39;

  // Time given is between winter solstice and summer solstice
  if(current >= winterSolstice && current <= summerSolstice){
    TimeSpan diff = current - winterSolstice; // Calculating time passed from winter solstice
    int daysPassed = diff.days(); // Number of days passed from winter solstice
    sunsetMinutes = winterMinutes + daysPassed*2; // Getting minutes from winter solstice sunset + days passed multiplied by average daily shift (2 minutes)
  }
  // Time given is between summer solstice and winter solstice
  else{
    TimeSpan diff = current - summerSolstice; // Calculating time passed from summer solstice
    int daysPassed = diff.days(); // Number of days passed from summer solstice
    sunsetMinutes = summerMinutes - daysPassed*2; // Getting minutes from summer solstice sunset - days passed multiplied by average daily shift (2 minutes)
  }

  // Ensuring that sunsetMinutes stays in realistic bounds
  if(sunsetMinutes > summerMinutes) sunsetMinutes = summerMinutes;  // latest possible sunset
  if(sunsetMinutes < winterMinutes) sunsetMinutes = winterMinutes;  // earliest possible sunset

  int hour = sunsetMinutes/60;  // sunset hour
  int minute = sunsetMinutes%60;  // sunset minute
  return DateTime(year,current.month(),current.day(),hour,minute,0);  // returning DateTime of sunset
}

// Function for getting sunrise time for date given
DateTime getSunriseTime(DateTime current){
  int year = current.year();
  DateTime winterSolstice(year,12,21,6,27,0); // winter solstice sunrise
  DateTime summerSolstice(year,6,21,5,13,0);  // summer solstice sunrise

  // If time given is after winter solstice, winter solstice was year before
  if(current < summerSolstice && current.month() < 6)
    winterSolstice = DateTime(year-1,12,21,6,27,0);

  TimeSpan total = summerSolstice - winterSolstice;
  int totalDays = total.days(); // Number of days between summer and winter solstice

  long sunriseMinutes;
  int winterMinutes = 6*60 + 27;
  int summerMinutes = 5*60 + 13;

  // Time given is between winter solstice and summer solstice
  if(current >= winterSolstice && current <= summerSolstice){
    TimeSpan diff = current - winterSolstice; // Calculating time passed from winter solstice
    int daysPassed = diff.days(); // Number of days passed from winter solstice
    sunriseMinutes = winterMinutes - daysPassed*2;  // Getting minutes from winter solstice sunrise - days passed multiplied by average daily shift (2 minutes)
  }
  // Time given is between summer solstice and winter solstice
  else{
    TimeSpan diff = current - summerSolstice; // Calculating time passed from summer solstice
    int daysPassed = diff.days(); // Number of days passed from summer solstice
    sunriseMinutes = summerMinutes + daysPassed*2;  // Getting minutes from summer solstice sunrise + days passed multiplied by average daily shift (2 minutes)
  }

  // Ensuring that sunriseMinutes stays in realistic bounds
  if(sunriseMinutes < summerMinutes) sunriseMinutes = summerMinutes;  // earliest possible sunrise
  if(sunriseMinutes > winterMinutes) sunriseMinutes = winterMinutes;  // latest possible sunrise

  int hour = sunriseMinutes/60; // sunrise hour
  int minute = sunriseMinutes%60; // sunrise minute
  return DateTime(year,current.month(),current.day(),hour,minute,0);  // returning DateTime of sunrise
}

// Getter for simulated time (for testing)
DateTime getSimulatedTime() {
  DateTime now = rtc.now();
  int hour = 12;
  int minute = 30;
  return DateTime(now.year(), now.month(), now.day(), hour, minute, 0);
}