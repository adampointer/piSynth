#ifndef MIDI_H
#define MIDI_H

#include "main.h"

/**
* @brief Open the ALSA sequencer
*
* @return seq_handle
*/
snd_seq_t *open_seq();

/**
* @brief Respons to MIDI events
*
* @return int
*/
int midi_callback();

#endif