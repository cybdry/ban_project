#include <WiFiManager.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include "time.h"

#define ACCESS_NAME "Connect"
#define ACCESS_PASSWORD "testpassword"
#define REPORTING_PERIOD_MS 1000
#define ALERT_PIN 13
#define FIREBASE_URL "https://bannewproject-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_API_KEY "AIzaSyDiwdeiVTdnFe_tyguP9U98u-VZ8tpDF8s"
#define USER_EMAIL "adjeviananarsene@gmail.com"
#define USER_PASSWORD "cyb12345@"
//Create MPU object
Adafruit_MPU6050 mpu;
// Create a PulseOximeter object

// Time at which the last beat occurred
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
uint8_t set_alert = 0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
// Variables to save date and time
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

bool init_wifimanager(const char *access_name, const char *password) {
  WiFiManager wm;
  //reset old setting
  wm.resetSettings();
  bool res;
  res = wm.autoConnect(access_name, password);

  if (!res) {
    return false;
  } else {
    return true;
  }
}


char* getLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return NULL;
  }
  char buffer[60];
  strftime(buffer, sizeof(buffer), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return buffer;
}


int PulseSensorPurplePin = 0;        // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0

int Signal;                // holds the incoming raw data. Signal value can range from 0-1024
int Threshold = 500;
float bpm = 0.0;
void setup() {

  Serial.begin(115200);
  //INIT alert pin
  pinMode(ALERT_PIN, OUTPUT);
  //First step, connect to wifi
  Serial.println("Connecting to  wifi...");
  bool res = init_wifimanager(ACCESS_NAME, ACCESS_PASSWORD);
  if (!res) {
    Serial.println("Failed to connect to wifi");
    ESP.restart();
  }
  //Config Firebase

  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_URL;
  Firebase.reconnectWiFi(true);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Connected successfully");
    signupOK = true;
  } else {
    Serial.printf("RRR %s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */

  Firebase.begin(&config, &auth);

  // initialized MPU and MAX sensor;

  if (!mpu.begin()) {
    Serial.println("Failed to initialized mpu ");

    while (1) {
      delay(10);
    }
  }

  //Hardware level are ready at this level, set sensors remaining configuration
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println(getLocalTime());
}

void loop() {

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Signal = analogRead(PulseSensorPurplePin); 
   if(Signal > Threshold){                          
    bpm = 60000.0/ (float)Signal;
    Serial.print("BPM:");
    Serial.println(bpm);
   } 


  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    //float heart_rate = pox.getHeartRate();
    //float spo2 = pox.getSpO2();
    if (Firebase.RTDB.setFloat(&fbdo, "sensor/acc/x", a.acceleration.x)) {
      Firebase.RTDB.setFloat(&fbdo, "sensor/acc/y", a.acceleration.y);
      Firebase.RTDB.setFloat(&fbdo, "sensor/acc/z", a.acceleration.z);
      Firebase.RTDB.setFloat(&fbdo, "sensor/gyro/x", g.gyro.x);
      Firebase.RTDB.setFloat(&fbdo, "sensor/gyro/y", g.gyro.y);
      Firebase.RTDB.setFloat(&fbdo, "sensor/gyro/z", g.gyro.z);
      Firebase.RTDB.setFloat(&fbdo, "sensor/temp", temp.temperature);
      Firebase.RTDB.setFloat(&fbdo, "sensor/heart/rate", bpm);
      //Firebase.RTDB.setString(&fbdo, "sensor/time", getLocalTime());

     
      //Firebase.RTDB.setInt(&fbdo, "sensor/alert", set_alert);

    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //get alert and write
    Serial.println("Reading...");
    if (Firebase.RTDB.getInt(&fbdo, "/sensor/alert")) {
      if (fbdo.dataType() == "int") {
        set_alert = fbdo.intData();
        digitalWrite(ALERT_PIN, set_alert);
      }
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}
