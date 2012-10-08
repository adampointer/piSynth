var osc = require('./build/Debug/oscillator.node');

function Oscillator(pcm_device) { 
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
    
    this._initHardware(pcm_device, function(err){
        console.log('Initialised');
    });
};

Oscillator.prototype._initHardware = function(pcm_name, callback) {
    console.log('Initialising PCM device...');
    osc.initPcm('hw:0', function(err){

        if(err) {
            return callback(err);
        } else {
            console.log('Starting loop');
            osc.startPcm(function(err){
                
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
    osc.noteOn(note, velocity, function() {
        
    });
};

Oscillator.prototype.onNoteOff = function(note) {
    osc.noteOff(note, function() {
        
    });
};

Oscillator.prototype._shutdown = function() {
    //osc.closePcm();
    setTimeout(function(){
        process.exit(0);
    }, 1000);
};

module.exports = Oscillator;
