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

