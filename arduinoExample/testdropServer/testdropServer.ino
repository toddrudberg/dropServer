#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>

char ssid[] = "Turkey Point";      // your network SSID (name)
char pass[] = "gobblegobble";   // your network password
char server[] = "192.168.1.31"; // address of your server
int port = 3000;
//char server[] = "64.23.202.34"; // address of your server


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

void loop() {
    // if there's a successful connection:
    // Build the JSON data
    static bool autoWateringRequest = false;
    static bool manualWateringRequest = false;
    bool watering = manualWateringRequest;
    char jsonData[512]; // adjust the size as needed
    unsigned long epoch = millis();
    snprintf(jsonData, sizeof(jsonData),
        "{"
        "\"DateStamp\": \"2023-05-25\","
        "\"TimeStamp\": \"12:34:56\","
        "\"Epoch\": \"%lu\","
        "\"OutsideAirTemp\": \"22.5\","
        "\"OutsideHumidity\": \"45.2\","
        "\"OutsideBaro\": \"30.12\","
        "\"SoilTemperature\": \"20.3\","
        "\"SoilElectricalConductivity\": \"1.2\","
        "\"SoilHumidity\": \"30.1\","
        "\"SoilPh\": \"6.5\","
        "\"Watering\": \"%s\","
        "\"autoWaterCycleEnabled\": \"%s\","
        "\"manualWateringRequest\": \"%s\","
        "\"TimeRemaining\": \"120\","
        "\"WifiError\": \"false\","
        "\"SDError\": \"false\","
        "\"RTCFailed\": \"false\""
        "}", epoch, watering ? "true" : "false",  autoWateringRequest ? "true" : "false", manualWateringRequest ? "true" : "false");
    Serial.println();
    Serial.print("manualWateringRequest: ");
    Serial.println(manualWateringRequest);
    Serial.print("autoWateringRequest: ");
    Serial.println(autoWateringRequest);
    Serial.println();
    Serial.println(jsonData);
    Serial.println(strlen(jsonData));
    
    if (client.connect(server, port)) {
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
    delay(2000); 

    read_dropServer(&autoWateringRequest, &manualWateringRequest);

    delay(2000);

    // WiFiClient client;
    // //const char* server = "64.23.202.34";  // Replace with your server's IP address
    // const int port = 3000;  // Replace with your server's port

    // if (client.connect(server, port)) 
    // {
    //     client.println("GET /manualWaterStatus HTTP/1.1");
    //     client.println("Host: " + String(server));
    //     client.println("Connection: close");
    //     client.println();
    //     unsigned long startTimems = millis();
    //     while (client.connected() && millis() - startTimems < 1000)
    //     {
    //         if (client.available()) 
    //         {
    //             String line = client.readStringUntil('\n');
    //             if (line == "1") 
    //             {
    //                 manualWateringRequest = true;
    //                 break;
    //             } 
    //             else if (line == "0") 
    //             {
    //                 manualWateringRequest = false;
    //                 break;
    //             }
    //         }
    //     }
    //     Serial.print("manualWateringRequest: ");
    //     Serial.println(manualWateringRequest);
    //     client.stop();
    //     //setManualWaterStatus(manualWateringRequest);
    // } 
    // else 
    // {
    //     Serial.println("connection failed in read_dropServer");
    //     client.stop();
    // }
    // delay(2500);


    // {
    //     WiFiClient client;
    //     //const char* server = "64.23.202.34";  // Replace with your server's IP address
    //     const int port = 3000;  // Replace with your server's port        

    //     if (client.connect(server, port)) {
    //         client.println("GET /autoWaterStatus HTTP/1.1");
    //         client.println("Host: " + String(server));
    //         client.println("Connection: close");
    //         client.println();
    //         unsigned long startTimems = millis();
    //         while (client.connected() && millis() - startTimems < 1000)
    //         {
    //             if (client.available()) {
    //                 String line = client.readStringUntil('\n');
    //                 if (line == "1") 
    //                 {
    //                     autoWateringRequest = true;
    //                     break;
    //                 } 
    //                 else if (line == "0") 
    //                 {
    //                     autoWateringRequest = false;
    //                     break;
    //                 }
    //             }
    //         }
    //         Serial.print("autoWateringRequest: ");
    //         Serial.println(autoWateringRequest);
    //         client.stop();
    //         //setAutolWaterStatus(autoWateringRequest);
    //     } 
    //     else 
    //     {
    //         Serial.println("connection failed in read_dropServer");
    //         client.stop();
    //     }
    // }
}


void read_dropServer(bool* autoWateringRequest, bool* manualWaterOverrideRequest)
{
    WiFiClient client;
    //     if (client.connect(server, port)) {
    //         client.println("GET /autoWaterStatus HTTP/1.1");
    //         client.println("Host: " + String(server));
    //         client.println("Connection: close");
    //         client.println();
    //         unsigned long startTimems = millis();
    //         while (client.connected() && millis() - startTimems < 1000)

    if (client.connect(server, port)) {
        client.println("GET /status HTTP/1.1"); // client.println("GET /autoWaterStatus HTTP/1.1");
        client.println("Host: " + String(server));
        client.println("Connection: close");
        client.println();
        unsigned long startTimems = millis();
        while (client.connected() && millis() - startTimems < 5000)
        {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                if (line.startsWith("{\"manualWaterOverride\":")) 
                {
                    int manualWaterOverrideStart = line.indexOf(":") + 1;
                    int manualWaterOverrideEnd = line.indexOf(",");
                    String manualWaterOverride = line.substring(manualWaterOverrideStart, manualWaterOverrideEnd);
                    manualWaterOverride.trim();
                    int autoWaterStatusStart = line.lastIndexOf(":") + 1;
                    int autoWaterStatusEnd = line.lastIndexOf("}");
                    String autoWaterStatus = line.substring(autoWaterStatusStart, autoWaterStatusEnd);
                    autoWaterStatus.trim();

                    *autoWateringRequest = (autoWaterStatus == "true");
                    *manualWaterOverrideRequest = (manualWaterOverride == "true");
                    break;
                }
            }
        }
        Serial.print("autoWateringRequest: ");
        Serial.println(*autoWateringRequest);
        Serial.print("manualWaterOverrideRequest: ");
        Serial.println(*manualWaterOverrideRequest);
        client.stop();
    } 
    else 
    {
        Serial.println("connection failed in read_dropServer");
        client.stop();
    }
}