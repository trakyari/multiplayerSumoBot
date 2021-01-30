const hostname = 'localhost';
const port = 3000;

var express = require('express');
var app = express();
var path = require('path');

app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});

app.get('/Bot1', function(req, res) {
  res.sendFile('/Bot1/Bot1.html');
  // res.sendFile(path.join(__dirname + '/Bot1/Bot1.html'));
});

app.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});