#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define SOIL_SENSOR_PIN 34
#define RAIN_SENSOR_PIN 35
#define RELAY_PIN 27
  
const char* ssid = "AHMED";
const char* password = "12345678";

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing DHT22...");
  dht.begin();
  delay(2000);  // Allow sensor to stabilize

  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Ensure pump is off initially

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilMoisture = analogRead(SOIL_SENSOR_PIN);
    int rainValue = analogRead(RAIN_SENSOR_PIN);
    
    String page = "<html><body>";
    page += "<h2>Plant Monitoring System</h2>";
    page += "<p>Temperature: " + String(temperature) + " °C</p>";
    page += "<p>Humidity: " + String(humidity) + " %</p>";
    page += "<p>Soil Moisture: " + String(soilMoisture) + "</p>";
    page += "<p>Rain Sensor: " + String(rainValue) + "</p>";
    page += "<p><a href=\"/water_on\">Water Plant</a></p>";
    page += "<p><a href=\"/water_off\">Stop Watering</a></p>";
    page += "</body></html>";

    request->send(200, "text/html", page);
  });

  server.on("/water_off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, HIGH);
    request->send(200, "text/plain", "Watering Stopped");
  });

  server.on("/water_on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, LOW);
    request->send(200, "text/plain", "Watering Started");
  });

  server.begin();
}

void loop() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilMoisture = analogRead(SOIL_SENSOR_PIN);
    int rainValue = analogRead(RAIN_SENSOR_PIN);

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Serial.print("Rain Sensor: ");
  Serial.println(rainValue);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  if (rainValue < 1500) {  
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Rain detected! Skipping watering.");
  } else {
    Serial.println("No rain detected. Proceeding with watering Automatically by soil Moisture condition.");
  
  if (soilMoisture < 3000) { // Adjust threshold as needed
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("No watering needed.");
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Watering plant...");
  }
  
}



    delay(2000); // Wait 5 seconds before next reading
}
