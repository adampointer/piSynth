/*
 *    Copyright (C) 2012 Adam Pointer <adam.pointer@gmx.com>
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
 *    files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
 *    modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 *    is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 *    IN THE SOFTWARE.
 */

#ifndef PCM_H
#define PCM_H

#include "main.h"

typedef double (*generator_function)(double);

double envelope(int *note_active, int gate, double *env_level, double t);

double fastSin(double x);

double squareWave(double x);

double triangleWave(double x);

double sawtoothWave(double x);

int playbackCallback(snd_pcm_sframes_t nframes);

unsigned int initPcm(char *pcm_name);

unsigned int startPcm();

unsigned int noteOn(int played_note, double played_velocity);

unsigned int noteOff(int played_note);

generator_function getFunction(unsigned int type);

#endif // PCM_H