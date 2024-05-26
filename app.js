const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');
const { log } = require('console');

const app = express();
const port = 3000;

app.use(bodyParser.json());

const csvFilePath = path.join(__dirname, 'data_log.csv');

const csvHeaders = [
  'DateStamp', 'TimeStamp', 'Epoch', 'OutsideAirTemp', 'OutsideHumidity', 
  'OutsideBaro', 'SoilTemperature', 'SoilElectricalConductivity', 
  'SoilHumidity', 'SoilPh', 'Watering', 'TimeRemaining', 'WifiError', 'SDError', 'RTCFailed'
];

const csvWriter = require('csv-writer').createObjectCsvWriter;

app.post('/log', (req, res) => {
  const logData = req.body;

  console.log('CSV file path:', csvFilePath);
  console.log('File exists:', fs.existsSync(csvFilePath));

  if (!fs.existsSync(csvFilePath)) {
    const headerRow = csvHeaders.join(',') + '\n';
    fs.writeFileSync(csvFilePath, headerRow);
  }

  const csvWriter = fs.createWriteStream(csvFilePath, { flags: 'a' });
  const csvRow = [
    logData.DateStamp, logData.TimeStamp, logData.Epoch, 
    logData.OutsideAirTemp, logData.OutsideHumidity, logData.OutsideBaro, 
    logData.SoilTemperature, logData.SoilElectricalConductivity, 
    logData.SoilHumidity, logData.SoilPh, logData.Watering, logData.TimeRemaining,
    logData.WifiError, logData.SDError, logData.RTCFailed
  ].join(',') + '\n';

  csvWriter.write(csvRow, (err) => {
    if (err) {
      console.error('Error writing to CSV', err);
      res.status(500).send('Error logging data');
    } else {
      console.log('Data logged to CSV');
      res.status(200).send('Data received');
    }
  });
});

app.get('/logs', (req, res) => {
  res.download(csvFilePath, (err) => {
    if (err) {
      console.error('Error sending file:', err);
      res.status(500).send('Error sending file');
    }
  });
});


const readLastLines = require('read-last-lines');
const csvParser = require('csv-parser');

app.get('/last-row', (req, res) => {
  readLastLines.read(csvFilePath, 2)
    .then((lines) => {
      const lastLine = lines.split('\n')[0];
      const data = lastLine.split(',');

      // const epochTime = parseInt(data[2]);
      // const date = new Date(epochTime * 1000);
      // const dateStr = date.toISOString().split('T')[0];
      // const timeStr = date.toISOString().split('T')[1].split('.')[0];
      // # json data:
      // char jsonData[] = "{"
      // 0    "\"DateStamp\": \"2023-05-25\","
      // 1    "\"TimeStamp\": \"12:34:56\","
      // 2    "\"Epoch\": 1679850896,"
      // 3    "\"OutsideAirTemp\": 22.5,"
      // 4    "\"OutsideHumidity\": 45.2,"
      // 5    "\"OutsideBaro\": 1013.1,"
      // 6    "\"SoilTemperature\": 20.3,"
      // 7    "\"SoilElectricalConductivity\": 1.2,"
      // 8    "\"SoilHumidity\": 30.1,"
      // 9    "\"SoilPh\": 6.5,"
      // 10    "\"Watering\": true,"
      // 11    "\"TimeRemaining\": 120,"
      // 12    "\"WifiError\": false,"
      // 13    "\"SDError\": false,"
      // 14    "\"RTCFailed\": false"
      // "}";

      // struct ServerResponse: Codable 
      // {
      //     let date: String
      //     let time: String
      //     let oat: Float
      //     let oah: Float
      //     let oap: Float
      //     let sm: Float
      //     let st: Float
      //     let sec: Float
      //     let sph: Float
      //     let watering: Bool
      //     let wateringTimeRemaining: Float
      
      //     enum CodingKeys: String, CodingKey 
      //     {
      //         case date = "Date"
      //         case time = "Time"
      //         case oat = "OAT"
      //         case oah = "OAH"
      //         case oap = "BP"
      //         case sm = "SM"
      //         case st = "ST"
      //         case sec = "SEC"
      //         case sph = "SPH"
      //         case watering = "WATERING"
      //         case wateringTimeRemaining = "WATERINGTIMEREMAINING"
      //     }
      // }

      const response = {
        Time: data[1],
        Date: data[0],
        OAT: Math.round(parseFloat(data[3]) * 10) / 10, 
        OAH: Math.round(parseFloat(data[4]) * 10) / 10,
        BP: Math.round(parseFloat(data[5]) * 10) / 10,
        SM: Math.round(parseFloat(data[6]) * 10) / 10,
        ST: Math.round(parseFloat(data[7]) * 10) / 10,
        SEC: Math.round(parseFloat(data[8]) * 10) / 10,
        SPH: Math.round(parseFloat(data[9]) * 10) / 10,
        WATERING: data[10] === 'true',
        WATERINGTIMEREMAINING: parseFloat(data[11])
      };

      res.json(response);
      console.log('Last line:', response);
    })
    .catch((err) => {
      console.error('Error reading last line:', err);
      res.status(500).send('Error reading last line');
    });
});

//{"Time":"12:34:56","Date":"2023-05-25","OAT":22.5,"OAH":45.2,"BP":1013.1,"SM":20.3,"ST":1.2,"SEC":30.1,"SPH":6.5,"WATERING":true,"WATERINGTIMEREMAINING":120}% 


app.listen(port, () => {
  console.log(`App listening at http://localhost:${port}`);
});
