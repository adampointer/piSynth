function FilterControl(selector) {
  var self      = this;
  this.selector = selector;
  $(function() {
    self._render();
    $(self.selector + ' ' + self.typeControl).change(function(e) {
      self._data.type = $(this).val();
      self._setValue();
    });
  });
};

FilterControl.prototype.typeControl = '.type';

FilterControl.prototype.resonanceControl = '.Q';

FilterControl.prototype.cutoffControl = '.cutoff';

FilterControl.prototype.gainControl = '.gain';

FilterControl.prototype._render = function() {
  var self = this;
  $(this.selector + ' ' + this.resonanceControl).slider({
    'orientation': 'horizontal',
    'min': 0,
    'max': 1,
    'step': 0.01,
    'stop': function(event, ui) {
      self._sliderCallback(event, ui, self);
    }
  });
  $(this.selector + ' ' + this.cutoffControl).slider({
    'orientation': 'horizontal',
    'min': 0,
    'max': 10000,
    'step': 10,
    'stop': function(event, ui) {
      self._sliderCallback(event, ui, self);
    }
  });
  $(this.selector + ' ' + this.gainControl).slider({
    'orientation': 'horizontal',
    'min': 0,
    'max': 2000,
    'step': 1,
    'stop': function(event, ui) {
      self._sliderCallback(event, ui, self);
    }
  });
  this._getInitialValues(function(error, data) {
    
    if(!error) {
      self._data = data;
      $.each(data, function(key) {
        
          if(key != 'type') {
                $(self.selector + ' .' + key).slider('value', this);
              }
          });
              $(self.selector + ' ' + self.typeControl).val(self._data.type); 
          }
      });
};
          
FilterControl.prototype._getInitialValues = function(callback) {
  $.getJSON('/filter', function(data) {
    
    if(typeof data.type != 'undefined' && 
      typeof data.Q != 'undefined' && 
      typeof data.cutoff != 'undefined' && 
      typeof data.gain != 'undefined') 
    {
      return callback(null, data);
    } else {
      return callback(new Error('Invalid response'));
    }
  });
};


FilterControl.prototype._sliderCallback = function(event, ui, self) {
  var slider = $(ui.handle).parent();
  self._data[slider.attr('data-parameter')] = ui.value;
  self._setValue();
}

FilterControl.prototype._setValue = function() {
  $.ajax({
    type: 'PUT',
    url: '/filter',
    data: this._data,
    success: function(data) {
    },
    dataType: 'json'
  });
};
          