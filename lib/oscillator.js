var osc    = require('../build/Debug/oscillator.node');
var events = require('events');
var util   = require('util');
var _      = require('underscore');

/**
 * Wraps the ALSA powered Oscillator extension
 * which does the actual hardware interaction
 *
 * @param {String} pcm_device
 *
 * @constructor
 */
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

/**
 * Events mixin
 */
util.inherits(Oscillator, events.EventEmitter);

/**
 * Play a note
 *
 * @param {Integer} note
 * @param {Number}  velocity
 */
Oscillator.prototype.onNoteOn = function(note, velocity) {
    var self = this;
    osc.noteOn(note, velocity, function() {
        self.emit('osc.note_played');
    });
};

/**
 * Indicate that a note should no longer be playing
 *
 * @param {Integer} note
 */
Oscillator.prototype.onNoteOff = function(note) {
    var self = this;
    osc.noteOff(note, function() {
        self.emit('osc.note_stopped');
    });
};

/**
 * Set envelope parameters
 *
 * @param {Object} envelope
 */
Oscillator.prototype.setEnvelope = function(envelope) {
    
    if(envelope.attack && envelope.decay && envelope.sustain && envelope.release) {
        osc.setEnvelope(
            parseFloat(envelope.attack), 
            parseFloat(envelope.decay), 
            parseFloat(envelope.sustain), 
            parseFloat(envelope.release)
        );
        this.emit('osc.envelope_changed');
    } else {
        console.log('Invalid value passed as envelope');
    }
};

/**
 * Get envelope parameters
 *
 * @param {Function} callback
 */
Oscillator.prototype.getEnvelope = function(callback) {
    osc.getEnvelope(function(envelope) {
        callback(envelope);
    });    
};

/**
 * Set modulator parameters
 *
 * @param {Integer} number
 * @param {Object}  envelope
 */
Oscillator.prototype.setModulator = function(number, modulator) {
    
    if(envelope.waveform && envelope.cm_ratio && envelope.amplitude) {
        osc.setModulator(
            parseInt(number, 10), 
            parseInt(modulator.waveform, 10), 
            parseFloat(modulator.cm_ratio), 
            parseFloat(modulator.amplitude)
        );
        this.emit('osc.modulator_ ' +number + '_changed');
    } else {
        console.log('Invalid values passed as modulator');
    }
};

/**
 * Get modulator parameters
 *
 * @param {Integer}  number
 * @param {Function} callback
 */
Oscillator.prototype.getModulator = function(number, callback) {
    osc.getModulator(number, function(modulator) {
        callback(modulator);
    });    
};

/**
 * Initalise the PCM hardware and start the
 * worker thread
 *
 * @param {String}   pcm_name
 * @param {Function} callback
 *
 * @private
 */
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

/**
 * Clean shutdown
 *
 * @private
 */
Oscillator.prototype._shutdown = function() {
    osc.closePcm();
    setTimeout(function(){
        process.exit(0);
    }, 1000);
};

module.exports = Oscillator;
