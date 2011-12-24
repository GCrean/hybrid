/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include "pref.h"
#include "util.h"

#include "conv.h"
#include "preference.h"
#include "gtkutils.h"

static HybridPreference *pref_window = NULL;

enum {
    TAB_POS_NAME_COL,
    TAB_POS_VALUE_COL,
    TAB_POS_COLS
};

static GtkTreeModel*
create_tab_pos_model(void)
{
    GtkTreeStore *store;
    GtkTreeIter iter;

    store = gtk_tree_store_new(TAB_POS_COLS,
                G_TYPE_STRING,
                G_TYPE_INT);

    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter,
            TAB_POS_NAME_COL, _("Top"),
            TAB_POS_VALUE_COL, GTK_POS_TOP,
            -1);

    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter,
            TAB_POS_NAME_COL, _("Right"),
            TAB_POS_VALUE_COL, GTK_POS_RIGHT,
            -1);

    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter,
            TAB_POS_NAME_COL, _("Bottom"),
            TAB_POS_VALUE_COL, GTK_POS_BOTTOM,
            -1);

    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter,
            TAB_POS_NAME_COL, _("Left"),
            TAB_POS_VALUE_COL, GTK_POS_LEFT,
            -1);

    return GTK_TREE_MODEL(store);
}

/**
 * Create the combo box for choosing the tab positons.
 */
static GtkWidget*
chat_theme_combo_create()
{
    GtkWidget *combo;
    GtkTreeStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    HybridChatTheme *themes;
    gchar *chat_theme_name = NULL;
    gchar *name = NULL;
    gint i;

    store = gtk_tree_store_new(TAB_POS_COLS,
                G_TYPE_STRING,
                G_TYPE_INT);

    themes = hybrid_chat_window_get_themes();

    for (i = 0; themes[i].name; i++) {
        gtk_tree_store_append(store, &iter, NULL);
        gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, themes[i].name, -1);
    }

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(
            GTK_CELL_LAYOUT(combo), renderer, FALSE);
    gtk_cell_layout_set_attributes(
            GTK_CELL_LAYOUT(combo), renderer,
            "text", TAB_POS_NAME_COL, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    /*
     * Set the active item with the local data.
     */
    if ((chat_theme_name = hybrid_pref_get_string("chat_theme")) != NULL) {
        
        if(gtk_tree_model_get_iter_root(GTK_TREE_MODEL(store), &iter)) {

            do {
                gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
                        TAB_POS_NAME_COL, &name, -1);

                if (g_strcmp0(name, chat_theme_name) == 0) {

                    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter);

                    break;
                }

            } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter));
        }
    }

    g_free(chat_theme_name);
    g_object_unref(store);
    
    return combo;
}

static GtkWidget*
tab_pos_combo_create()
{
    GtkWidget *combo;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    gint value;
    gint tab_pos;

    model = create_tab_pos_model();
    combo = gtk_combo_box_new_with_model(model);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(
            GTK_CELL_LAYOUT(combo), renderer, FALSE);
    gtk_cell_layout_set_attributes(
            GTK_CELL_LAYOUT(combo), renderer,
            "text", TAB_POS_NAME_COL, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    /*
     * Set the active item with the local data.
     */
    if ((tab_pos = hybrid_pref_get_int("tab_pos")) != -1) {
        
        if(gtk_tree_model_get_iter_root(model, &iter)) {

            do {
                gtk_tree_model_get(model, &iter, TAB_POS_VALUE_COL, &value, -1);

                if (value == tab_pos) {

                    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter);

                    break;
                }

            } while (gtk_tree_model_iter_next(model, &iter));
        }
    }

    g_object_unref(model);
    
    return combo;
}

/**
 * Initialize the basic settings page.
 */
void
pref_basic_init(GtkFixed *fixed)
{
    GtkWidget *label;

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), _("<b>Chat Window：</b>"));

    gtk_fixed_put(fixed, label, 20, 10);

    pref_window->hcb_check = 
        gtk_check_button_new_with_label(_("Hide Action Buttons"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->hcb_check),
            hybrid_pref_get_boolean("hide_chat_buttons"));
    gtk_fixed_put(fixed, pref_window->hcb_check, 20, 35);

    label = gtk_label_new(_("Chat Theme:"));
    gtk_fixed_put(fixed, label, 20, 65);

    pref_window->chat_theme_combo = chat_theme_combo_create();
    gtk_fixed_put(fixed, pref_window->chat_theme_combo, 120, 60);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label),
            _("<b>Tabs:</b>"));
    gtk_fixed_put(fixed, label, 20, 95);

    pref_window->single_cw_check =
        gtk_check_button_new_with_label(_("Show Messages In A Single Window With Tabs"));

    gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(pref_window->single_cw_check),
            hybrid_pref_get_boolean("single_chat_window"));
    gtk_fixed_put(fixed, pref_window->single_cw_check, 20, 125);

    label = gtk_label_new(_("Tab Position:"));
    gtk_fixed_put(fixed, label, 20, 165);

    pref_window->tab_pos_combo = tab_pos_combo_create();
    gtk_fixed_put(fixed, pref_window->tab_pos_combo, 120, 160);
}

