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


#define BUILDING_NODE_EXTENSION

#include "oscillator.h"

using namespace v8;

static void Oscillator::Init(Handle<Object> target) 
{
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Oscillator"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("initPcm"), FunctionTemplate::New(InitPcm)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("startPcm"), FunctionTemplate::New(StartPcm)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("closePcm"), FunctionTemplate::New(ClosePcm)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("noteOn"), FunctionTemplate::New(NoteOn)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("noteOff"), FunctionTemplate::New(NoteOff)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("getEnvelope"), FunctionTemplate::New(GetEnvelope)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("setEnvelope"), FunctionTemplate::New(SetEnvelope)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("getModulator"), FunctionTemplate::New(GetModulator)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("setModulator"), FunctionTemplate::New(SetModulator)->GetFunction());
    // Constructor
    Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
    target->Set(String::NewSymbol("Oscillator"), constructor);
}

Handle<Value> Oscillator::New(const Arguments& args) {
  HandleScope scope;

  Oscillator* osc = new Oscillator();
  osc->Wrap(args.This());

  return args.This();
}

Handle<Value> Oscillator::InitPcm(const Arguments& args) {
    HandleScope scope;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    const unsigned argc = 1;
    rate = 44100;

    if (args.Length() < 2) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsString() || !args[1]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Wrong arguments")));
        return scope.Close(Undefined());
    }

    Local<Function> cb = Local<Function>::Cast(args[1]);
    std::string pcm_name(*String::AsciiValue(args[0]));

    if(snd_pcm_open(&playback_handle, pcm_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Cannot open PCM device")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
    snd_pcm_hw_params_alloca(&hw_params);

    if(snd_pcm_hw_params_any(playback_handle, hw_params) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Cannot configure PCM device")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting access")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting format")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting rate")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting channels")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params_set_periods(playback_handle, hw_params, 2, 0) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting periods")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params_set_period_size(playback_handle, hw_params, BUFSIZE, 0) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting period size")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_hw_params(playback_handle, hw_params) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting hardware params")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
    snd_pcm_sw_params_alloca(&sw_params);

    if(snd_pcm_sw_params_current(playback_handle, sw_params) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting current software params")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, BUFSIZE) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting available min")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    if(snd_pcm_sw_params(playback_handle, sw_params) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting software params")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }

    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::StartPcm(const Arguments& args) {
    HandleScope    scope;
    const unsigned argc = 1;
    pthread_attr_t attr;
    pthread_t loop;

    if (!args[0]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Wrong arguments")));
        return scope.Close(Undefined());
    }

    Local<Function> cb = Local<Function>::Cast(args[0]);

    pitch      = 0;
    buffer     = (short *) malloc(2* sizeof(short) * BUFSIZE);
    attack     = 0.01;
    decay      = 0.8;
    sustain    = 0.1;
    release    = 0.2;
    cm_ratio_1 = cm_ratio_2 = 2.0;
    mod_amp_1  = mod_amp_2  = 100;

    run_worker = true;

    car_func      = mod_func_1      = mod_func_2      = &fast_sin;
    car_func_type = mod_func_1_type = mod_func_2_type = SINE;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&loop, &attr, &start_loop, NULL);
    
    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::NoteOn(const Arguments& args) {
    HandleScope scope;
    const unsigned argc = 1;
    int n;

    if (args.Length() < 3) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Argument 0 should be a number")));
        return scope.Close(Undefined());
    }

    if (!args[1]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Argument 1 should be a number")));
        return scope.Close(Undefined());
    }

    if (!args[2]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Argument 2 should be a function")));
        return scope.Close(Undefined());
    }
    
    int played_note        = args[0]->IntegerValue(); 
    double played_velocity = args[1]->NumberValue();
    Local<Function> cb     = Local<Function>::Cast(args[2]);

    for(n = 0; n < POLY; n++) {

        if(!note_active[n]) {
            note[n]        = played_note;
            velocity[n]    = played_velocity / 127.0;
            note_active[n] = 1;
            env_time[n]    = 0;
            gate[n]        = 1;
            break;
        }
    }

    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::NoteOff(const Arguments& args) {
    HandleScope scope;
    const unsigned argc = 1;
    int n;

    if (args.Length() < 2) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Argument 0 should be a number")));
        return scope.Close(Undefined());
    }

    if (!args[1]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Argument 2 should be a function")));
        return scope.Close(Undefined());
    }
    
    int played_note    = args[0]->IntegerValue(); 
    Local<Function> cb = Local<Function>::Cast(args[1]);

    for(n = 0; n < POLY; n++) {

        if(gate[n] && note_active[n] && (note[n] == played_note)) { 
            env_time[n]    = 0;
            gate[n]        = 0;
        }
    }
    
    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::ClosePcm(const Arguments& args) { 
    HandleScope scope;
    run_worker = false;
    snd_pcm_close(playback_handle);
    free(buffer);
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::GetEnvelope(const Arguments& args) {
    HandleScope scope;
    const unsigned argc = 1;

    if (args.Length() < 1) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Argument should be a function")));
        return scope.Close(Undefined());
    }
     
    Local<Function> cb = Local<Function>::Cast(args[0]);
    Local<Object> envelope = Object::New();
    envelope->Set(String::New("attack"), Number::New(attack));
    envelope->Set(String::New("decay"), Number::New(decay));
    envelope->Set(String::New("sustain"), Number::New(sustain));
    envelope->Set(String::New("release"), Number::New(release));
    Local<Value> argv[argc] = { envelope };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::SetEnvelope(const Arguments& args) {
    HandleScope scope;    

    if (args.Length() < 4) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Arguments should be numbers")));
        return scope.Close(Undefined());
    }

    attack  = args[0]->NumberValue();
    decay   = args[1]->NumberValue();
    sustain = args[2]->NumberValue();
    release = args[3]->NumberValue();

    return scope.Close(Undefined());
}

Handle<Value> Oscillator::GetModulator(const Arguments& args) {
    HandleScope scope;
    const unsigned argc = 1;

    if (args.Length() < 2) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Argument should be a number")));
        return scope.Close(Undefined());
    }

    if (!args[1]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Argument should be a function")));
        return scope.Close(Undefined());
    }
    
    Local<Function> cb      = Local<Function>::Cast(args[1]);
    Local<Object> modulator = Object::New();
    unsigned int mod_number = args[0]->NumberValue();

    if(mod_number == 1) {
        modulator->Set(String::New("waveform"), Number::New(mod_func_1_type));
        modulator->Set(String::New("cm_ratio"), Number::New(cm_ratio_1));
        modulator->Set(String::New("amplitude"), Number::New(mod_amp_1));
    } else if(mod_number == 2) {
        modulator->Set(String::New("waveform"), Number::New(mod_func_2_type));
        modulator->Set(String::New("cm_ratio"), Number::New(cm_ratio_2));
        modulator->Set(String::New("amplitude"), Number::New(mod_amp_2));
    } else {
        ThrowException(Exception::TypeError(String::New("Should be a valid modulator number (1 or 2)")));
        return scope.Close(Undefined());
    }
    Local<Value> argv[argc] = { modulator };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
}

Handle<Value> Oscillator::SetModulator(const Arguments& args) {
    HandleScope scope;    
    unsigned int mod_number;

    if (args.Length() < 4) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
        ThrowException(Exception::TypeError(String::New("Invalid argument types")));
        return scope.Close(Undefined());
    }

    mod_number         = args[0]->NumberValue();
    unsigned int type  = args[1]->NumberValue();

    if(mod_number == 1) {
        mod_func_1       = GetFunction(type);
        mod_func_1_type  = type;
        cm_ratio_1       = args[2]->NumberValue();
        mod_amp_1        = args[3]->NumberValue();
    } else if(mod_number == 2) {
        mod_func_2       = GetFunction(type);
        mod_func_2_type  = type;
        cm_ratio_2       = args[2]->NumberValue();
        mod_amp_2        = args[3]->NumberValue();
    } else {
        ThrowException(Exception::TypeError(String::New("Should be a valid modulator number (1 or 2)")));
        return scope.Close(Undefined());
    }

    return scope.Close(Undefined());
}