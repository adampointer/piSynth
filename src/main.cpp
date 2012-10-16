#define BUILDING_NODE_EXTENSION

#include <string>
#include <math.h>
#include <node/node.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

#define BUFSIZE  512
#define RATE     44100.0
#define POLY     8
#define GAIN     500.0

#define TRIANGLE 1
#define SQUARE   2
#define SAWTOOTH 3
#define SINE     4

#define M_TWO_PI   6.2831853071796

using namespace v8;

snd_pcm_t    *playback_handle;
short        *buffer;
double       pitch, velocity[POLY], env_level[POLY], env_time[POLY], mod_phase_1[POLY], mod_phase_2[POLY], car_phase[POLY],
             attack, decay, sustain, release, cm_ratio_1, cm_ratio_2, mod_amp_1, mod_amp_2;
int          note[POLY], note_active[POLY], gate[POLY];
unsigned int rate;
bool         run_worker;
double       (*mod_func_1)(double);
double       (*mod_func_2)(double);
double       (*car_func)(double);
unsigned int mod_func_1_type, mod_func_2_type, car_func_type;

typedef double (*func)(double);

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
    double x2 = x * x;
    double x4 = x2 * x2;

    double t1  = x * (1.0 - x2 / (2*3));
    double x5  = x * x4;
    double t2  = x5 * (1.0 - x2 / (6*7)) / (1.0 * 2 * 3 * 4 * 5);
    double x9  = x5 * x4;
    double t3  = x9 * (1.0 - x2 / (10*11)) / (1.0 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9);
    double x13 = x9 * x4;
    double t4  = x13 * (1.0 - x2 / (14 * 15)) / (1.0 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * 11 * 12 * 13);

    double y = t4;
    y += t3;
    y += t2;
    y += t1;

    return y;
}

double square_wave(double x) {
    
    if(x > M_PI) {
        return 1.0;
    } else {
        return -1.0;
    }
}

double triangle_wave(double x) {
    double f = x / M_TWO_PI;
    return abs(4 * (f - floor(f + 0.5))) - 1.0;
}

double sawtooth_wave(double x) {
    double f = x / M_TWO_PI;
    return  2.0 * (f - floor(f)) - 1.0;
}

int playback_callback(snd_pcm_sframes_t nframes) {
    int    poly, n;
    double constant, freq, freq_rad, mod_phase_1_increment, mod_phase_2_increment, car_phase_increment,
           sound, mod_value_1, mod_value_2;
  
    constant = (log(2.0) / 12.0);
    freq_rad = M_TWO_PI / rate;
    memset(buffer, 0, nframes * 4);

    for(poly = 0; poly < POLY; poly++) {

        if(note_active[poly]) {
            freq                  = 8.176 * exp((double) note[poly] * constant);
            mod_phase_1_increment = freq_rad * (freq * cm_ratio_1);
            mod_phase_2_increment = freq_rad * (freq * cm_ratio_2);

            if(!car_phase[poly]) car_phase[poly]     = 0.0;
            if(!mod_phase_1[poly]) mod_phase_1[poly] = 0.0;
            if(!mod_phase_2[poly]) mod_phase_2[poly] = 0.0;

            for(n = 0; n < nframes; n++) {
                sound = envelope(&note_active[poly], gate[poly], &env_level[poly], env_time[poly], attack, decay, sustain, release) *
                        GAIN * velocity[poly] * (*car_func)(car_phase[poly]); 
                
                env_time[poly]       += 1.0 / rate;
                buffer[2 * n]        += sound;
                buffer[2 * n + 1]    += sound;
                mod_value_1           = mod_amp_1 * (*mod_func_1)(mod_phase_1[poly]);
                mod_value_2           = mod_amp_2 * (*mod_func_2)(mod_phase_2[poly]);
                car_phase_increment   = freq_rad * (freq + mod_value_1 + mod_value_2);
                
                car_phase[poly]      += car_phase_increment;
                if(car_phase[poly]   >= M_TWO_PI) car_phase[poly] -= M_TWO_PI;

                mod_phase_1[poly]    += mod_phase_1_increment; 
                if(mod_phase_1[poly] >= M_TWO_PI)  mod_phase_1[poly] -= M_TWO_PI;
            
                mod_phase_2[poly]    += mod_phase_2_increment; 
                if(mod_phase_2[poly] >= M_TWO_PI)  mod_phase_2[poly] -= M_TWO_PI;
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

Handle<Value> GetEnvelope(const Arguments& args) {
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

Handle<Value> SetEnvelope(const Arguments& args) {
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

func GetFunction(int type) {
    
    switch(type) {
            
        case SQUARE:
            return &square_wave;

        case TRIANGLE:
            return &triangle_wave;

        case SAWTOOTH:
            return &sawtooth_wave;

        case SINE:
        default:
            return &fast_sin;
    }
}

Handle<Value> GetModulator(const Arguments& args) {
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

Handle<Value> SetModulator(const Arguments& args) {
    HandleScope scope;    
    unsigned int mod_number;

    if (args.Length() < 4) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsNumber() || !args[1]->IsString() || !args[2]->IsNumber() || !args[3]->IsNumber()) {
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

void Init(Handle<Object> target) {
    target->Set(String::NewSymbol("initPcm"), FunctionTemplate::New(InitPcm)->GetFunction());
    target->Set(String::NewSymbol("startPcm"), FunctionTemplate::New(StartPcm)->GetFunction());
    target->Set(String::NewSymbol("closePcm"), FunctionTemplate::New(ClosePcm)->GetFunction());
    target->Set(String::NewSymbol("noteOn"), FunctionTemplate::New(NoteOn)->GetFunction());
    target->Set(String::NewSymbol("noteOff"), FunctionTemplate::New(NoteOff)->GetFunction());
    target->Set(String::NewSymbol("getEnvelope"), FunctionTemplate::New(GetEnvelope)->GetFunction());
    target->Set(String::NewSymbol("setEnvelope"), FunctionTemplate::New(SetEnvelope)->GetFunction());
    target->Set(String::NewSymbol("getModulator"), FunctionTemplate::New(GetModulator)->GetFunction());
    target->Set(String::NewSymbol("setModulator"), FunctionTemplate::New(SetModulator)->GetFunction());
}

NODE_MODULE(oscillator, Init)
