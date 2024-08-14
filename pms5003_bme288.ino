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
  Serial.begin(115200);

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

  if (!bme.begin(0x76)) {
    Serial.println(F("BME280 initialization failed!"));
    while (1);
  }

  if (!display.begin(SSD1306_PAGEADDR, 0x3C)) {
    Serial.println(F("SSD1306 initialization failed!"));
    while (1);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  pms.setToPassiveReporting();

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
    response += "    } else {";
    response += "      console.error('Failed to fetch data');";
    response += "    }";
    response += "  };";
    response += "  xhr.onerror = function() {";
    response += "    console.error('Request error');";
    response += "  };";
    response += "  xhr.send();";
    response += "}";
    response += "window.onload = function() {";
    response += "  fetchData();";
    response += "  setInterval(fetchData, 10000);"; // Set interval to 10 seconds
    response += "};";
    response += "</script>";
    response += "</head><body>";
    response += "<h1>Sensor Readings</h1>";
    response += "<div id='sensor-data'>Loading...</div>";
    response += "</body></html>";
    server.send(200, "text/html", response);
  });

  server.on("/data", []() {
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;

    pms.poll();
    delay(20);
    pms.read();

    String response = "<p>Temperature: " + String(temperature) + " °C</p>";
    response += "<p>Humidity: " + String(humidity) + " %</p>";
    response += "<p>Pressure: " + String(pressure) + " hPa</p>";
    response += "<p>PM1.0 (STD): " + String(pms.getPM1_STD()) + " µg/m³</p>";
    response += "<p>PM2.5 (STD): " + String(pms.getPM2_5_STD()) + " µg/m³</p>";
    response += "<p>PM10.0 (STD): " + String(pms.getPM10_STD()) + " µg/m³</p>";

    server.send(200, "text/html", response);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  pms.poll();
  delay(20);
  pms.read();

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

  delay(2000); // Update OLED every 2 seconds
}
