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
    double freq, phase, phase_increment, sound;
  
    memset(buffer, 0, nframes * 4);

    for(poly = 0; poly < POLY; poly++) {

        if(note_active[poly]) {
            freq = 8.176 * exp((double) note[poly] * harmonic_const);
	    phase_increment = (two_pi / rate) * freq;
	    phase = 0;

	    for(n = 0; n < nframes; n++) {
	      sound = GAIN * velocity[poly] * sin(phase);
	      buffer[2 * n] += sound;
	      buffer[2 * n + 1] += sound;
	      phase += phase_increment;
	      
	      if(phase >= two_pi) {
		phase -= two_pi;
	      }
	    }
	}
    }
    
    return snd_pcm_writei(playback_handle, buffer, nframes);
}