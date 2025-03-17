#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "Ali";
const char* password = "s321Yedlk74";

// Static IP Configuration
IPAddress local_IP(192, 168, 137, 160);  // Set your desired static IP
IPAddress gateway(192, 168, 137, 1);     // Set your gateway IP
IPAddress subnet(255, 255, 255, 0);      // Set your subnet mask
IPAddress primaryDNS(8, 8, 8, 8);        // Set primary DNS (optional)
IPAddress secondaryDNS(8, 8, 4, 4);      // Set secondary DNS (optional)

typedef struct {
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// 4 for flash led or 33 for normal led
#define LED_GPIO_NUM   4

#else
#error "Camera model not selected"
#endif

// Start the web server on port 80
WiFiServer server(80);

void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void setup() {
  Serial.begin(115200);

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

  // Start the camera
  startCamera();

  // Configure the flash LED pin as an output
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, LOW); // Ensure the flash is off initially

  // Start the web server
  server.begin();
}

void loop() {
  // Check for client connections
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");

    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

// Inside the loop() function, modify the HTTP POST request section:
  if (request.indexOf("GET /CAPTURE_PHOTO") >= 0) {
    Serial.println("Capture photo command received");

    // Turn on the flash LED
    digitalWrite(LED_GPIO_NUM, HIGH);
    delay(500); // Allow time for the flash to stabilize

    // Capture a photo
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        client.println("HTTP/1.1 500 Internal Server Error");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("Camera capture failed");
    } else {
        Serial.printf("Captured photo with size: %d bytes\n", fb->len);

        // Generate a unique filename using a timestamp
        String timestamp = String(millis()); // Use milliseconds since boot as a unique identifier
        String filename = "photo_" + timestamp + ".jpg";

        // Send the photo to the local server using HTTP POST
        HTTPClient http;
        http.begin("http://192.168.137.1/smart_irrigation/upload.php"); // Replace with your local server IP

        // Create a multipart/form-data request
        String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
        http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

        String body = "--" + boundary + "\r\n";
        body += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";
        body += "Content-Type: image/jpeg\r\n\r\n";
        body += String((char *)fb->buf, fb->len);
        body += "\r\n--" + boundary + "--\r\n";

        int httpResponseCode = http.POST(body);
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();

        // Return the frame buffer to the driver for reuse
        esp_camera_fb_return(fb);

        // Turn off the flash LED
        digitalWrite(LED_GPIO_NUM, LOW);

        // Send response to the client
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("Photo captured and sent to server");
    }
} else {
      // Invalid request
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Content-Type: text/plain");
      client.println();
      client.println("Invalid request");
    }

    delay(30);
    client.stop();
    Serial.println("Client disconnected");
  }
}