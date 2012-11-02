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

#ifndef SERVER_H
#define SERVER_H

#include "main.h"

///
/// JSON response on success
///
static const char *ok_response = "{\"ok\": true}";

///
/// JSON response on failure
///
static const char *fail_response = "{\"ok\": false}";

///
/// Initialise HTTP server
///
/// \return bool Success
///
unsigned int initServer();

///
/// Fired on an HTTP event
///
/// \param event  Event type
/// \param *conn  HTTP connection
///
static void *httpCallback ( enum mg_event event, struct mg_connection *conn );

///
/// Processes requests made to the envelope endpoint
///
/// \param *conn HTTP connection
/// \param *ri   Request info
///
void envelopeHandler ( struct mg_connection *conn, const struct mg_request_info *ri );

///
/// Processes requests made to the modulator endpoint
///
/// \param *conn HTTP connection
/// \param *ri   Request info
/// \param num   Modulator number
///
void modulatorHandler ( struct mg_connection *conn,
                        const struct mg_request_info *ri, int num );

///
/// Processes requests made to the carrier endpoint
///
/// \param *conn HTTP connection
/// \param *ri   Request info
///
void carrierHandler ( struct mg_connection *conn, const struct mg_request_info *ri);

///
/// Write a response to the client
/// \param *conn            HTTP connection
/// \param *response_string HTTP response e.g. "404 Not Found"
/// \param *body            Response body
///
void writeResponse(struct mg_connection *conn, const char *response_string, const char *body);

#endif // SERVER_H
