function CarrierControl(selector) {
  var self      = this;
  this.selector = selector;
  $(function() {
    self._render();
    $(self.selector + ' ' + self.waveformControl).change(function(e) {
      self._data.waveform = $(this).val();
      self._setValue();
    });
  });
};

CarrierControl.prototype.waveformControl = '.waveform';

CarrierControl.prototype._render = function() {
  var self = this;
  this._getInitialValues(function(error, data) {

    if(!error) {
      self._data = data;
      $(self.selector + ' ' + self.waveformControl).val(self._data.waveform); 
    }
});
 };

CarrierControl.prototype._getInitialValues = function(callback) {
  $.getJSON('/carrier', function(data) {

    if(typeof data.waveform != 'undefined')
    {
      return callback(null, data);
    } else {
      return callback(new Error('Invalid response'));
    }
  });
};

CarrierControl.prototype._setValue = function() {
  $.ajax({
    type: 'PUT',
    url: '/carrier',
    data: this._data,
    success: function(data) {
    },
    dataType: 'json'
  });
};