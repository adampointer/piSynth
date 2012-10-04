var midi   = require('midi');
var events = require('events');
var _      = require('underscore');

var input = new midi.input();

function MidiHandler() {

};

MidiHandler.prototype.eventMap = {
    144: 'NOTEON',
    128: 'NOTEOFF'
};

MidiHandler.prototype.onMessage = function(message) {
    
    if(_.isArray(message) && message.length == 3) {
        var type = this.eventMap[message[0]];
        
        if(typeof type != 'undefined') {
            this.type     = type;
            this.data     = message[1];
            this.velocity = message[2];
        } else {
            console.log('Undefined event received, skipping.');
        }
    }
};

handler = new MidiHandler();

input.on('message', function(deltaTime, message) {
    handler.onMessage(message);    
});

input.openVirtualPort("Test Input");

//input.closePort();
