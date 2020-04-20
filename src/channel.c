#include "channel.h"
#include <gtk/gtk.h>

static int on_selem_changed(snd_mixer_elem_t *elem, unsigned int mask) {
    bbf_channel_t *c =
        (bbf_channel_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        bbf_channel_reset(c);
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (c->no_signals)
            return 0;
        bbf_update_sliders(c);
    }

    return 0;
}

static int on_selem_changed_48V(snd_mixer_elem_t *elem, unsigned int mask) {
    bbf_channel_t *c =
        (bbf_channel_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        c->phantom = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (c->no_signals)
            return 0;
        bbf_update_switches(c);
    }

    return 0;
}

static int on_selem_changed_pad(snd_mixer_elem_t *elem, unsigned int mask) {
    bbf_channel_t *c =
        (bbf_channel_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        c->pad = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (c->no_signals)
            return 0;
        bbf_update_switches(c);
    }

    return 0;
}

static int on_selem_changed_sens(snd_mixer_elem_t *elem, unsigned int mask) {
    bbf_channel_t *c =
        (bbf_channel_t*)snd_mixer_elem_get_callback_private(elem);

    if (mask == SND_CTL_EVENT_MASK_REMOVE) {
        c->sens = NULL;
    }
    else if (mask == SND_CTL_EVENT_MASK_VALUE) {
        if (c->no_signals)
            return 0;
	bbf_update_switches(c);
    }

    return 0;
}

static void on_bt_toggled_48V(GtkWidget* button, gpointer user_data) {
    bbf_channel_t *c = (bbf_channel_t*)user_data;
    if (c->no_signals || !c->phantom)
        return;

    gboolean v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    c->no_signals = true;
    snd_mixer_selem_set_playback_switch(c->phantom, 0, v ? 1 : 0);
    c->no_signals = false;
}

static void on_bt_toggled_PAD(GtkWidget* button, gpointer user_data) {
    bbf_channel_t *c = (bbf_channel_t*)user_data;
    if (c->no_signals || !c->pad)
        return;

    gboolean v = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    c->no_signals = true;
    snd_mixer_selem_set_playback_switch(c->pad, 0, v ? 1 : 0);
    c->no_signals = false;
}

static void on_cb_sens(GtkWidget* combo, gpointer user_data) {
    bbf_channel_t *c = (bbf_channel_t*)user_data;
    if (c->no_signals || !c->sens)
        return;
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    if (active < 0 || active > 1)
        return;

    c->no_signals = true;
    snd_mixer_selem_set_enum_item(c->sens, 0, active);
    c->no_signals = false;
}

static void on_slider_changed(GtkWidget* slider, gpointer user_data) {
    bbf_channel_t *c = (bbf_channel_t*)user_data;
    if (c->no_signals || !c->cur_output || !c->cur_output->elem_l ||
         !c->cur_output->elem_r)
        return;

    double val_l = 0;
    double val_r = 0;
    double pan = gtk_range_get_value(GTK_RANGE(c->sc_pan));
    double vol = gtk_range_get_value(GTK_RANGE(c->sc_vol));

   if (pan < 0) {
        // Rechts reduzieren
        pan = pan * -1; // normalisieren
        double diff = vol / 100. * pan;
        val_r = vol - diff;
        val_l = vol;
    }
    else if (pan > 0){
        // links reduzieren
        double diff = vol / 100. * pan;
        val_l = vol - diff;
        val_r = vol;
    }
    else {
        val_l = vol;
        val_r = vol;
    }
    c->no_signals = true;
    snd_mixer_selem_set_playback_volume_all(c->cur_output->elem_l, (int)val_l);
    snd_mixer_selem_set_playback_volume_all(c->cur_output->elem_r, (int)val_r);
    c->no_signals = false;
}


/** \brief Initialises a channel
 *
 * \param a pointer to an instance of bbf_channel_t
 * \param the type of the channel (MIC, LINE, INSTR, PCM)
 */
void bbf_channel_init(bbf_channel_t *channel, bbf_channel_type type,
                      const char *name) {

    channel->cur_output = NULL;
    channel->pad = NULL;
    channel->phantom = NULL;
    channel->sens = NULL;
    channel->no_signals = false;
    channel->name = name;
    channel->type = type;

    for (int i = 0 ; i < BBF_NOF_OUTPUTS ; ++i) {
        channel->outputs[i].name_l = BBF_OUTPUTS[i][0];
        channel->outputs[i].name_r = BBF_OUTPUTS[i][1];
        channel->outputs[i].elem_l = NULL;
        channel->outputs[i].elem_r = NULL;
    }

    channel->lbl_name = gtk_label_new(name);
    channel->sc_pan = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                               -100, 100, 1);
    gtk_scale_add_mark(GTK_SCALE(channel->sc_pan), 0, GTK_POS_TOP, NULL);
    g_signal_connect(channel->sc_pan, "value-changed",
                     *G_CALLBACK(on_slider_changed), channel);

    channel->sc_vol = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL,
                                               0, 46341, 1);
    gtk_range_set_inverted(GTK_RANGE(channel->sc_vol), 1);
    gtk_scale_add_mark(GTK_SCALE(channel->sc_vol), 32768, GTK_POS_RIGHT, 
                       NULL);
    g_signal_connect(channel->sc_vol, "value-changed",
                     *G_CALLBACK(on_slider_changed), channel);

    if (channel->type == MIC) {
        channel->bt_48V = gtk_toggle_button_new_with_label("48V");
        g_signal_connect(channel->bt_48V, "toggled",
                         *G_CALLBACK(on_bt_toggled_48V), channel);
        channel->bt_PAD = gtk_toggle_button_new_with_label("PAD");
        g_signal_connect(channel->bt_PAD, "toggled",
                         *G_CALLBACK(on_bt_toggled_PAD), channel);
    } else if (channel->type == INSTR) {
        channel->cb_Sens = gtk_combo_box_text_new();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(channel->cb_Sens),
                                  NULL, "-10 dBu");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(channel->cb_Sens),
                                  NULL, "+4 dBV");
        g_signal_connect(channel->cb_Sens, "changed",
                         *G_CALLBACK(on_cb_sens), channel);
    }
}

