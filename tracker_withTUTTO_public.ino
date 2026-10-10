#include <Servo.h>
#include <SoftwareSerial.h>

// PINS
const int LU_PIN = A0;
const int LD_PIN = A1;
const int RD_PIN = A3;
const int RU_PIN = A2;

const int solarPin = A4;
const int MIN_LIGHT_THRESHOLD = 2;

const int H_SERVO_PIN = 9;   // Horizontal
const int V_SERVO_PIN = 10;  // Vertical
bool isNight = false;
bool limitReached = false;

const int RX = 2;
const int TX = 3;
const String SERVER_IP  = "YOUR_SERVER_IP";
const int SERVER_PORT = 8000;

SoftwareSerial esp(RX, TX);
String ssid = "YOUR_WIFI_SSID";
String pass = "YOUR_WIFI_PASSWORD";

Servo horizontalServo;
Servo verticalServo;

//calibrated coeficents 
const float kLU = 1.000;
const float kRU = 2.110;
const float kLD = 1.546;
const float kRD = 0.976;

// initial SERVO POSITIONS
int horizontalAngle = 90;
int verticalAngle = 90; 


// Safe physical limits
const int H_MIN = 0;
const int H_MAX = 180;

const int V_MIN = 0;
const int V_MAX = 90;

// TRACKING PARAMETERS

// Tolerable misdirection
const float H_TOLERANCE = 0.1;
const float V_TOLERANCE = 0.1;

// Movement per correction (in deg)
const int STEP = 1;

// Time between corrections
const int TRACKING_DELAY = 50;

// Timers
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 1000;
unsigned long lastServerTime = 0;
const unsigned long SERVER_INTERVAL = 30000;

//GLOBAL VARIABLES FOR DATA
float globalSolarVoltage = 0.0;
int globalLightPercent = 0;

//HELPER FUNCTION FOR ESP AT COMMANDS
String sendCommand(String cmd, int waitTime) {
  esp.println(cmd);
  delay(waitTime);
  String response = "";
  while (esp.available()) {
    char c = esp.read();
    response += c;
  }
  return response;
}

void setup() {

  Serial.begin(9600);
  esp.begin(9600);

  // Connection to WI-FI
  sendCommand("AT+RST", 2000);
  sendCommand("AT+CWMODE=1", 1000);
  Serial.println("Connecting to Wi-Fi...");
  sendCommand("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"", 6000);
  Serial.println("Wi-Fi connected.");


  horizontalServo.attach(H_SERVO_PIN);
  verticalServo.attach(V_SERVO_PIN);

  // Initial position
  horizontalServo.write(horizontalAngle);
  verticalServo.write(verticalAngle);

  delay(3000);

  Serial.println("Solar tracker started.");
}

void loop() {
  // 1. Read values
  int rawLU = analogRead(LU_PIN);
  int rawLD = analogRead(LD_PIN);
  int rawRD = analogRead(RD_PIN);
  int rawRU = analogRead(RU_PIN);

  int rawSolar = analogRead(solarPin);

  // 2. Apply calibrations
  float LU = rawLU * kLU;
  float LD = rawLD * kLD;
  float RD = rawRD * kRD;
  float RU = rawRU * kRU;

  float pinVoltage = rawSolar * (5.0 / 1023.0);
  float solarVoltage = pinVoltage * 2.0; // Multiply bcs of voltage divider 

  // 3. Calculate dimensions
  float left  = LU + LD;
  float right = RU + RD;
  float upper = LU + RU;
  float lower = LD + RD;

  // 4. Calculate errors 
  float horizontalError = (right - left) / (right + left);
  float verticalError   = (upper - lower) / (upper + lower);

  // 5. Calculate light
  int lightPercent = map(rawSolar, 0, 650, 0, 100); 
  lightPercent = constrain(lightPercent, 0, 100);
  
  bool isNight = (lightPercent < MIN_LIGHT_THRESHOLD);

  // 6. Print values
  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = millis();

    Serial.print("LU: "); Serial.print(LU);
    Serial.print(" RU: "); Serial.print(RU);
    Serial.print(" LD: "); Serial.print(LD);
    Serial.print(" RD: "); Serial.print(RD);
    Serial.print(" | H: "); Serial.print(horizontalError, 3);
    Serial.print(" V: "); Serial.print(verticalError, 3);
    Serial.print(" | Servo H: "); Serial.print(horizontalAngle);
    Serial.print(" V: "); Serial.print(verticalAngle);
    Serial.print(" | Volt: "); Serial.print(solarVoltage, 2);
    Serial.print("V | Light: "); Serial.print(lightPercent);
    Serial.print("%");

    if (isNight) {
      Serial.print(" | [STATUS: Night mode - Low light, servos stopped]");
    } else if (limitReached) {
      Serial.print(" | [WARNING: Cannot reach light - out of physical reach!]");
    }

    Serial.println();
  }

  // 7. SEND DATA TO SERVER (по таймеру)
  if (millis() - lastServerTime >= SERVER_INTERVAL) {
    lastServerTime = millis();
    sendDataToServer(solarVoltage, lightPercent);
  }

  // 8. Dimensional tracking
  limitReached = false; 

  if (!isNight) {
    // Horizontal movement
    if (horizontalError > H_TOLERANCE) {
      if (horizontalAngle < H_MAX) horizontalAngle += STEP;
      else limitReached = true;
    }
    else if (horizontalError < -H_TOLERANCE) {
      if (horizontalAngle > H_MIN) horizontalAngle -= STEP;
      else limitReached = true;
    }

    // Vertical movement
    if (verticalError > V_TOLERANCE) {
      if (verticalAngle > V_MIN) verticalAngle -= STEP;
      else limitReached = true;
    }
    else if (verticalError < -V_TOLERANCE) {
      if (verticalAngle < V_MAX) verticalAngle += STEP;
      else limitReached = true;
    }

    // 9. Move servos
    horizontalServo.write(horizontalAngle);
    verticalServo.write(verticalAngle);
  }

  // 10. Wait for adjustment
  delay(TRACKING_DELAY);
}

//FUNCTION TO SEND HTTP VIA WI-FI
void sendDataToServer(float voltage, int light) {
  Serial.println("Sending data to server...");

  horizontalServo.detach();
  verticalServo.detach();

  String data = "voltage=" + String(voltage, 2) + "&light=" + String(light);
  String httpRequest = "";
  httpRequest += "POST /data HTTP/1.1\r\n";
  httpRequest += "Host: " + SERVER_IP + "\r\n";
  httpRequest += "Content-Type: application/x-www-form-urlencoded\r\n";
  httpRequest += "Content-Length: " + String(data.length()) + "\r\n\r\n";
  httpRequest += data;

  sendCommand("AT+CIPSTART=\"TCP\",\"" + SERVER_IP + "\"," + String(SERVER_PORT), 1000);

  esp.print("AT+CIPSEND=");
  esp.println(httpRequest.length());
  delay(500);

  esp.print(httpRequest);
  delay(1000);

  sendCommand("AT+CIPCLOSE", 500);
  Serial.println("Data sent!");

  horizontalServo.attach(H_SERVO_PIN);
  verticalServo.attach(V_SERVO_PIN);
  horizontalServo.write(horizontalAngle);
  verticalServo.write(verticalAngle);
}