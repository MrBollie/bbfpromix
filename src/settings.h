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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

#include "global.h"

typedef struct __bbf_settings {
	GtkWidget *cb_clock;
	GtkWidget *bt_spdif;
	GtkWidget *bt_spdif_pro;
	GtkWidget *bt_spdif_emph;
	snd_mixer_elem_t *clock;
	snd_mixer_elem_t *spdif;
	snd_mixer_elem_t *spdif_pro;
	snd_mixer_elem_t *spdif_emph;
	bool no_signals;
} bbf_settings_t;

void bbf_settings_init(bbf_settings_t*);
bool bbf_settings_find_and_set(bbf_settings_t*, snd_mixer_elem_t*);

#endif
