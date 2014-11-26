var ports = require("serialport");
var SerialPort = require("serialport").SerialPort;
var express = require('express');
var server = express();
var prompt = require('prompt');
var chalk = require('chalk');
var serialPort;

server.disable('x-powered-by');
server.disable('etag');

function parseResponse(requestString, callback) {

  var states = {
    headers: 0,
    body: 1,
    status: 2
  };

  var statusExpression = /HTTP\/(\d)\.(\d) (\d{3}) (.*)/i;

  var res = {
    body: "",
    statusCode: 0,
    statusText: "",
    httpVersion: 0,
    headers: []
  };

  var currentLine = [];

  var state = states.status;

  for (var i = 0; i < requestString.length; i++) {

    var char = requestString.charAt(i);

    if (state == states.body) {
      try {
        res.body = JSON.parse(requestString.substring(i, requestString.length));
      }
      catch (e) {
        res.body = requestString.substring(i, requestString.length);
      }
      break;
    }
    if (char == '\n') {

      if (currentLine.length == 0) {
        state = states.body;
      }
      else {

        var line = currentLine.join("");

        if (state == states.status) {

          var match = statusExpression.exec(line);

          if (match == null) {
            return callback(new Error("Failed to parse response"), null);
          }

          res.statusCode = match[3];
          res.statusText = match[4];

        }
        else {

          var lastIndex = line.indexOf(":");

          res.headers.push({
            name: line.substring(0, lastIndex),
            value: line.substring(lastIndex + 1).trim()
          });

        }

        state = states.headers;
        currentLine = [];

      }

      continue;

    } else if (char == '\r') {

      continue;

    }

    currentLine.push(char);

  }

  return callback(null, res);

}


ports.list(function (err, ports) {

  var baudRates = [300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200];
  var portNumber;
  var baudRate;

  prompt.start();

  for (var i = 0; i < ports.length; i++) {
    console.log(i + 1 + '. ' + ports[i].comName);
  }

  prompt.get([
    {
      name: 'portNumber',
      description: 'Select port to proxy (1 - ' + ports.length + ')',
      required: true
    }
  ], function (err, result) {

    if (err) {
      console.log(err);
    }

    portNumber = result.portNumber - 1;

    for (var i = 0; i < baudRates.length; i++) {
      console.log(i + 1 + '. ' + baudRates[i]);
    }

    prompt.get([
      {
        name: 'baudRate',
        description: "Choose baudrate (1 - " + baudRates.length + ")",
        required: true
      }
    ], function (err, result) {

      if (err) {
        console.log(err);
      }

      serialPort = new SerialPort(ports[portNumber].comName, {
        baudrate: baudRates[result.baudRate - 1],
        bufferSize: 2560

      }, false);

      serialPort.open();

      serialPort.on("open", function () {

        server.listen(3000);

      });

    });

  });

});


function addHeaders(expressHeaders) {

  var headers = '';

  if (expressHeaders['content-length']) {
    headers += 'Content-Length: ' + expressHeaders['content-length'] + '\r\n';
  }

  if (expressHeaders['cookie']) {
    headers += 'Cookie: ' + expressHeaders['cookie'] + '\r\n';
  }

  if (expressHeaders['authorization']) {
    headers += 'Authorization: ' + expressHeaders['authorization'] + '\r\n';
  }

  if (expressHeaders['host']) {
    headers += 'Host: ' + expressHeaders['host'] + '\r\n';
  }

  headers += "\r\n";


  return headers;

}

server.use(function (req, res, next) {

  req.rawBody = '';
  req.setEncoding('utf-8');

  req.on('data', function (chunk) {
    req.rawBody += chunk;
  });

  req.on('end', function () {
    next();
  });

});


server.all('/*', function (req, res) {

  var request = "";
  var url = req.path;
  var body = req.rawBody;

  switch (req.method) {

    case 'POST':
      request += "POST " + url + " HTTP 1.0\r\n";
      request += addHeaders(req.headers);
      request += body;
      break;

    case 'PUT':
      request += "PUT " + url + " HTTP 1.0\r\n";
      request += addHeaders(req.headers);
      request += body;
      break;

    case 'PATCH':
      request += "PATCH " + url + " HTTP 1.0\r\n";
      request += addHeaders(req.headers);
      request += body;
      break;

    case 'DELETE':
      request += "DELETE " + url + " HTTP 1.0\r\n";
      request += addHeaders(req.headers);
      break;

    case 'GET':
      request += "GET " + url + " HTTP 1.0 \r\n";
      request += addHeaders(req.headers);
      break;

  }

  console.log(chalk.yellow(request));

  serialPort.write(request, function () {

    var responseReceived;
    var responseTimeout;
    var response = '';

    responseTimeout = setTimeout(function () {

      serialPort.removeAllListeners('data');
      res.send(504);


    }, 60000);

    serialPort.on("data", function (data) {

      response += data;

      clearTimeout(responseReceived);
      clearTimeout(responseTimeout);

      responseReceived = setTimeout(function () {


        console.log(chalk.blue(response));

        parseResponse(response, function (err, parsedResponse) {

          for (var i = 0; i < parsedResponse.headers.length; i++) {
            res.header(parsedResponse.headers[i].name, parsedResponse.headers[i].value);

          }

          res.status(parsedResponse.statusCode).send(parsedResponse.body);

          serialPort.removeAllListeners('data');

        });

      }, 1);

    });

  });

});
