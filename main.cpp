#define BUILDING_NODE_EXTENSION

#include <string>
#include <math.h>
#include <node/node.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

#define BUFSIZE  512
#define RATE     44100.0
#define TWO_PI   M_PI * 2
#define POLY     8
#define GAIN     500.0

using namespace v8;

snd_pcm_t    *playback_handle;
short        *buffer;
double       pitch, velocity[POLY], env_level[POLY], env_time[POLY], mod_phase[POLY], car_phase[POLY], modulation,
             attack, decay, sustain, release, cm_ratio, mod_amp;
int          note[POLY], note_active[POLY], gate[POLY];
unsigned int rate;
bool         run_worker;

double envelope(int *note_active, int gate, double *env_level, double t, double attack, double decay, double sustain, double release) {
    
    if(gate) {
	
        if(t > attack + decay) {
	    return(*env_level = sustain);
	}
	
        if(t > attack) {
	    return (*env_level = 1.0 - (1.0 - sustain) * (t - attack) / decay);
	}
        return(*env_level = t / attack);
    } else {
	
        if(t > release) {
	    
            if (note_active) {
		*note_active = 0;
	    }
            return(*env_level = 0);
        }
        return(*env_level * (1.0 - t / release));
    }
}

double fast_sin(double x) {
    const double a = -0.40319426317E-08;
    const double b = 0.21683205691E+03;
    const double c = 0.28463350538E-04;
    const double d = -0.30774648337E-02;

    double y;
    bool invert = false;

    if(x > 2048) {
        invert = true;
        x -= 2048;
    }

    if(x > 1024) {
        x = 2048 - x;
    }
    y = (a + x) / (b + c * x * x) + d * x;

    if(invert) {
        return -y;
    } else {
        return y;
    }
}

double square_wave(double x) {
    double y;
    y = sin(x);
    
    if(y > 0) {
        return 1.0;
    } else {
        return -1.0;
    }
}

int playback_callback(snd_pcm_sframes_t nframes) {
    int    poly, n;
    double constant, freq, freq_rad, mod_phase_increment, car_phase_increment, sound;
  
    constant = (log(2.0) / 12.0);
    freq_rad = TWO_PI / rate;
    memset(buffer, 0, nframes * 4);

    for(poly = 0; poly < POLY; poly++) {

        if(note_active[poly]) {
            freq = 8.176 * exp((double) note[poly] * constant);
    	    mod_phase_increment = freq_rad * (freq * cm_ratio);

            if(!mod_phase[poly] || !car_phase[poly]) {
                mod_phase[poly] = 0.0;
                car_phase[poly] = 0.0;
            }

	        for(n = 0; n < nframes; n++) {
	            sound = envelope(&note_active[poly], gate[poly], &env_level[poly], env_time[poly], attack, decay, sustain, release) * 
                    GAIN * velocity[poly] * sin(car_phase[poly]);
                //fprintf(stdout, "%f\n", sound);
                env_time[poly] += 1.0 / rate;
                buffer[2 * n] += sound;
	            buffer[2 * n + 1] += sound;
	            car_phase_increment = freq_rad * (freq + (mod_amp * sin(mod_phase[poly])));
                car_phase[poly] += car_phase_increment;

                if(car_phase[poly] >= TWO_PI) {
                    car_phase[poly] -= TWO_PI;
                }
                mod_phase[poly] += mod_phase_increment; 
	            
                if(mod_phase[poly] >= TWO_PI) {
                    mod_phase[poly] -= TWO_PI;
                }
            }
	    }
    }
    
    return snd_pcm_writei(playback_handle, buffer, nframes);
}

void* start_loop() {
    int     nfds, n;
    struct  pollfd *pfds;
    // HACK
    void*   dummy;

    nfds = snd_pcm_poll_descriptors_count(playback_handle);
    pfds = (struct pollfd *) alloca(sizeof(struct pollfd) * nfds);
    
    snd_pcm_poll_descriptors(playback_handle, pfds, nfds);

    for(n = 0; n < POLY; note_active[n++] = 0);

    while(run_worker) {

        if(poll(pfds, nfds, 1000) > 0) {

            for(n = 0; n < nfds; n++) {

                if(pfds[n].revents > 0) {
		    
                    if(playback_callback(BUFSIZE) < BUFSIZE) { 
                        fprintf(stderr, "---BUFFER UNDERRUN---\n");
                        snd_pcm_prepare(playback_handle);
                    }
                }
            }
        }
    }
    pthread_exit(0);
    return dummy;
}

