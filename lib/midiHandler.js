var util       = require('util');
var events     = require('events');
var _          = require('underscore');

/**
 * Class wrapping basic MIDI functions and binding to
 * events
 *
 * @param {Object} input
 *
 * @constructor
 */
function MidiHandler(input) {
    events.EventEmitter.call(this);

    var self      = this;
    this.input    = input;

    // Ensure clean shutdown
    var hadSigInt = false;
    process.on('SIGINT', function() {
        if (hadSigInt) {
            process.exit(0);
        } else {
            hadSigInt = true;
            console.log('Shutting down MIDI...');
            self._shutdown();
        }
    });
    process.on('SIGTERM', function() {
        console.log('Killing process...');
        process.exit(0);
    });

    // Start listening
    this._bindMidiEvents();
    this.input.openVirtualPort("piSynth");
    console.log('MIDI initialised');
};

/**
 * Events mixin
 */
util.inherits(MidiHandler, events.EventEmitter);

/**
 * Map MIDI events to our internal events
 *
 * @type {Object}
 */
MidiHandler.prototype.eventMap = {
    144: 'note_on',
    128: 'note_off'
};

/**
 * Handler triggered when a MIDI event is received
 *
 * @param {Object} message
 */
MidiHandler.prototype.onMessage = function(message) {
    
    if(_.isArray(message) && message.length == 3) {
        var type = this.eventMap[message[0]];
        
        if(typeof type != 'undefined') {
            this.type     = type;
            this.data     = message[1];
            this.velocity = message[2];

            this.emit('midi.event_received.' + type, 
                {
                    data: this.data, 
                    velocity: this.velocity
           });
        } else {
            console.log('Undefined event received, skipping.');
        }
    }
};

/**
 * Bind external events to internal handlers
 *
 * @private
 */
MidiHandler.prototype._bindMidiEvents = function() {   
    var self = this;
    this.input.on('message', function(deltaTime, message) {
        self.onMessage(message);    
    });
};

/**
 * Clean shutdown
 *
 * @private
 */
MidiHandler.prototype._shutdown = function() {
    this.input.closePort();
    setTimeout(function(){
        process.exit(0);
    }, 1000);
};

module.exports = MidiHandler;