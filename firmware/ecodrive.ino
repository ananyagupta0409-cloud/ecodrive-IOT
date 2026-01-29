#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "DHT.h"
#include <time.h>


#define MQ135_PIN 34
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define WIFI_SSID        "SSID"
#define WIFI_PASSWORD    "password"

#define API_KEY          "APIKEY"
#define DATABASE_URL     "URL"

#define USER_EMAIL       "ecodrive@test.com"
#define USER_PASSWORD    "test1234"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastUpdate = 0;
int interval = 5000;

String getIST() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char buffer[32];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday,
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec);
  return String(buffer);
}

String getDateOnly() {
  time_t now = time(nullptr);
  struct tm t;
  localtime_r(&now, &t);

  char buffer[16];
  sprintf(buffer, "%04d-%02d-%02d",
          t.tm_year + 1900,
          t.tm_mon + 1,
          t.tm_mday);
  return String(buffer);
}

String getTimeOnly() {
  time_t now = time(nullptr);
  struct tm t;
  localtime_r(&now, &t);

  char buffer[16];
  sprintf(buffer, "%02d-%02d-%02d",
          t.tm_hour,
          t.tm_min,
          t.tm_sec);
  return String(buffer);
}

int calculateEmissionScore(int rawGas) {
    float normalized = (float)rawGas / 4095.0;
    float boosted = normalized * 1490.0;
    int score = (int)boosted;

    if (score > 500) score = 500;
    if (score < 0) score = 0;

    return score;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi Connected!");

  dht.begin();

  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// ==============================
// MAIN LOOP
// ==============================
void loop() {

  if (Firebase.ready() && millis() - lastUpdate > interval) {
    lastUpdate = millis();

    int rawGas = analogRead(MQ135_PIN);
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) return;

    int emissionScore = calculateEmissionScore(rawGas);

    String fullTime = getIST();
    String dateFolder = getDateOnly();
    String timeKey = getTimeOnly();

    FirebaseJson json;
    json.set("rawGas", rawGas);
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("emissionScore", emissionScore);
    json.set("timestamp", fullTime);

    // WRITE TO LIVE DATA
    Firebase.RTDB.setJSON(&fbdo, "/carEmissions/liveData", &json);

    // WRITE TO HISTORY (clean, date/time based)
    String historyPath = "/carEmissions/history/" + dateFolder + "/" + timeKey;
    Firebase.RTDB.setJSON(&fbdo, historyPath.c_str(), &json);

    // ==============================
    // ALERTS — CLEAN ID, NO RANDOM KEYS
    // ==============================
    if (emissionScore >= 400) {
      FirebaseJson alertJson;
      alertJson.set("emissionScore", emissionScore);
      alertJson.set("message", "High emission event");
      alertJson.set("timestamp", fullTime);

      String alertId = dateFolder + "_" + timeKey;

      Firebase.RTDB.setJSON(&fbdo, ("/carEmissions/alerts/" + alertId).c_str(), &alertJson);

      Serial.println("ALERT Triggered & Written: " + alertId);
    }

    Serial.println("Logged: " + fullTime);
  }
}
