#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h> 
#include <ArduinoJson.h>
#define DEBUGER

const char* ssid = "Turkey Point";
const char* password = "gobblegobble";
const char* remoteServer = "192.168.1.31"; // address of your server
//const char* remoteServer = "64.23.202.34"; // address of your server
const int remoteServerPort = 3000; // port of your server

//char server[] = "64.23.202.34"; // address of your server


int status = WL_IDLE_STATUS;     // the Wifi radio's status
WiFiClient client;

bool gRefreshRequest = false;
bool gAutoWateringEnabled = false;
bool gAutoWateringCycleOn = false;
bool gManualWateringOn = false;
unsigned long gWateringTimeStart = 0;
unsigned long gWateringDuration = 0;
bool rtcFailed = false;
bool wifiConnectionFailed = false;

struct sSoilSensorData
{
  char* dateStamp;
  char* timeStamp;
  unsigned long epochTime;
  float outsideAirTemp;
  float outsideAirHumidity;
  float baroPressure;
  float soilTemperature;
  float soilElectricalConductivity;
  float soilMoisture;
  float soilPh;
}; 

struct sTotalState
{
  sSoilSensorData soilSensorData;
  unsigned long wateringTimeStart;
  unsigned long wateringDuration;
  bool watering;
  bool autoWaterCycleActive;
};

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



    sSoilSensorData soilSensorData;
    soilSensorData.dateStamp = "2023-05-25";
    soilSensorData.timeStamp = "12:34:56";
    soilSensorData.epochTime = 123456;
    soilSensorData.outsideAirTemp = 22.5;
    soilSensorData.outsideAirHumidity = 45.2;
    soilSensorData.baroPressure = 30.12;
    soilSensorData.soilTemperature = 20.3;
    soilSensorData.soilElectricalConductivity = 1.2;
    soilSensorData.soilMoisture = 30.1;
    soilSensorData.soilPh = 6.5;

    time_t epochTime = 123456;


    manageDropServer(&soilSensorData, epochTime);

}


//bool manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime)
bool manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    const unsigned long serverRest = 10000; //general rest time between server requests
    const unsigned long serverUpdateLogDelay = 1000; //if a state change, we wait this long before sending the update.  must be smaller than serverRest
    const unsigned long serverUpdateLogInterval = 60000; //generally, we log every 60 seconds
    static unsigned long lastServerRequestTime = 0;
    static unsigned long lastLogtime = 0;
    static int state = 0;
    bool serverConnection = true;
    bool manualWateringRequest = gManualWateringOn;
    bool autoWateringRequest = gAutoWateringEnabled;
    bool refreshRequest = false;

    static bool refreshRequestLast = false;
    static bool autoWateringRequestlast = false;
    static bool manualWateringRequestlast = false;

    bool updateServer = false;
    if (millis() - lastServerRequestTime > serverRest) 
    {
        switch (state) 
        {
            case 0:
                serverConnection = read_dropServer(&autoWateringRequest, &manualWateringRequest, &refreshRequest);
                Serial.println(" ");
                Serial.println("Manual Watering Request: " + String(manualWateringRequest ? "True" : "False"));
                Serial.println("Auto Watering Request: " + String(autoWateringRequest ? "True" : "False"));
                Serial.println("Refresh Request: " + String(refreshRequest ? "True" : "False"));
                Serial.println(" ");
                if(serverConnection)
                {
                    setAutolWaterStatus(autoWateringRequest);
                    setManualWaterStatus(manualWateringRequest);
                    updateServer  =  autoWateringRequest != autoWateringRequestlast || 
                                        manualWateringRequest != manualWateringRequestlast|| 
                                        refreshRequest != refreshRequestLast ;
                    if(millis() - lastLogtime > serverUpdateLogInterval || updateServer)
                    {
                        autoWateringRequestlast = autoWateringRequest;
                        manualWateringRequestlast = manualWateringRequest;
                        refreshRequestLast = refreshRequest;
                        state++;
                    }
                }
                lastServerRequestTime = updateServer ? millis() + serverRest - serverUpdateLogDelay : millis();
                break;
            case 1:
                serverConnection = update_dropServer(soilSensorData, epochTime);                
                state = 0;
                lastLogtime = millis();
                lastServerRequestTime = millis();
                break;
            default:
                state = 0;
                lastServerRequestTime = millis();
                break;
        }

    }
    return serverConnection;
}

// app.get('/status', (req, res) => {
//   const manualWaterOverridePath = path.join(__dirname, 'manualWaterOverride.csv');
//   const autoWaterStatusPath = path.join(__dirname, 'autoWaterStatus.csv');

//   fs.readFile(manualWaterOverridePath, 'utf8', (err, manualWaterOverride) => {
//     if (err) {
//       console.error('Error reading manualWaterOverride.csv', err);
//       return res.status(500).send('Error reading manual water override status');
//     }

//     fs.readFile(autoWaterStatusPath, 'utf8', (err, autoWaterStatus) => {
//       if (err) {
//         console.error('Error reading autoWaterStatus.csv', err);
//         return res.status(500).send('Error reading auto water status');
//       }
//       gRefreshRequest = !gRefreshRequest;

