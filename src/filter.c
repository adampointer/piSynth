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
/// \brief  12db/8ve butterworth filter
///

#include "filter.h"

void initFilter ( filter_t *filter )
{
  filter->type = lowpass;
  filter->input_delay1 = 0;
  filter->input_delay2 = 0;
  filter->output_delay1 = 0;
  filter->output_delay2 = 0;
  filter->cutoff = 5000;
  filter->gain = 1;

  calculateCoefficients( filter );
}

void calculateCoefficients ( filter_t* filter )
{
  double c, c2, d, csqr2;

  switch ( filter->type )
    {
    case lowpass:
      c = 1 / tan ( ( M_PI / rate ) * filter->cutoff );
      c2 = c * c;
      csqr2 = SQRT2 * c;
      d = ( c2 + csqr2 + 1 );
      filter->coefficients.amp_in0 = 1 / d;
      filter->coefficients.amp_in1 = filter->coefficients.amp_in0 +
                                     filter->coefficients.amp_in0;
      filter->coefficients.amp_in2 = filter->coefficients.amp_in0;
      filter->coefficients.amp_out1 = ( 2 * ( 1 - c2 ) ) / d;
      filter->coefficients.amp_out2 = ( c2 - csqr2 + 1 ) / d;
      break;

    case highpass:
      c = 1 / tan ( ( M_PI / rate ) * filter->cutoff );
      c2 = c * c;
      csqr2 = SQRT2 * c;
      d = ( c2 + csqr2 + 1 );
      filter->coefficients.amp_in0 = 1 / d;
      filter->coefficients.amp_in1 = - ( filter->coefficients.amp_in0 +
                                         filter->coefficients.amp_in0 );
      filter->coefficients.amp_in2 = filter->coefficients.amp_in0;
      filter->coefficients.amp_out1 = ( 2 * ( c2 - 1 ) ) / d;
      filter->coefficients.amp_out2 = ( 1 - csqr2 + c2 ) / d;
      break;

    case bandpass:
      c = 1 / tan ( ( M_PI / rate ) * filter->cutoff );
      d = 1 + c;
      filter->coefficients.amp_in0 = 1 / d;
      filter->coefficients.amp_in1 = 0;
      filter->coefficients.amp_in2 = -filter->coefficients.amp_in0;
      filter->coefficients.amp_out1 = ( -c * 2 *
                                        cos ( M_TWO_PI * filter->cutoff / rate )
                                      ) / d;
      filter->coefficients.amp_out2 = ( c - 1 ) / d;
      break;
    }
}

double filter ( double input, filter_t* filter )
{
  double output;

  output = ( filter->coefficients.amp_in0 * input ) +
           ( filter->coefficients.amp_in1 * filter->input_delay1 ) +
           ( filter->coefficients.amp_in2 * filter->input_delay2 ) -
           ( filter->coefficients.amp_out1 * filter->output_delay1 ) -
           ( filter->coefficients.amp_out2 * filter->output_delay2 );
  filter->output_delay2 = filter->output_delay1;
  filter->output_delay1 = output;
  filter->input_delay2 = filter->input_delay1;
  filter->input_delay1 = input;

  return output * filter->gain;
}
