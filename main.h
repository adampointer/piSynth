#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include "synth.h"
#include "midi.h"

#define POLY       2
#define GAIN       5000.0
#define BUFSIZE    512 
#define SEQ_NAME   "piSynth"
#define SAMPLE_RATE 44100.0

snd_seq_t *seq_handle;
snd_pcm_t *playback_handle;
short     *buffer;
double    pitch, modulation, velocity[POLY], env_level[POLY], two_pi, period, harmonic_const;
int       harmonic, subharmonic, note[POLY], note_active[POLY], rate;