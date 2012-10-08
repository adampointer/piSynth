#include "synth.h"

snd_pcm_t *open_pcm(char *pcm_name) {
    snd_pcm_t           *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;

    if(snd_pcm_open(&playback_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "Cannot open audio device %s\n", pcm_name);
        exit(1);
    }
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(playback_handle, hw_params);
    snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0);
    snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2);
    snd_pcm_hw_params_set_periods(playback_handle, hw_params, 2, 0);
    snd_pcm_hw_params_set_period_size(playback_handle, hw_params, BUFSIZE, 0);
    snd_pcm_hw_params(playback_handle, hw_params);
    
    snd_pcm_sw_params_alloca(&sw_params);
    snd_pcm_sw_params_current(playback_handle, sw_params);
    snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, BUFSIZE);
    snd_pcm_sw_params(playback_handle, sw_params);
    
    return(playback_handle);
}

int playback_callback(snd_pcm_sframes_t nframes) {
    int    poly, n;
    double freq, freq_rad, mod_phase_increment, car_phase_increment, sound;
  
    freq_rad = two_pi / rate;
    memset(buffer, 0, nframes * 4);

    for(poly = 0; poly < POLY; poly++) {

        if(note_active[poly]) {
            freq = 8.176 * exp((double) note[poly] * harmonic_const);
    	    mod_phase_increment = freq_rad * (freq * cm_ratio);

            if(!mod_phase[poly] || !car_phase[poly]) {
                mod_phase[poly] = 0.0;
                car_phase[poly] = 0.0;
            }

	        for(n = 0; n < nframes; n++) {
	            sound = envelope(&note_active[poly], gate[poly], &env_level[poly], env_time[poly], attack, decay, sustain, release) * 
                    GAIN * velocity[poly] * square_wave(car_phase[poly]);
                fprintf(stdout, "%f\n", sound);
                env_time[poly] += 1.0 / rate;
                buffer[2 * n] += sound;
	            buffer[2 * n + 1] += sound;
	            car_phase_increment = freq_rad * (freq + (mod_amp * fast_sin(mod_phase[poly])));
                car_phase[poly] += car_phase_increment;

                if(car_phase[poly] >= two_pi) {
                    car_phase[poly] -= two_pi;
                }
                mod_phase[poly] += mod_phase_increment; 
	            
                if(mod_phase[poly] >= two_pi) {
                    mod_phase[poly] -= two_pi;
                }
            }
	    }
    }
    
    return snd_pcm_writei(playback_handle, buffer, nframes);
}

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
