#include "midi.h"

snd_seq_t *open_seq() {
    snd_seq_t *seq_handle;
    rate = SAMPLE_RATE;

    if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        exit(1);
    }
    snd_seq_set_client_name(seq_handle, SEQ_NAME);

    if(snd_seq_create_simple_port(seq_handle, SEQ_NAME, SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION) < 0) {
        fprintf(stderr, "Error creating sequencer port.\n");
        exit(1);
    }
    return(seq_handle);
}

int midi_callback() {
    snd_seq_event_t *ev;
    int             n;

    do {
        snd_seq_event_input(seq_handle, &ev);

        switch(ev->type) {

            case SND_SEQ_EVENT_PITCHBEND:
                pitch = (double) ev->data.control.value / 8192.0;
                break;

            case SND_SEQ_EVENT_CONTROLLER:

                if(ev->data.control.param == 1) {
                    modulation = (double) ev->data.control.value / 10.0;
                }
                break;

            case SND_SEQ_EVENT_NOTEON:
                
                for(n = 0; n < POLY; n++) {

                    if(!note_active[n]) {
                        note[n]        = ev->data.note.note;
                        velocity[n]    = ev->data.note.velocity / 127.0;
                        note_active[n] = 1;
                        env_time[n]    = 0;
                        gate[n]        = 1;
                        break;
                    }
                }
                break;

            case SND_SEQ_EVENT_NOTEOFF:

                for(n = 0; n < POLY; n++) {

                    if(gate[n] && note_active[n] && (note[n] == ev->data.note.note)) { 
                        env_time[n]    = 0;
                        gate[n]        = 0;
                    }
                }
                break;
        }
        snd_seq_free_event(ev);

    } while(snd_seq_event_input_pending(seq_handle, 0) > 0);
    
    return(0);
}
