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

#ifndef ALSACONNECT_H
#define ALSACONNECT_H

#include "main.h"

#define INITIAL_INPUTS 5

///
/// An input port
///
/// \typedef
/// \struct
///
typedef struct {
    int client; ///> Client number XXX:yy
    int port;   ///> Port number xxx:YY
    char *name; ///> Port name
} input_t;

///
/// Store an array on input_t
//
input_t* inputs;

///
/// Find ports and write to the inputs structure
///
/// \param *seq Sequencer handle
///
/// \return unsigned int
///
unsigned int searchPorts ( snd_seq_t *seq );

///
/// Do we have an input port?
///
/// \param *pinfo Port info structure
///
/// \return unsigned int
///
unsigned int isInput ( snd_seq_port_info_t *pinfo );

#endif //ALSACONNECT_H
