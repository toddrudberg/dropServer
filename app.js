const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 3000;

app.use(bodyParser.json());

const csvFilePath = path.join(__dirname, 'data_log.csv');

const csvHeaders = [
  'DateStamp', 'TimeStamp', 'Epoch', 'OutsideAirTemp', 'OutsideHumidity', 
  'OutsideBaro', 'SoilTemperature', 'SoilElectricalConductivity', 
  'SoilHumidity', 'SoilPh', 'Watering', 'WifiError', 'SDError', 'RTCFailed'
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
    logData.SoilHumidity, logData.SoilPh, logData.Watering, 
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

app.listen(port, () => {
  console.log(`App listening at http://localhost:${port}`);
  console.log('CSV file path:', csvFilePath);
});
