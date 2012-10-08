var Oscillator = require('./build/Debug/oscillator.node');

console.log('Initialising PCM device...');
Oscillator.initPcm('hw:0', function(err){

    if(err) {
        console.log(err);
    } else {
        console.log('Starting loop');
        Oscillator.startPcm(function(err){
            
            if(err) {
                console.log(err);
            } else {
                console.log('Listening for events');
                Oscillator.noteOn(72, 127.0, function(err) {
                    setTimeout(function(){
                        Oscillator.noteOff(72, function(err) {
                            console.log('off');
                        });
                    }, 1000);
                });
            }
        });
    }
    //console.log('Closing PCM device');
    //Oscillator.closePcm();
});
