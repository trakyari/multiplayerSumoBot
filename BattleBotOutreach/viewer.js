/* window.onload = () => {
  initStream();
} */

async function initStream(){
  const peer = createPeer();
  peer.addTransceiver("video", { direction: "recvonly"})
}

function createPeer() {
const peer = new RTCPeerConnection({
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
peer.ontrack = handleTrackEvent;
peer.onnegotiationneeded = () => handleNegotiationNeededEvent(peer);

return peer;
}

async function handleNegotiationNeededEvent(peer) {
const offer = await peer.createOffer();
await peer.setLocalDescription(offer);
const payload = {
    sdp: peer.localDescription
};

const { data } = await axios.post('/consumer', payload);
const desc = new RTCSessionDescription(data.sdp);
peer.setRemoteDescription(desc).catch(e => console.log(e));
}

function handleTrackEvent(e) {
document.getElementById("video").srcObject = e.streams[0];
};
