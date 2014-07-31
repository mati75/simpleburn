#include <gtk/gtk.h>
#include <libintl.h>
#include <string.h>

#include "simpleburn.h"
#include "callbacks.h"
#include "mediainfos.h"
#include "progress.h"

void linkcallbacks() {}
	
	
void on_ejectmedia_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gchar *commandline = g_strdup_printf("cdrecord -eject dev=%s", opticaldevice);
	g_spawn_command_line_sync(commandline, NULL, NULL, NULL, NULL );
	g_free(commandline);
	g_free(opticaldevice);
	on_detectmedia_clicked(NULL, NULL);
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
	gchar *opticaldevice;
	if (! mediainfos.emptycontent) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s blank fast", opticaldevice);
	startprogress(commandline, TRUE, gettext("Fast blanking ..."), BLANK);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_blankfull_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice;
	if (! mediainfos.emptycontent) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s blank all", opticaldevice);
	startprogress(commandline, TRUE, gettext("Full blanking ..."), BLANK);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_fileburn_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *source, *message;
	if (! mediainfos.emptycontent) {
		if (gtk_dialog_run(ui.dialog_confirmerase) != 1) return;
	}
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	source = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_burnfile);
	if (source == NULL) {
		gtk_label_set_text(ui.info_log, gettext("Error: please select file to burn."));
		return;
	}
	gchar *quotetsource = g_shell_quote(source);
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s burn %s", opticaldevice, quotetsource);
	startprogress(commandline, FALSE, gettext("Burning file ..."), BURN);
	g_free(commandline);
	g_free(quotetsource);
	g_free(source);
	g_free(opticaldevice);
}


void on_dirburn_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *source, *message;
	if (! mediainfos.emptycontent) {
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
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s burn %s", opticaldevice, quotetsource);
	startprogress(commandline, FALSE,  gettext("Burning directory ..."), BURN);
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
	//~ gchar *commandline = g_strdup_printf("gvfs-mount cdda://%s", opticaldevice + 5); //+5 chars: skip "/dev/"
	//~ g_spawn_command_line_sync(commandline, NULL, NULL, NULL, NULL);
	//~ g_free(commandline);
	//~ commandline = g_strdup_printf("gvfs-open 'cdda://%s/Track %d.wav'", opticaldevice + 5, id);
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
			if (strlen(idslist) == 0) {
				tmpidslist = g_strdup_printf("%d", id);
			} else {
				tmpidslist = g_strdup_printf("%s,%d", idslist, id);
			}
			g_free(idslist);
			idslist = tmpidslist;
		}
		hasnext = gtk_tree_model_iter_next((GtkTreeModel *) ui.model_tracks, &iter);
	}
	gtk_combo_box_get_active_iter(ui.ctrl_audioformat, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_audioformat, &iter, 0, &format, -1);
	gchar *destination = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_audioextractdir);
	gchar *quotetdestination = g_shell_quote(destination);
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s extract %s %s %s", opticaldevice, quotetdestination, format, idslist);
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
	gchar *opticaldevice, *commandline, *cropinfos;
	gint title, aid, sid;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videotitle, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videotitle, &iter, 0, &title, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videolanguage, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videolanguage, &iter, 0, &aid, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videosubtitles, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videosubtitles, &iter, 0, &sid, -1);
	if (strlen(gtk_entry_get_text(ui.ctrl_videocropinfos)) != 0) {
		cropinfos = g_strdup_printf("-vf crop=%s", gtk_entry_get_text(ui.ctrl_videocropinfos));
	} else {
		cropinfos = g_strdup("");
	}
	if (sid == -1) {
		commandline = g_strdup_printf("gmplayer -dvd-device %s dvd://%d -aid %d -slang none %s", opticaldevice, title, aid, cropinfos);
	} else {
		commandline = g_strdup_printf("gmplayer -dvd-device %s dvd://%d -aid %d -sid %d %s", opticaldevice, title, aid, sid, cropinfos);
	}
	g_spawn_command_line_async(commandline, NULL);
	g_free(cropinfos);
	g_free(commandline);
	g_free(opticaldevice);
}


void on_extractvideo_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gchar *opticaldevice, *quality;
	gint title, aid, sid;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videotitle, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videotitle, &iter, 0, &title, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videolanguage, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videolanguage, &iter, 0, &aid, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videosubtitles, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videosubtitles, &iter, 0, &sid, -1);
	gtk_combo_box_get_active_iter(ui.ctrl_videoquality, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_videoquality, &iter, 0, &quality, -1);
	gchar *destination = gtk_file_chooser_get_filename((GtkFileChooser *) ui.ctrl_videoextractdir);
	const gchar *filename = gtk_entry_get_text(ui.ctrl_videoextractfile);
	if (strlen(filename) == 0) {
		gtk_label_set_text(ui.info_log, gettext("Error: please enter a filename."));
		return;
	}
	gchar *quotetdestination = g_shell_quote(destination);
	gchar *quotetfilename = g_shell_quote(filename);
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s extract %s/%s %d %s %d %d %s", opticaldevice, quotetdestination, quotetfilename, title, quality, aid, sid, gtk_entry_get_text(ui.ctrl_videocropinfos));
	startprogress(commandline, FALSE, gettext("Extracting video DVD ..."), EXTRACT);
	g_free(commandline);
	g_free(quotetfilename);
	g_free(quotetdestination);
	g_free(opticaldevice);
	g_free(quality);
	g_free(destination);
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
	gchar *commandline = g_strdup_printf("simpleburn.sh -cr %s extract %s/%s", opticaldevice, destination, filename);
	startprogress(commandline, FALSE, gettext("Extracting ISO image ..."), EXTRACT);
	g_free(commandline);
	g_free(quotetfilename);
	g_free(quotetdestination);
	g_free(destination);
	g_free(opticaldevice);
}
