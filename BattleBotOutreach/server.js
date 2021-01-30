// const http = require('http');
// const fs = require('fs');
const hostname = 'localhost';
const port = 3000;

var express = require('express');
var app = express();
var path = require('path');

app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});

app.get('/Bot1', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot1/Bot1.html'));
});

/* const server = http.createServer((req, res) => {
  res.statusCode = 200;
  res.setHeader('Content-Type', 'text/html');
  fs.createReadStream('index.html').pipe(res);
  res.end('ETC Sumobot');
});
*/
app.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});