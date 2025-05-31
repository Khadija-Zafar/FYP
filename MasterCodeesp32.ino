 #include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// Pin Definitions
#define SOIL_MOISTURE_PIN 34
#define RAIN_SENSOR_PIN 35
#define PUMP_RELAY_PIN 27
#define DHT_PIN 4  // GPIO pin for DHT22 sensor

// Thresholds
#define SOIL_MOISTURE_THRESHOLD 2500  // Adjust based on your sensor readings
#define RAIN_THRESHOLD 3000            // Adjust based on your sensor readings

// Wi-Fi Credentials
const char* ssid = "Ali";
const char* password = "s321Yedlk74";

// Static IP Configuration
IPAddress local_IP(192, 168, 137, 150);  // Set your desired static IP
IPAddress gateway(192, 168, 137, 1);     // Set your gateway IP
IPAddress subnet(255, 255, 255, 0);      // Set your subnet mask
IPAddress primaryDNS(8, 8, 8, 8);        // Set primary DNS (optional)
IPAddress secondaryDNS(8, 8, 4, 4);      // Set secondary DNS (optional)

// Apache Server Details
const char* serverUrl = "http://192.168.137.1/smart_irrigation";

// ESP32-CAM IP address
const char* esp32camIP = "192.168.137.160"; // Replace with the ESP32-CAM's IP address

// DHT11 Sensor
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

void sendSensorData(int soilMoisture, int rainSensor, bool pumpStatus, float temperature, float humidity) {
  HTTPClient http;
  String url = String(serverUrl) + "/data.php"; // Updated endpoint
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"soilMoisture\":" + String(soilMoisture) +
                    ",\"rainSensor\":" + String(rainSensor) +
                    ",\"pumpStatus\":" + String(pumpStatus ? "true" : "false") +
                    ",\"temperature\":" + String(temperature) +
                    ",\"humidity\":" + String(humidity) + "}";

  int httpResponseCode = http.POST(jsonData);
  if (httpResponseCode > 0) {
    Serial.printf("Sensor data sent successfully, HTTP response code: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error sending sensor data, HTTP response code: %d\n", httpResponseCode);
  }

  http.end();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Initialize Pins
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW);  // Ensure the pump is off initially

  // Initialize DHT11 Sensor
  dht.begin();


    // Configure Static IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Failed to configure static IP!");
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  // Print connection details
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read Soil Moisture Sensor
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);

  // Read Rain Sensor
  int rainSensorValue = analogRead(RAIN_SENSOR_PIN);
  Serial.print("Rain Sensor: ");
  Serial.println(rainSensorValue);

  // Read DHT22 Sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  // Check for NaN values and replace them with 0
  if (isnan(temperature)) temperature = 0;
  if (isnan(humidity)) humidity = 0;

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Control the Pump
  bool pumpStatus = false;
  if (soilMoistureValue > SOIL_MOISTURE_THRESHOLD && rainSensorValue > RAIN_THRESHOLD) {
    digitalWrite(PUMP_RELAY_PIN, LOW);
    pumpStatus = true;
    Serial.println("Pump ON - Soil is dry and no rain detected");
  } else {
    digitalWrite(PUMP_RELAY_PIN, HIGH);
    pumpStatus = false;
    Serial.println("Pump OFF - Soil is wet or rain detected");
  }

  // Send Sensor Data to Server
  sendSensorData(soilMoistureValue, rainSensorValue, pumpStatus, temperature, humidity);

  // Send command to ESP32-CAM to take a photo
  Serial.println("Sending capture command to ESP32-CAM");
  Serial.println("CAPTURE_PHOTO");
  
  // Send the capture command to the ESP32-CAM
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(esp32camIP) + "/CAPTURE_PHOTO";
    http.begin(url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.printf("Photo capture command sent, HTTP response code: %d\n", httpResponseCode);
    } else {
      Serial.printf("Error sending command, HTTP response code: %d\n", httpResponseCode);
    }

    http.end();
  }


  // Delay for stability
  delay(10000);  // Check every 10 seconds
} 