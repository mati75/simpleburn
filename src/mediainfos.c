#include <gtk/gtk.h>
#include <libintl.h>
#include <string.h>

#include "simpleburn.h"
#include "mediainfos.h"

	
void init_mediainfos() {
	mediainfos.mediatype = NULL;
	mediainfos.hasmedia = FALSE;
	mediainfos.cdtype = FALSE;
	mediainfos.mediacontent = NULL;
	mediainfos.emptycontent = TRUE;
	mediainfos.videocontent = FALSE;
	mediainfos.audiocontent = FALSE;
	mediainfos.iso9660content = FALSE;
	mediainfos.rewritablemedia = FALSE;
	mediainfos.mediacapacity = 0; //bytes
	mediainfos.mediasize = 0;
	mediainfos.trackscount = 0;
	mediainfos.medialabel = NULL;
	mediainfos.titles = NULL;
}


void free_mediainfos() {
	if (mediainfos.mediatype != NULL) g_free(mediainfos.mediatype);
	if (mediainfos.mediacontent != NULL) g_free(mediainfos.mediacontent);
	if (mediainfos.medialabel != NULL) g_free(mediainfos.medialabel);
	if (mediainfos.videocontent) {
		for (gint i = 0; i < mediainfos.trackscount; i++) {
			for (gint j = 0; j < mediainfos.titles[i]->langscount; j++) g_free(mediainfos.titles[i]->languages[j]);
			g_free(mediainfos.titles[i]->languages);
			mediainfos.titles[i]->langscount = 0;
			for (gint j = 0; j < mediainfos.titles[i]->subscount; j++) g_free(mediainfos.titles[i]->subtitles[j]);
			g_free(mediainfos.titles[i]->subtitles);
			mediainfos.titles[i]->subscount = 0;
			g_free(mediainfos.titles[i]->cropinfos);
			g_free(mediainfos.titles[i]);
		}
		g_free(mediainfos.titles);
	}
	init_mediainfos();
}


gchar **load_mediasinfos() {
	GtkTreeIter iter;
	gchar *opticaldevice;
	gchar **infolines;
	gtk_combo_box_get_active_iter(ui.ctrl_opticaldevice, &iter);
	gtk_tree_model_get((GtkTreeModel *) ui.model_opticaldevice, &iter, 0, &opticaldevice, -1);
	gchar *command = g_strdup_printf("simpleburn-detect.sh -cr %s", opticaldevice);
	gchar *output;
	g_spawn_command_line_sync(command, &output, NULL, NULL, NULL );
	g_free(command);
	g_free(opticaldevice);
	infolines = g_strsplit(output, "\n", 0);
	g_free(output);
	return infolines;
}

void commoninfos(gchar *firstinfoline) {
	gchar **infos = g_strsplit(firstinfoline, ":", 0);
	//set:
	if (g_ascii_strcasecmp(infos[1], "audio") == 0) {
		mediainfos.emptycontent = FALSE;
		mediainfos.audiocontent = TRUE;
		mediainfos.mediacontent = g_strdup_printf("%s", gettext("audio"));
	} else if (g_ascii_strcasecmp(infos[1], "video") == 0) {
		mediainfos.emptycontent = FALSE;
		mediainfos.videocontent = TRUE;
		mediainfos.mediacontent = g_strdup_printf("%s", gettext("video"));
	} else if (g_ascii_strcasecmp(infos[1], "iso9660") == 0) {
		mediainfos.emptycontent = FALSE;
		mediainfos.iso9660content = TRUE;
		//~ mediainfos.mediacontent = g_strdup_printf("%s", infos[1]);
		mediainfos.mediacontent = g_strdup_printf("%s", gettext("data"));
	} else {
		mediainfos.mediacontent = g_strdup("/");
	}
	if (g_ascii_strtoull(infos[4], NULL, 10) == 1) {
		mediainfos.rewritablemedia = TRUE;
	}
	gchar *basemediatype;
	if (g_ascii_strcasecmp(infos[0], "cd") == 0) {
		mediainfos.hasmedia = TRUE;
		mediainfos.cdtype = TRUE;
		basemediatype = g_strdup("CD");
	} else	if (g_ascii_strcasecmp(infos[0], "dvd") == 0) {
		mediainfos.hasmedia = TRUE;
		basemediatype = g_strdup("DVD");
	} else {
		mediainfos.mediatype = g_strdup("/");
	}
	if (mediainfos.hasmedia) {
		if (mediainfos.rewritablemedia) {
			mediainfos.mediatype = g_strdup_printf("%s / RW", basemediatype);
		} else if (mediainfos.emptycontent) {
			mediainfos.mediatype = g_strdup_printf("%s / R", basemediatype);
		} else {
			mediainfos.mediatype = g_strdup(basemediatype);
		}
		g_free(basemediatype);
	}
	mediainfos.mediacapacity = g_ascii_strtoull(infos[2], NULL, 10);
	mediainfos.mediasize = g_ascii_strtoull(infos[3], NULL, 10);
	mediainfos.trackscount = (guint8) g_ascii_strtoull(infos[5], NULL, 10);
	mediainfos.medialabel = g_ascii_strdown(infos[6], -1);
	g_strfreev(infos);
	//display:
	if (mediainfos.hasmedia) {
		gtk_label_set_text(ui.info_medialabel, mediainfos.medialabel);
	} else {
		gtk_label_set_text(ui.info_medialabel, gettext("no media"));
	}
	gtk_label_set_text(ui.info_mediatype, mediainfos.mediatype);
	gtk_label_set_text(ui.info_mediacontent, mediainfos.mediacontent);
	if (mediainfos.mediacapacity == 0) {
		gtk_label_set_text(ui.info_mediacapacity, "/");
	} else {
		gchar *capacity = g_strdup_printf("%d MB", mediainfos.mediacapacity / 1048576);
		gtk_label_set_text(ui.info_mediacapacity,capacity);
		g_free(capacity);
	}
	if (mediainfos.mediasize == 0) {
		gtk_label_set_text(ui.info_mediasize, "/");
	} else {
		gchar *size = g_strdup_printf("%d MB", mediainfos.mediasize / 1048576);
		gtk_label_set_text(ui.info_mediasize, size);
		g_free(size);
	}
}


