var util    = require('util');
var events  = require('events');
var express = require('express');

var app = express();

function AppServer(oscillator) {
    events.EventEmitter.call(this);
    var self = this;
    this.oscillator = oscillator;
    this._initRoutes(function() {
        console.log('AppServer listening on ' + self.PORT); 
    });
};

util.inherits(AppServer, events.EventEmitter);

AppServer.prototype.PORT = 8989;

AppServer.prototype._initRoutes = function(callback) {
    var self = this;
    app.use(express.static(__dirname + '/public'));
    app.get('/', function(request, response) {
        response.sendfile(__dirname + '/public/index.html');
    });
    app.get('/envelope', function(request, response) {
        self._getEnvelope(request, response);
    });
    app.put('/envelope', function(request, response) {
        self._setEnvelope(request, response);
    });
    return callback();
};

AppServer.prototype._getEnvelope = function(request, response) {
    this.oscillator.getEnvelope(function(envelope) {
        response.set('Content-Type', 'application/json');
        response.send(JSON.stringify(envelope));
    });        
};

AppServer.prototype._setEnvelope = function(request, response) {
    var data = request.body;
    this.oscillator.setEnvelope(envelope);
    response.set('Content-Type', 'application/json');
    response.send({"ok": true});
};

AppServer.prototype.start = function() {
    app.listen(this.PORT);
};

module.exports = AppServer;
