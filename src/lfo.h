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

#ifndef LFO_H
#define LFO_H

#include "main.h"

///
/// LFO
///
/// \typedef
/// \struct
///
typedef struct {
  double frequency;     ///< LFO frequency
  double *phase;        ///< Stores the phase between samples
  double amplitude;     ///< LFO amplitude
  double initial;       ///< Stores initial value of modulated param
  unsigned int enabled; ///< Enable the LFO
} lfo_t;

///
/// LFO for the primary filter
///
lfo_t filter_lfo;

///
/// Initialise the LFO
///
/// \param *lfo Pointer to LFO structure
///
void initLFO ( lfo_t *lfo );

///
/// Modulate a parameter with the LFO
///
/// \param *param Pointer to modulated parameter
/// \param *lfo   Pointer to LFO structure
/// \param poly   Poly number
///
void lfo ( double *param, lfo_t *lfo, unsigned int poly );

///
/// Reset to initial state
///
/// \param lfo  Pointer to LFO structure
/// \param poly Poly number
///
void resetLFO ( lfo_t *lfo, unsigned int poly );
#endif // LFO_H