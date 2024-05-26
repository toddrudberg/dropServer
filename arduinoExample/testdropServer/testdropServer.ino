#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>

char ssid[] = "Turkey Point";      // your network SSID (name)
char pass[] = "gobblegobble";   // your network password
char server[] = "192.168.1.31"; // address of your server

int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiClient client;

void setup() {
    // attempt to connect to WiFi network:
    // Set static IP address
    IPAddress ip(192, 168, 1, 8); // Replace with your desired IP
    IPAddress gateway(192, 168, 1, 1); // Replace with your gateway address
    IPAddress subnet(255, 255, 255, 0); // Replace with your subnet mask
    IPAddress dns(8, 8, 8, 8); // Optional: Replace with your preferred DNS server's IP

    WiFi.config(ip, dns, gateway, subnet);
    
    WiFi.begin(ssid, pass);
}


char jsonData[] = "{"
    "\"DateStamp\": \"2023-05-25\","
    "\"TimeStamp\": \"12:34:56\","
    "\"Epoch\": 1679850896,"
    "\"OutsideAirTemp\": 22.5,"
    "\"OutsideHumidity\": 45.2,"
    "\"OutsideBaro\": 1013.1,"
    "\"SoilTemperature\": 20.3,"
    "\"SoilElectricalConductivity\": 1.2,"
    "\"SoilHumidity\": 30.1,"
    "\"SoilPh\": 6.5,"
    "\"Watering\": true,"
    "\"WifiError\": false,"
    "\"SDError\": false,"
    "\"RTCFailed\": false"
"}";

void loop() {
    // if there's a successful connection:
    if (client.connect(server, 3000)) {
        // send the HTTP POST request:
        client.println("POST /log HTTP/1.1");
        client.println("Host: " + String(server));
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        // calculate the length of the JSON data
        client.print("Content-Length: ");
        client.println(strlen(jsonData));
        client.println();
        // send the JSON data:
        client.println(jsonData);
    } 
    else {
        // if you couldn't make a connection:
        Serial.println("connection failed");
    }
    delay(5000); 
}