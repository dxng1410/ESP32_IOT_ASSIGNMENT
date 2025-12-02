var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// Init web socket when the page loads
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
    initButton();
}

function getReadings(){
    websocket.send("getReadings");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

// When websocket is established, call the getReadings() function
function onOpen(event) {
    console.log('Connection opened');
    getReadings();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

// Function that receives the message from the ESP32 with the readings
function onMessage(event) {
    console.log('Received data:', event.data);
    
    try {
        var myObj = JSON.parse(event.data);
        var keys = Object.keys(myObj);

        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            if (document.getElementById(key)) {
                document.getElementById(key).innerHTML = myObj[key];
            }
        }
        
        // Handle LED state update
        if (myObj.hasOwnProperty('ledState')) {
            updateLedStatus(myObj.ledState);
        }
    } catch (error) {
        // Handle simple LED state messages (0 or 1)
        if (event.data === "0" || event.data === "1") {
            updateLedStatus(event.data);
        } else {
            console.error('Error parsing JSON:', error);
        }
    }
}

function updateLedStatus(state) {
    const ledStateElement = document.getElementById('ledState');
    const toggleButton = document.getElementById('toggleButton');
    
    if (state == "1" || state === true) {
        ledStateElement.innerHTML = "ON";
        ledStateElement.className = "led-status-on";
        toggleButton.className = "toggle-button led-on";
        toggleButton.innerHTML = '<i class="fas fa-lightbulb"></i> Turn OFF';
    } else {
        ledStateElement.innerHTML = "OFF";
        ledStateElement.className = "led-status-off";
        toggleButton.className = "toggle-button led-off";
        toggleButton.innerHTML = '<i class="far fa-lightbulb"></i> Turn ON';
    }
}

function initButton() {
    document.getElementById('toggleButton').addEventListener('click', toggleLed);
}

function toggleLed() {
    console.log('Toggle LED button clicked');
    if (websocket.readyState === WebSocket.OPEN) {
        websocket.send('toggle');
    } else {
        console.error('WebSocket is not connected');
        alert('Connection lost. Please refresh the page.');
    }
}