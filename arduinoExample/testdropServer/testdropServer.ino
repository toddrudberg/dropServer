#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h> 
#include <ArduinoJson.h>


// Include necessary libraries at the top of your file
#include <NTPClient.h> // Ensure this library is installed in your Arduino IDE
#include <RTClib.h> // Ensure this library is installed in your Arduino IDE
//#include <WiFi.h> // Include WiFi library for WiFi functions


#define DEBUGER

const char* ssid = "Turkey Point";
const char* password = "gobblegobble";
const char* remoteServer = "192.168.1.35"; // address of your server
//const char* remoteServer = "64.23.202.34"; // address of your server
const int remoteServerPort = 3000; // port of your server

//char server[] = "64.23.202.34"; // address of your server


int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient dropServerClient;

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

sTotalState totalState;

void setup() {
    Serial.begin(9600); // Initialize serial communication at 9600 baud    
    // attempt to connect to WiFi network:
    // Set static IP address
    IPAddress ip(192, 168, 1, 8); // Replace with your desired IP
    IPAddress gateway(192, 168, 1, 1); // Replace with your gateway address
    IPAddress subnet(255, 255, 255, 0); // Replace with your subnet mask
    IPAddress dns(8, 8, 8, 8); // Optional: Replace with your preferred DNS server's IP

    if(true)
    {
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
    }

    delay(1000);
}

void loop() 
{
    static unsigned long lastPrintTime = 0;
    static int dropServerFailCount = 0;

    if (false && (millis() - lastPrintTime >= 5000)) 
    {
        Serial.print("dropServerFailCount: ");
        Serial.println(dropServerFailCount);
        lastPrintTime = millis();
    }

    sSoilSensorData soilSensorData;
    soilSensorData.dateStamp = "2023-05-25";
    soilSensorData.timeStamp = "12:34:56";
    soilSensorData.epochTime = 123456;
    soilSensorData.outsideAirTemp = 22.5;
    soilSensorData.outsideAirHumidity = 45.2;
    soilSensorData.baroPressure = 30.12;
    soilSensorData.soilTemperature = 20.3;
    soilSensorData.soilElectricalConductivity = 1.2;
    soilSensorData.soilMoisture = 22.5;
    soilSensorData.soilPh = 6.5;

    time_t myTime = 123456;
    static unsigned long epochTime = 0;

    CheckNtpTime(&epochTime);

    //copy epocTime to myTime
    myTime = epochTime;

    bool dropServerFailed = manageDropServer(&soilSensorData, myTime);
    if(!dropServerFailed)
    {
        dropServerFailCount++;
    }

    //void manageWateringValves(time_t myTime, sSoilSensorData* soilSensorData)
    manageWateringValves(myTime, &soilSensorData);
}



//bool manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime)
bool manageDropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    const unsigned long serverRest = 5000; //general rest time between server requests
    const unsigned long serverUpdateLogDelay = 500; //if a state change, we wait this long before sending the update.  must be smaller than serverRest
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

        //if (dropServerClient.connect(remoteServer, remoteServerPort)) 
    if (millis() - lastServerRequestTime > serverRest) 
    {
        if(!dropServerClient.connected())
        {
            if(!dropServerClient.connect(remoteServer, remoteServerPort))
            {
                return false;
            }
        }
        switch (state) 
        {
            case 0:
                serverConnection = read_dropServer(&autoWateringRequest, &manualWateringRequest, &refreshRequest);
                if( false )
                {
                Serial.println(" ");
                Serial.println("Manual Watering Request: " + String(manualWateringRequest ? "True" : "False"));
                Serial.println("Auto Watering Request: " + String(autoWateringRequest ? "True" : "False"));
                Serial.println("Refresh Request: " + String(refreshRequest ? "True" : "False"));
                Serial.println(" ");
                }
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
                        lastServerRequestTime = updateServer ? millis() + serverRest - serverUpdateLogDelay : millis();
                        state++;
                    }
                    else
                    {
                        // no immediate requests, so stop the dropServerClient. 
                        dropServerClient.stop();
                    }
                }
                else
                {
                    //something wrong, let's stop the dropServerClient
                    dropServerClient.stop();
                }
                //lastServerRequestTime = millis();  //this is more conservative. we will always wait serverRest time before trying again
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