void audioinfos(gchar **infolines) {
	GtkTreeIter iter;
	gtk_list_store_clear(ui.model_tracks);
	for (gint i=0; strlen(infolines[i+1]) != 0; i++) {
		gchar **trackinfos = g_strsplit(infolines[i+1], ";", 0);
		gtk_list_store_append(ui.model_tracks, &iter);
		//~ gtk_list_store_set(ui.model_tracks, &iter, 0, TRUE, 1, g_ascii_strtoull(trackinfos[0], NULL, 10), 2, trackinfos[1], 3, trackinfos[2], -1);
		gtk_list_store_set(ui.model_tracks, &iter, 0, TRUE, 1, i+1, 2, trackinfos[1], 3, trackinfos[2], -1);
		g_strfreev(trackinfos);
	}
}


void videoinfos(gchar **infolines) {
	GtkTreeIter iter;
	mediainfos.titles = g_new(T_TitleInfos *, mediainfos.trackscount);
	gtk_combo_box_set_model(ui.ctrl_videotitle, NULL);
	gtk_list_store_clear(ui.model_videotitle);
	for (gint i=0; strlen(infolines[i+1]) != 0 && i < mediainfos.trackscount; i++) {
		mediainfos.titles[i] = g_new(T_TitleInfos, 1);
		gchar **titleinfos = g_strsplit(infolines[i+1], ";", 0);
		mediainfos.titles[i]->id = g_ascii_strtoll(titleinfos[0], NULL, 10);
		mediainfos.titles[i]->length = g_ascii_strtoll(titleinfos[1], NULL, 10) / 60;
		gtk_list_store_append(ui.model_videotitle, &iter);
		gchar *title = g_strdup_printf("%d (%d')", mediainfos.titles[i]->id, mediainfos.titles[i]->length);
		gtk_list_store_set(ui.model_videotitle, &iter, 0, mediainfos.titles[i]->id, 1, title, -1);
		g_free(title);
		gchar **languages = g_strsplit(titleinfos[2], ",", 0);
		mediainfos.titles[i]->langscount = g_strv_length(languages);
		mediainfos.titles[i]->languages = g_new(T_LanguageInfos *, mediainfos.titles[i]->langscount);
		for (gint j=0; j<mediainfos.titles[i]->langscount; j++) {
			mediainfos.titles[i]->languages[j] = g_new(T_LanguageInfos, 1);
			gchar **languageparts = g_strsplit(languages[j], "/", 0);
			mediainfos.titles[i]->languages[j]->id = g_ascii_strtoll(languageparts[0], NULL, 10);
			for (gint k=0; k<3; k++) mediainfos.titles[i]->languages[j]->lang[k] = languageparts[1][k];
			g_free(languageparts);
		}
		g_strfreev(languages);
		gchar **subtitles = g_strsplit(titleinfos[3], ",", 0);
		mediainfos.titles[i]->subscount = g_strv_length(subtitles);
		mediainfos.titles[i]->subtitles = g_new(T_LanguageInfos *, mediainfos.titles[i]->subscount);
		for (gint j=0; j<mediainfos.titles[i]->subscount; j++) {
			mediainfos.titles[i]->subtitles[j] = g_new(T_LanguageInfos, 1);
			gchar **subtitleparts = g_strsplit(subtitles[j], "/", 0);
			mediainfos.titles[i]->subtitles[j]->id = g_ascii_strtoll(subtitleparts[0], NULL, 10);
			for (gint k=0; k<3; k++) mediainfos.titles[i]->subtitles[j]->lang[k] = subtitleparts[1][k];
			g_free(subtitleparts);
		}
		g_strfreev(subtitles);
		mediainfos.titles[i]->cropinfos = g_strdup(titleinfos[4]);
		g_strfreev(titleinfos);
	}
	gtk_combo_box_set_model(ui.ctrl_videotitle, (GtkTreeModel *) ui.model_videotitle);
	gtk_combo_box_set_active(ui.ctrl_videotitle, 0);
	gchar *filename = g_strdup_printf("%s.avi", mediainfos.medialabel);
	gtk_entry_set_text(ui.ctrl_videoextractfile, filename);
	g_free(filename);
}


