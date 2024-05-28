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
  'SoilHumidity', 'SoilPh', 'Watering', 'TimeRemaining', 'autoWaterCycleEnabled', 'WifiError', 'SDError', 'RTCFailed'
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
    logData.SoilHumidity, logData.SoilPh, logData.Watering, logData.TimeRemaining, logData.autoWaterCycleEnabled,
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

const autoWaterStatusFilePath = path.join(__dirname, 'autoWaterStatus.csv');

app.post('/enableAutoWater', (req, res) => {
  try {
    fs.writeFileSync(autoWaterStatusFilePath, 'true');
    console.log('Auto water enabled - command from iPhone');
    res.status(200).send('Auto water enabled');
  } catch (err) {
    console.error('Error writing to autoWaterStatus.csv', err);
    res.status(500).send('Error enabling auto water');
  }
});

app.post('/disableAutoWater', (req, res) => {
  try {
    fs.writeFileSync(autoWaterStatusFilePath, 'false');
    console.log('Auto water disabled');
    res.status(200).send('Auto water disabled');
  } catch (err) {
    console.error('Error writing to autoWaterStatus.csv', err);
    res.status(500).send('Error disabling auto water');
  }
});

app.get('/autoWaterStatus', (req, res) => {
  fs.readFile(autoWaterStatusFilePath, 'utf8', (err, data) => {
    if (err) {
      console.error('Error reading from autoWaterStatus.csv', err);
      res.status(200).send('-1');
    } else {
      console.log('Auto water status:', data);
      if (data.trim() === 'true') {
        res.status(200).send('1');
      } else {
        res.status(200).send('0');
      }
    }
  });
});

const manualWaterOverrideFilePath = path.join(__dirname, 'manualWaterOverride.csv');

app.post('/enableManualWater', (req, res) => {
  try {
    fs.writeFileSync(manualWaterOverrideFilePath, 'true');
    console.log('Manual water enabled');
    res.status(200).send('Manual water enabled');
  } catch (err) {
    console.error('Error writing to manualWaterOverride.csv', err);
    res.status(500).send('Error enabling manual water');
  }
});

app.post('/disableManualWater', (req, res) => {
  try {
    fs.writeFileSync(manualWaterOverrideFilePath, 'false');
    console.log('Manual water disabled');
    res.status(200).send('Manual water disabled');
  } catch (err) {
    console.error('Error writing to manualWaterOverride.csv', err);
    res.status(500).send('Error disabling manual water');
  }
});

app.get('/manualWaterStatus', (req, res) => {
  fs.readFile(manualWaterOverrideFilePath, 'utf8', (err, data) => {
    if (err) {
      console.error('Error reading from manualWaterOverride.csv', err);
      res.status(200).send('-1');
    } else {
      console.log('Manual water status:', data);
      if (data.trim() === 'true') {
        res.status(200).send('1');
      } else {
        res.status(200).send('0');
      }
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

      // 0 doc["DateStamp"] = dateBuffer;
      // 1 doc["TimeStamp"] = timeBuffer;
      // 2 doc["Epoch"] = epochTime;
      // 3 doc["OutsideAirTemp"].set(round(totalState.soilSensorData.outsideAirTemp * 10.0) / 10.0);
      // 4 doc["OutsideHumidity"].set(round(totalState.soilSensorData.outsideAirHumidity * 10.0) / 10.0);
      // 5 doc["OutsideBaro"].set(round(totalState.soilSensorData.baroPressure * 100.0) / 100.0);
      // 6 doc["SoilTemperature"].set(round(totalState.soilSensorData.soilTemperature * 10.0) / 10.0);
      // 7 doc["SoilElectricalConductivity"].set(round(totalState.soilSensorData.soilElectricalConductivity * 10.0) / 10.0);
      // 8 doc["SoilHumidity"].set(round(totalState.soilSensorData.soilMoisture * 10.0) / 10.0);
      // 9 doc["SoilPh"].set(round(totalState.soilSensorData.soilPh * 10.0) / 10.0);
      // 10 doc["Watering"] = totalState.watering;
      // float wateringTimeRemaining = (totalState.wateringDuration - (logger.getUnixTime() - totalState.wateringTimeStart)) / 60.0;
      // if (wateringTimeRemaining < 0 || wateringTimeRemaining > 100000) {
      //     wateringTimeRemaining = 0;
      // }
      // 11 doc["autoWaterCycleEnabled"] = totalState.autoWaterCycleEnabled;
      // 12 doc["TimeRemaining"] = wateringTimeRemaining;
      // 13 doc["WifiError"] = wifiConnectionFailed;
      // 14 doc["SDError"] = !SD.exists(FileName);
      // 15 doc["RTCFailed"] = rtcFailed;

      const response = {
        Time: data[1],
        Date: data[0],
        OAT: Math.round(parseFloat(data[3]) * 10) / 10, 
        OAH: Math.round(parseFloat(data[4]) * 10) / 10,
        BP: Math.round(parseFloat(data[5]) * 100) / 100,
        SM: Math.round(parseFloat(data[8]) * 10) / 10,
        ST: Math.round(parseFloat(data[6]) * 10) / 10,
        SEC: Math.round(parseFloat(data[7]) * 10) / 10,
        SPH: Math.round(parseFloat(data[9]) * 10) / 10,
        WATERING: data[10] === 'true',
        WATERINGTIMEREMAINING: parseFloat(data[11]),
        AUTO: data[12] === 'true'
      };

      res.json(response);
      //console.log('Last line:', response);
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
