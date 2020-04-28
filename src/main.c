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

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "channel.h"

typedef struct __app_data {
    bbf_channel_t input_channels[BBF_NOF_INPUTS];
    bbf_channel_t playback_channels[BBF_NOF_INPUTS];
    snd_mixer_t *mixer;
    snd_mixer_elem_t *clock;
    GtkWidget *cb_clock;
    bool no_signals;
} bbf_app_data_t;


static void update_clock(bbf_app_data_t *app_data) {
    if (!app_data->clock) 
        return;

    unsigned int item = 0;
    snd_mixer_selem_get_enum_item(app_data->clock, 0, &item);
    app_data->no_signals = true;
    gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->cb_clock), item);
    app_data->no_signals = false;
}

static int on_selem_changed_clock(snd_mixer_elem_t *elem, unsigned int mask) {
    bbf_app_data_t *app_data = 
        (bbf_app_data_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        app_data->clock = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (app_data->no_signals)
            return 0;
        update_clock(app_data);
    }

    return 0;
}

static int connect_alsa_mixer(bbf_app_data_t *app_data) {
    int err;
    const char* card = NULL;
    snd_ctl_card_info_t* info;
    snd_ctl_card_info_alloca(&info);
    int number = -1;
    while (!card) {
        int err = snd_card_next(&number);
        if (err < 0 || number < 0) {
            break;
        }
        snd_ctl_t* ctl;
        char buf[16];
        sprintf (buf, "hw:%d", number);
        err = snd_ctl_open(&ctl, buf, 0);
        if (err < 0) {
            continue;
        }
        err = snd_ctl_card_info(ctl, info);
        snd_ctl_close(ctl);
        if (err < 0) {
            continue;
        }
        const char* card_name = snd_ctl_card_info_get_name (info);
        if (!card_name) {
            continue;
        }
        if (strstr(card_name, "Babyface Pro") != NULL) {
            // card found
            card = buf;
        }
    }
    if (!card)
        return -1;
    err = snd_mixer_open(&app_data->mixer, 0);
    if (err < 0)
        return -2;

    err = snd_mixer_attach(app_data->mixer, card);
    if (err < 0) {
        snd_mixer_close(app_data->mixer);
        app_data->mixer = NULL;
        return -3;
    }

    err = snd_mixer_selem_register(app_data->mixer, NULL, NULL);
    if (err < 0) {
        snd_mixer_close(app_data->mixer);
        app_data->mixer = NULL;
        return -4;
    }

    err = snd_mixer_load(app_data->mixer);
    if (err < 0) {
        snd_mixer_close(app_data->mixer);
        app_data->mixer = NULL;
        return -5;
    }
    return 0;
}

static void connect_alsa_mixer_elems(bbf_app_data_t *app_data) {
    snd_mixer_elem_t* elem;
    for (elem = snd_mixer_first_elem (app_data->mixer); elem;
         elem = snd_mixer_elem_next (elem)) {

        if (strcmp("Sample Clock Source", snd_mixer_selem_get_name(elem)) == 0) {
            app_data->clock = elem;
            snd_mixer_elem_set_callback(elem, on_selem_changed_clock);
            snd_mixer_elem_set_callback_private(elem, app_data);
            update_clock(app_data);
            continue;
	}
	
        for (int i = 0 ; i < BBF_NOF_INPUTS ; ++i) {
            if (bbf_channel_find_and_set(&app_data->input_channels[i], elem))
                continue;
            if (bbf_channel_find_and_set(&app_data->playback_channels[i], elem))
                continue;
        }
    }
}

static void reset_alsa_mixer_elems(bbf_app_data_t *app_data) {
    for (int i = 0 ; i < BBF_NOF_INPUTS ; ++i) {
        bbf_channel_reset(&app_data->input_channels[i]);
        bbf_channel_reset(&app_data->playback_channels[i]);
    }
}

static void on_clock_changed(GtkComboBox* combo, gpointer user_data) {
    bbf_app_data_t *app_data = (bbf_app_data_t*)user_data;
    if (app_data->no_signals || !app_data->clock)
        return;
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if (active < 0 || active > 1)
        return;

    app_data->no_signals = true;
    snd_mixer_selem_set_enum_item(app_data->clock, 0, active);
    app_data->no_signals = false;
}

static void on_output_changed(GtkComboBox* combo, gpointer user_data) {
    bbf_app_data_t *app_data = (bbf_app_data_t*)user_data;
    gint entry_id = gtk_combo_box_get_active(combo);
    for (int i = 0 ; i < BBF_NOF_INPUTS ; ++i) {
        bbf_channel_set_output(&app_data->input_channels[i], entry_id);
        bbf_channel_set_output(&app_data->playback_channels[i], entry_id);
    }
}

static gint on_timeout(gpointer user_data) {
    bbf_app_data_t *app_data = (bbf_app_data_t*)user_data;
    if (!app_data->mixer) {
        int r = connect_alsa_mixer(app_data);
        if (r == 0) {
            printf("Connected.\n");
            connect_alsa_mixer_elems(app_data);
        }
    } else {
        int r = snd_mixer_handle_events(app_data->mixer);
        if (r < 0) {
            snd_mixer_close(app_data->mixer);
            app_data->mixer = NULL;
            printf("disonnected.\n");
            reset_alsa_mixer_elems(app_data);
        }
    }
    return 1;
}

