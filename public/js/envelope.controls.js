function EnvelopeControl() {
    var self = this;
    $(function() {
        self._render();
    });
};

EnvelopeControl.prototype.sliderSelector = '.envelope-control .slider';

EnvelopeControl.prototype._render = function() {
    $(this.sliderSelector).slider({
        'orientation': 'vertical',
        'min': 0,
        'max': 1,
        'step': 0.01
    });
};

var adsr = new EnvelopeControl();