DynamicJsonDocument doc(512);
char jsonString[512];
bool read_dropServer(bool* autoWateringRequest, bool* manualWaterOverrideRequest, bool* aRefreshRequest)
{
    bool connectionSolid = true;

    //if (dropServerClient.connect(remoteServer, remoteServerPort)) 
    if( dropServerClient.connected() )
    {
        dropServerClient.println("GET /status HTTP/1.1");
        dropServerClient.println("Host: " + String(remoteServer));
        dropServerClient.println("Connection: close");
        dropServerClient.println();
        unsigned long startTimems = millis();
        while (dropServerClient.connected() && millis() - startTimems < 5000)
        {
            if (dropServerClient.available()) {
                String line = dropServerClient.readStringUntil('\n');
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
        if( false )
        {
        Serial.print("autoWateringRequest: ");
        Serial.println(*autoWateringRequest);
        Serial.print("manualWaterOverrideRequest: ");
        Serial.println(*manualWaterOverrideRequest);
        Serial.print("gRefreshRequest: ");
        Serial.println(*aRefreshRequest);
        //dropServerClient.stop();
        }
    } 
    else 
    {
        connectionSolid = false;
        Serial.println("connection failed in read_dropServer");
        //dropServerClient.stop();
    }
    return connectionSolid;
}

bool update_dropServer(sSoilSensorData* soilSensorData, time_t epochTime)
{
    if(false ) 
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
    doc["WifiError"] = WiFi.status() != WL_CONNECTED;
#ifndef DEBUGER
    doc["SDError"] = !SD.exists(FileName);
#else
    doc["SDError"] = false;
#endif
    doc["RTCFailed"] = rtcFailed;

#ifndef DEBUGER
    if (WiFi.status() != WL_CONNECTED) 
    {
        Serial.println("update_dropServer: WiFi not connected. Attempting to reconnect...");
    }
#endif

    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    Serial.println(strlen(jsonString));

    //if (dropServerClient.connect(remoteServer, remoteServerPort)) 
    if( dropServerClient.connected())
    {
        // send the HTTP POST request:
        dropServerClient.println("POST /log HTTP/1.1");
        dropServerClient.println("Host: " + String(remoteServer));
        dropServerClient.println("Content-Type: application/json");
        dropServerClient.println("Connection: close");
        // calculate the length of the JSON data
        dropServerClient.print("Content-Length: ");
        dropServerClient.println(strlen(jsonString));
        dropServerClient.println();
        // send the JSON data:
        dropServerClient.println(jsonString);
        Serial.println("Data Sent");
        //dropServerClient.stop();  // Ensure the client is stopped
    } 
    else {
        // if you couldn't make a connection:
        connectionSolid = false;
        Serial.println("connection failed in update_dropServer");
        //dropServerClient.stop();  // Ensure the client is stopped
    }
    return connectionSolid;
}

void setManualWaterStatus(bool request)
{
    bool startWaterReceived = request;
    static bool startWaterReceivedLast = false;

    if(startWaterReceived && !gManualWateringOn && !startWaterReceivedLast)
    {
        gManualWateringOn = true;
        gWateringDuration = 60 * 10; // 10 minutes
#ifndef DEBUGER
        gWateringTimeStart = logger.getUnixTime();
#endif
    }
    else if(!startWaterReceived)
    {
        gManualWateringOn = false;
    }
    startWaterReceivedLast = startWaterReceived;
}

void setAutolWaterStatus(bool request)
{
    // we may make more logic here...so we will keep this function
    bool startWaterReceived = request;
    gAutoWateringEnabled = startWaterReceived;
}

const unsigned int Valve1 = 6;
const unsigned int Valve2 = 7;
const unsigned int Valve3 = 8;

void manageWateringValves(time_t myTime, sSoilSensorData* soilSensorData)
{
  static bool autoCycleActive = false;
  static unsigned long autoCycleStartTime = 0;
  static time_t displayData = myTime;

  struct tm *myTimeStruct = localtime(&myTime);
  bool startCycle09 = myTimeStruct->tm_hour == 9 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);
  bool startCycle14 = myTimeStruct->tm_hour == 14 && (myTimeStruct->tm_min >= 0 && myTimeStruct->tm_min < 1);
  bool startCycle17 = myTimeStruct->tm_hour == 16 && (myTimeStruct->tm_min >= 30 && myTimeStruct->tm_min < 31);

  float soilMoisture = soilSensorData->soilMoisture;

    // Ensure wateringDuration is initialized
  static unsigned long wateringDuration = 0; // Default duration


  if( difftime(myTime, displayData) > 5)
  {
    Serial.println();
    
    Serial.print("startCycle1: ");
    Serial.println(startCycle09);
    
    Serial.print("startCycle2: ");
    Serial.println(startCycle14);

        
    Serial.print("startCycle17: ");
    Serial.println(startCycle17);

    
    Serial.print("autoCycleActive: ");
    Serial.println(autoCycleActive);
    
    Serial.print("soilMoisture: ");
    Serial.println(soilSensorData->soilMoisture);

    // output the autoCycleStartTime
    Serial.print("autoCycleStartTime: ");
    Serial.println(autoCycleStartTime);
    //output the wateringDuration
    Serial.print("wateringDuration: ");
    Serial.println(wateringDuration);
    //output the remainting time:
    Serial.print("Remaining Time: ");
    unsigned long remainingTime = autoCycleActive ? wateringDuration - (millis() / 1000 - autoCycleStartTime) : 0;
    Serial.println(remainingTime);
    Serial.print("Current Time: ");
    Serial.println(millis() / 1000);

    
    tm *myTimeStruct = localtime(&myTime);
    Serial.print("Time: ");
    char timeStr[10]; // Correctly declare the buffer to hold the time string
    timeStr[9]; // Buffer to hold the time string
    sprintf(timeStr, "%02d:%02d:%02d", myTimeStruct->tm_hour, myTimeStruct->tm_min, myTimeStruct->tm_sec);
    Serial.println(timeStr);
    Serial.println();

    displayData = myTime;
  }



  if (!autoCycleActive && (startCycle09 || startCycle14 || startCycle17)) 
  {
    autoCycleStartTime = millis() / 1000;

    if (startCycle09 && soilMoisture < 28.0) 
    {
      float lowMoisture = 22.0; // 22% soil moisture - moisture is low
      float highMoisture = 28.0; // 28% soil moisture - moisture is high
      float lowDuration = 20.0; // 20 minutes if soil moisture is 22%
      float highDuration = 8.0; // 5 minutes if soil moisture is 28%
      const float m = (lowDuration - highDuration) / (lowMoisture - highMoisture);
      const float b = lowDuration - m * lowMoisture; 
      float wateringTime = m * soilMoisture + b;
      wateringDuration = 60 * wateringTime;
    } 
    else if ((startCycle14 || startCycle17) && soilMoisture < 25.0) 
    {
      wateringDuration = 60 * 5; // For both startCycle14 and startCycle17
    }

    // Activate the cycle if wateringDuration is set
    if (wateringDuration > 0) 
    {
      autoCycleActive = true;
    }
  }

  if (autoCycleActive && ((millis() / 1000 - autoCycleStartTime) > wateringDuration)) 
  {
    autoCycleActive = false;
  }

  //indicate to the global state that the auto watering cycle is active
  gAutoWateringCycleOn = autoCycleActive;

  if( gManualWateringOn || autoCycleActive )
  {
    digitalWrite(Valve1, HIGH);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
 #ifndef DEBUGER   
    if( logger.getUnixTime() - gWateringTimeStart > gWateringDuration)
    {
      gManualWateringOn = false;
    }
#endif
  }
  else
  {
    digitalWrite(Valve1, LOW);
    digitalWrite(Valve2, LOW);
    digitalWrite(Valve3, LOW);
#ifndef DEBUGER    
    gWateringTimeStart = logger.getUnixTime() - gWateringDuration;
#endif    
  }

}

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", -25200); // PDT offset: UTC-7 hours

bool CheckNtpTime(unsigned long *epochTime)
{
    static unsigned long startTime = millis();
    static unsigned long lastNtpTime = 0;

    unsigned long currentTime = millis();
    bool isTimeForUpdate = (currentTime - startTime > 3600000) || (lastNtpTime == 0);

    if (!isTimeForUpdate) 
    {
        *epochTime = lastNtpTime + ((currentTime - startTime) / 1000);
        return true;
    }

    if (WiFi.status() != WL_CONNECTED || !timeClient.update()) 
    {
        Serial.println("NTP time update failed.");
        delay(1000);
        return false;
    }

    unsigned long currentEpochTime = timeClient.getEpochTime();
    DateTime now = DateTime(currentEpochTime);
    if (now.year() < 2024 || now.year() > 2025) 
    {
        Serial.println("Invalid NTP time.");
        return false;
    }

    Serial.println("NTP time updated.");
    *epochTime = timeClient.getEpochTime();
    lastNtpTime = *epochTime;
    startTime = currentTime;

    return true;
}
