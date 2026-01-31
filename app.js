const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');
const { log } = require('console');
const cors = require('cors');

const app = express();
const port = 3000;

app.use(express.static(path.join(__dirname, 'public')));

let gRefreshRequest = false;
let gRefreshRequestReceived = false;

app.use(cors()); 
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));


const csvFilePath = path.join(__dirname, 'data_log.csv');

const csvHeaders = [
  'DateStamp', 'TimeStamp', 'Epoch', 'OutsideAirTemp', 'OutsideHumidity', 
  'OutsideBaro', 'SoilTemperature', 'SoilElectricalConductivity', 
  'SoilHumidity', 'SoilPh', 'Watering', 'TimeRemaining', 'autoWaterCycleEnabled', 'WifiError', 'SDError', 'RTCFailed', 'avgOATYesterday'
];

const csvWriter = require('csv-writer').createObjectCsvWriter;

app.post('/log', (req, res) => {
  try {
    const logData = req.body;

    if (!fs.existsSync(csvFilePath)) {
      fs.writeFileSync(csvFilePath, csvHeaders.join(',') + '\n');
    }

    const csvRow = [
      logData.DateStamp, logData.TimeStamp, logData.Epoch,
      logData.OutsideAirTemp, logData.OutsideHumidity, logData.OutsideBaro,
      logData.SoilTemperature, logData.SoilElectricalConductivity,
      logData.SoilHumidity, logData.SoilPh, logData.Watering,
      logData.TimeRemaining, logData.autoWaterCycleEnabled,
      logData.WifiError, logData.SDError, logData.RTCFailed,
      logData.AvgTempPrevDay
    ].join(',') + '\n';

    fs.appendFile(csvFilePath, csvRow, (err) => {
      if (err) {
        console.error('Error writing to CSV', err);
        return res.status(500).send('Error logging data');
      }
      res.status(200).send('Data received');
    });

  } catch (err) {
    console.error('Unexpected /log error:', err);
    res.status(500).send('Server error');
  }
});


app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
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

//let's get manualWatherOverride status and autoWaterStatus in one call
//const fs = require('fs');
//const path = require('path');

app.get('/status', (req, res) => {
  gRefreshRequestReceived = gRefreshRequest;
  const manualWaterOverridePath = path.join(__dirname, 'manualWaterOverride.csv');
  const autoWaterStatusPath = path.join(__dirname, 'autoWaterStatus.csv');

  fs.readFile(manualWaterOverridePath, 'utf8', (err, manualWaterOverride) => {
    if (err) {
      console.error('Error reading manualWaterOverride.csv', err);
      return res.status(500).send('Error reading manual water override status');
    }

    fs.readFile(autoWaterStatusPath, 'utf8', (err, autoWaterStatus) => {
      if (err) {
        console.error('Error reading autoWaterStatus.csv', err);
        return res.status(500).send('Error reading auto water status');
      }

      const manualWaterOverrideStatus = manualWaterOverride.trim().toLowerCase() === 'true';
      const autoWaterStatusStatus = autoWaterStatus.trim().toLowerCase() === 'true';
      console.log(" ");
      console.log('-------------------')
      console.log('Status request received');
      console.log(`Manual Water Override Status: ${manualWaterOverrideStatus}`);
      console.log(`Auto Water Status: ${autoWaterStatusStatus}`);
      console.log(`Refresh Request: ${gRefreshRequest}`);
      console.log('-------------------');

      res.json({
        manualWaterOverride: manualWaterOverrideStatus,
        autoWaterStatus: autoWaterStatusStatus,
        gRefreshRequest: gRefreshRequest
      });
    });
  });
});

const readLastLines = require('read-last-lines');
const csvParser = require('csv-parser');

