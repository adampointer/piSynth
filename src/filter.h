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

#ifndef FILTER_H
#define FILTER_H

#include "main.h"

///
/// Filter type
///
enum filter_type {
  lowpass,  ///< Low pass
  highpass, ///< High pass
  bandpass  ///< Band pass
};

///
/// Structure defining the complete filter
/// \typedef
/// \struct
///
typedef struct {
  enum filter_type type;   ///< Filter type
  double cutoff;           ///< Cutoff frequency (Hz)
  double Q;                ///< Resonance 0 - 1
  double feedback;         ///< Feedback
  double coefficient;      ///< Coefficient
  double gain;             ///< Gain
} filter_t;

///
/// The main filter
///
filter_t primary_filter;

///
/// Buffers for the delayed signals
///
double **delay_buffer;

///
/// Initialise the filter
///
void initFilter ( filter_t *filter );

///
/// Pre-calculate the coefficients
///
/// \param *filter Pointer to the filter structure
///
void calculateCoefficients (filter_t *filter );

///
/// Apply the filter to the input sample
///
/// \param input   Input sample
/// \param *filter Pointer to filter structure
/// \param poly    Note number
///
double filter ( double input, filter_t *filter, unsigned int poly );

#endif //FILTER_H
