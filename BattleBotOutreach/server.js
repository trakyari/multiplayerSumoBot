// Server.js 
const hostname = 'localhost';
const port = 3000;
const webRTCPort = 5000;
// webRTC 
const express = require('express');
const app = express();
const bodyParser = require('body-parser');
const webrtc = require("wrtc");
// Express
// var express = require('express');
// var app = express();
var path = require('path');

// webRTC implementation for live stream broadcast
let senderStream;

app.use(express.static(__dirname)); // main directory
app.use(express.static('assets')); // includes 'assets' folder 
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

// Viewer code
app.post("/consumer", async ({ body }, res) => {
    const peer = new webrtc.RTCPeerConnection({
        iceServers: [
            {
                urls: "stun:stun.l.google.com:19302"
            },
            {                                    
                urls: "turn:sumobot.ddns.net:3478",
                username: "guest",                                           
                credential: "somepassword"                              
            }  
        ]
    });
    const desc = new webrtc.RTCSessionDescription(body.sdp);
    await peer.setRemoteDescription(desc);
    senderStream.getTracks().forEach(track => peer.addTrack(track, senderStream));
    const answer = await peer.createAnswer();
    await peer.setLocalDescription(answer);
    const payload = {
        sdp: peer.localDescription
    }

    res.json(payload);
});

// Streamer code
app.post('/broadcast', async ({ body }, res) => {
    const peer = new webrtc.RTCPeerConnection({
        iceServers: [
            {
                urls: "stun:stun.l.google.com:19302"
            },                                   
            {                                    
                urls: "turn:sumobot.ddns.net:3478",       
                username: "guest",                                       
                credential: "somepassword"                              
            }  
        ]
    });
    peer.ontrack = (e) => handleTrackEvent(e, peer);
    const desc = new webrtc.RTCSessionDescription(body.sdp);
    await peer.setRemoteDescription(desc);
    const answer = await peer.createAnswer();
    await peer.setLocalDescription(answer);
    const payload = {
        sdp: peer.localDescription
    }

    res.json(payload);
});

function handleTrackEvent(e, peer) {
    senderStream = e.streams[0];
};

app.listen(webRTCPort, () => console.log('webRTC server started'));

// Express send out static webpage code
// Index
app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/index.html'));
});

// Bots
app.get('/Bot1', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot1/Bot1.html'));
});

app.get('/Bot2', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot2/Bot2.html'));
  });

app.get('/Bot3', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot3/Bot3.html')); 
});

app.get('/Bot4', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot4/Bot4.html'));
});

app.get('/Bot5', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot5/Bot5.html'));
});

app.get('/Bot6', function(req, res) {
  res.sendFile(path.join(__dirname + '/Bot6/Bot6.html'));
});

app.get('/admin', function(req, res) {
  res.sendFile(path.join(__dirname + '/admin/admin.html'));
});

app.listen(port, hostname, () => {
  console.log(`NodeJS Server running at http://${hostname}:${port}/`);
});
