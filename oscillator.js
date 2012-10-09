var osc    = require('./build/Release/oscillator.node');
var events = require('events');
var util   = require('util');

function Oscillator(pcm_device) {
    events.EventEmitter.call(this);
    var self = this;
    var hadSigInt = false;
    process.on('SIGINT', function() {
        if (hadSigInt) {
            process.exit(0);
        } else {
            hadSigInt = true;
            console.log('Shutting down PCM...');
            self._shutdown();
        }
    });
    
    process.on('SIGTERM', function() {
        console.log('Killing process...');
        process.exit(0);
    });
    
    this._initHardware(pcm_device, function(err) {
        console.log('Initialised');
    });
};

util.inherits(Oscillator, events.EventEmitter);

Oscillator.prototype._initHardware = function(pcm_name, callback) {
    console.log('Initialising PCM device...');
    osc.initPcm('hw:0', function(err) {

        if(err) {
            return callback(err);
        } else {
            console.log('Starting loop');
            osc.startPcm(function(err) {
                
                if(err) {
                    return callback(err);
                } else {
                    console.log('Listening for events...');
                    return callback();
                }
            });
        }
    });
};

Oscillator.prototype.onNoteOn = function(note, velocity) {
    var self = this;
    osc.noteOn(note, velocity, function() {
        self.emit('osc.note_played');
    });
};

Oscillator.prototype.onNoteOff = function(note) {
    var self = this;
    osc.noteOff(note, function() {
        self.emit('osc.note_stopped');
    });
};

Oscillator.prototype.setEnvelope = function(envelope) {
    
    if(envelope.attack, envelope.decay, envelope.sustain, envelope.release) {
        osc.setEnvelope(envelope.attack, envelope.decay, envelope.sustain, envelope.release);
        this.emit('osc.envelope_changed');
    } else {
        console.log('Invalid value passed as envelope');
    }
};

Oscillator.prototype.getEnvelope = function(callback) {
    osc.getEnvelope(function(envelope) {
        callback(envelope);
    });    
};

Oscillator.prototype._shutdown = function() {
    osc.closePcm();
    setTimeout(function(){
        process.exit(0);
    }, 1000);
};

module.exports = Oscillator;
