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

///
/// \file   lfo.h
/// \author Adam Pointer <adam.pointer@gmx.com>
/// \brief  Flexible and reusable LFO
///

#include "lfo.h"
#include "pcm.h"

void initLFO ( lfo_t* lfo )
{
  lfo->enabled = TRUE;
  lfo->phase = ( double * ) calloc ( POLY, sizeof ( double ) );
  lfo->frequency = 0.2;
  lfo->amplitude = 1000;
}

void lfo ( double* param, lfo_t* lfo, unsigned int poly )
{
  double phase_increment, mod;

  if ( lfo->enabled )
    {
      if ( !lfo->initial ) lfo->initial = *param;

      phase_increment = ( M_TWO_PI / rate ) * lfo->frequency;
      mod = lfo->amplitude * fastSin( lfo->phase[poly] );
      *param = lfo->initial + mod;
      lfo->phase[poly] += phase_increment;

      if ( lfo->phase[poly] >= M_TWO_PI ) lfo->phase[poly] -= M_TWO_PI;
    }
}

void resetLFO ( lfo_t* lfo, unsigned int poly )
{
  lfo->phase[poly] = 0;
}