Handle<Value> InitPcm(const Arguments& args) {
    HandleScope scope;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    const unsigned argc = 1;
    rate = 44100;
fprintf(stderr,"starting init\n");
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
 fprintf(stderr,"starting init 1\n");
    if(snd_pcm_open(&playback_handle, pcm_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Cannot open PCM device")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
    snd_pcm_hw_params_alloca(&hw_params);
 fprintf(stderr,"starting init 2\n");
    if(snd_pcm_hw_params_any(playback_handle, hw_params) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Cannot configure PCM device")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 3\n");
    if(snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting access")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 4\n");
    if(snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting format")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 5\n");
    if(snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting rate")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 6\n");
    if(snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2) < 0) {
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting channels")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 7\n");
    if(snd_pcm_hw_params_set_periods(playback_handle, hw_params, 2, 0) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting periods")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 8\n");
    if(snd_pcm_hw_params_set_period_size(playback_handle, hw_params, BUFSIZE, 0) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting period size")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 9\n");
    if(snd_pcm_hw_params(playback_handle, hw_params) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting hardware params")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
    snd_pcm_sw_params_alloca(&sw_params);
 fprintf(stderr,"starting init 10\n");
    if(snd_pcm_sw_params_current(playback_handle, sw_params) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting current software params")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 11\n");
    if(snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, BUFSIZE) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting available min")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
 fprintf(stderr,"starting init 12\n");
    if(snd_pcm_sw_params(playback_handle, sw_params) < 0) { 
        Local<Value> argv[argc] = { Local<Value>::New(String::New("Error setting software params")) };
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
        return scope.Close(Undefined());
    }
   fprintf(stderr, "Init done\n"); 
    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(Undefined());
}

Handle<Value> StartPcm(const Arguments& args) {
    HandleScope    scope;
    const unsigned argc = 1;
    pthread_attr_t attr;
    pthread_t loop;

    if (!args[0]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Wrong arguments")));
        return scope.Close(Undefined());
    }

    Local<Function> cb = Local<Function>::Cast(args[0]);

    pitch    = 0;
    buffer   = (short *) malloc(2* sizeof(short) * BUFSIZE);
    attack   = 0.01;
    decay    = 0.8;
    sustain  = 0.1;
    release  = 0.2;
    cm_ratio = 7.5;
    mod_amp  = 100;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&loop, &attr, &start_loop, NULL);
    
    Local<Value> argv[argc] = { Local<Value>::New(Number::New(0)) };
    cb->Call(Context::GetCurrent()->Global(), argc, argv);
    
    return scope.Close(Undefined());
}

Handle<Value> NoteOn(const Arguments& args) {
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

Handle<Value> NoteOff(const Arguments& args) {
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

Handle<Value> ClosePcm(const Arguments& args) { 
    HandleScope scope;
    run_worker = false;
    snd_pcm_close(playback_handle);
    free(buffer);
    return scope.Close(Undefined());
}

Handle<Value> SetEnvelope(const Arguments& args) {
    HandleScope scope;
    const unsigned argc = 1;    

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

void Init(Handle<Object> target) {
    target->Set(String::NewSymbol("initPcm"), FunctionTemplate::New(InitPcm)->GetFunction());
    target->Set(String::NewSymbol("startPcm"), FunctionTemplate::New(StartPcm)->GetFunction());
    target->Set(String::NewSymbol("closePcm"), FunctionTemplate::New(ClosePcm)->GetFunction());
    target->Set(String::NewSymbol("noteOn"), FunctionTemplate::New(NoteOn)->GetFunction());
    target->Set(String::NewSymbol("noteOff"), FunctionTemplate::New(NoteOff)->GetFunction());
    target->Set(String::NewSymbol("setEnvelope"), FunctionTemplate::New(SetEnvelope)->GetFunction());
}

NODE_MODULE(oscillator, Init)
