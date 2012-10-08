var midi       = require('midi');
var events     = require('events');
var _          = require('underscore');
var util       = require('util');
var oscillator = require('./build/Release/oscillator.node');

var input = new midi.input();
var osc   = new oscillator.Oscillator();
osc.initSound('hw:0');

function MidiHandler() {
    events.EventEmitter.call(this);

    var self = this;
    var hadSigInt = false;

    process.on('SIGINT', function() {
        if (hadSigInt) {
            process.exit(0);
        } else {
            hadSigInt = true;
            console.log('Shutting down...');
            self._shutdown();
        }
    });
    
    process.on('SIGTERM', function() {
        console.log('Killing process...');
        process.exit(0);
    });

    this._bindMidiEvents();

    input.openVirtualPort("piSynth");
};

util.inherits(MidiHandler, events.EventEmitter);

MidiHandler.prototype.eventMap = {
    144: 'note_on',
    128: 'note_off'
};

MidiHandler.prototype.onMessage = function(message) {
    
    if(_.isArray(message) && message.length == 3) {
        var type = this.eventMap[message[0]];
        
        if(typeof type != 'undefined') {
            this.type     = type;
            this.data     = message[1];
            this.velocity = message[2];

            this.emit('midi.event_received.' + type);
        } else {
            console.log('Undefined event received, skipping.');
        }
    }
};

MidiHandler.prototype._bindMidiEvents = function() {   
    var self = this;
    input.on('message', function(deltaTime, message) {
        self.onMessage(message);    
    });
    this.on('midi.event_received.note_on', function(){
        console.log('Note played [' + self.data  + '] with velocity [' + self.velocity + ']');
    });
    this.on('midi.event_received.note_off', function(){
        console.log('Note off');
    });
};

MidiHandler.prototype._shutdown = function() {
    input.closePort();
    setTimeout(function(){
        process.exit(0);
    }, 1000);
};

handler = new MidiHandler();

