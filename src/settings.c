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

#include "settings.h"


static void on_bt_toggled_spdif_pro(GtkWidget *button, gpointer *user_data) {
    bbf_settings_t *gs = (bbf_settings_t*)user_data;

    if (gs->no_signals)
    return;

    gboolean v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    gs->no_signals = true;
    snd_mixer_selem_set_playback_switch(gs->spdif_pro, 0, v ? 1 : 0);
    gs->no_signals = false;

}

void bbf_settings_init(bbf_settings_t *gs) {
    gs->spdif_pro = NULL;
    gs->spdif_emph = NULL;
    gs->no_signals = false;

    gs->bt_spdif_pro = gtk_toggle_button_new_with_label("SPDIF Pro");
    g_signal_connect(gs->bt_spdif_pro, "toggled",
                         *G_CALLBACK(on_bt_toggled_spdif_pro), gs); 
    gs->bt_spdif_emph = gtk_toggle_button_new_with_label("SPDIF Emph.");
}

void bbf_settings_connect_spdif_pro(bbf_settings_t* gs, snd_mixer_elem_t* elem) {
    gs->spdif_pro = elem;
    // TODO: event handler
}

void bbf_settings_connect_spdif_emph(bbf_settings_t* gs, snd_mixer_elem_t* elem) {
    gs->spdif_emph = elem;
    // TODO: event handler
}
