#include <gtk/gtk.h>
#include <libintl.h>
#include <string.h>
#include <cdio/cdio.h>
#include <cdio/mmc_cmds.h>
#include <cdio/cd_types.h>

#include "simpleburn.h"
#include "callbacks.h"
#include "progress.h"
#include "mediainfos.h"


void callbackslink() {}
	
	
void on_ejectmedia_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice;
	
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gchar *commandline = g_strdup_printf("cdrecord -eject dev=%s", opticaldevice);
	g_spawn_command_line_sync(commandline, NULL, NULL, NULL, NULL );
	g_free(commandline);
	g_free(opticaldevice);
	reset_mediadetection();
	gtk_label_set_text(ui.info_log, "");
}


void on_selected_toggled(GtkCellRendererToggle *cell_renderer, gchar *path, gpointer user_data) {
	GtkTreeIter iter;
	gboolean selected;
	
	gtk_tree_model_get_iter_from_string((GtkTreeModel *) ui.model_tracks, &iter, path);
	gtk_tree_model_get((GtkTreeModel *) ui.model_tracks, &iter, 0, &selected, -1);
	selected = !selected;
	gtk_list_store_set(ui.model_tracks, &iter, 0, selected, -1);
}


void on_blankfast_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *commandline;
	
	if (g_ascii_strncasecmp(gettext("none"), gtk_label_get_text(ui.info_mediacontent), 4) != 0) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	commandline = g_strdup_printf("simpleburn.sh %s b-blank fast", opticaldevice);
	startprogress(commandline, TRUE, gettext("Fast blanking ..."), BLANK);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_blankfull_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *commandline;
	
	if (g_ascii_strncasecmp(gettext("none"), gtk_label_get_text(ui.info_mediacontent), 4) != 0) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	commandline = g_strdup_printf("simpleburn.sh %s b-blank all", opticaldevice);
	startprogress(commandline, TRUE, gettext("Full blanking ..."), BLANK);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_fileburn_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *source, *quotetsource, *commandline;
	
	if (g_ascii_strncasecmp(gettext("none"), gtk_label_get_text(ui.info_mediacontent), 4) != 0) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	source = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_burnfile);
	if (source == NULL) {
		gtk_label_set_text(ui.info_log, gettext("Error: please select file to burn."));
		return;
	}
	//TODO: check size
	quotetsource = g_shell_quote(source);
	commandline = g_strdup_printf("simpleburn.sh %s b-iso %s", opticaldevice, quotetsource);
	startprogress(commandline, FALSE, gettext("Burning file ..."), BURN);
	g_free(commandline);
	g_free(quotetsource);
	g_free(source);
	g_free(opticaldevice);
}


void on_dirburn_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *source;
	if (g_ascii_strncasecmp(gettext("none"), gtk_label_get_text(ui.info_mediacontent), 4) != 0) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	source = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_burndir);
	if (source == NULL) {
		gtk_label_set_text(ui.info_log, gettext("Error: please select directory to burn."));
		return;
	}
	//TODO: check size
	gchar *quotetsource = g_shell_quote(source);
	gchar *commandline = g_strdup_printf("simpleburn.sh %s b-data %s", opticaldevice, quotetsource);
	startprogress(commandline, FALSE,  gettext("Burning data ..."), BURN);
	g_free(commandline);
	g_free(quotetsource);
	g_free(source);
	g_free(opticaldevice);
}


void on_audioburn_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *source;
	if (g_ascii_strncasecmp(gettext("none"), gtk_label_get_text(ui.info_mediacontent), 4) != 0) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	source = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_burndir);
	if (source == NULL) {
		gtk_label_set_text(ui.info_log, gettext("Error: please select directory to burn."));
		return;
	}
	gchar *quotetsource = g_shell_quote(source);
	gchar *commandline = g_strdup_printf("simpleburn.sh %s b-audio %s", opticaldevice, quotetsource);
	startprogress(commandline, FALSE,  gettext("Burning audio CD ..."), BURN);
	g_free(commandline);
	g_free(quotetsource);
	g_free(source);
	g_free(opticaldevice);
}


void on_tracks_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
	GtkTreeIter iter;
	gint id;
	gchar * opticaldevice;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gtk_tree_model_get_iter((GtkTreeModel *) ui.model_tracks, &iter, path);
	gtk_tree_model_get((GtkTreeModel *) ui.model_tracks, &iter, 1, &id, -1);
	gchar *commandline = g_strdup_printf("gmplayer -cdrom-device %s cdda://%d", opticaldevice, id);
	g_spawn_command_line_async(commandline, NULL);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_extractaudio_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gint id;
	gboolean selected, hasnext;
	gchar *opticaldevice, *format, *idslist, *tmpidslist;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	idslist = g_strdup("");
	hasnext = gtk_tree_model_get_iter_first((GtkTreeModel *) ui.model_tracks, &iter);
	while (hasnext) {
		gtk_tree_model_get((GtkTreeModel *) ui.model_tracks, &iter, 0, &selected, 1, &id, -1);
		if (selected) {
			tmpidslist = g_strdup_printf("%s%d ", idslist, id);
			g_free(idslist);
			idslist = tmpidslist;
		}
		hasnext = gtk_tree_model_iter_next((GtkTreeModel *) ui.model_tracks, &iter);
	}
	idslist[strlen(idslist)-1] = '\0';
	gtk_combo_box_get_active_iter(ui.ctrl_audioformat, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_audioformat, &iter, 0, &format, -1);
	gchar *destination = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_audioextractdir);
	gchar *quotetdestination = g_shell_quote(destination);
	gchar *commandline = g_strdup_printf("simpleburn.sh %s e-audio %s %s %s", opticaldevice, quotetdestination, format, idslist);
	startprogress(commandline, FALSE, gettext("Extracting audio CD ..."), EXTRACT);
	g_free(commandline);
	g_free(quotetdestination);
	g_free(destination);
	g_free(format);
	g_free(idslist);
	g_free(opticaldevice);
}


