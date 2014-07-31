#include <gtk/gtk.h>
#include <math.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>

#include "simpleburn.h"
#include "progress.h"


void on_abort_clicked(GtkWidget *widget, gpointer user_data) {
	gtk_widget_set_sensitive((GtkWidget *) ui.bt_abort, FALSE);
	gchar *commandline = g_strdup_printf("simpleburn.sh abort %d", commandinfos.pid);
	g_spawn_command_line_sync(commandline, NULL, NULL, NULL, NULL);
	g_free(commandline);
}


void startprogress(gchar *commandline, gboolean pulse, gchar *message, gint actiontype) {
	g_print("command: %s\n", commandline);
	GtkTreeIter iter;
	gint stdoutfd;
	gchar **command;
	enablebuttons(FALSE);
	gtk_label_set_text(ui.info_log, message);
	//~ gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	//~ gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &commandinfos.opticaldevice, -1);
	g_get_current_time(&commandinfos.starttime);
	commandinfos.elapsedtime_handler = g_timeout_add(333, elapsedtime_handler, NULL);
	commandinfos.actiontype = actiontype;
	g_shell_parse_argv(commandline, NULL, &command, NULL);
	g_spawn_async_with_pipes(NULL, command, NULL, G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &commandinfos.pid, NULL, &stdoutfd, NULL, NULL);
	g_strfreev (command);
	g_child_watch_add(commandinfos.pid, stopprogress, NULL);
	commandinfos.estimatedtime = 0;
	if (pulse) {
		commandinfos.stdoutioc = NULL;
		commandinfos.pulsebar_handler = 0;
		commandinfos.progressbar_handler = g_timeout_add(100, pulsebar_handler, NULL);
	} else {
		commandinfos.stdoutioc = g_io_channel_unix_new(stdoutfd);
		commandinfos.pulsebar_handler = g_timeout_add(100, pulsebar_handler, NULL); //first use a pulse bar until the progress can be displayed
		commandinfos.progressbar_handler = g_io_add_watch(commandinfos.stdoutioc, G_IO_IN, progressbar_handler, NULL);
	}
}


void stopprogress(GPid pid, gint status, gpointer data) {
	commandinfos.pid = 0;
	if (commandinfos.pulsebar_handler != 0) g_source_remove(commandinfos.pulsebar_handler);
	g_source_remove(commandinfos.progressbar_handler);
	g_source_remove(commandinfos.elapsedtime_handler);
	if (commandinfos.stdoutioc != NULL) {
		g_io_channel_shutdown(commandinfos.stdoutioc, FALSE, NULL);
		g_io_channel_unref(commandinfos.stdoutioc);
	}
	if (status == 0) {
		gtk_label_set_text(ui.info_log, gettext("Operation completed."));
	} else {
		if (commandinfos.actiontype == BURN) {
			gtk_label_set_text(ui.info_log, gettext("Error: please check there is enough space on media."));
		} else {
			gtk_label_set_text(ui.info_log, gettext("Error: something went wrong."));
		}
	}
	gtk_progress_bar_set_fraction(ui.info_progressbar, 0);
	gtk_progress_bar_set_text(ui.info_progressbar, "");
	gtk_label_set_text(ui.info_estimatedtime, "");
	gtk_label_set_text(ui.info_elapsedtime, "");
	gtk_label_set_text(ui.info_remainingtime, "");
	//~ gchar *commandline = g_strdup_printf("cdrecord -eject dev=%s", commandinfos.opticaldevice);
	//~ g_spawn_command_line_sync(commandline, NULL, NULL, NULL, NULL);
	//~ g_free(commandline);
	//~ g_free(commandinfos.opticaldevice);
	if (commandinfos.actiontype != EXTRACT) {
		while (gtk_notebook_get_n_pages(ui.tabs_actions) != 0) {
			gtk_notebook_remove_page(ui.tabs_actions, -1);
		}
	}
	enablebuttons(TRUE);
}