/**
 * Initialize the sound settings panel.
 */
static void
pref_sound_init(GtkFixed *fixed)
{
    pref_window->mute_check = gtk_check_button_new_with_label(_("Mute"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_window->mute_check),
            hybrid_pref_get_boolean("mute"));
    gtk_fixed_put(fixed, pref_window->mute_check, 20, 15);
}

/**
 * Callback function for the cancel button.
 */
static void
cancel_cb(GtkWidget *widget, gpointer user_data)
{
    g_return_if_fail(pref_window != NULL);

    gtk_widget_destroy(pref_window->window);
}

/**
 * Callback function for the save button.
 */
static void
save_cb(GtkWidget *widget, gpointer user_data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint tab_pos;
    gchar *chat_theme;

    g_return_if_fail(pref_window != NULL);

    if (gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(pref_window->mute_check))) {

        hybrid_pref_set_boolean("mute", TRUE);

    } else {
        hybrid_pref_set_boolean("mute", FALSE);
    }

    if (gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(pref_window->hcb_check))) {

        hybrid_pref_set_boolean("hide_chat_buttons", TRUE);

    } else {
        hybrid_pref_set_boolean("hide_chat_buttons", FALSE);
    }

    if (gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(pref_window->single_cw_check))) {

        hybrid_pref_set_boolean("single_chat_window", TRUE);

    } else {
        hybrid_pref_set_boolean("single_chat_window", FALSE);
    }

    /*
     * Tab position settings.
     */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(pref_window->tab_pos_combo));

    gtk_combo_box_get_active_iter(
            GTK_COMBO_BOX(pref_window->tab_pos_combo), &iter);

    gtk_tree_model_get(model, &iter, TAB_POS_VALUE_COL, &tab_pos, -1);

    hybrid_pref_set_int("tab_pos", tab_pos);

    /*
     * Chat theme settings.
     */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(pref_window->chat_theme_combo));

    gtk_combo_box_get_active_iter(
            GTK_COMBO_BOX(pref_window->chat_theme_combo), &iter);

    gtk_tree_model_get(model, &iter, TAB_POS_NAME_COL, &chat_theme, -1);

    hybrid_pref_set_string("chat_theme", chat_theme);

    hybrid_pref_save();

    gtk_widget_destroy(pref_window->window);
}

/**
 * Initialize the preference window.
 */
void
pref_window_init(void)
{
    GtkWidget *fixed;
    GtkWidget *label;
    GtkWidget *vbox;
    GtkWidget *action_area;
    GtkWidget *button;

    g_return_if_fail(pref_window != NULL);

    pref_window->notebook = gtk_notebook_new();

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(pref_window->window), vbox);

    gtk_box_pack_start(GTK_BOX(vbox), pref_window->notebook, TRUE, TRUE, 0);

    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(pref_window->notebook), GTK_POS_TOP);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(pref_window->notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(pref_window->notebook), TRUE);

    fixed = gtk_fixed_new();
    label = gtk_label_new(_("Basic Settings"));
    gtk_notebook_append_page(GTK_NOTEBOOK(pref_window->notebook),
                             fixed, label);

    pref_basic_init(GTK_FIXED(fixed));

    fixed = gtk_fixed_new();
    label = gtk_label_new(_("Sound"));
    gtk_notebook_append_page(GTK_NOTEBOOK(pref_window->notebook),
                            fixed, label);

    pref_sound_init(GTK_FIXED(fixed));

    action_area = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5);

    button = gtk_button_new_with_label(_("Save"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 0);
    g_signal_connect(button, "clicked", G_CALLBACK(save_cb), NULL);

    button = gtk_button_new_with_label(_("Cancel"));
    gtk_widget_set_size_request(button, 100, 30);
    gtk_box_pack_end(GTK_BOX(action_area), button, FALSE, FALSE, 5);
    g_signal_connect(button, "clicked", G_CALLBACK(cancel_cb), NULL);

}

/**
 * Callback function for destroying the preference window.
 */
void
destroy_cb(GtkWidget *widget, gpointer user_data)
{
    if (pref_window) {
        g_free(pref_window);
        pref_window = NULL;
    }
}

void
hybrid_pref_create(void)
{
    if (pref_window) {

        gtk_window_present(GTK_WINDOW(pref_window->window));

        return;
    }

    pref_window = g_new0(HybridPreference, 1);

    pref_window->window = hybrid_create_window(_("Preference"), NULL,
                                        GTK_WIN_POS_CENTER, FALSE);

    g_signal_connect(pref_window->window, "destroy",
                     G_CALLBACK(destroy_cb), NULL);

    gtk_widget_set_size_request(pref_window->window, 450, 300);
    gtk_container_set_border_width(GTK_CONTAINER(pref_window->window), 8);

    pref_window_init();

    gtk_widget_show_all(pref_window->window);
}
