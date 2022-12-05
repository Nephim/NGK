// NOTE: ROUTE IS CHAT
const socket = new WebSocket('ws://localhost:8080/chat');

// Listener for Connection opened
socket.addEventListener('open', function (event) {
	// Sending a message to the web socket server...
	socket.send('Updates coming here');
});

// Listener for response messages
socket.addEventListener('message', function (message) {
	console.log('Message from server ', message.data);
	getData();
});