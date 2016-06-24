#include <gtk/gtk.h>
#include <math.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>

#include "simpleburn.h"
#include "progress.h"
#include "callbacks.h"


void on_abort_clicked(GtkWidget *widget, gpointer user_data) {
	gchar *commandline;
	
	gtk_widget_set_sensitive((GtkWidget *) ui.bt_abort, FALSE);
	commandline = g_strdup_printf("simpleburn.sh abort %d", commandinfos.pid);
	g_spawn_command_line_sync(commandline, NULL, NULL, NULL, NULL);
	g_free(commandline);
}


void startprogress(gchar *commandline, gboolean pulse, gchar *message, gint actiontype) {
	GtkTreeIter iter;
	gchar **command;
	
	g_print("command: %s\n", commandline);
	enablebuttons(FALSE);
	gtk_label_set_text(ui.info_log, message);
	commandinfos.actiontype = actiontype;
	g_shell_parse_argv(commandline, NULL, &command, NULL);
	g_spawn_async(NULL, command, NULL, G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &commandinfos.pid, NULL);
	g_strfreev(command);
	g_child_watch_add(commandinfos.pid, stopprogress, NULL);
	commandinfos.pulseupdater = g_timeout_add(100, pulseupdater, NULL);
}


void stopprogress(GPid pid, gint status, gpointer data) {
	gchar *message;
	
	commandinfos.pid = 0;
	g_source_remove(commandinfos.pulseupdater);
	if (status == 0) message = g_strdup_printf("%s %s", gtk_label_get_text(ui.info_log), gettext("Done"));
	else message = g_strdup_printf("%s %s", gtk_label_get_text(ui.info_log), gettext("Error"));
	gtk_label_set_text(ui.info_log, message);
	g_free(message);
	
	if (commandinfos.actiontype != EXTRACT) {
		reset_mediadetection();
		while (gtk_notebook_get_n_pages(ui.tabs_actions) != 0) {
			gtk_notebook_remove_page(ui.tabs_actions, -1);
		}
	}
	enablebuttons(TRUE);
	gtk_progress_bar_set_fraction (ui.info_progressbar, 0.0);
}


gboolean pulseupdater(gpointer data) {
	gtk_progress_bar_pulse(ui.info_progressbar);
	return TRUE;
}


void enablebuttons(gboolean enabled) {
	gint i;
	
	for (i=0; i < ui.buttonscount; i++) {
		gtk_widget_set_sensitive((GtkWidget *) ui.buttons[i], enabled);
	}
	gtk_widget_set_sensitive((GtkWidget *) ui.bt_abort, !enabled);
}
