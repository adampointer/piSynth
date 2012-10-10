var midi        = require('midi');
var util        = require('util');
var midiHandler = require('./midiHandler');
var Oscillator  = require('./oscillator');
var Server      = require('./server');

/**
 * Instantiate objects
 */
var input      = new midi.input();
var handler    = new midiHandler(input);
var oscillator = new Oscillator();
var appserver  = new Server(oscillator);

/**
 * Start HTTP server
 */
appserver.start();

/**
 * Bind events
 */
handler.on('midi.event_received.note_on', function(data){
    oscillator.onNoteOn(data.data, data.velocity);
});
handler.on('midi.event_received.note_off', function(data){
    oscillator.onNoteOff(data.data);
});