void bbf_channel_reset(bbf_channel_t *channel) {
    for (int i = 0 ; i < BBF_NOF_OUTPUTS ; ++i) {
        channel->outputs[i].elem_l = NULL;
        channel->outputs[i].elem_r = NULL;
    }
    channel->pad = NULL;
    channel->phantom = NULL;
    channel->sens = NULL;
}

void bbf_channel_set_output(bbf_channel_t *channel, unsigned int output) {
    if (output > BBF_NOF_OUTPUTS)
        return;
    channel->cur_output = &channel->outputs[output];
    bbf_update_sliders(channel);
}



/** \brief finds and sets the channels mixer elements
 *
 * \param pointer to the channel to be checked/modified
 * \param mixer element to be assigned
 * \return true if the matching output was found and set
 *
 */
bool bbf_find_and_set(bbf_channel_t *channel, snd_mixer_elem_t *elem) {
    char name[32];
    if (channel->type == MIC) {
        snprintf(name, 32, "Mic-%s 48V", channel->name);
        if (strcmp(name, snd_mixer_selem_get_name(elem)) == 0) {
            channel->phantom = elem;
            snd_mixer_elem_set_callback(elem, on_selem_changed_48V);
            snd_mixer_elem_set_callback_private(elem, channel);
            bbf_update_switches(channel);
            return true;
        }
        snprintf(name, 32, "Mic-%s PAD", channel->name);
        if (strcmp(name, snd_mixer_selem_get_name(elem)) == 0) {
            channel->pad = elem;
            snd_mixer_elem_set_callback(elem, on_selem_changed_pad);
            snd_mixer_elem_set_callback_private(elem, channel);
            bbf_update_switches(channel);
            return true;
        }
    } else if (channel->type == INSTR) {
        snprintf(name, 32, "Line-%s Sens.", channel->name);
        if (strcmp(name, snd_mixer_selem_get_name(elem)) == 0) {
            channel->sens = elem;
            snd_mixer_elem_set_callback(elem, on_selem_changed_sens);
            snd_mixer_elem_set_callback_private(elem, channel);
            bbf_update_switches(channel);
            return true;
        }
    }
    for (int i = 0 ; i < BBF_NOF_OUTPUTS ; ++i) {
        for (int j = 0 ; j < 2 ; ++j) {
            snprintf(name, 32, "%s-%s-%s",
                     (channel->type == MIC ? "Mic" :
                      (channel->type == PCM ? "PCM" : "Line")),
                     channel->name,
                     (j == 0 ? channel->outputs[i].name_l :
                      channel->outputs[i].name_r)
            );
            if (strcmp(name, snd_mixer_selem_get_name(elem)) == 0) {
                if (j == 0)
                    channel->outputs[i].elem_l = elem;
                else
                    channel->outputs[i].elem_r = elem;

                snd_mixer_elem_set_callback(elem, on_selem_changed);
                snd_mixer_elem_set_callback_private(elem, channel);

                return true;
            }
        }
    }
    return false;
}

void bbf_update_switches(bbf_channel_t *channel) {
    channel->no_signals = true;
    if (channel->type == MIC) {
        if (channel->phantom) {
            int phantom = 0;
            snd_mixer_selem_get_playback_switch(channel->phantom, 0, &phantom);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(channel->bt_48V),
                                         phantom == 1);
        }

        if (channel->pad) {
            int pad = 0;
            snd_mixer_selem_get_playback_switch(channel->pad, 0, &pad);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(channel->bt_PAD),
                                         pad == 1);
        }
    } else if (channel->type == INSTR) {
        if (channel->sens) {
            unsigned int item = 0;
            snd_mixer_selem_get_enum_item(channel->sens, 0, &item);
            gtk_combo_box_set_active(GTK_COMBO_BOX(channel->cb_Sens), item);
        }
    }
    channel->no_signals = false;
}

void bbf_update_sliders(bbf_channel_t *channel) {
    if (!channel->cur_output || !channel->cur_output->elem_l ||
        !channel->cur_output->elem_r)
        return;

    long val_r = 0;
    long val_l = 0;
    channel->no_signals = true;
    snd_mixer_selem_channel_id_t cid = (snd_mixer_selem_channel_id_t)0;
    snd_mixer_selem_get_playback_volume(channel->cur_output->elem_l, cid,
                                        &val_l);
    snd_mixer_selem_get_playback_volume(channel->cur_output->elem_r, cid,
                                        &val_r);
    int32_t diff = val_r - val_l;
    int8_t pan = 0;
    uint32_t fader = 0;
    if (diff < 0) {
        // l ist lauter
        pan = (int8_t)(100./val_l * diff);
        fader = val_l;
    }
    else if (diff > 0) {
        // r ist lauter
        pan = (int8_t)(100./val_r * diff);
        fader = val_r;
    }
    else {
        pan = 0;
        fader = val_l;
    }
    gtk_range_set_value(GTK_RANGE(channel->sc_pan), pan);
    gtk_range_set_value(GTK_RANGE(channel->sc_vol), fader);
    channel->no_signals = false;
}
