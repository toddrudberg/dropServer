# dropServer
This is the digital ocean webserver for the gardenBot project

# to build the application
docker build -t dropserver .

# to run the container we use this command:
docker run -d -p 3000:3000 --name the-dropserver dropserver

# to view running containers:
docker ps
# to view all containers
docker ps -a  

# to view the logs:
docker logs the-dropserver

# to stop it:
docker stop the-dropserver

# to remove it:
docker rm the-dropserver

# to view docker logs
docker logs the-dropserver

# json data:
char jsonData[] = "{"
0    "\"DateStamp\": \"2023-05-25\","
1    "\"TimeStamp\": \"12:34:56\","
2    "\"Epoch\": 1679850896,"
3    "\"OutsideAirTemp\": 22.5,"
4    "\"OutsideHumidity\": 45.2,"
5    "\"OutsideBaro\": 1013.1,"
6    "\"SoilTemperature\": 20.3,"
7    "\"SoilElectricalConductivity\": 1.2,"
8    "\"SoilHumidity\": 30.1,"
9    "\"SoilPh\": 6.5,"
10    "\"Watering\": true,"
11    "\"TimeRemaining\": 120,"
12    "\"WifiError\": false,"
13    "\"SDError\": false,"
14    "\"RTCFailed\": false"
"}";

