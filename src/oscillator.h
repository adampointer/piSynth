#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <node/node.h>
#include <alsa/asoundlib.h>

#define BUFSIZE  512
#define RATE     44100.0
#define POLY     8
#define GAIN     500.0

#define TRIANGLE 1
#define SQUARE   2
#define SAWTOOTH 3
#define SINE     4

#define M_TWO_PI   6.2831853071796

using namespace v8;

class Oscillator : public node::ObjectWrap {
public:
    struct ADSR {
        double attack, decay, sustain, release;
    }
    static void Init(Handle<Object> target);
    
private:
    Oscillator();
    ~Oscillator();
    
    static double Envelope(int *note_active, int gate, double *env_level, double t, ADSR envelope);
    static double FastSine(double x);
    static double SquareWave(double x);
    static double TriangleWave(double x);
    static double SawtoothWave(double x);
    static int    PlaybackCallback(snd_pcm_sframes_t nframes);
    static void*  StartLoop();

    Handle<Value> InitPcm(const Arguments& args);
    Handle<Value> StartPcm(const Arguments& args);
    Handle<Value> NoteOn(const Arguments& args);
    Handle<Value> NoteOff(const Arguments& args);
    Handle<Value> ClosePcm(const Arguments& args);
    Handle<Value> GetEnvelope(const Arguments& args);
    Handle<Value> SetEnvelope(const Arguments& args);
    Handle<Value> GetModulator(const Arguments& args);
    Handle<Value> SetModulator(const Arguments& args);
};

#endif