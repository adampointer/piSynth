#ifndef SYNTH_H
#define SYNTH_H

#include "main.h"

/**
* @brief Open PCM device for playback
*
* @param pcm_name
*
* @return playback_handle
*/
snd_pcm_t *open_pcm(char *pcm_name);

/**
* @brief Does the actual synthesis
*
* @param nframes
*
* @return int
*/
int playback_callback(snd_pcm_sframes_t nframes);

/**
 * @brief Envelope generator
 * 
 * @param note_active
 * @param gate
 * @param env_level
 * @param t
 * @param attack
 * @param decay
 * @param sustain
 * @param release
 */
double envelope(int *note_active, int gate, double *env_level, double t, double attack, double decay, double sustain, double release);

#endif