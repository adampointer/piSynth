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

///
/// \file   filter.h
/// \author Adam Pointer <adam.pointer@gmx.com>
/// \brief  Second order resonant filter
///

#include "filter.h"
#include "lfo.h"

void initFilter ( filter_t *filter )
{
  int i;

  delay_buffer = ( double ** ) calloc ( POLY, sizeof ( double * ) );

  if ( delay_buffer == NULL )
    {
      fprintf ( stderr, "Could not allocate delay buffer" );
      exit ( 1 );
    }

  for ( i = 0; i < POLY; i++ )
    {
      delay_buffer[i] = ( double * ) calloc ( 2, sizeof ( double ) );

      if ( delay_buffer[i] == NULL )
        {
          fprintf ( stderr, "Could not allocate delay buffer" );
          exit ( 1 );
        }
    }

  filter->type = lowpass;
  filter->cutoff = 2000;
  filter->Q = 0.9;
  filter->gain = GAIN;

  calculateCoefficients ( filter );
}

void calculateCoefficients ( filter_t* filter )
{
  filter->coefficient = 2.0 * M_PI * filter->cutoff / rate;
  filter->feedback = filter->Q + filter->Q / ( 1.0 - filter->coefficient );
}

double filter ( double input, filter_t* filter, unsigned int poly )
{
  double hp, bp, output;

  lfo ( &( filter->cutoff ), &filter_lfo, poly );
  calculateCoefficients ( filter );

  hp = input - delay_buffer[poly][0];
  bp = delay_buffer[poly][0] - delay_buffer[poly][1];
  delay_buffer[poly][0] = delay_buffer[poly][0] + filter->coefficient *
                    ( hp + filter->feedback * bp );
  delay_buffer[poly][1] = delay_buffer[poly][1] + filter->coefficient *
                    ( delay_buffer[poly][0] - delay_buffer[poly][1] );

  switch ( filter->type )
    {
    case lowpass:
      output = delay_buffer[poly][1];
      break;

    case highpass:
      output = hp;
      break;

    case bandpass:
      output = bp;
      break;
    }

  return filter->gain * output;
}
