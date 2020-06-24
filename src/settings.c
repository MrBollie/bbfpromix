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

static void on_bt_toggled_spdif(GtkWidget *button, gpointer *user_data) {
    bbf_settings_t *gs = (bbf_settings_t*)user_data;

    if (gs->no_signals)
        return;

    gboolean v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    gs->no_signals = true;
    snd_mixer_selem_set_playback_switch(gs->spdif, 0, v ? 1 : 0);
    gs->no_signals = false;

}

static void on_bt_toggled_spdif_emph(GtkWidget *button, gpointer *user_data) {
    bbf_settings_t *gs = (bbf_settings_t*)user_data;

    if (gs->no_signals)
        return;

    gboolean v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    gs->no_signals = true;
    snd_mixer_selem_set_playback_switch(gs->spdif_emph, 0, v ? 1 : 0);
    gs->no_signals = false;

}

static void on_bt_toggled_spdif_pro(GtkWidget *button, gpointer *user_data) {
    bbf_settings_t *gs = (bbf_settings_t*)user_data;

    if (gs->no_signals)
        return;

    gboolean v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    gs->no_signals = true;
    snd_mixer_selem_set_playback_switch(gs->spdif_pro, 0, v ? 1 : 0);
    gs->no_signals = false;

}

static void on_clock_changed(GtkComboBox* combo, gpointer user_data) {
    bbf_settings_t *gs = (bbf_settings_t*)user_data;
    if (gs->no_signals || !gs->clock)
        return;

    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if (active < 0 || active > 1)
        return;

    gs->no_signals = true;
    snd_mixer_selem_set_enum_item(gs->clock, 0, active);
    gs->no_signals = false;
}

static void update_settings(bbf_settings_t* gs) {
    gs->no_signals = true;

    if (gs->clock) {
        unsigned int item = 0;
        snd_mixer_selem_get_enum_item(gs->clock, 0, &item);
        gtk_combo_box_set_active(GTK_COMBO_BOX(gs->cb_clock), item);
    }
    if (gs->spdif) {
        int spdif = 0;
        snd_mixer_selem_get_playback_switch(gs->spdif, 0, &spdif);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gs->bt_spdif),
                                     spdif == 1);
    }
    if (gs->spdif_emph) {
        int emph = 0;
        snd_mixer_selem_get_playback_switch(gs->spdif_emph, 0, &emph);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gs->bt_spdif_emph),
                                     emph == 1);
    }
    if (gs->spdif_pro) {
        int pro = 0;
        snd_mixer_selem_get_playback_switch(gs->spdif_pro, 0, &pro);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gs->bt_spdif_pro),
                                     pro == 1);
    }

    gs->no_signals = false;
}

static int on_selem_changed_clock(snd_mixer_elem_t *elem, unsigned int mask) {
    bbf_settings_t *gs =
        (bbf_settings_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        gs->clock = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (gs->no_signals)
            return 0;
        update_settings(gs);
    }

    return 0;
}

static int on_selem_changed_spdif(snd_mixer_elem_t *elem,
                                       unsigned int mask) {
    bbf_settings_t *gs =
        (bbf_settings_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        gs->spdif = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (gs->no_signals)
            return 0;
        update_settings(gs);
    }

    return 0;
}

static int on_selem_changed_spdif_emph(snd_mixer_elem_t *elem,
                                       unsigned int mask) {
    bbf_settings_t *gs =
        (bbf_settings_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        gs->spdif_emph = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (gs->no_signals)
            return 0;
        update_settings(gs);
    }

    return 0;
}

static int on_selem_changed_spdif_pro(snd_mixer_elem_t *elem,
                                       unsigned int mask) {
    bbf_settings_t *gs =
        (bbf_settings_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        gs->spdif_pro = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (gs->no_signals)
            return 0;
        update_settings(gs);
    }

    return 0;
}

bool bbf_settings_find_and_set(bbf_settings_t* gs, snd_mixer_elem_t* elem) {
    if (strcmp("Sample Clock Source", snd_mixer_selem_get_name(elem)) == 0) {
        gs->clock = elem;
        snd_mixer_elem_set_callback(elem, on_selem_changed_clock);
        snd_mixer_elem_set_callback_private(elem, gs);
        update_settings(gs);
        return true;
    }
    else if (strcmp("IEC958", snd_mixer_selem_get_name(elem)) == 0) {
        gs->spdif = elem;
        snd_mixer_elem_set_callback(elem, on_selem_changed_spdif);
        snd_mixer_elem_set_callback_private(elem, gs);
        update_settings(gs);
        return true;
    }
    else if (strcmp("IEC958 Emphasis", snd_mixer_selem_get_name(elem)) == 0) {
        gs->spdif_emph = elem;
        snd_mixer_elem_set_callback(elem, on_selem_changed_spdif_emph);
        snd_mixer_elem_set_callback_private(elem, gs);
        update_settings(gs);
        return true;
    }
    else if (strcmp("IEC958 Pro Mask", snd_mixer_selem_get_name(elem)) == 0) {
        gs->spdif_pro = elem;
        snd_mixer_elem_set_callback(elem, on_selem_changed_spdif_pro);
        snd_mixer_elem_set_callback_private(elem, gs);
        update_settings(gs);
        return true;
    }

    return false;
}

void bbf_settings_init(bbf_settings_t *gs) {
    gs->clock = NULL;
    gs->spdif = NULL;
    gs->spdif_emph = NULL;
    gs->spdif_pro = NULL;
    gs->no_signals = false;
    gs->cb_clock = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gs->cb_clock), NULL,
                              "AutoSync");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gs->cb_clock), NULL,
                              "Internal");
    g_signal_connect(gs->cb_clock, "changed",
                     *G_CALLBACK(on_clock_changed),
                     gs);

    gs->bt_spdif = gtk_toggle_button_new_with_label("SPDIF");
    g_signal_connect(gs->bt_spdif, "toggled",
                         *G_CALLBACK(on_bt_toggled_spdif), gs); 

    gs->bt_spdif_emph = gtk_toggle_button_new_with_label("SPDIF Emph.");
    g_signal_connect(gs->bt_spdif_emph, "toggled",
                         *G_CALLBACK(on_bt_toggled_spdif_emph), gs); 

    gs->bt_spdif_pro = gtk_toggle_button_new_with_label("SPDIF Pro");
    g_signal_connect(gs->bt_spdif_pro, "toggled",
                         *G_CALLBACK(on_bt_toggled_spdif_pro), gs); 
}
