var midi        = require('midi');
var midiHandler = require('./midiHandler');
var Oscillator  = require('./oscillator');

var input = new midi.input();

handler =    new midiHandler(input);
oscillator = new Oscillator();

handler.on('midi.event_received.note_on', function(data){
    oscillator.onNoteOn(data.data, data.velocity);
});
handler.on('midi.event_received.note_off', function(data){
    oscillator.onNoteOff(data.data);    
});

