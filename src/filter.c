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

void initFilter ( filter_t *filter )
{
  filter->type = lowpass;
  filter->cutoff = 3000;
  filter->Q = 0.5;
  filter->buffer0 = filter->buffer1 = 0;

  calculateCoefficients ( filter );
}

void calculateCoefficients ( filter_t* filter )
{
  filter->coefficient = 2.0 * M_PI * filter->cutoff / rate;
  filter->feedback = filter->Q + filter->Q / ( 1.0 - filter->coefficient );
}

double filter ( double input, filter_t* filter )
{
  double hp, bp, output;

  hp = input - filter->buffer0;
  bp = filter->buffer0 - filter->buffer1;
  filter->buffer0 = filter->buffer0 + filter->coefficient *
                    ( hp + filter->feedback * bp );
  filter->buffer1 = filter->buffer1 + filter->coefficient *
                    ( filter->buffer0 - filter->buffer1 );

  switch ( filter->type )
    {
    case lowpass:
      output = filter->buffer1;
      break;

    case highpass:
      output = hp;
      break;

    case bandpass:
      output = bp;
      break;
    }

  return GAIN * output;
}
