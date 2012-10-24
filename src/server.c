/*
 *    Copyright (C) 2012 Adam Pointer <adam.pointer@gmx.com>
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
 *    files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
 *    modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 *    is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 *    IN THE SOFTWARE.
 */

#include "server.h"

unsigned int initServer() {
    const char *options[] = {"listening_ports", "8989", NULL};
    ctx = mg_start(&httpCallback, NULL, options);
    return(TRUE);
}

static void *httpCallback(enum mg_event event, struct mg_connection *conn) {
    const struct mg_request_info *request_info = mg_get_request_info(conn);

    if (event == MG_NEW_REQUEST) {
        char content[1024];
        int content_length = snprintf(content, sizeof(content),
                                    "Hello from mongoose! Remote port: %d",
                                    request_info->remote_port);
        mg_printf(conn,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %d\r\n"        // Always set Content-Length
                "\r\n"
                "%s",
                content_length, content);
        // Mark as processed
        return "";
    } else {
        return NULL;
    }
    
}