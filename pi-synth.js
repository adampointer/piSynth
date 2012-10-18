var midi        = require('midi');
var util        = require('util');
var midiHandler = require('./lib/midiHandler');
var Oscillator  = require('./lib/oscillator');
var Server      = require('./lib/server');

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

