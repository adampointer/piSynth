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
/// \file   main.h
/// \author Adam Pointer <adam.pointer@gmx.com>
/// \brief  Main application entry point
///

#include "main.h"
#include "pcm.h"
#include "midi.h"
#include "server.h"

void* startLoop ()
{
  int     seq_nfds, nfds, result, i, j, k;
  struct  pollfd *pfds;

  seq_nfds = snd_seq_poll_descriptors_count ( seq_handle, POLLIN );
  nfds = snd_pcm_poll_descriptors_count ( playback_handle );
  pfds = ( struct pollfd * ) alloca ( sizeof ( struct pollfd ) *
                                          ( seq_nfds + nfds ) );

  snd_seq_poll_descriptors ( seq_handle, pfds, seq_nfds, POLLIN );
  snd_pcm_poll_descriptors ( playback_handle, pfds+seq_nfds, nfds );

  for ( i = 0; i < POLY; note_active[i++] = 0 );
  memset ( buffer, 0, BUFSIZE * 4 );

  while ( run_worker )
    {

      if ( poll ( pfds, seq_nfds + nfds, 1000 ) > 0 )
        {

          for ( j = 0; j < seq_nfds; j++ )
              if ( pfds[j].revents > 0 ) midiCallback();

          for ( k = seq_nfds; k < seq_nfds + nfds; k++ )
            {

              if ( pfds[k].revents > 0 )
                {
                  result = playbackCallback ( BUFSIZE );
                  
                  if ( result == -EPIPE )
                    {
                       fprintf ( stderr, "Buffer underrun\n" );
                       snd_pcm_prepare ( playback_handle );
                     }
                  else if ( result < 0 )
                     {
                       fprintf ( stderr, "Playback error: %s\n", snd_strerror ( result ) );
                     }
                  else if ( result != BUFSIZE )
                     {
                       fprintf ( stderr, "Short write, only %d frames written", result );
                     }
                }
            }
        }
    }
  pthread_exit ( 0 );
}

void cleanShutdown ()
{
  fprintf ( stderr, "\nCTRL+C caught, attempting clean shutdown...\n" );
  run_worker = FALSE;
  sleep ( 1 );
  snd_pcm_close ( playback_handle );
  snd_seq_close ( seq_handle );
  mg_stop ( ctx );
  free ( buffer );
}

int main ( int argc, char *argv[] )
{

  if ( argc < 2 )
    {
      fprintf ( stderr, "Usage: piSynth <pcm_name>\n" );
      exit ( TRUE );
    }
  signal ( SIGINT, cleanShutdown );

  if ( initMidi() == TRUE )
      fprintf ( stderr, "MIDI initialised\n" );

  if ( initPcm ( argv[1] ) == TRUE )
      fprintf ( stderr, "PCM initialised\n" );

  if ( initServer() == TRUE )
      fprintf ( stderr, "HTTP interface initialised\n" );

  startPcm();
  exit ( 0 );
}
