// Including necessary libraries for WiFi, Sensors, and LCD
#include <WiFi.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <LiquidCrystal.h>
#include <WiFiS3.h>

// DHT Sensor Configuration
#define DHTPIN 7           // Pin number for DHT sensor
#define DHTTYPE DHT11      // Type of DHT sensor (DHT11 in this case)
DHT dht(DHTPIN, DHTTYPE);  // Initializing the DHT sensor object

// MQ-2 Gas Sensor Configuration
#define Board "Arduino UNO"
#define Pin A0
#define Type "MQ-2"
#define Voltage_Resolution 5
#define ADC_Bit_Resolution 10
#define RatioMQ2CleanAir 9.83
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);  // Initializing the MQ2 sensor object

// Soil Moisture Sensor Configuration
#define soilsensorPin A1  // Analog pin for soil moisture sensor
#define wetSoil 277       // Value representing wet soil
#define drySoil 380       // Value representing dry soil

// LCD Configurations
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;  // Pin numbers for the LCD
int contrast = 30;                                           // Contrast for the LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                   // Initializing the LCD object
String displayString = "";                                   // String to hold the message to be displayed on LCD
unsigned long prevMillis = 0;                                // Previous time in milliseconds
const long intervalScrolling = 800;                          // Time interval for scrolling effect on LCD

// WiFi Configurations
char ssid[] = "";                // SSID of your WiFi
char pass[] = "";                // Password of your WiFi
WiFiServer server(80);           // Setting up a web server on port 80

void setup() {
  Serial.begin(9600);        // Begin serial communication for debugging
  dht.begin();               // Start the DHT sensor
  lcd.begin(16, 2);          // Initialize the 16x2 LCD
  analogWrite(6, contrast);  // Set the LCD contrast
  initMQ2();                 // Initialize the MQ2 sensor
  connectToWiFi();           // Connect to the specified WiFi network
  server.begin();            // Start the web server
}

void loop() {
  updateSensorData();      // Fetch sensor data
  scrollDisplay();         // Scroll the display data on LCD
  handleClientRequests();  // Handle incoming web requests
  delay(100);              // Delay for 100 milliseconds
}

// Initialize the MQ-2 gas sensor
void initMQ2() {
  MQ2.setRegressionMethod(1);  // Set regression method for MQ2 sensor
  MQ2.setA(574.25);
  MQ2.setB(-2.222);
  MQ2.init();
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ2.update();
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);  // Calibrate the sensor
  }
  MQ2.setR0(calcR0 / 10);  // Set R0 value for the MQ2 sensor
}

// Read the soil moisture value
int readSoilMoisture() {
  return analogRead(soilsensorPin);
}

// Scroll the message on the LCD
void scrollDisplay() {
  if (millis() - prevMillis >= intervalScrolling) {
    displayString = displayString + displayString.charAt(0);
    displayString.remove(0, 1);
    lcd.setCursor(0, 0);
    lcd.print(displayString.substring(0, 16));
    prevMillis = millis();
  }
}

// Fetch and update the data from sensors
void updateSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float gasPPM = MQ2.readSensor();
  int soilMoisture = map(readSoilMoisture(), drySoil, wetSoil, 0, 100);

  if (displayString.length() == 0) {
    displayString = "Temp: " + String(temperature) + "C   Hum: " + String(humidity) + "%   Gas: " + String(gasPPM) + "PPM   Soil: " + String(soilMoisture) + "%     ";
  }
}

// Handle client requests and serve the sensor data over HTTP
void handleClientRequests() {
  WiFiClient client = server.available();
  if (client) {
    // Fetch and map sensor data to their respective descriptions
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    float gasPPM = MQ2.readSensor();
    int soilMoisture = map(readSoilMoisture(), drySoil, wetSoil, 0, 100);

    String temperatureDesc = (temperature < 18) ? "Cold" : (temperature <= 26) ? "Comfortable"
                                                                               : "Warm";
    String humidityDesc = (humidity < 40) ? "Low" : (humidity <= 60) ? "Comfortable"
                                                                     : "High";
    String gasDesc = (gasPPM < 50) ? "Good" : "Bad";
    String soilDesc = (soilMoisture < 30) ? "Dry (Add Water)" : "Moist";

    // Send HTTP response to the client with the sensor data and their descriptions
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE HTML><html>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; }");
    client.println(".grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 20px; }");
    client.println(".sensor { border: 1px solid #ddd; padding: 15px; border-radius: 7px; }");
    client.println(".title { font-weight: bold; }");
    client.println(".bar { height: 25px; }");
    client.println("</style>");
    client.println("<div class='grid'>");

    //  HTML code for displaying sensor data
    client.println("<div class='sensor'>");
    client.println("<div class='title'>Temperature</div>");
    client.println("<div>" + String(temperature) + "&deg;C (" + temperatureDesc + ")</div>");
    client.println("</div>");

    client.println("<div class='sensor'>");
    client.println("<div class='title'>Humidity</div>");
    client.println("<div>" + String(humidity) + "% (" + humidityDesc + ")</div>");
    client.println("</div>");

    client.println("<div class='sensor'>");
    client.println("<div class='title'>Gas Quality</div>");
    client.println("<div>" + String(gasPPM) + " PPM (" + gasDesc + ")</div>");
    client.println("</div>");

    client.println("<div class='sensor'>");
    client.println("<div class='title'>Soil Moisture</div>");
    client.println("<div>" + String(soilMoisture) + "% (" + soilDesc + ")</div>");
    client.println("</div>");

    client.println("</div>");
    client.println("</html>");
    client.stop();
  }
}

// Function to connect to the WiFi network
void connectToWiFi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
