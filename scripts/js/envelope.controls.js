function EnvelopeControl() {
    var self = this;
    $(function() {
        self._render();
    });
};

EnvelopeControl.prototype.sliderSelector = '.envelope-control .slider';

EnvelopeControl.prototype._render = function() {
    var self = this;
    $(this.sliderSelector).slider({
        'orientation': 'vertical',
        'min': 0,
        'max': 1,
        'step': 0.01,
        'stop': function(event, ui) {
            var slider = $(ui.handle).parent();
            self._data[slider.attr('id')] = ui.value;
            self._setValue();
        }
    });
    this._getInitialValues(function(error, data) {
        
        if(!error) {
            self._data = data;
            $.each(data, function(key) {
                $('#' + key).slider('value', this);
            });
        }
    });
};

EnvelopeControl.prototype._getInitialValues = function(callback) {
    $.getJSON('/envelope', function(data) {
        
        if(typeof data.attack != 'undefined' && 
            typeof data.decay != 'undefined' && 
            typeof data.sustain != 'undefined' && 
            typeof data.release != 'undefined') 
        {
            return callback(null, data);
        } else {
            return callback(new Error('Invalid response'));
        }
    });
};

EnvelopeControl.prototype._setValue = function() {
    $.ajax({
        type: 'PUT',
        url: '/envelope',
        data: this._data,
        success: function(data) {
        },
        dataType: 'json'
    });
};
