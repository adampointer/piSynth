/*
 * Copyright (C) 2012 Adam Pointer <adam.pointer@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "config.h"
#include "pcm.h"
#include "midi.h"
#include "mongoose.h"
#include "server.h"

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

#define PORT 8989

/**
 * Handle for ALSA PCM
 */
snd_pcm_t    *playback_handle;

/**
 * Handle for ALSA MIDI
 */
snd_seq_t    *seq_handle;

/**
 * Buffer the PCM data before writting to hardware
 */
short        *buffer;

/**
 * Sound properties
 */
double       pitch, velocity[POLY], env_level[POLY], env_time[POLY],
             mod_phase_1[POLY], mod_phase_2[POLY], car_phase[POLY],
             cm_ratio_1, cm_ratio_2, mod_amp_1, mod_amp_2;

/**
 * Node properties
 */
int          note[POLY], note_active[POLY], gate[POLY];

/**
 * Actual sample rate
 */
unsigned int rate;

/**
 * Set to false to halt the worker thread
 */
unsigned int run_worker;

/**
 * Pointer to the function generating first modulator signal
 */
double       (*mod_func_1)(double);

/**
 * Pointer to the function generating second modulator signal
 */
double       (*mod_func_2)(double);

/**
 * Pointer to the function generating carrier signal
 */
double       (*car_func)(double);

/**
 * The function type to return to the client
 */
unsigned int mod_func_1_type, mod_func_2_type, car_func_type;

/**
 * Mongoose HTTP handle
 */
struct       mg_context *ctx;

/**
 * Defines properties of an modulator
 *
 * @typedef
 * @struct
 */
typedef struct {
    unsigned int type; /** < Function type */
    double cm_ratio;   /** < Carrier to Modulator ratio */
    double mod_amp;    /** < Amplitude of modulator signal */
} oscillator;

/**
 * Defines properties of an ADSR envelope
 *
 * @typedef
 * @struct
 */
typedef struct {
    double attack, decay, sustain, release;
} adsr_envelope;

/**
 * ADSR envelope for the main carrier signal
 */
adsr_envelope carrier_envelope;

/**
 * Callback for pthread_create loops and responds to ALSA events
 *
 * @return void
 */
void* startLoop();

/**
 * Try to clean up as much as possible before quitting
 *
 * @return void
 */
void cleanShutdown();

#endif // MAIN_H