app.get('/last-row', (req, res) => {
  gRefreshRequest = !gRefreshRequestReceived;
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

// ------------------------------
// Nursery endpoints (simple GET/POST + logging)
// ------------------------------

const nurseryCsvFilePath = path.join(__dirname, 'nursery_log.csv');
const nurseryLatestPath = path.join(__dirname, 'nursery_latest.json');

// OPTIONAL: set NURSERY_TOKEN in docker env to prevent random writes
// docker run ... -e NURSERY_TOKEN="some-long-random-string"
const NURSERY_TOKEN = process.env.NURSERY_TOKEN || "";

// choose your fields (keep it simple)
const nurseryHeaders = [
  'DateStamp', 'TimeStamp', 'Epoch',
  'TempC', 'TempF', 'Humidity', 'Pressure_hPa',
  'WifiError'
];

function nurseryAuthOk(req) {
  if (!NURSERY_TOKEN) return true; // if not set, allow
  const token = req.query.token || req.headers['x-nursery-token'];
  return token === NURSERY_TOKEN;
}

function ensureFileWithHeader(filePath, headers) {
  if (!fs.existsSync(filePath)) {
    fs.writeFileSync(filePath, headers.join(',') + '\n');
  }
}

function appendCsvRow(filePath, rowArray) {
  const line = rowArray.join(',') + '\n';
  fs.appendFile(filePath, line, (err) => {
    if (err) console.error('CSV append error:', err);
  });
}

app.post('/nursery/log', (req, res) => {
  try {
    if (!nurseryAuthOk(req)) return res.status(403).send('Forbidden');

    const d = req.body || {};

    // Minimal validation: must have TempC (or TempF) to be meaningful
    const tempC = (d.TempC !== undefined) ? Number(d.TempC) : NaN;
    const tempF = (d.TempF !== undefined) ? Number(d.TempF) : (isFinite(tempC) ? (tempC * 9/5 + 32) : NaN);

    const latest = {
      DateStamp: d.DateStamp || '',
      TimeStamp: d.TimeStamp || '',
      Epoch: d.Epoch || '',
      TempC: isFinite(tempC) ? tempC : null,
      TempF: isFinite(tempF) ? tempF : null,
      Humidity: (d.Humidity !== undefined) ? Number(d.Humidity) : null,
      Pressure_hPa: (d.Pressure_hPa !== undefined) ? Number(d.Pressure_hPa) : null,
      WifiError: d.WifiError ?? '',
      serverReceivedMs: Date.now()
    };

    // save "latest"
    fs.writeFileSync(nurseryLatestPath, JSON.stringify(latest, null, 2));

    // append CSV
    ensureFileWithHeader(nurseryCsvFilePath, nurseryHeaders);

    appendCsvRow(nurseryCsvFilePath, [
      latest.DateStamp,
      latest.TimeStamp,
      latest.Epoch,
      latest.TempC,
      latest.TempF,
      latest.Humidity,
      latest.Pressure_hPa,
      latest.WifiError
    ]);

    res.status(200).send('ok');
  } catch (err) {
    console.error('nursery/log error:', err);
    res.status(500).send('error');
  }
});

app.get('/nursery/latest', (req, res) => {
  try {
    if (!fs.existsSync(nurseryLatestPath)) {
      return res.status(200).json({ ok: false, message: "No nursery data yet" });
    }
    const txt = fs.readFileSync(nurseryLatestPath, 'utf8');
    res.type('json').send(txt);
  } catch (err) {
    console.error('nursery/latest error:', err);
    res.status(500).json({ ok: false, message: "Error reading latest" });
  }
});

app.get('/nursery', (req, res) => {
  res.type('html').send(`
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <meta http-equiv="refresh" content="5">
  <title>Nursery</title>
  <style>
    body { font-family: Arial, sans-serif; font-size: 22px; padding: 12px; }
    .ok { color: #0a0; }
    .bad { color: #a00; }
    .small { font-size: 14px; opacity: 0.7; margin-top: 10px; }
    code { background: #f3f3f3; padding: 2px 6px; border-radius: 6px; }
  </style>
</head>
<body>
  <h2>Nursery</h2>
  <div id="out">Loading...</div>
  <div class="small">Endpoint: <code>/nursery/latest</code></div>

<script>
fetch('/nursery/latest')
  .then(r => r.json())
  .then(d => {
    if (!d || d.ok === false) {
      document.getElementById('out').innerHTML = '<span class="bad">No data yet</span>';
      return;
    }
    const tF = (d.TempF !== null && d.TempF !== undefined) ? d.TempF.toFixed(1) : 'n/a';
    const h  = (d.Humidity !== null && d.Humidity !== undefined) ? d.Humidity.toFixed(1) : 'n/a';
    const p  = (d.Pressure_hPa !== null && d.Pressure_hPa !== undefined) ? d.Pressure_hPa.toFixed(1) : 'n/a';
    const ageSec = Math.floor((Date.now() - (d.serverReceivedMs || Date.now())) / 1000);

    document.getElementById('out').innerHTML =
      'Temp: <b>' + tF + ' F</b><br>' +
      'Humidity: <b>' + h + ' %</b><br>' +
      'Pressure: <b>' + p + ' hPa</b><br>' +
      '<div class="small">Age: ' + ageSec + 's | ' + (d.DateStamp||'') + ' ' + (d.TimeStamp||'') + '</div>';
  })
  .catch(e => {
    document.getElementById('out').innerHTML = '<span class="bad">Error loading data</span>';
  });
</script>
</body>
</html>
  `);
});

app.get('/nursery/logs', (req, res) => {
  if (!fs.existsSync(nurseryCsvFilePath)) {
    return res.status(404).send('No nursery logs yet.');
  }
  res.download(nurseryCsvFilePath, (err) => {
    if (err) {
      console.error('Error sending nursery log file:', err);
      res.status(500).send('Error sending file');
    }
  });
});


app.listen(port, () => {
  console.log(`App listening at http://localhost:${port}`);
});
