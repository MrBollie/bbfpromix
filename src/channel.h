/*  bbfpromix
 *
 *  Copyright 2020 Thomas Ebeling <penguins@bollie.de>
 *
 *  This file is part of bbfpromix.
 *
 *  Foobar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with bbfpromix.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CHANNEL_H_
#define __CHANNEL_H__

#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

#include "global.h"

typedef enum __channel_type {
    MIC = 0,
    INSTR,
    LINE,
    PCM
} bbf_channel_type;

typedef struct __output {
    bbf_channel_type type;
    const char *name_l;
    const char *name_r;
    snd_mixer_elem_t *elem_l;
    snd_mixer_elem_t *elem_r;
} bbf_output_t;

typedef struct __channel {
    const char *name;
    bbf_output_t outputs[BBF_NOF_OUTPUTS];
    bbf_output_t *cur_output;
    GtkWidget *bt_48V;
    GtkWidget *bt_PAD;
    GtkWidget *cb_Sens;
    GtkWidget *sc_vol;
    GtkWidget *sc_pan;
    GtkWidget *lbl_name;
    bool no_signals;
    bbf_channel_type type;
    snd_mixer_elem_t *phantom;
    snd_mixer_elem_t *pad;
    snd_mixer_elem_t *sens;
} bbf_channel_t;

bool bbf_channel_find_and_set(bbf_channel_t*, snd_mixer_elem_t*);
void bbf_channel_init(bbf_channel_t*, bbf_channel_type, const char*);
void bbf_channel_reset(bbf_channel_t*);
void bbf_channel_set_output(bbf_channel_t*,unsigned int);
void bbf_update_sliders(bbf_channel_t*);
void bbf_update_switches(bbf_channel_t*);

#endif // __CHANNEL_H_
