// Copyright (C) 2012 Adam Pointer <adam.pointer@gmx.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "config.h"
#include "mongoose.h"

#define BUFSIZE  1024    // 23ms latency
#define RATE     44100.0
#define POLY     8
#define GAIN     1000.0

#define M_TWO_PI 6.2831853071796

#define TRUE  1
#define FALSE 0

#define PORT "8989"

///
/// Waveform type
///
enum waveform
{
    triangle, ///< Triangle wave
    square,   ///< Square wave
    sawtooth, ///< Sawtooth wave
    sine      ///< Sine wave
};

///
/// Handle for ALSA PCM
///
snd_pcm_t *playback_handle;

///
/// Handle for ALSA MIDI
///
snd_seq_t *seq_handle;

///
/// Buffer the PCM data before writting to hardware
///
short *buffer;

///
/// Sound properties
///
double pitch, velocity[POLY], env_level[POLY], env_time[POLY];

///
/// Node properties
///
int note[POLY], note_active[POLY], gate[POLY];

///
/// Actual sample rate
///
unsigned int rate;

///
/// Set to false to halt the worker thread
///
unsigned int run_worker;

///
/// Mongoose HTTP handle
///
struct mg_context *ctx;

struct mg_callbacks callbacks;

///
/// Try to clean up as much as possible before quitting
///
/// \return void
///
void cleanShutdown();

#endif // MAIN_H
