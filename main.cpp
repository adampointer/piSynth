#include <string>
#include <v8.h>
#include <nodejs/node.h>
#include <alsa/asoundlib.h>

#define BUFSIZE 512

#define REQ_FUNC_ARG(I, VAR)                                     \
    if(args.Length() <= (I) || !args[I]->IsFunction())           \
        return ThrowException(Exception::TypeError(              \
            String::New("Argument " #I " must be a function"))); \
    Local<Function> VAR = Local<Function>::Cast(args[I]);

using namespace node;
using namespace v8;

class Oscillator: ObjectWrap {
private:
    snd_pcm_t *playback_handle;
public:
    Oscillator() {
    }

    ~Oscillator() {
    }

    static Persistent<FunctionTemplate> s_ct;

    static void Init(Handle<Object> target) {
        HandleScope scope;
        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("Oscillator"));

        NODE_SET_PROTOTYPE_METHOD(s_ct, "initSound", InitSound);

        target->Set(String::NewSymbol("Oscillator"), s_ct->GetFunction());
    }

    static Handle<Value> New(const Arguments& args) {
        HandleScope scope;
        Oscillator* osc = new Oscillator();
        osc->Wrap(args.This());
        return args.This();
    }

    static Handle<Value> InitSound(const Arguments& args) {
        HandleScope scope;    
        unsigned int rate = 44100;

        snd_pcm_hw_params_t *hw_params;
        snd_pcm_sw_params_t *sw_params;
        
        Oscillator* osc = ObjectWrap::Unwrap<Oscillator>(args.This());
        
        //REC_FUNC_ARG(1, cb);

        if(args.Length() == 0 || !args[0]->IsString()) {
            return ThrowException(Exception::TypeError(String::New("First argument must be a string")));
        }
        std::string pcm_name(*String::AsciiValue(args[0]));
        
        if(snd_pcm_open(&osc->playback_handle, pcm_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0) < 0) {
            fprintf(stderr, "Cannot open audio device %s\n", pcm_name.c_str());
            exit(1);
        }
        snd_pcm_hw_params_alloca(&hw_params);
        snd_pcm_hw_params_any(osc->playback_handle, hw_params);
        snd_pcm_hw_params_set_access(osc->playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(osc->playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_rate_near(osc->playback_handle, hw_params, &rate, 0);
        snd_pcm_hw_params_set_channels(osc->playback_handle, hw_params, 2);
        snd_pcm_hw_params_set_periods(osc->playback_handle, hw_params, 2, 0);
        snd_pcm_hw_params_set_period_size(osc->playback_handle, hw_params, BUFSIZE, 0);
        snd_pcm_hw_params(osc->playback_handle, hw_params);
        
        snd_pcm_sw_params_alloca(&sw_params);
        snd_pcm_sw_params_current(osc->playback_handle, sw_params);
        snd_pcm_sw_params_set_avail_min(osc->playback_handle, sw_params, BUFSIZE);
        snd_pcm_sw_params(osc->playback_handle, sw_params);
        
        return scope.Close(Undefined());
    }
};

Persistent<FunctionTemplate> Oscillator::s_ct;

extern "C" {
    static void init(Handle<Object> target) {
        Oscillator::Init(target);
    }

    NODE_MODULE(oscillator, init);
}

