#include "main.h"

int main(int argc, char *argv[]) {
    int    nfds, seq_nfds, n;
    struct pollfd *pfds;
    
    if(argc < 2) {
        fprintf(stderr, "pisynth <device>\n");
        exit(1);
    }

    pitch          = 0;
    buffer         = (short *) malloc(2* sizeof(short) * BUFSIZE);
    two_pi         = M_PI * 2;
    
    playback_handle = open_pcm(argv[1]);
    seq_handle      = open_seq();
    seq_nfds        = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    nfds            = snd_pcm_poll_descriptors_count(playback_handle);
    pfds            = (struct pollfd *) alloca(sizeof(struct pollfd) * (seq_nfds + nfds));
    harmonic_const  = (log(2.0) / 12.0);
    
    attack   = 0.01;
    decay    = 0.8;
    sustain  = 0.1;
    release  = 0.2;
    mod_amp  = 100;
    cm_ratio = 7.5;

    snd_seq_poll_descriptors(seq_handle, pfds, seq_nfds, POLLIN);
    snd_pcm_poll_descriptors(playback_handle, pfds + seq_nfds, nfds);

    for(n = 0; n < POLY; note_active[n++] = 0);

    while(1) {

        if(poll(pfds, seq_nfds + nfds, 1000) > 0) {

            for(n = 0; n < seq_nfds; n++) {

                if(pfds[n].revents > 0) {
                    midi_callback();
                }
            }

            for(n = seq_nfds; n < seq_nfds + nfds; n++) {

                if(pfds[n].revents > 0) {
		    
                    if(playback_callback(BUFSIZE) < BUFSIZE) {
                        fprintf(stderr, "---- BUFFER UNDERRUN ----\n");
                        snd_pcm_prepare(playback_handle);
                    }
                }
            }
        }
    }
    snd_pcm_close(playback_handle);
    snd_seq_close(seq_handle);
    free(buffer);
    return(0);
}
