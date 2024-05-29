#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>


const char* ssid = "Turkey Point";
const char* password = "gobblegobble";
const char* remoteServer = "192.168.1.31"; // address of your server
//const char* remoteServer = "64.23.202.34"; // address of your server
const int remoteServerPort = 3000; // port of your server

//char server[] = "64.23.202.34"; // address of your server


int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiClient client;

bool gRefreshRequest = false;

void setup() {
    Serial.begin(9600); // Initialize serial communication at 9600 baud    
    // attempt to connect to WiFi network:
    // Set static IP address
    IPAddress ip(192, 168, 1, 8); // Replace with your desired IP
    IPAddress gateway(192, 168, 1, 1); // Replace with your gateway address
    IPAddress subnet(255, 255, 255, 0); // Replace with your subnet mask
    IPAddress dns(8, 8, 8, 8); // Optional: Replace with your preferred DNS server's IP

    WiFi.config(ip, dns, gateway, subnet);
    
    WiFi.begin(ssid, password);

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




    manageDropServer();




}


//bool manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime)
bool manageDropServer()
{
        // if there's a successful connection:
    // Build the JSON data
    static bool autoWateringRequest = false;
    static bool manualWateringRequest = false;
    bool watering = manualWateringRequest;
    static bool refreshRequest = false;
    static bool autoWateringRequestlast = false;
    static bool manualWateringRequestlast = false;

    static unsigned long lastUpdate = 0;
    static unsigned long lastLogtime = 0;
    static int state = 0;
    bool serverConnection = true;
    bool updatetheServer;
    if (millis() - lastUpdate > 10000) 
    {
        switch (state) 
        {

            case 0:

                //serverConnection = read_dropServer(&autoWateringRequest, &manualWateringRequest);
                read_dropServer(&autoWateringRequest, &manualWateringRequest, &refreshRequest);
                // setAutolWaterStatus(autoWateringRequest);
                // setManualWaterStatus(manualWateringRequest);
                updatetheServer  =  autoWateringRequest != autoWateringRequestlast || 
                                    manualWateringRequest != manualWateringRequestlast|| 
                                    refreshRequest != gRefreshRequest;
                if(millis() - lastLogtime > 60000 || updatetheServer)
                {
                    gRefreshRequest = refreshRequest;
                    state++;
                }
                break;
            case 1:
                //serverConnection = update_dropServer(soilSensorData, epochTime);
                update_dropServer(autoWateringRequest, manualWateringRequest, watering);
                lastLogtime = millis();
                state = 0;
                break;
        }
        lastUpdate = millis();
    }
    return serverConnection;
}

void update_dropServer(bool autoWateringRequest, bool manualWateringRequest, bool watering)
{
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
    
    if (client.connect(remoteServer, remoteServerPort)) {
        // send the HTTP POST request:
        client.println("POST /log HTTP/1.1");
        client.println("Host: " + String(remoteServer));
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
}

bool read_dropServer(bool* autoWateringRequest, bool* manualWaterOverrideRequest, bool* aRefreshRequest)
{
    bool connectionSolid = true;
    WiFiClient client;

    if (client.connect(remoteServer, remoteServerPort)) {
        client.println("GET /status HTTP/1.1");
        client.println("Host: " + String(remoteServer));
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

                    int autoWaterStatusStart = line.indexOf(":", manualWaterOverrideEnd) + 1;
                    int autoWaterStatusEnd = line.indexOf(",", manualWaterOverrideEnd);
                    String autoWaterStatus = line.substring(autoWaterStatusStart, autoWaterStatusEnd);
                    autoWaterStatus.trim();

                    int gRefreshRequestStart = line.lastIndexOf(":") + 1;
                    int gRefreshRequestEnd = line.lastIndexOf("}");
                    String refreshRequest = line.substring(gRefreshRequestStart, gRefreshRequestEnd);
                    refreshRequest.trim();

                    *autoWateringRequest = (autoWaterStatus == "true");
                    *manualWaterOverrideRequest = (manualWaterOverride == "true");
                    *aRefreshRequest = (refreshRequest == "true");
                    break;
                }
            }
        }
        Serial.print("autoWateringRequest: ");
        Serial.println(*autoWateringRequest);
        Serial.print("manualWaterOverrideRequest: ");
        Serial.println(*manualWaterOverrideRequest);
        Serial.print("gRefreshRequest: ");
        Serial.println(*aRefreshRequest);
        client.stop();
    } 
    else 
    {
        connectionSolid = false;
        Serial.println("connection failed in read_dropServer");
        client.stop();
    }
    return connectionSolid;
}