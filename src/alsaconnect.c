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
/// \file   alsaconnect.h
/// \author Adam Pointer <adam.pointer@gmx.com>
/// \brief  ALSA connections - its aconnect!
///

#include "alsaconnect.h"

unsigned int searchPorts ( snd_seq_t *seq )
{
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t   *pinfo;
    
    int len = INITIAL_INPUTS;
    int count = 0;

    inputs = ( input_t* ) malloc ( sizeof ( input_t ) * len );

    if ( inputs == NULL )
      {
        fprintf ( stderr, "Failed to allocate memory\n" );
        exit ( 1 );
      }
    snd_seq_client_info_alloca ( &cinfo );
    snd_seq_port_info_alloca ( &pinfo );
    snd_seq_client_info_set_client ( cinfo, -1 );

    while ( snd_seq_query_next_client ( seq, cinfo ) >= 0 )
      {
        snd_seq_port_info_set_client ( pinfo, snd_seq_client_info_get_client ( cinfo ) );
        snd_seq_port_info_set_port ( pinfo, -1 );

        while ( snd_seq_query_next_port ( seq, pinfo ) >= 0 )
          {

            if ( isInput ( pinfo ) )
              {

                if ( count > len )
                  {
                    input_t *temp_inputs = realloc ( inputs, sizeof ( input_t ) * len++ );

                    if ( temp_inputs == NULL )
                      {
                        fprintf ( stderr, "Failed to allocate memory\n" );
                        exit ( 1 );
                      }
                      inputs = temp_inputs;
                  }
                inputs[count].client = snd_seq_client_info_get_client ( cinfo );
                inputs[count].port = snd_seq_port_info_get_port ( pinfo );
                inputs[count].name = malloc ( sizeof ( char ) * 
                                              strlen ( snd_seq_port_info_get_name ( pinfo ) ) );
                
                if ( inputs[count].name == NULL )
                  {
                    fprintf ( stderr, "Failed to allocate memory\n" );
                    exit ( 1 );
                  }
                strcpy ( inputs[count].name, snd_seq_port_info_get_name ( pinfo ) );
                count++;
              }
          }
      }
    return count;
}

unsigned int isInput ( snd_seq_port_info_t *pinfo )
{
    unsigned int bits = SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ;

    if ( ( snd_seq_port_info_get_capability ( pinfo ) & ( bits ) ) == ( bits ) )
      {

        if ( snd_seq_port_info_get_capability ( pinfo ) & SND_SEQ_PORT_CAP_NO_EXPORT )
            return FALSE;
        return TRUE;
      }
    return FALSE;
}