void on_detectmedia_clicked(GtkWidget *widget, gpointer user_data) {
	free_mediainfos();
	gchar **infolines = load_mediasinfos();
	commoninfos(infolines[0]);
	if (mediainfos.audiocontent) {
		audioinfos(infolines);
	}
	if (mediainfos.videocontent) {
		videoinfos(infolines);
	}
	if (mediainfos.iso9660content) {
		gchar *filename = g_strdup_printf("%s.iso", mediainfos.medialabel);
		gtk_entry_set_text(ui.ctrl_isoextractfile, filename);
		g_free(filename);
	}
	g_strfreev(infolines);
	//hide & show tabs:
	while (gtk_notebook_get_n_pages(ui.tabs_actions) != 0) {
		gtk_notebook_remove_page(ui.tabs_actions, -1);
	}
	if (mediainfos.audiocontent) gtk_notebook_append_page(ui.tabs_actions, ui.tab_extractaudio, gtk_label_new(gettext("Extract audio")));
	else if (mediainfos.videocontent) gtk_notebook_append_page(ui.tabs_actions, ui.tab_extractvideo, gtk_label_new(gettext("Extract video")));
	else if (mediainfos.iso9660content) gtk_notebook_append_page(ui.tabs_actions, ui.tab_extractiso, gtk_label_new(gettext("Extract ISO")));
	if (mediainfos.rewritablemedia || (mediainfos.emptycontent && mediainfos.hasmedia)) gtk_notebook_append_page(ui.tabs_actions, ui.tab_burn, gtk_label_new(gettext("Burn")));
	if (mediainfos.rewritablemedia) gtk_notebook_append_page(ui.tabs_actions, ui.tab_blank, gtk_label_new(gettext("Erase")));
}


void on_videotitle_changed(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	gint titlepos = gtk_combo_box_get_active(ui.ctrl_videotitle);
	gint langpos = gtk_combo_box_get_active(ui.ctrl_videolanguage);
	if (langpos == -1) langpos = 0;
	gtk_combo_box_set_model(ui.ctrl_videolanguage, NULL);
	gtk_list_store_clear(ui.model_videolanguage);
	for (gint i=0; i<mediainfos.titles[titlepos]->langscount; i++) {
		gtk_list_store_append(ui.model_videolanguage, &iter);
		gtk_list_store_set(ui.model_videolanguage, &iter, 0, mediainfos.titles[titlepos]->languages[i]->id, 1, mediainfos.titles[titlepos]->languages[i]->lang, -1);
	}
	gtk_combo_box_set_model(ui.ctrl_videolanguage, (GtkTreeModel *) ui.model_videolanguage);
	gtk_combo_box_set_active(ui.ctrl_videolanguage, langpos);
	gint subpos = gtk_combo_box_get_active(ui.ctrl_videosubtitles);
	if (subpos == -1) subpos = 0;
	gtk_combo_box_set_model(ui.ctrl_videosubtitles, NULL);
	gtk_list_store_clear(ui.model_videosubtitles);
	gtk_list_store_append(ui.model_videosubtitles, &iter);
	gtk_list_store_set(ui.model_videosubtitles, &iter, 0, -1, 1, gettext("none"), -1);
	for (gint i=0; i<mediainfos.titles[titlepos]->subscount; i++) {
		gtk_list_store_append(ui.model_videosubtitles, &iter);
		gtk_list_store_set(ui.model_videosubtitles, &iter, 0, mediainfos.titles[titlepos]->subtitles[i]->id, 1, mediainfos.titles[titlepos]->subtitles[i]->lang, -1);
	}
	gtk_combo_box_set_model(ui.ctrl_videosubtitles, (GtkTreeModel *) ui.model_videosubtitles);
	gtk_combo_box_set_active(ui.ctrl_videosubtitles, subpos);
	gtk_entry_set_text(ui.ctrl_videocropinfos, mediainfos.titles[titlepos]->cropinfos);
}