gboolean pulsebar_handler(gpointer data) {
	gtk_progress_bar_pulse(ui.info_progressbar);
	return TRUE;
}


gboolean progressbar_handler(GIOChannel *source, GIOCondition condition, gpointer data) {
	gchar *line;
	GTimeVal currenttime;
	g_io_channel_read_line(commandinfos.stdoutioc, &line, NULL, NULL, NULL);
	if (line != NULL) {
		line [strlen(line)-1] = '\0';
		gdouble progressfraction = g_ascii_strtoull(line, NULL, 10) / 100.0;
		if (commandinfos.pulsebar_handler != 0) { //the pulse bar thread has to be ended
			g_source_remove(commandinfos.pulsebar_handler);
			commandinfos.pulsebar_handler = 0;
			g_get_current_time(&commandinfos.progress_startttime);
			gtk_progress_bar_set_fraction(ui.info_progressbar, 0);
			gtk_progress_bar_set_text(ui.info_progressbar, "  0 %");
		}
		if (progressfraction != gtk_progress_bar_get_fraction(ui.info_progressbar)) {
			gtk_progress_bar_set_fraction(ui.info_progressbar, progressfraction);
			gchar *s_progressfraction = g_strdup_printf("%3s %c", line, '%');
			gtk_progress_bar_set_text(ui.info_progressbar, s_progressfraction);
			g_free(s_progressfraction);
			g_get_current_time(&currenttime);
			glong elapsedtime = currenttime.tv_sec - commandinfos.starttime.tv_sec;
			glong progresselapsedtime = currenttime.tv_sec - commandinfos.progress_startttime.tv_sec;
			glong totaltime = (glong) (progresselapsedtime / progressfraction) + (elapsedtime - progresselapsedtime);
			if (labs(commandinfos.estimatedtime - totaltime) > 5) commandinfos.estimatedtime = totaltime;
		} else {
			gtk_progress_bar_set_fraction(ui.info_progressbar, progressfraction); //unneeded but force window repainting
		}
		g_free(line);
	}
	return TRUE;
}


gboolean elapsedtime_handler(gpointer data) {
	GTimeVal currenttime;
	g_get_current_time(&currenttime);
	glong elapsedtime = currenttime.tv_sec - commandinfos.starttime.tv_sec;
	gchar *s_elapsedtime = timeformat(elapsedtime);
	gtk_label_set_text(ui.info_elapsedtime, s_elapsedtime);
	g_free(s_elapsedtime);
	if (commandinfos.estimatedtime != 0) {
		if (gtk_progress_bar_get_fraction(ui.info_progressbar) < 1) {
			gchar *s_estimatedtime = timeformat(commandinfos.estimatedtime);
			gtk_label_set_text(ui.info_estimatedtime, s_estimatedtime);
			g_free(s_estimatedtime);
			glong remainingtime = commandinfos.estimatedtime - elapsedtime;
			gchar *s_remainingtime = timeformat(remainingtime);
			gtk_label_set_text(ui.info_remainingtime, s_remainingtime);
			g_free(s_remainingtime);
		} else {
			gtk_label_set_text(ui.info_elapsedtime, "");
			gtk_label_set_text(ui.info_remainingtime, "");
		}
	}
	return TRUE;
}


gchar *timeformat(glong timeinseconds) {
	glong minutes, seconds;
	minutes = (guint) floor(timeinseconds / 60);
	seconds = (guint) timeinseconds % 60;
	return g_strdup_printf("%02ld' %02ld''", minutes, seconds);
}


void enablebuttons(gboolean enabled) {
	for (gint i=0; i < ui.buttonscount; i++) {
		gtk_widget_set_sensitive((GtkWidget *) ui.buttons[i], enabled);
	}
	gtk_widget_set_sensitive((GtkWidget *) ui.bt_abort, !enabled);
}
