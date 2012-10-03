#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include "synth.h"
#include "midi.h"

#define POLY       8
#define GAIN       5000.0
#define BUFSIZE    512 
#define SEQ_NAME   "piSynth"
#define SAMPLE_RATE 44100.0

snd_seq_t *seq_handle;
snd_pcm_t *playback_handle;
short     *buffer;
double    pitch, velocity[POLY], env_level[POLY], env_time[POLY], mod_phase[POLY], car_phase[POLY], two_pi, modulation, harmonic_const,
          attack, decay, sustain, release;
int       note[POLY], note_active[POLY], rate, gate[POLY];

#endif
