#include "main.h"

double envelope(int *note_active, int gate, double *env_level, double t) {
    
    if(gate) {

        if(t > carrier_envelope.attack + carrier_envelope.decay) {
            return(*env_level = carrier_envelope.sustain);
        }

        if(t > carrier_envelope.attack) {
            return (*env_level = 1.0 - (1.0 - carrier_envelope.sustain) * (t - carrier_envelope.attack) / carrier_envelope.decay);
        }
        return(*env_level = t / carrier_envelope.attack);
    } else {

        if(t > carrier_envelope.release) {

            if (note_active) {
                *note_active = 0;
            }
            return(*env_level = 0);
        }
        return(*env_level * (1.0 - t / carrier_envelope.release));
    }
}

double fastSin(double x) {
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

double squareWave(double x) {
    
    if(x > M_PI) {
        return 1.0;
    } else {
        return -1.0;
    }
}

double triangleWave(double x) {
    double f = x / M_TWO_PI;
    return abs(4 * (f - floor(f + 0.5))) - 1.0;
}

double sawtoothWave(double x) {
    double f = x / M_TWO_PI;
    return  2.0 * (f - floor(f)) - 1.0;
}

int midiCallback() {
    snd_seq_event_t *ev;
  
    do {
        snd_seq_event_input(seq_handle, &ev);
	
        switch (ev->type) {
	  
            case SND_SEQ_EVENT_PITCHBEND:
                pitch = (double)ev->data.control.value / 8192.0;
                break;
		
	    case SND_SEQ_EVENT_NOTEON:
                noteOn(ev->data.note.note, ev->data.note.velocity);
                break;   
		
            case SND_SEQ_EVENT_NOTEOFF:
                noteOff(ev->data.note.note);
                break;        
        }
        snd_seq_free_event(ev);
    } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
    return TRUE;
}

int playbackCallback(snd_pcm_sframes_t nframes) {
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
                sound = envelope(&note_active[poly], gate[poly], &env_level[poly], env_time[poly]) *
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

void* startLoop() {
    int     seq_nfds, nfds, n;
    struct  pollfd *pfds;

    nfds = snd_pcm_poll_descriptors_count(playback_handle);
    pfds = (struct pollfd *) alloca(sizeof(struct pollfd) * nfds);
    
    snd_pcm_poll_descriptors(playback_handle, pfds, nfds);

    for(n = 0; n < POLY; note_active[n++] = 0);

    while(run_worker) {

        if (poll (pfds, seq_nfds + nfds, 1000) > 0) {
	  
            for (n = 0; n < seq_nfds; n++) {
               if (pfds[n].revents > 0) midiCallback();
            }
            
            for (n = seq_nfds; n < seq_nfds + nfds; n++) {   
		
                if (pfds[n].revents > 0) { 

                    if (playbackCallback(BUFSIZE) < BUFSIZE) {
                        fprintf (stderr, "=== Buffer Underrun ===\n");
                        snd_pcm_prepare(playback_handle);
                    }
                }
            }        
        }
    }

    pthread_exit(0);
}

unsigned int initMidi() {
    rate = 44100;
    
    if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        return FALSE;
    }
    snd_seq_set_client_name(seq_handle, "piSynth");
    if (snd_seq_create_simple_port(seq_handle, "piSynth",
        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION) < 0) {
        fprintf(stderr, "Error creating sequencer port.\n");
        return FALSE;
    }
    return TRUE;
}

unsigned int initPcm(char *pcm_name) {
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    const unsigned argc = 1;
    rate = 44100;

    if(snd_pcm_open(&playback_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0) < 0) { 
        fprintf(stderr, "Cannot open PCM device");
        return FALSE;
    }
    snd_pcm_hw_params_alloca(&hw_params);

    if(snd_pcm_hw_params_any(playback_handle, hw_params) < 0) {
	fprintf(stderr, "Cannot configure PCM device");
        return FALSE;
    }

    if(snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
	fprintf(stderr, "Error setting access");
        return FALSE;
    }

    if(snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
	fprintf(stderr, "Error setting format");
        return FALSE;
    }

    if(snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0) < 0) {
	fprintf(stderr, "Error setting rate");
        return FALSE;
    }

    if(snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2) < 0) {
	fprintf(stderr, "Error setting channels");
        return FALSE;
    }

    if(snd_pcm_hw_params_set_periods(playback_handle, hw_params, 2, 0) < 0) { 
	fprintf(stderr, "Error setting periods");
        return FALSE;
    }

    if(snd_pcm_hw_params_set_period_size(playback_handle, hw_params, BUFSIZE, 0) < 0) { 
	fprintf(stderr, "Error setting period size");
        return FALSE;
    }

    if(snd_pcm_hw_params(playback_handle, hw_params) < 0) { 
	fprintf(stderr, "Error setting hardware params");
        return FALSE;
    }
    snd_pcm_sw_params_alloca(&sw_params);

    if(snd_pcm_sw_params_current(playback_handle, sw_params) < 0) { 
	fprintf(stderr, "Error setting current software params");
        return FALSE;
    }

    if(snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, BUFSIZE) < 0) { 
	fprintf(stderr, "Error setting available min");
        return FALSE;
    }

    if(snd_pcm_sw_params(playback_handle, sw_params) < 0) { 
	fprintf(stderr, "Error setting software params");
        return FALSE;
    }
    return TRUE;
}

unsigned int startPcm() {
    pthread_t loop;

    pitch                    = 0;
    buffer                   = (short *) malloc(2* sizeof(short) * BUFSIZE);
    carrier_envelope.attack  = 0.01;
    carrier_envelope.decay   = 0.8;
    carrier_envelope.sustain = 0.1;
    carrier_envelope.release = 0.2;
    cm_ratio_1 = cm_ratio_2  = 2.0;
    mod_amp_1  = mod_amp_2   = 100;

    run_worker = TRUE;

    car_func      = mod_func_1      = mod_func_2      = &fastSin;
    car_func_type = mod_func_1_type = mod_func_2_type = SINE;

    pthread_create(&loop, NULL, &startLoop, NULL);
    pthread_join(loop, NULL);
    
    return TRUE;
}

unsigned int noteOn(int played_note, double played_velocity) {
    int n;
    
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
    return TRUE;
}

unsigned int noteOff(int played_note) {
    int n;

    for(n = 0; n < POLY; n++) {

        if(gate[n] && note_active[n] && (note[n] == played_note)) { 
            env_time[n]    = 0;
            gate[n]        = 0;
        }
    }
    return TRUE;
}

unsigned int closePcm() { 
    run_worker = FALSE;
    snd_pcm_close(playback_handle);
    free(buffer);
    return TRUE;
}

generator_function getFunction(unsigned int type) {
    
    switch(type) {
            
        case SQUARE:
            return &squareWave;

        case TRIANGLE:
            return &triangleWave;

        case SAWTOOTH:
            return &sawtoothWave;

        case SINE:
        default:
            return &fastSin;
    }
}

int main(int argc, char *argv) {
    initMidi();
    initPcm("hw:0");
    startPcm();
    exit(0);
}