static void activate(GtkApplication *app, gpointer *user_data) {
    bbf_app_data_t *app_data = (bbf_app_data_t*)user_data;
    GtkWidget *main_window;
    GtkGrid *main_grid;
    GtkWidget *label_inputs, *label_playbacks, *label_output, *label_clock;
    GtkWidget *separator;
    GtkWidget *cb_output;

    // Initialize the main window
    main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(main_window), "Babyface Pro Mixer");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);

    // add the main grid
    main_grid = GTK_GRID(gtk_grid_new());
    gtk_grid_set_column_homogeneous (main_grid, 1);

    // Inputs
    label_inputs = gtk_label_new("Inputs");
    gtk_widget_set_hexpand(label_inputs, TRUE);
    gtk_grid_attach(main_grid, label_inputs, 0, 0, 24, 1);

    for (int i = 0 ; i < BBF_NOF_INPUTS ; ++i) {
        bbf_channel_t *ic = &app_data->input_channels[i];
        if (i < 2) {
            // Mic channel
            bbf_channel_init(ic, MIC, BBF_INPUTS[i]);
            gtk_grid_attach(main_grid, ic->lbl_name, i*2, 1, 2, 1);
            gtk_grid_attach(main_grid, ic->bt_PAD, i*2, 2, 1, 1);
            gtk_grid_attach(main_grid, ic->bt_48V, i*2+1, 2, 1, 1);
        } else if (i > 1 && i < 4) {
            // Instrument channel
            bbf_channel_init(ic, INSTR, BBF_INPUTS[i]);
            gtk_grid_attach(main_grid, ic->lbl_name, i*2, 1, 2, 1);
            gtk_grid_attach(main_grid, ic->cb_Sens, i*2, 2, 2, 1);
        } else {
            // Line channel
            bbf_channel_init(ic, LINE, BBF_INPUTS[i]);
            gtk_grid_attach(main_grid, ic->lbl_name, i*2, 1, 2, 1);
        }
        gtk_grid_attach(main_grid, ic->sc_pan, i*2, 3, 2, 1);
        gtk_widget_set_vexpand(ic->sc_vol, TRUE);
        gtk_grid_attach(main_grid, ic->sc_vol, i*2, 4, 2, 2);
    }

    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(main_grid, separator, 0, 6, 24, 1);

    // Playbacks
    label_playbacks = gtk_label_new("Playback");
    gtk_widget_set_hexpand(label_playbacks, TRUE);
    gtk_grid_attach(main_grid, label_playbacks, 0, 7, 24, 1);

    int pc_count = 0;
    for (int i = 0 ; i < BBF_NOF_OUTPUTS ; ++i) {
        bbf_channel_t *pc = &app_data->playback_channels[pc_count];
        bbf_channel_init(pc, PCM, BBF_OUTPUTS[i][0]);
        gtk_grid_attach(main_grid, pc->lbl_name, pc_count*2, 8, 2, 1);
        gtk_grid_attach(main_grid, pc->sc_pan, pc_count*2, 9, 2, 1);
        gtk_widget_set_vexpand(pc->sc_vol, TRUE);
        gtk_grid_attach(main_grid, pc->sc_vol, pc_count*2, 10, 2, 2);
        pc_count++;

        pc = &app_data->playback_channels[pc_count];
        bbf_channel_init(pc, PCM, BBF_OUTPUTS[i][1]);
        gtk_grid_attach(main_grid, pc->lbl_name, pc_count*2, 8, 2, 1);
        gtk_grid_attach(main_grid, pc->sc_pan, pc_count*2, 9, 2, 1);
        gtk_widget_set_vexpand(pc->sc_vol, TRUE);
        gtk_grid_attach(main_grid, pc->sc_vol, pc_count*2, 10, 2, 2);
        pc_count++;
    }

    // Output selector
    label_output = gtk_label_new("Output:");
    gtk_grid_attach(main_grid, label_output, 0, 12, 2, 1);
    cb_output = gtk_combo_box_text_new();
    for (int i = 0 ; i < BBF_NOF_OUTPUTS ; ++i) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cb_output), NULL,
                                  g_strdup_printf("%s/%s", BBF_OUTPUTS[i][0],
                                                  BBF_OUTPUTS[i][1]));
    }
    g_signal_connect(cb_output, "changed", *G_CALLBACK(on_output_changed),
                     app_data);
    gtk_grid_attach(main_grid, cb_output, 2, 12, 2, 1);

    // Clock
    label_clock = gtk_label_new("Clock Mode:");
    gtk_grid_attach(main_grid, label_clock, 4, 12, 2, 1);
    app_data->cb_clock = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app_data->cb_clock), NULL, 
                              "AutoSync");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app_data->cb_clock), NULL, 
                              "Internal");
    g_signal_connect(app_data->cb_clock, "changed", 
                     *G_CALLBACK(on_clock_changed),
                     app_data);
    gtk_grid_attach(main_grid, app_data->cb_clock, 6, 12, 2, 1);


    gtk_widget_set_hexpand(GTK_WIDGET(main_grid), TRUE);
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(main_grid));

    gtk_widget_show_all(main_window);
    g_timeout_add(10, on_timeout, app_data);
}


int main(int argc, char** argv)
{
    GtkApplication *app;
    bbf_app_data_t app_data;
    app_data.mixer = NULL;
    app_data.clock = NULL;
    app_data.no_signals = false;
    int status;

    app = gtk_application_new ("de.bollie.babymixpro", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), &app_data);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    if (app_data.mixer)
        snd_mixer_close(app_data.mixer);
    g_object_unref (app);

    return status;
}
