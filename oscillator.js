var Oscillator = require('./build/Release/oscillator.node');

console.log('Initialising PCM device...');
Oscillator.initPcm('hw:0', function(err){

    if(err) {
        console.log(err);
    } else {
        console.log('PCM started...');    
    }
    console.log('Closing PCM device');
    Oscillator.closePcm();
});
