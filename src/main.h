#include <math.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

#define BUFSIZE  512
#define RATE     44100.0
#define POLY     8
#define GAIN     500.0

#define TRIANGLE 1
#define SQUARE   2
#define SAWTOOTH 3
#define SINE     4

#define M_TWO_PI 6.2831853071796

#define TRUE  1
#define FALSE 0

snd_pcm_t    *playback_handle;
snd_seq_t    *seq_handle;
short        *buffer;
double       pitch, velocity[POLY], env_level[POLY], env_time[POLY], mod_phase_1[POLY], mod_phase_2[POLY], car_phase[POLY],
             cm_ratio_1, cm_ratio_2, mod_amp_1, mod_amp_2;
int          note[POLY], note_active[POLY], gate[POLY];
unsigned int rate;
unsigned int run_worker;
double       (*mod_func_1)(double);
double       (*mod_func_2)(double);
double       (*car_func)(double);
unsigned int mod_func_1_type, mod_func_2_type, car_func_type;

typedef double (*generator_function)(double);

typedef struct {
    unsigned int type; 
    double cm_ratio, mod_amp;
} oscillator;

typedef struct {
    double attack, decay, sustain, release;
} adsr_envelope;

adsr_envelope carrier_envelope;

double fastSin(double x);

double squareWave(double x);

double triangleWave(double x);

double sawtoothWave(double x);

int playbackCallback(snd_pcm_sframes_t nframes);

void* startLoop();

unsigned int initMidi();

unsigned int initPcm(char *pcm_name);

unsigned int startPcm();

unsigned int noteOn(int played_note, double played_velocity);

unsigned int noteOff(int played_note);

unsigned int closePcm();

generator_function getFunction(unsigned int type);