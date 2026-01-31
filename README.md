# dropServer
DigitalOcean webserver for the GardenBot project (Docker container).

## Deploy / Update on the droplet
Login to DigitalOcean → open Console (or SSH).

```bash
docker rm -f remote_dropserver
docker pull turkeypoint/dropserver:latest
docker run -d --restart unless-stopped -p 3000:3000 --name remote_dropserver turkeypoint/dropserver:latest

docker logs --tail 50 remote_dropserver
docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"
ss -ltnp | grep :3000 || echo "not listening on 3000"
curl -s http://127.0.0.1:3000/status && echo

## to update the docker container:
# to production deploy:
docker buildx build \
  --platform linux/amd64,linux/arm64 \
  -t turkeypoint/dropserver:latest \
  --push .

# local test only (no push)
docker build -t turkeypoint/dropserver:local .


ssh root@64.23.202.34
curl http://64.23.202.34:3000/status
curl http://64.23.202.34:3000/Refresh

curl http://64.23.202.34:3000/last-row
curl http://64.23.202.34:3000/enableManualWater
curl http://64.23.202.34:3000/disableManualWater


curl http://192.168.1.31:3000/last-row
curl -X GET http://localhost:3000/logs --output /users/toddrudberg/downloads/logs.csv
curl -X GET http://64.23.202.34:3000/logs --output /users/toddrudberg/downloads/logs.csv  (you have to delete any logs that might be there, this won't overwrite them)

ssh-keygen -t ed25519 -C "todd.rudberg@icloud.com"

# to log into the drop server
ssh root@64.23.202.34



# i think we don't need to run these anymore, but they allow us to do multi-architecture builds:
docker buildx create --use
docker buildx inspect --bootstrap

# to run the container we use this command locally:
docker run -d -p 3000:3000 --name local_dropserver dropserver

# to deploy and run on the droplet:
docker pull turkeypoint/dropserver:latest
docker run -d -p 3000:3000 --name remote_dropserver turkeypoint/dropserver:latest

# steps to follow to change the remote dropserver:
I think you have to stop the container, 
remove the container, 
then build the container, 
then deploy and run.  

I literally have no idea how this PC connects to the remote dropserver, but if you log into digital ocean you can access a terminal and it seems to magically work. 



# to execute docker commands on the remote server
docker exec -it remote_dropserver ls -l
docker cp remote_dropserver:data_log.csv gardenBot:root
ssh root@64.23.202.34 docker cp remote_dropserver:data_log.csv /root/
scp -P 22 root@64.23.202.34:data_log.csv /users/toddrudberg/downloads/



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

# to view logs live:
docker logs -f <container_id_or_name>


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

# more docker tidbits:
# SSH into Your Droplet
ssh root@your_droplet_ip

# Docker Commands

# List Running Containers
docker ps

# View Logs of a Specific Container
docker logs <container_id_or_name>

# Follow Logs in Real-Time
docker logs -f <container_id_or_name>

# Open a Shell in a Running Container
docker exec -it <container_id_or_name> /bin/bash

# List All Containers (including stopped ones)
docker ps -a

# Remove a Container
docker rm <container_id_or_name>

# Remove an Image
docker rmi <image_id_or_name>

# Linux Commands

# List Files and Directories
ls

# Change Directory
cd <directory_name>

# View File Contents
cat <file_name>

# Edit Files (using nano)
nano <file_name>

# View System Logs
tail -f /var/log/syslog

# Useful Tips

# Keep Your System Updated
apt-get update && apt-get upgrade

# Backup Important Data
# (Ensure you have backups for critical data and configuration files)

# Use Docker Volumes to Persist Data
docker run -d -p 3000:3000 -v /path/on/host:/path/in/container dropserver

# Monitor Your Server
# (Use monitoring tools to keep an eye on resource usage and application performance)
