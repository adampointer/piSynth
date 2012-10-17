function ModulatorControl(selector) {
    var self      = this;
    this.selector = selector;
    $(function() {
        self.mod_number = self._getModNumber();
        self._getInitialValues(function(error, data) {
            
            if(!error) {
                self.data = data; 
            }
        });
    });
};

ModulatorControl.prototype._getModNumber = function() {
    return $(this.selector).attr('data-modnumber');
};

ModulatorControl.prototype._getInitialValues = function(callback) {
    $.getJSON('/modulator/' + this.mod_number, function(data) {
        
        if(data.waveform && data.cm_ratio && data.amplitude) {
            return callback(null, data);
        } else {
            return callback(new Error('Invalid response'));
        }
    });
};
