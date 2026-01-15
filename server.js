const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// Serve static files from 'public' directory
app.use(express.static('public'));

let port;

// Handle Socket.io connections
io.on('connection', (socket) => {
    console.log('A user connected');

    // Send list of available ports to the client
    SerialPort.list().then((ports) => {
        socket.emit('ports-list', ports);
    });

    // Handle 'connect-port' event
    socket.on('connect-port', (path) => {
        if (port && port.isOpen) {
            port.close();
        }

        port = new SerialPort({ path: path, baudRate: 38400 }, (err) => {
            if (err) {
                console.error('Error opening port:', err.message);
                let msg = `Error: ${err.message}`;
                if (err.message.includes('Access denied')) {
                    msg += '. Check if Arduino IDE Serial Monitor is open.';
                }
                socket.emit('status', msg);
            } else {
                console.log(`Connected to ${path}`);
                socket.emit('status', `Connected to ${path}`);
            }
        });

        const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));
        parser.on('data', (data) => {
            console.log('Arduino:', data);
        });
    });

    // Handle 'command' event (e.g., "BASE LEFT")
    socket.on('command', (cmd) => {
        if (port && port.isOpen) {
            console.log(`Sending: ${cmd}`);
            port.write(cmd + '\n', (err) => { // Add newline if Arduino expects it, though code uses String.equalsIgnoreCase which might handle it. 
                // The Arduino code reads 'bt' but doesn't show *how* it reads it. 
                // Usually Serial.readString() or similar. 
                // Wait, the Arduino code provided uses `Bluetooth.available() > 0` but doesn't show the reading logic!
                // Ah, looking at the user request: "String bt, btS;" and "if (Bluetooth.available() > 0) { ... }"
                // The snippet provided is INCOMPLETE in the reading part. 
                // "if (Bluetooth.available() > 0) { if (bt.equalsIgnoreCase..." 
                // It implies `bt` is already populated. 
                // I will assume standard Serial reading. I'll send the string.
                if (err) {
                    return console.log('Error on write: ', err.message);
                }
            });
        } else {
            console.log('Port not open, cannot send:', cmd);
            socket.emit('status', 'Error: Port not open');
        }
    });

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });
});

const PORT = 3000;
server.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
});
