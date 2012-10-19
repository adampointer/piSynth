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


#include "pcmworker.h"

PCMWorker::PCMWorker()
{

}

PCMWorker::~PCMWorker()
{

}

void PCMWorker::InternalThreadEntry()
{
    int     nfds, n;
    struct  pollfd *pfds;

    nfds = snd_pcm_poll_descriptors_count(playback_handle);
    pfds = (struct pollfd *) alloca(sizeof(struct pollfd) * nfds);
    
    snd_pcm_poll_descriptors(playback_handle, pfds, nfds);

    for(n = 0; n < POLY; note_active[n++] = 0);

    while(true) {

        if(poll(pfds, nfds, 1000) > 0) {

            for(n = 0; n < nfds; n++) {

                if(pfds[n].revents > 0) {

                    if(PlaybackCallback(BUFSIZE) < BUFSIZE) { 
                        fprintf(stderr, "---BUFFER UNDERRUN---\n");
                        snd_pcm_prepare(playback_handle);
                    }
                }
            }
        }
    }
}
    
int PCMWorker::PlaybackCallback(snd_pcm_sframes_t nframes)
{
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

double PCMWorker::Envelope(int *note_active, int gate, double *env_level, double t, ADSR *envelope)
{
    if(gate) {

        if(t > envelope->attack + envelope->decay) {
            return(*env_level = envelope->sustain);
        }

        if(t > envelope->attack) {
            return (*env_level = 1.0 - (1.0 - envelope->sustain) * (t - envelope->attack) / envelope->decay);
        }
        return(*env_level = t / envelope->attack);
    } else {

        if(t > envelope->release) {

            if (note_active) {
                *note_active = 0;
            }
            return(*env_level = 0);
        }
        return(*env_level * (1.0 - t / envelope->release));
    }
}

double PCMWorker::FastSine(double x)
{
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

double PCMWorker::SquareWave(double x)
{
    
    if(x > M_PI) {
        return 1.0;
    } else {
        return -1.0;
    }
}

double PCMWorker::TriangleWave(double x)
{
    double f = x / M_TWO_PI;
    return abs(4 * (f - floor(f + 0.5))) - 1.0;
}

double PCMWorker::SawtoothWave(double x)
{
    double f = x / M_TWO_PI;
    return  2.0 * (f - floor(f)) - 1.0;
}

