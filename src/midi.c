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
/// \file   midi.h
/// \author Adam Pointer <adam.pointer@gmx.com>
/// \brief  MIDI handling code
///

#include "midi.h"
#include "pcm.h"
#include "lfo.h"

unsigned int midiCallback()
{
  snd_seq_event_t *ev;

  do
    {
      snd_seq_event_input ( seq_handle, &ev );

      switch ( ev->type )
        {

        case SND_SEQ_EVENT_PITCHBEND:
          pitch = ( double ) ev->data.control.value / 8192.0;
          break;

        case SND_SEQ_EVENT_NOTEON:
          noteOn ( ev->data.note.note, ev->data.note.velocity );
          break;

        case SND_SEQ_EVENT_NOTEOFF:
          noteOff ( ev->data.note.note );
          resetLFO( &filter_lfo );
          break;
        }
      snd_seq_free_event ( ev );
    }
  while ( snd_seq_event_input_pending ( seq_handle, 0 ) > 0 );
  return ( TRUE );
}

unsigned int initMidi()
{
  rate = 44100;

  if ( snd_seq_open ( &seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0 ) < 0 )
    {
      fprintf ( stderr, "Error opening ALSA sequencer.\n" );
      return ( FALSE );
    }
  snd_seq_set_client_name ( seq_handle, "piSynth" );
  if ( snd_seq_create_simple_port ( seq_handle, "piSynth",
                                    SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                                    SND_SEQ_PORT_TYPE_APPLICATION ) < 0 )
    {
      fprintf ( stderr, "Error creating sequencer port.\n" );
      return ( FALSE );
    }
  return ( TRUE );
}
