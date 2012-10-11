# piSynth #

My rather daft pet project to build a fuly configurable FM synth with browser-based control panel using Node.js and ALSA
with a little C++ thrown in. It is aimed at the Raspberry Pi as that is what I run this on, but it can conceivably run
on anything Linux based.

## Dependencies ##

Needs ALSA libs and headers, Node.js (0.8) with NPM and node-gyp (Latest versions).

## Installation ##

    git clone git@github.com:adampointer/piSynth.git
    cd pisynth
    npm install

## Running ##

    node pi-synth.js

Then use `aconnect` to link your MIDI hardware to the synth. The HTTP interface will be running on port 8989.


