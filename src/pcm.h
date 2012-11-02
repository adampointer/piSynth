// Copyright (C) 2012 Adam Pointer <adam.pointer\gmx.com>
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

#ifndef PCM_H
#define PCM_H

#include "main.h"

///
/// Taylor series sine aproximation
///
/// \param x
///
/// \return y
///
double fastSin ( double x );

///
/// Square wave
///
/// \param x
///
/// \return y
///
double squareWave ( double x );

///
/// Triangle wave
///
/// \param x
///
/// \return y
///
double triangleWave ( double x );

///
/// Sawtooth wave
///
/// \param x
///
/// \return y
///
double sawtoothWave ( double x );

///
/// Function pointer to signal generation function
///
/// \typedef
///
typedef double ( *generator_function ) ( double );

///
/// Defines properties of an modulator
/// \typedef
/// \struct
///
typedef struct
{
  enum waveform type;      ///< Function type
  generator_function func; ///< Function pointer
  double cm_ratio;         ///< Carrier to Modulator ratio
  double amplitude;        ///< Amplitude of modulator signal
  double phase[POLY];      ///< Phase
} oscillator;

///
/// Default values for oscillators
///
const oscillator default_osc;

///
/// Define the modulators and the carrier
///
oscillator modulator_1, modulator_2, modulator_3, modulator_4, carrier;

///
/// Defines properties of an ADSR envelope
///
/// \typedef
/// \struct
///
typedef struct
{
  double attack, decay, sustain, release;
} adsr_envelope;

///
/// Default values for ADSR envelopes
///
const adsr_envelope default_env;

///
/// ADSR envelope for the main carrier signal
///
adsr_envelope carrier_envelope;

///
/// Implements the envelope as defined in carrier_envelope
///
/// \todo Make more generic
///
/// \param *note_active Is note active?
/// \param gate         Gate
/// \param *env_level   Level
/// \param t            Time
///
/// \return amplitude
///
double envelope ( int *note_active, int gate, double *env_level, double t );

///
/// Fired when a PCM event is recieved
///
/// \param nframes Number of frames to write
///
/// \return Number of frames written
///
int playbackCallback ( snd_pcm_sframes_t nframes );

///
/// Initialise PCM
///
/// \param *pcm_name ALSA device name
///
unsigned int initPcm ( char *pcm_name );

///
/// Start the worker thread
///
unsigned int startPcm();

///
/// Turn MIDI data into sound!
///
/// \param played_note     Note
/// \param played_velocity Velocity
///
/// \return bool Success
///
unsigned int noteOn ( int played_note, double played_velocity );

///
/// Stop playing note
///
/// \param played_note     Note
///
/// \return bool Success
///
unsigned int noteOff ( int played_note );

///
/// Convert function type into an actual function pointer
///
/// \param type I.e. SINE, TRIANGLE, SAWTOOTH, SQUARE
///
/// \return generator_function
///
generator_function getFunction ( enum waveform type );

#endif // PCM_H
