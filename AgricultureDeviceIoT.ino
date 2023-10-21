#include <WiFi.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <LiquidCrystal.h>
#include <WiFiS3.h>

// DHT Sensor Configuration
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-2 Gas Sensor Configuration
#define Board "Arduino UNO"
#define Pin A0
#define Type "MQ-2"
#define Voltage_Resolution 5
#define ADC_Bit_Resolution 10
#define RatioMQ2CleanAir 9.83
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

// Soil Moisture Sensor Configuration
#define soilsensorPin A1
#define wetSoil 277
#define drySoil 380

// LCD Configurations
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
int contrast = 30;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
String displayString = "";
unsigned long prevMillis = 0;
const long intervalScrolling = 800; 

// WiFi Configurations
char ssid[] = "Gylden72 GUTTA";
char pass[] = "TerjeGsNett";
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.begin(16, 2);
  analogWrite(6, contrast);
  initMQ2();
  connectToWiFi();
  server.begin();
}

void loop() {
  updateSensorData();
  scrollDisplay();
  handleClientRequests();
  delay(100);
}

void initMQ2() {
  MQ2.setRegressionMethod(1);
  MQ2.setA(574.25); 
  MQ2.setB(-2.222);
  MQ2.init();
  float calcR0 = 0;
  for(int i = 1; i <= 10; i++) {
    MQ2.update();
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
  }
  MQ2.setR0(calcR0 / 10);
}

int readSoilMoisture() {
  return analogRead(soilsensorPin);
}

void scrollDisplay() {
  if (millis() - prevMillis >= intervalScrolling) {
    displayString = displayString + displayString.charAt(0);
    displayString.remove(0, 1);
    lcd.setCursor(0, 0);
    lcd.print(displayString.substring(0, 16));
    prevMillis = millis();
  }
}

void updateSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float gasPPM = MQ2.readSensor();
  int soilMoisture = map(readSoilMoisture(), drySoil, wetSoil, 0, 100);

  if (displayString.length() == 0) {
    displayString = "Temp: " + String(temperature) + "C   Hum: " + String(humidity) + "%   Gas: " + String(gasPPM) + "PPM   Soil: " + String(soilMoisture) + "%     ";
  }
}
void handleClientRequests() {
  WiFiClient client = server.available();
  if (client) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    float gasPPM = MQ2.readSensor();
    int soilMoisture = map(readSoilMoisture(), drySoil, wetSoil, 0, 100);
    
    // Determine descriptions based on values
    String temperatureDesc = (temperature < 18) ? "Cold" : (temperature <= 26) ? "Comfortable" : "Warm";
    String humidityDesc = (humidity < 40) ? "Low" : (humidity <= 60) ? "Comfortable" : "High";
    String gasDesc = (gasPPM < 50) ? "Good" : "Bad";
    String soilDesc = (soilMoisture < 30) ? "Dry (Add Water)" : "Moist";

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



void connectToWiFi() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
