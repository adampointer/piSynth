var adsr = new EnvelopeControl();
var NUM_MODULATORS = 4;
var modulators = [];

for(var i = 1; i <= NUM_MODULATORS; i++) {
    modulators.push(new ModulatorControl('#mod'+i));
}

var carrier = new CarrierControl('#carrier');
var filter = new FilterControl('#primary-filter');
