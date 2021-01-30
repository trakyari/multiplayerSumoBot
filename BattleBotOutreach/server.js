// Server.js 
const hostname = 'localhost';
const port = 3000;

var express = require('express');
var app = express();
var path = require('path');

// Index
app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});

// Bots
app.get('/Bot1', function(req, res) {
  res.sendFile('/Bot1/Bot1.html');
});

app.get('/Bot2', function(req, res) {
  res.sendFile('/Bot2/Bot2.html');
  });

app.get('/Bot3', function(req, res) {
  res.sendFile('/Bot3/Bot3.html'); 
});

app.get('/Bot4', function(req, res) {
  res.sendFile('/Bot4/Bot4.html');
});

app.get('/Bot5', function(req, res) {
  res.sendFile('/Bot5/Bot5.html');
});

app.get('/Bot6', function(req, res) {
  res.sendFile('/Bot6/Bot6.html');
});

app.get('/admin', function(req, res) {
  res.sendFile('/admin/admin.html');
});

app.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});