void on_previewvideo_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *commandline, *alang, *slang;
	gint title;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videotitle, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videotitle, &iter, 0, &title, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videolanguage, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videolanguage, &iter, 0, &alang, -1);
	if (gtk_combo_box_get_active(ui.ctrl_videosubtitles) == 0) {
		commandline = g_strdup_printf("gmplayer -dvd-device %s dvd://%d -alang %s -slang none", opticaldevice, title, alang);
	} else {
		gtk_combo_box_get_active_iter(ui.ctrl_videosubtitles, &iter);
		gtk_tree_model_get((GtkTreeModel *) ui.model_videosubtitles, &iter, 0, &slang, -1);
		commandline = g_strdup_printf("gmplayer -dvd-device %s dvd://%d -alang %s -slang %s", opticaldevice, title, alang, slang);
		g_free(slang);
	}
	g_free(alang);
	g_spawn_command_line_async(commandline, NULL);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_extractvideo_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *commandline, *filename, *alang, *slang;
	gint title;
	
	filename = g_shell_quote(gtk_entry_get_text(ui.ctrl_videoextractfile));
	if (strlen(filename) == 0) {
		gtk_label_set_text(ui.info_log, gettext("Error: please enter a filename."));
		g_free(filename);
		return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videotitle, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videotitle, &iter, 0, &title, -1);
	gchar *destination = g_shell_quote(gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_videoextractdir));
	gtk_combo_box_get_active_iter(ui.ctrl_videolanguage, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videolanguage, &iter, 0, &alang, -1);
	if (gtk_combo_box_get_active(ui.ctrl_videosubtitles) == 0) {
		commandline = g_strdup_printf("simpleburn.sh %s e-video %s/%s %d %s", opticaldevice, destination, filename, title, alang);
	} else {
		gtk_combo_box_get_active_iter(ui.ctrl_videosubtitles, &iter);
		gtk_tree_model_get((GtkTreeModel *) ui.model_videosubtitles, &iter, 0, &slang, -1);
		commandline = g_strdup_printf("simpleburn.sh %s e-video %s/%s %d %s %s", opticaldevice, destination, filename, title, alang, slang);
		g_free(slang);
	}
	g_free(alang);
	g_free(filename);
	g_free(destination);
	g_free(opticaldevice);
	startprogress(commandline, FALSE, gettext("Extracting video DVD ..."), EXTRACT);
	g_free(commandline);
}


void on_extractiso_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	const gchar *filename = gtk_entry_get_text(ui.ctrl_isoextractfile);
	if (strlen(filename) == 0) {
		gtk_label_set_text(ui.info_log, gettext("Error: please enter a filename."));
		return;
	}
	gchar *destination = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_isoextractdir);
	gchar *quotetdestination = g_shell_quote(destination);
	gchar *quotetfilename = g_shell_quote(filename);
	gchar *commandline = g_strdup_printf("simpleburn.sh %s e-iso %s/%s", opticaldevice, destination, filename);
	startprogress(commandline, FALSE, gettext("Extracting ISO image ..."), EXTRACT);
	g_free(commandline);
	g_free(quotetfilename);
	g_free(quotetdestination);
	g_free(destination);
	g_free(opticaldevice);
}


void on_videotitle_changed(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *device;
	gint title;
	
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &device, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videotitle, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videotitle, &iter, 0, &title, -1);
	load_titleinfos(device, title);
	g_free(device);
}


void on_detectmedia_clicked(GtkWidget *widget, gpointer user_data) {
	gchar *device;
	GtkTreeIter iter;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &device, -1);
	reset_mediadetection();
	gtk_label_set_text(ui.info_log, "");
	load_mediainfos(device);
	g_free(device);
}


void reset_mediadetection() {
	gtk_label_set_text(ui.info_medialabel, "");
	gtk_label_set_text(ui.info_mediatype, "");
	gtk_label_set_text(ui.info_mediacontent, "");
	gtk_label_set_text(ui.info_mediasize, "");
	gtk_label_set_text(ui.info_mediacapacity, "");
	while (gtk_notebook_get_n_pages(ui.tabs_actions) != 0) {
		gtk_notebook_remove_page(ui.tabs_actions, -1);
	}
}
