var util    = require('util');
var events  = require('events');
var express = require('express');

var app = express();

/**
 * Simple Express based HTTP server for modifiying
 * synth params
 *
 * @param {Object} oscillator
 *
 * @constructor
 */
function AppServer(oscillator) {
    events.EventEmitter.call(this);
    var self = this;
    app.configure(function(){
          app.use(express.bodyParser());
    });
    this.oscillator = oscillator;
    this._initRoutes(function() {
        console.log('AppServer listening on ' + self.PORT); 
    });
};

/**
 * Events mixin
 */
util.inherits(AppServer, events.EventEmitter);

/**
 * Listen for connections on this port
 *
 * @type {Number}
 */
AppServer.prototype.PORT = 8989;

/**
 * Start the server
 */
AppServer.prototype.start = function() {
    app.listen(this.PORT);
};

/**
 * Initialise routes
 *
 * @param {Function} callback
 * @return {Function}
 *
 * @private
 */
AppServer.prototype._initRoutes = function(callback) {
    var self = this;
    app.use(express.static(__dirname + '/../public'));
    app.get('/', function(request, response) {
        response.sendfile(__dirname + '/../public/index.html');
    });
    app.get('/envelope', function(request, response) {
        self._getEnvelope(request, response);
    }); 
    app.put('/envelope', function(request, response) {
        self._setEnvelope(request, response);
    });
    app.get('/modulator/1', function(request, response) {
        self._getFirstModulator(request, response);
    }); 
    app.put('/modulator/1', function(request, response) {
        self._setFirstModulator(request, response);
    });
    app.get('/modulator/2', function(request, response) {
        self._getSecondModulator(request, response);
    }); 
    app.put('/modulator/2', function(request, response) {
        self._setSecondModulator(request, response);
    });
    return callback();
};

/**
 * Get envelope parameters
 *
 * @param {Object} request
 * @param {Object} response
 *
 * @private
 */
AppServer.prototype._getEnvelope = function(request, response) {
    this.oscillator.getEnvelope(function(envelope) {
        response.set('Content-Type', 'application/json');
        response.send(JSON.stringify(envelope));
    });        
};

/**
 * Set envelope parameters
 *
 * @param {Object} request
 * @param {Object} response
 *
 * @private
 */
AppServer.prototype._setEnvelope = function(request, response) {
    var data = request.body;
    this.oscillator.setEnvelope(data);
    response.set('Content-Type', 'application/json');
    response.send({"ok": true});
};

/**
 * Get first modulator parameters
 *
 * @param {Object} request
 * @param {Object} response
 *
 * @private
 */
AppServer.prototype._getFirstModulator = function(request, response) {
    this.oscillator.getModulator(1, function(modulator) {
        response.set('Content-Type', 'application/json');
        response.send(JSON.stringify(modulator));
    });        
};

/**
 * Set first modulator parameters
 *
 * @param {Object} request
 * @param {Object} response
 *
 * @private
 */
AppServer.prototype._setFirstModulator = function(request, response) {
    var data = request.body;
    this.oscillator.setModulator(1, data);
    response.set('Content-Type', 'application/json');
    response.send({"ok": true});
};

/**
 * Get second modulator parameters
 *
 * @param {Object} request
 * @param {Object} response
 *
 * @private
 */
AppServer.prototype._getSecondModulator = function(request, response) {
    this.oscillator.getModulator(2, function(modulator) {
        response.set('Content-Type', 'application/json');
        response.send(JSON.stringify(modulator));
    });        
};

/**
 * Set second modulator parameters
 *
 * @param {Object} request
 * @param {Object} response
 *
 * @private
 */
AppServer.prototype._setSecondModulator = function(request, response) {
    var data = request.body;
    this.oscillator.setModulator(2, data);
    response.set('Content-Type', 'application/json');
    response.send({"ok": true});
};
module.exports = AppServer;
