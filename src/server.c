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
#include "pcm.h"

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
      else if ( strcmp ( ri->uri, "/carrier" ) == 0 )
        {
          carrierHandler ( conn, ri );
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
      writeResponse ( conn, "200 OK", json );
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
          writeResponse ( conn, "200 OK", ok_response );
        }
      else
        {
          writeResponse ( conn, "400 Bad Request", fail_response );
        }
    }
}

void modulatorHandler ( struct mg_connection *conn,
                        const struct mg_request_info *ri, int num )
{
  oscillator *osc;

  if ( num == 1 )
    osc = &modulator_1;
  else if ( num == 2 )
    osc = &modulator_2;
  else if ( num == 3 )
    osc = &modulator_3;
  else if ( num == 4 )
    osc = &modulator_4;
  else
    {
      writeResponse ( conn, "404 Not Found", fail_response );
      return;
    }

  if ( strcmp ( ri->request_method, "GET" ) == 0 )
    {
      char json[100];
      sprintf ( json, "{\"waveform\":%d, \"cm_ratio\":%f, \"amplitude\":%f}",
                osc->type, osc->cm_ratio, osc->amplitude );
      writeResponse ( conn, "200 OK", json );
    }
  else if ( strcmp ( ri->request_method, "PUT" ) == 0 )
    {
      char data[1024], waveform[10], cm_ratio[10], amplitude[10];
      int  len = mg_read ( conn, data, sizeof ( data ) );

      if ( ( mg_get_var ( data, len, "waveform",  waveform, sizeof ( waveform ) ) > 0 ) &&
           ( mg_get_var ( data, len, "cm_ratio",   cm_ratio, sizeof ( cm_ratio ) ) > 0 ) &&
           ( mg_get_var ( data, len, "amplitude", amplitude, sizeof ( amplitude ) ) > 0 ) )
        {
          osc->type = atoi ( waveform );
          osc->func = getFunction ( osc->type );
          osc->cm_ratio = atof ( cm_ratio );
          osc->amplitude = atof ( amplitude );
          writeResponse ( conn, "200 OK", ok_response );
        }
      else
        {
          writeResponse ( conn, "400 Bad Request", fail_response );
        }
    }
}

void carrierHandler ( struct mg_connection *conn, const struct mg_request_info *ri )
{
  if ( strcmp ( ri->request_method, "GET" ) == 0 )
    {
      char json[100];
      sprintf ( json, "{\"waveform\":%d}", carrier.type );
      writeResponse ( conn, "200 OK", json );
    }
  else if ( strcmp ( ri->request_method, "PUT" ) == 0 )
    {
      char data[1024], waveform[10];
      int  len = mg_read ( conn, data, sizeof ( data ) );

      if ( mg_get_var ( data, len, "waveform",  waveform, sizeof ( waveform ) ) > 0 )
        {
          carrier.type = atoi ( waveform );
          carrier.func = getFunction ( carrier.type );
          writeResponse ( conn, "200 OK", ok_response );
        }
      else
        {
          writeResponse ( conn, "400 Bad Request", fail_response );
        }
    }
}

void writeResponse ( struct mg_connection *conn, const char *response_string,
                     const char *body )
{
  mg_printf ( conn, "HTTP/1.0 %s\r\nContent-Length: %d\r\n"
              "Content-Type: application/json\r\n\r\n%s",
              response_string, ( int ) strlen ( body ), body );
}
