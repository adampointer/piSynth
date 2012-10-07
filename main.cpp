#define BUILDING_NODE_EXTENSION

#include <string>
#include <math.h>
#include <node/node.h>
#include <alsa/asoundlib.h>

#define BUFSIZE 512
#define RATE    44100.0
#define TWO_PI  M_PI * 2
#define POLY    8

using namespace v8;


snd_pcm_t    *playback_handle;
short        *buffer;
double       pitch, velocity[POLY], env_level[POLY], env_time[POLY], mod_phase[POLY], car_phase[POLY], modulation,
             attack, decay, sustain, release;
int          note[POLY], note_active[POLY], gate[POLY];
unsigned int rate;

Handle<Value> InitPcm(const Arguments& args) {
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

Handle<Value> StartPcm(const Arguments& args) {
    HandleScope    scope;
    const unsigned argc = 1;
    int            nfds, n;
    struct         pollfd *pfds;

    if (args.Length() < 2) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Wrong arguments")));
        return scope.Close(Undefined());
    }

    Local<Function> cb = Local<Function>::Cast(args[1]);

    pitch   = 0;
    buffer  = (short *) malloc(2* sizeof(short) * BUFSIZE);
    attack  = 0.01;
    decay   = 0.8;
    sustain = 0.1;
    release = 0.2;
    nfds    = snd_pcm_poll_descriptors_count(playback_handle);
    pfds    = (struct pollfd *) alloca(sizeof(struct pollfd) * nfds);
    
    snd_pcm_poll_descriptors(playback_handle, pfds + seq_nfds, nfds);

    for(n = 0; n < POLY; note_active[n++] = 0);

    while(1) {

        if(poll(pfds, nfds, 1000) > 0) {

            for(n = 0; n < nfds; n++) {

                if(pfds[n].revents > 0) {
		    
                    if(playback_callback(BUFSIZE) < BUFSIZE) { 
                        Local<Value> argv[argc] = { Local<Value>::New(String::New("Buffer underrun")) };
                        cb->Call(Context::GetCurrent()->Global(), argc, argv);
                        snd_pcm_prepare(playback_handle);
                    }
                }
            }
        }
    }
    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined());
}

Handle<Value> ClosePcm(const Arguments& args) { 
    HandleScope scope;
    snd_pcm_close(playback_handle);
    free(buffer);
    return scope.Close(Undefined());
}

int playback_callback(snd_pcm_sframes_t nframes) {
    return BUFSIZE;
}

void Init(Handle<Object> target) {
    target->Set(String::NewSymbol("initPcm"), FunctionTemplate::New(InitPcm)->GetFunction());
    target->Set(String::NewSymbol("startPcm"), FunctionTemplate::New(StartPcm)->GetFunction());
    target->Set(String::NewSymbol("closePcm"), FunctionTemplate::New(ClosePcm)->GetFunction());
}

NODE_MODULE(oscillator, Init)
