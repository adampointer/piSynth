function ModulatorControl(selector) {
    var self      = this;
    this.selector = selector;
    $(function() {
        self.mod_number = self._getModNumber();
        self._render();
    });
};

ModulatorControl.prototype.amplitudeControl = '.amplitude';

ModulatorControl.prototype.ratioControl = '.cm_ratio';

ModulatorControl.prototype._getModNumber = function() {
    return $(this.selector).attr('data-modnumber');
};

ModulatorControl.prototype._render = function() {
    var self = this;
    $(this.selector + ' ' + this.amplitudeControl).slider({
        'orientation': 'horizontal',
        'min': 0,
        'max': 200,
        'step': 1,
        'stop': function(event, ui) {
            self._sliderCallback(event, ui, self);
        }
    });
    $(this.selector + ' ' + this.ratioControl).slider({
        'orientation': 'horizontal',
        'min': 0,
        'max': 10,
        'step': 0.1,
        'stop': function(event, ui) {
            self._sliderCallback(event, ui, self);
        }
    });
    this._getInitialValues(function(error, data) {
        
        if(!error) {
            self._data = data;
            $.each(data, function(key) {
                $(self.selector + ' .' + key).slider('value', this);
            });
        }
    });
};

ModulatorControl.prototype._getInitialValues = function(callback) {
    $.getJSON('/modulator/' + this.mod_number, function(data) {
        
        if(typeof data.waveform != 'undefined' && 
            typeof data.cm_ratio != 'undefined' && 
            typeof data.amplitude != 'undefined') 
        {
            return callback(null, data);
        } else {
            return callback(new Error('Invalid response'));
        }
    });
};


ModulatorControl.prototype._sliderCallback = function(event, ui, self) {
    var slider = $(ui.handle).parent();
    self._data[slider.attr('data-parameter')] = ui.value;
    self._setValue();
}

ModulatorControl.prototype._setValue = function() {
    $.ajax({
        type: 'PUT',
        url: '/modulator/' + this.mod_number,
        data: this._data,
        success: function(data) {
        },
        dataType: 'json'
    });
};
