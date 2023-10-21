
# Smart Environment Monitoring System 

Monitor temperature, humidity, gas levels, and soil moisture with this versatile IoT project. The readings are displayed both on an LCD and via a web interface accessible over WiFi.
The goal is to provide a good overview of the environment for inside or indoor farms.


## Features

-   Real-time temperature and humidity readings using DHT11 sensor.
-   Gas PPM monitoring with MQ-2 gas sensor.
-   Soil moisture percentage reading to keep your plants healthy.
-   Display the sensor readings on an LCD.
-   Access the sensor readings via a web interface over WiFi.
-   Visual indicators on the web interface for:
    -   Temperature status: Cold, Comfortable, Warm.
    -   Humidity status: Low, Comfortable, High.
    -   Gas Quality: Good or Bad.
    -   Soil Moisture status: Dry (Add Water) or Moist.

## Hardware Components

1.  Arduino UNO board
2.  DHT11 temperature and humidity sensor
3.  MQ-2 gas sensor
4.  Soil moisture sensor
5.  LCD 16x2 display
6.  Built in WIFI in the Arduino Board for WiFi connectivity
7.  Necessary jumper wires, resistors, and breadboard

## Setup

1.  Connect the DHT11, MQ-2, soil moisture sensor, and LCD to the Arduino UNO as described in the circuit diagram.
2.  Configure the WiFi credentials in the source code.
3.  Upload the code to Arduino UNO.
4.  Access the web interface by connecting to the provided IP address over the same WiFi network.

## Code Explanation

-   **Temperature & Humidity**: Using the DHT library to read data from the DHT11 sensor.
-   **Gas PPM**: Using the "MQUnifiedsensor" library to calibrate and read data from the MQ-2 sensor.
-   **Soil Moisture**: Analog reading is mapped between 0 to 100%.
-   **Web Interface**: Data is served using the built-in ESP8266 web server.

## Future Enhancements

1.  Integration with cloud services for remote monitoring.
2.  Power optimisation for battery-operated deployments.
3. Improved overview webpage

## Contributors

-   Clifford Valiente (https://github.com/cliffordvaliente)
-   Contact Email:  Cliffordv@uia.no 

## Copyright

Copyright (c) [2001] [Clifford Valiente]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this code and associated documentation files (the "Code"), to deal
in the Code without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Code, and to permit persons to whom the Code is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Code.

