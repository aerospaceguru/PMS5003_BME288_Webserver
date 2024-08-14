#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PMS5003.h>

#define RX2 16
#define TX2 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_ADDRESS 0x3C

// Wi-Fi credentials
const char* ssid = "TALKTALKA39E14";
const char* password = "UR6QFVH6";

// Create objects
Adafruit_BME280 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
GuL::PMS5003 pms(Serial2);
WebServer server(80);

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize BME280 sensor
  if (!bme.begin(0x76)) {
    Serial.println(F("BME280 initialization failed!"));
    while (1); // Halt if initialization fails
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_PAGEADDR, 0x3C)){
    Serial.println(F("SSD1306 initialization failed!"));
    while (1); // Halt if initialization fails
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  // Initialize PMS5003 sensor
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  pms.setToPassiveReporting();

  // Define the root URL
  server.on("/", []() {
    String response = "<html><head>";
    response += "<title>Sensor Readings</title>";
    response += "<script>";
    response += "function fetchData() {";
    response += "  var xhr = new XMLHttpRequest();";
    response += "  xhr.open('GET', '/data', true);";
    response += "  xhr.onload = function() {";
    response += "    if (xhr.status === 200) {";
    response += "      document.getElementById('sensor-data').innerHTML = xhr.responseText;";
    response += "    }";
    response += "  };";
    response += "  xhr.send();";
    response += "}";
    response += "setInterval(fetchData, 2000);"; // Fetch data every 2 seconds
    response += "</script>";
    response += "</head><body onload='fetchData()'>";
    response += "<h1>Sensor Readings</h1>";
    response += "<div id='sensor-data'></div>";
    response += "</body></html>";
    server.send(200, "text/html", response);
  });

  // Define the data endpoint
  server.on("/data", []() {
    // Read sensor data
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;  // Convert to hPa

    pms.poll();
    delay(20);
    pms.read();

    // Create the response for the data endpoint
    String response = "<p>Temperature: " + String(temperature) + " °C</p>";
    response += "<p>Humidity: " + String(humidity) + " %</p>";
    response += "<p>Pressure: " + String(pressure) + " hPa</p>";
    response += "<p>PM1.0 (STD): " + String(pms.getPM1_STD()) + " µg/m³</p>";
    response += "<p>PM2.5 (STD): " + String(pms.getPM2_5_STD()) + " µg/m³</p>";
    response += "<p>PM10.0 (STD): " + String(pms.getPM10_STD()) + " µg/m³</p>";

    server.send(200, "text/html", response);
  });

  // Start the server
  server.begin();
}

void loop() {
  // Handle web server requests
  server.handleClient();

  // Read sensor data
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;  // Convert to hPa

  pms.poll();
  delay(20);
  pms.read();

  // Update OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  display.print("Pressure: ");
  display.print(pressure);
  display.println(" hPa");
  display.print("PM1.0: ");
  display.print(pms.getPM1_STD());
  display.println(" µg/m³");
  display.print("PM2.5: ");
  display.print(pms.getPM2_5_STD());
  display.println(" µg/m³");
  display.print("PM10: ");
  display.print(pms.getPM10_STD());
  display.println(" µg/m³");
  display.display();

  delay(2000); // Update every 2 seconds
}
