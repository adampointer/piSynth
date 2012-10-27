# piSynth #

An FM subtractive synthesiser with a browser based front-end written in C and Javascript. This is my pet project for my Raspberry Pi but it can run on any environment that can support it. I use a Ion Key49 USB MIDI keyboard to control it, but as it is built on top of ALSA it can use any hardware that ALSA can.

## License ##

MIT - It's open source, do what you want with it but if you do anything interesting, let me know!

## Contributions ##

I welcome anybody that wishes to help out with this project, just send me an email.

## Dependencies ##

+ ALSA libs and headers
+ POSIX threads libs and headers
+ CMake 2.8
+ Optionally, Doxygen to compile the API manpages

## Installation ##

    git clone git@github.com:adampointer/piSynth.git
    cd piSynth
    cmake ./
    make
    make install

## Running ##

    piSynth hw:0

Substitute `hw:0` for the ALSA PCM hardware string you wish to use. Then use `aconnect` to link your MIDI hardware to the synth. The HTTP interface will be running on port 8989.


