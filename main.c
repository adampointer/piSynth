#include "main.h"

int main(int argc, char *argv[]) {
    int    nfds, seq_nfds, l1, frames_written;
    struct pollfd *pfds;
    
    if(argc < 5) {
        fprintf(stderr, "pisynth <device> <FM> <harmonic> <subharmonic>\n");
        exit(1);
    }
    modulation     = atof(argv[2]);
    harmonic       = atoi(argv[3]);
    subharmonic    = atoi(argv[4]);
    pitch          = 0;
    buffer         = (short *) malloc(2* sizeof(short) * BUFSIZE);
    two_pi         = M_PI * 2;
    harmonic_const = (log(2.0) / 12.0);
    period         = 1 / SAMPLE_RATE; 
    
    playback_handle = open_pcm(argv[1]);
    seq_handle      = open_seq();
    seq_nfds        = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    nfds            = snd_pcm_poll_descriptors_count(playback_handle);
    pfds            = (struct pollfd *) alloca(sizeof(struct pollfd) * (seq_nfds + nfds));

    snd_seq_poll_descriptors(seq_handle, pfds, seq_nfds, POLLIN);
    snd_pcm_poll_descriptors(playback_handle, pfds + seq_nfds, nfds);

    for(l1 = 0; l1 < POLY; note_active[l1++] = 0);

    while(1) {

        if(poll(pfds, seq_nfds + nfds, 1000) > 0) {

            for(l1 = 0; l1 < seq_nfds; l1++) {

                if(pfds[l1].revents > 0) {
                    midi_callback();
                }
            }

            for(l1 = seq_nfds; l1 < seq_nfds + nfds; l1++) {

                if(pfds[l1].revents > 0) {
		    frames_written = playback_callback(BUFSIZE);
		    
                    if(frames_written < BUFSIZE) {
                        fprintf(stderr, "xrun!\n");
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
