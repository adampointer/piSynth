var util    = require('util');
var events  = require('events');
var express = require('express');

var app = express();

function AppServer() {
    events.EventEmitter.call(this);
    var self = this;
    this._initRoutes(function() {
        console.log('AppServer listening on ' + self.PORT); 
    });
};

util.inherits(AppServer, events.EventEmitter);

AppServer.prototype.PORT = 8989;

AppServer.prototype._initRoutes = function(callback) {
    app.use(express.static(__dirname + '/public'));
    app.get('/', function(request, response) {
        response.sendfile(__dirname + '/public/index.html');
    });
    return callback();
};

AppServer.prototype.start = function() {
    app.listen(this.PORT);
};

module.exports = AppServer;
