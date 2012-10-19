/*
    Copyright (c) 2012 Adam Pointer <adam.pointer@gmx.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/


#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <nodejs/node.h>
#include <alsa/asoundlib.h>

using namespace v8;

class Oscillator : public node::ObjectWrap {
public:
    static void Init(Handle<Object> target);
    
private:
    snd_pcm_t *playback_handle;
    Oscillator();
    ~Oscillator();
    Handle<Value> New(const Arguments& args);
    Handle<Value> InitPcm(const Arguments& args);
    Handle<Value> StartPcm(const Arguments& args);
    Handle<Value> NoteOn(const Arguments& args);
    Handle<Value> NoteOff(const Arguments& args);
    Handle<Value> ClosePcm(const Arguments& args);
    Handle<Value> GetEnvelope(const Arguments& args);
    Handle<Value> SetEnvelope(const Arguments& args);
    Handle<Value> GetModulator(const Arguments& args);
    Handle<Value> SetModulator(const Arguments& args);
};

#endif //OSCILLATOR_H