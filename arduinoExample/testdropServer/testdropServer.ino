#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>

char ssid[] = "Turkey Point";      // your network SSID (name)
char pass[] = "gobblegobble";   // your network password
char server[] = "192.168.1.31"; // address of your server

int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiClient client;

void setup() {
    Serial.begin(9600); // Initialize serial communication at 9600 baud    
    // attempt to connect to WiFi network:
    // Set static IP address
    IPAddress ip(192, 168, 1, 8); // Replace with your desired IP
    IPAddress gateway(192, 168, 1, 1); // Replace with your gateway address
    IPAddress subnet(255, 255, 255, 0); // Replace with your subnet mask
    IPAddress dns(8, 8, 8, 8); // Optional: Replace with your preferred DNS server's IP

    WiFi.config(ip, dns, gateway, subnet);
    
    WiFi.begin(ssid, pass);

    while(WiFi.status() != WL_CONNECTED) {
        // print dots while we wait to connect
        Serial.print(".");
        delay(1000);
    }

    // you're connected now, so print out the data
    Serial.print("You're connected to the network");
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());


    delay(1000);
}


// char jsonData[] = "{"
//     "\"DateStamp\": \"2023-05-25\","
//     "\"TimeStamp\": \"12:34:56\","
//     "\"Epoch\": 1679850896,"
//     "\"OutsideAirTemp\": 22.5,"
//     "\"OutsideHumidity\": 45.2,"
//     "\"OutsideBaro\": 1013.1,"
//     "\"SoilTemperature\": 20.3,"
//     "\"SoilElectricalConductivity\": 1.2,"
//     "\"SoilHumidity\": 30.1,"
//     "\"SoilPh\": 6.5,"
//     "\"Watering\": true,"
//     "\"TimeRemaining\": 120,"
//     "\"WifiError\": false,"
//     "\"SDError\": false,"
//     "\"RTCFailed\": false"
// "}";

void loop() {
    // if there's a successful connection:
    // Build the JSON data
    char jsonData[512]; // adjust the size as needed
    unsigned long epoch = millis();
    snprintf(jsonData, sizeof(jsonData),
        "{"
        "\"DateStamp\": \"2023-05-25\","
        "\"TimeStamp\": \"12:34:56\","
        "\"Epoch\": %lu,"
        "\"OutsideAirTemp\": 22.5,"
        "\"OutsideHumidity\": 45.2,"
        "\"OutsideBaro\": 1013.1,"
        "\"SoilTemperature\": 20.3,"
        "\"SoilElectricalConductivity\": 1.2,"
        "\"SoilHumidity\": 30.1,"
        "\"SoilPh\": 6.5,"
        "\"Watering\": true,"
        "\"TimeRemaining\": 120,"
        "\"WifiError\": false,"
        "\"SDError\": false,"
        "\"RTCFailed\": false"
        "}", epoch);
    Serial.println(jsonData);
    Serial.println(strlen(jsonData));
    
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
        Serial.println("Data Sent");
    } 
    else {
        // if you couldn't make a connection:
        Serial.println("connection failed");
    }
    delay(10000); 
}