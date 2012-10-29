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
/// \file   server.h
/// \author Adam Pointer <adam.pointer@gmx.com>
/// \brief  HTTP server
///

#include "server.h"

unsigned int initServer()
{
  const char *options[] =
  {
    "listening_ports", "8989",
    "document_root", DOCUMENT_ROOT,
    NULL
  };
  ctx = mg_start ( &httpCallback, NULL, options );
  return ( TRUE );
}

static void *httpCallback ( enum mg_event event, struct mg_connection *conn )
{
  const struct mg_request_info *ri = mg_get_request_info ( conn );
  int arg_1;

  if ( event == MG_NEW_REQUEST )
    {

      if ( strcmp ( ri->uri, "/envelope" ) == 0 )
        {
          envelopeHandler ( conn, ri );
          return "";
        }
      else if ( sscanf ( ri->uri, "/modulator/%d", &arg_1 ) == 1 )
        {
          modulatorHandler ( conn, ri, arg_1 );
          return "";
        }
    }
  return NULL;
}

void envelopeHandler ( struct mg_connection *conn, const struct mg_request_info *ri )
{

  if ( strcmp ( ri->request_method, "GET" ) == 0 )
    {
      char json[100];
      sprintf ( json,
                "{\"attack\":%f, \"decay\":%f, \"sustain\":%f,\"release\":%f}",
                carrier_envelope.attack, carrier_envelope.decay,
                carrier_envelope.sustain, carrier_envelope.release );
      mg_printf ( conn, "HTTP/1.0 200 OK\r\n"
                  "Content-Length: %d\r\n"
                  "Content-Type: application/json\r\n\r\n%s",
                  ( int ) strlen ( json ), json );
    }
  else if ( strcmp ( ri->request_method, "PUT" ) == 0 )
    {
      char data[1024], attack[10], decay[10], sustain[10], release[10];
      int  len = mg_read ( conn, data, sizeof ( data ) );

      if ( ( mg_get_var ( data, len, "attack",  attack, sizeof ( attack ) ) > 0 ) &&
           ( mg_get_var ( data, len, "decay",   decay, sizeof ( decay ) ) > 0 ) &&
           ( mg_get_var ( data, len, "sustain", sustain, sizeof ( sustain ) ) > 0 ) &&
           ( mg_get_var ( data, len, "release", release, sizeof ( release ) ) > 0 ) )
        {
          carrier_envelope.attack = atof ( attack );
          carrier_envelope.decay = atof ( decay );
          carrier_envelope.sustain = atof ( sustain );
          carrier_envelope.release = atof ( release );
          mg_printf ( conn, "HTTP/1.0 200 OK\r\n"
                      "Content-Length: %d\r\n"
                      "Content-Type: application/json\r\n\r\n%s",
                      ( int ) strlen ( ok_response ), ok_response );
        }
      else
        {
          mg_printf ( conn, "HTTP/1.0 400 Bad Request\r\n"
                      "Content-Length: %d\r\n"
                      "Content-Type: application/json\r\n\r\n%s",
                      ( int ) strlen ( fail_response ), fail_response );
        }
    }
}

void modulatorHandler ( struct mg_connection *conn,
                        const struct mg_request_info *ri, int num )
{

  if ( strcmp ( ri->request_method, "GET" ) == 0 )
    {
      char json[100];

      if ( num == 1 )
        sprintf ( json, "{\"waveform\":%d, \"cm_ratio\":%f, \"amplitude\":%f",
                  mod_func_1_type, cm_ratio_1, mod_amp_1 );
      else if ( num == 2 )
        sprintf ( json, "{\"waveform\":%d, \"cm_ratio\":%f, \"amplitude\":%f",
                  mod_func_2_type, cm_ratio_2, mod_amp_2 );
      else
        mg_printf ( conn, "HTTP/1.0 404 Not Found\r\n"
                    "Content-Length: %d\r\n"
                    "Content-Type: application/json\r\n\r\n%s",
                    ( int ) strlen ( fail_response ), fail_response );
        
      mg_printf ( conn, "HTTP/1.0 200 OK\r\n"
                  "Content-Length: %d\r\n"
                  "Content-Type: application/json\r\n\r\n%s",
                  ( int ) strlen ( json ), json );
    }
  else if ( strcmp ( ri->request_method, "PUT" ) == 0 )
    {

    }
}