//       const manualWaterOverrideStatus = manualWaterOverride.trim().toLowerCase() === 'true';
//       const autoWaterStatusStatus = autoWaterStatus.trim().toLowerCase() === 'true';
//       console.log(" ");
//       console.log('-------------------')
//       console.log('Status request received');
//       console.log(`Manual Water Override Status: ${manualWaterOverrideStatus}`);
//       console.log(`Auto Water Status: ${autoWaterStatusStatus}`);
//       console.log(`Refresh Request: ${gRefreshRequest}`);
//       console.log('-------------------');

//       res.json({
//         manualWaterOverride: manualWaterOverrideStatus,
//         autoWaterStatus: autoWaterStatusStatus,
//         gRefreshRequest: gRefreshRequest
//       });
//     });
//   });
// });

DynamicJsonDocument doc(512);
char jsonString[512];
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
                    DynamicJsonDocument doc(1024);
                    deserializeJson(doc, line);

                    *autoWateringRequest = doc["autoWaterStatus"];
                    *manualWaterOverrideRequest = doc["manualWaterOverride"];
                    *aRefreshRequest = doc["gRefreshRequest"];
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


//std::string tag;
//std::string currentLine;
sTotalState totalState;
bool update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    Serial.println("update_dropServer...");
    bool connectionSolid = true;
    totalState.soilSensorData = *soilSensorData;
    totalState.wateringTimeStart = gWateringTimeStart;
    totalState.wateringDuration = gWateringDuration;
    totalState.watering = gManualWateringOn || gAutoWateringCycleOn;
    totalState.autoWaterCycleActive = gAutoWateringEnabled;
    // Create a JSON document
    
    // Convert the epoch time to a struct tm
    struct tm *timeStruct = localtime(&epochTime);
    // Create a buffer to hold the time string
    char dateBuffer[11];
    char timeBuffer[9];
    // Format the struct tm as a date string
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeStruct);

    // Format the struct tm as a time string
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeStruct);

    // Add the time string to the JSON document
    doc["DateStamp"] = dateBuffer;
    doc["TimeStamp"] = timeBuffer;
    doc["Epoch"] = epochTime;
    doc["OutsideAirTemp"].set(round(totalState.soilSensorData.outsideAirTemp * 10.0) / 10.0);
    doc["OutsideHumidity"].set(round(totalState.soilSensorData.outsideAirHumidity * 10.0) / 10.0);
    doc["OutsideBaro"].set(round(totalState.soilSensorData.baroPressure * 100.0) / 100.0);
    doc["SoilTemperature"].set(round(totalState.soilSensorData.soilTemperature * 10.0) / 10.0);
    doc["SoilElectricalConductivity"].set(round(totalState.soilSensorData.soilElectricalConductivity * 10.0) / 10.0);
    doc["SoilHumidity"].set(round(totalState.soilSensorData.soilMoisture * 10.0) / 10.0);
    doc["SoilPh"].set(round(totalState.soilSensorData.soilPh * 10.0) / 10.0);
    doc["Watering"] = totalState.watering;
#ifndef DEBUGER
    float wateringTimeRemaining = (totalState.wateringDuration - (logger.getUnixTime() - totalState.wateringTimeStart)) / 60.0;
#else
    float wateringTimeRemaining = (totalState.wateringDuration - (epochTime - totalState.wateringTimeStart)) / 60.0;
#endif
    if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
        wateringTimeRemaining = 0;
    }
    doc["autoWaterCycleEnabled"] = totalState.autoWaterCycleActive;
    doc["TimeRemaining"] = wateringTimeRemaining;
    doc["WifiError"] = wifiConnectionFailed;
#ifndef DEBUGER
    doc["SDError"] = !SD.exists(FileName);
#else
    doc["SDError"] = false;
#endif
    doc["RTCFailed"] = rtcFailed;

    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("update_dropServer: WiFi not connected. Attempting to reconnect...");

    }

    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    Serial.println(strlen(jsonString));
    WiFiClient client;
    if (client.connect(remoteServer, remoteServerPort)) {
        // send the HTTP POST request:
        client.println("POST /log HTTP/1.1");
        client.println("Host: " + String(remoteServer));
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        // calculate the length of the JSON data
        client.print("Content-Length: ");
        client.println(strlen(jsonString));
        client.println();
        // send the JSON data:
        client.println(jsonString);
        Serial.println("Data Sent");
        client.stop();  // Ensure the client is stopped
    } 
    else {
        // if you couldn't make a connection:
        connectionSolid = false;
        Serial.println("connection failed in update_dropServer");
        client.stop();  // Ensure the client is stopped
    }
    return connectionSolid;
}

void setManualWaterStatus(bool request)
{
    bool startWaterReceived = request;

    if(startWaterReceived && !gManualWateringOn)
    {
        gManualWateringOn = true;
        gWateringDuration = 60 * 10; // 10 minutes
        gWateringTimeStart = millis();
    }
    else if(!startWaterReceived)
    {
        gManualWateringOn = false;
    }
}

void setAutolWaterStatus(bool request)
{
    // we may make more logic here...so we will keep this function
    bool startWaterReceived = request;
    gAutoWateringEnabled = startWaterReceived;
}
