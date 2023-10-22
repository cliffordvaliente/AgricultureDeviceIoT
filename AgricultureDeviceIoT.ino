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
const long intervalScrolling = 400;                          // Time interval for scrolling effect on LCD
int scrollIndex = 0;

// WiFi Configurations
char ssid[] = "Gylden72 GUTTA";  // SSID of your WiFi
char pass[] = "TerjeGsNett";     // Password of your WiFi
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
  delay(50);               // Delay for 50 milliseconds
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
    lcd.setCursor(0, 0);
    lcd.print(displayString.substring(scrollIndex, scrollIndex + 16));

    scrollIndex++;
    if (scrollIndex + 16 > displayString.length()) {
      scrollIndex = 0;  // reset to the beginning when we've reached the end
    }

    prevMillis = millis();
  }
}

// Fetch and update the data from sensors
void updateSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float gasPPM = MQ2.readSensor();
  int soilMoisture = map(readSoilMoisture(), drySoil, wetSoil, 0, 100);

  displayString = "Temp: " + String(temperature) + "C   Hum: " + String(humidity) + "%   Gas: " + String(gasPPM) + "PPM   Soil: " + String(soilMoisture) + "%     ";
  displayString += "                    ";  // Add padding for smooth scrolling

  // Print sensor data to the serial monitor
  Serial.println("Temperature: " + String(temperature) + "C");
  Serial.println("Humidity: " + String(humidity) + "%");
  Serial.println("Gas Quality (PPM): " + String(gasPPM));
  Serial.println("Soil Moisture (%): " + String(soilMoisture));
  Serial.println("-----------------------------------------");
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
    client.println("<head>");
    client.println("<meta http-equiv='refresh' content='10';URL='http://192.168.68.132/'>");  // Auto-refresh every 5 seconds
    client.println("<script>");
    client.println("var count = 10;");  // Begin countdown from 10 seconds
    client.println("function countdown() {");
    client.println("  document.getElementById('timer').innerHTML = 'Refreshing in: ' + count + ' seconds';");
    client.println("  count--;");
    client.println("  if(count < 0) count = 10;");  // Reset to 10 seconds when countdown reaches 0
    client.println("  setTimeout(countdown, 1000);");
    client.println("}");
    client.println("</script>");
    client.println("</head>");
    client.println("<body onload='countdown()'>");
    client.println("<h1>Sensor Data</h1>");
    client.println("<p>Temperature: " + String(temperature) + "C (" + temperatureDesc + ")</p>");
    client.println("<p>Humidity: " + String(humidity) + "% (" + humidityDesc + ")</p>");
    client.println("<p>Gas Quality: " + String(gasPPM) + "PPM (" + gasDesc + ")</p>");
    client.println("<p>Soil Moisture: " + String(soilMoisture) + "% (" + soilDesc + ")</p>");
    client.println("<div id='timer'>Refreshing in: 10 seconds</div>");  // Display timer starting from 10 seconds
    client.println("</body>");
    client.println("</html>");
    client.stop();
  }
}

// Connect to the WiFi network
void connectToWiFi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
