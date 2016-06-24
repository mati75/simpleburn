#include <gtk/gtk.h>
#include <libintl.h>
#include <string.h>
#include <cddb/cddb.h>
#include <cdio/cdio.h>
#include <cdio/mmc_cmds.h>
#include <cdio/cd_types.h>
#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_read.h>

#include "simpleburn.h"
#include "mediainfos.h"
#include "cdio_mmc.c"


gboolean load_videoinfos(gchar *device) {
	GtkTreeIter iter;
	dvd_reader_t *dvd;
	ifo_handle_t *mainifo, **ifo;
	pgc_t *pgc;
	gboolean video;
	gchar *title;
	gint i, len, ts, ttn;
	
	video = TRUE;
	dvd = DVDOpen(device);
	if (!dvd) {
		video = FALSE;
	} else {
		mainifo = ifoOpen(dvd, 0);
		if (!mainifo) {
			video = FALSE;
		} else {
			//load all IFOs
			ifo = (ifo_handle_t **) g_malloc((mainifo->vts_atrt->nr_of_vtss) * sizeof(ifo_handle_t *));
			for (gint i=0; i<mainifo->vts_atrt->nr_of_vtss; i++) {
				ifo[i] = ifoOpen(dvd, i+1);
			}
			//list titles (and retrieve lengths)
			gtk_combo_box_set_model(ui.ctrl_videotitle, NULL);
			gtk_list_store_clear(ui.model_videotitle);
			for (i=0; i <= mainifo->tt_srpt->nr_of_srpts; i++) { //for each title
				ts = mainifo->tt_srpt->title[i].title_set_nr;
				if (ts != 0) {
					ttn = mainifo->tt_srpt->title[i].vts_ttn;
					pgc = ifo[ts-1]->vts_pgcit->pgci_srp[ifo[ts-1]->vts_ptt_srpt->title[ttn-1].ptt[0].pgcn-1].pgc;
					len = pgc->playback_time.hour * 60 + pgc->playback_time.minute;
					if (len > 5) { //during at least 5 minutes
						gtk_list_store_append(ui.model_videotitle, &iter);
						title = g_strdup_printf("%d (%d')", i+1, len);
						gtk_list_store_set(ui.model_videotitle, &iter, 0, i+1, 1, title, -1);
						g_free(title);
					}
				}
			}
			gtk_combo_box_set_model(ui.ctrl_videotitle, (GtkTreeModel *) ui.model_videotitle);
			if (mainifo->vts_atrt->nr_of_vtss != 0) {
				gtk_combo_box_set_active(ui.ctrl_videotitle, 0);
			}
			for (i=0; i < mainifo->vts_atrt->nr_of_vtss; i++) {
				ifoClose(ifo[i]);
			}
			g_free(ifo);
			ifoClose(mainifo);
		}
		DVDClose(dvd);
	}
	return video;
}


void load_titleinfos(gchar *device, gint title) {
	GtkTreeIter iter;
	dvd_reader_t *dvd;
	ifo_handle_t *mainifo, *titleifo;
	pgc_t *pgc;
	audio_attr_t *audio;
	subp_attr_t *sub;
	gchar *lang, *filename;
	gint i, ts, ttn, langpos, subpos;
	guint lastcode = 0;
	
	dvd = DVDOpen(device);
	mainifo = ifoOpen(dvd, 0);
	ts = mainifo->tt_srpt->title[title-1].title_set_nr;
	titleifo = ifoOpen(dvd, ts);
	ttn = mainifo->tt_srpt->title[title-1].vts_ttn;
	pgc = titleifo->vts_pgcit->pgci_srp[titleifo->vts_ptt_srpt->title[ttn-1].ptt[0].pgcn-1].pgc;
	
	langpos = gtk_combo_box_get_active(ui.ctrl_videolanguage);
	if (langpos == -1) langpos = 0;
	gtk_combo_box_set_model(ui.ctrl_videolanguage, NULL);
	gtk_list_store_clear(ui.model_videolanguage);
	for (i=0; i<titleifo->vtsi_mat->nr_of_vts_audio_streams; i++) {
		if ((pgc->audio_control[i] & 0x8000) != 0) {
			audio = &titleifo->vtsi_mat->vts_audio_attr[i];
			if (audio->lang_code != lastcode) {
				lang = g_strdup_printf("%c%c", audio->lang_code>>8, audio->lang_code & 0xff);
				gtk_list_store_append(ui.model_videolanguage, &iter);
				gtk_list_store_set(ui.model_videolanguage, &iter, 0, lang, -1);
				g_free(lang);
				lastcode = audio->lang_code;
			}
		}
	}
	gtk_combo_box_set_model(ui.ctrl_videolanguage, (GtkTreeModel *) ui.model_videolanguage);
	gtk_combo_box_set_active(ui.ctrl_videolanguage, langpos);
	
	subpos = gtk_combo_box_get_active(ui.ctrl_videosubtitles);
	if (subpos == -1) subpos = 0;
	gtk_combo_box_set_model(ui.ctrl_videosubtitles, NULL);
	gtk_list_store_clear(ui.model_videosubtitles);
	gtk_list_store_append(ui.model_videosubtitles, &iter);
	gtk_list_store_set(ui.model_videosubtitles, &iter, 0, gettext("none"), -1);
	for (i=0; i<titleifo->vtsi_mat->nr_of_vts_subp_streams; i++) {
		if ((pgc->subp_control[i] & 0x80000000) != 0) {
			sub = &titleifo->vtsi_mat->vts_subp_attr[i];
			lang = g_strdup_printf("%c%c", sub->lang_code>>8, sub->lang_code & 0xff);
			gtk_list_store_append(ui.model_videosubtitles, &iter);
			gtk_list_store_set(ui.model_videosubtitles, &iter, 0, lang, -1);
			g_free(lang);
		}
	}
	gtk_combo_box_set_model(ui.ctrl_videosubtitles, (GtkTreeModel *) ui.model_videosubtitles);
	gtk_combo_box_set_active(ui.ctrl_videosubtitles, subpos);
	
	filename = g_strdup_printf("%s-%d", gtk_label_get_text(ui.info_medialabel), title);
	lowercase(filename);
	gtk_entry_set_text(ui.ctrl_videoextractfile, filename);
	g_free(filename);
	
	ifoClose(mainifo);
	ifoClose(titleifo);
	DVDClose(dvd);
}


CdIo_t *loaddevice(gchar *device) {
	CdIo_t *cdio;
	guint8 status[2]; //[1]= 0: not ready, 1: no media, 2: loaded
	gint countdown;
	
	cdio = cdio_open (device, DRIVER_UNKNOWN);
	if (cdio != NULL) {
		countdown = 5;
		while (countdown != 0 && mmc_get_event_status (cdio, status) == DRIVER_OP_SUCCESS && status[1] == 0) {
			sleep (1);
			countdown--;
		}
	}
	return cdio;
}


void load_audioinfos(CdIo_t *cdio) {
	GtkTreeIter iter;
	gint i, sec, min;
	gchar *len;
	cddb_disc_t *disc;
	cddb_track_t *track;
	cddb_conn_t *conn;
	
	gtk_list_store_clear(ui.model_tracks);
	disc = cddb_disc_new();
	cddb_disc_set_length(disc, cdio_get_disc_last_lsn(cdio) / CDIO_CD_FRAMES_PER_SEC);
	for (i=1; i <= cdio_get_num_tracks(cdio); i++) {
		track = cddb_track_new();
		cddb_track_set_frame_offset(track, cdio_get_track_lba(cdio, i));
		cddb_disc_add_track (disc, track);
	}
	conn = cddb_new();
	if (cddb_query(conn, disc) == 1) { //if there is one and only one match
		gtk_label_set_text(ui.info_medialabel, cddb_disc_get_title(disc));
		cddb_read(conn, disc);
	}
	
	for (i=0; i<cddb_disc_get_track_count(disc); i++) {
		track = cddb_disc_get_track(disc, i);
		sec = cddb_track_get_length(track);
		min = (gint) sec / 60;
		sec = sec - min * 60;
		len = g_strdup_printf("%d'%02d\"", min, sec);
		gtk_list_store_append(ui.model_tracks, &iter);
		gtk_list_store_set(ui.model_tracks, &iter, 0, TRUE, 1, i+1, 2, cddb_track_get_title(track), 3, len, -1);
		g_free(len);
	}
	
	cddb_disc_destroy(disc);
}


void load_mediainfos(gchar *device) {
	CdIo_t *cdio;
	cdio_mmc_feature_profile_t disctype;
	cdio_iso_analysis_t isoinfo;
	cdio_fs_anal_t fs;
	gchar *basemediatype, *mediatype;
	gboolean mediapresent, rewritablemedia, cdmedia, emptymedia;
	gint i;
	
	mediapresent = FALSE;
	rewritablemedia = FALSE;
	emptymedia = FALSE;
	cdmedia = FALSE;
	cdio = loaddevice(device);
	if (cdio != NULL) {
		//media type detection (/ (no media), CD / R, CD / RW, DVD / R, DVD / RW)
		
		if (mmc_get_disctype(cdio, 0, &disctype) == DRIVER_OP_SUCCESS) {
			if (mmc_is_disctype_cdrom(disctype)) {
				mediapresent = TRUE;
				cdmedia = TRUE;
				basemediatype = g_strdup("CD");
				gtk_widget_show((GtkWidget *) ui.bt_audioburn);
			} else if (mmc_is_disctype_dvd(disctype)) {
				mediapresent = TRUE;
				basemediatype = g_strdup("DVD");
				gtk_widget_set_sensitive((GtkWidget *) ui.bt_audioburn, FALSE);
				gtk_widget_hide((GtkWidget *) ui.bt_audioburn);
			}
			if (mediapresent) {
				if (mmc_disctype_is_writable(disctype)) { //is_writable
					if (mmc_disctype_is_rewritable(disctype)) {
						rewritablemedia = TRUE;
						mediatype = g_strdup_printf("%s / RW", basemediatype);
					} else {
						mediatype = g_strdup_printf("%s / R", basemediatype);
					}
				} else {
					mediatype = g_strdup_printf("%s", basemediatype);
				}
				g_free(basemediatype);
				gtk_label_set_text(ui.info_mediatype, mediatype);
				g_free(mediatype);
				
				//media content detection (audio, data, video)
				if (mmc_get_disc_empty(cdio, (bool*) &emptymedia) == DRIVER_OP_SUCCESS) {
					if (emptymedia) {
						gtk_label_set_text(ui.info_mediacontent, gettext("none"));
					} else {
						fs = cdio_guess_cd_type (cdio, 0, 1, &isoinfo);
					}
				}
				if (! emptymedia) {
					if (CDIO_FSTYPE (fs) == CDIO_FS_AUDIO) {
						gtk_notebook_append_page(ui.tabs_actions, ui.tab_extractaudio, gtk_label_new(gettext("Extract audio")));
						gtk_label_set_text(ui.info_mediacontent, gettext("audio"));
						//~ gchar *mediasize = g_strdup_printf("%d %s", cdio_get_disc_last_lsn(cdio) / 512, gettext("MB")); //sectors -> MB
						gchar *mediasize = g_strdup_printf("%d'", (gint) cdio_get_disc_last_lsn(cdio) / CDIO_CD_FRAMES_PER_SEC / 60);
						gtk_label_set_text(ui.info_mediasize, mediasize);
						g_free(mediasize);
						load_audioinfos(cdio);
					} else if (CDIO_FSTYPE (fs) == CDIO_FS_ISO_9660 || CDIO_FSTYPE (fs) == CDIO_FS_ISO_UDF) { //CDIO_FSTYPE (fs) == CDIO_FS_UDF
						for (i=0; isoinfo.iso_label[i] != ' ' && isoinfo.iso_label[i] != '\0'; i++); isoinfo.iso_label[i] = '\0'; //trailing spaces
						recase(isoinfo.iso_label);
						gtk_label_set_text(ui.info_medialabel, isoinfo.iso_label);
						if (cdmedia || ! load_videoinfos(device)) {
							gchar *filename = g_strdup_printf("%s.iso", isoinfo.iso_label);
							lowercase(filename);
							gtk_entry_set_text(ui.ctrl_isoextractfile, filename);
							g_free(filename);
							gtk_notebook_append_page(ui.tabs_actions, ui.tab_extractiso, gtk_label_new(gettext("Extract ISO")));
							gtk_label_set_text(ui.info_mediacontent, gettext("data"));
						} else {
							gtk_notebook_append_page(ui.tabs_actions, ui.tab_extractvideo, gtk_label_new(gettext("Extract video")));
							gtk_label_set_text(ui.info_mediacontent, gettext("video"));
						}
						gchar *size = g_strdup_printf("%d %s", isoinfo.isofs_size / 512, gettext("MB"));
						gtk_label_set_text(ui.info_mediasize, size);
						g_free(size);
					} else if (fs == 15) { //blanked DVD/RW
						emptymedia = TRUE;
						gtk_label_set_text(ui.info_mediacontent, gettext("none"));
					}
				}
				
				if (emptymedia || rewritablemedia) {
					gtk_notebook_append_page(ui.tabs_actions, ui.tab_burn, gtk_label_new(gettext("Burn")));
					uint32_t capacity;
					if (mmc_get_disc_capacity(cdio, &capacity) == DRIVER_OP_SUCCESS) {
						gchar *mediacapacity = g_strdup_printf("%d %s", capacity / 1024, gettext("MB"));
						gtk_label_set_text(ui.info_mediacapacity, mediacapacity);
						g_free(mediacapacity);
					}
				}
				
				if (rewritablemedia && ! emptymedia) {
					gtk_notebook_append_page(ui.tabs_actions, ui.tab_blank, gtk_label_new(gettext("Erase")));
				}
			}	
		}
		cdio_destroy(cdio);
	}
}


void recase(gchar *str) {
	gboolean start;
	gint i;
	
	start = TRUE;
	for (i=0; i<strlen(str); i++) {
		if (str[i] == ' ' || str[i] == '-' || str[i] == '_') {
			start = TRUE;
		} else {
			if (start) {
				str[i] = g_ascii_toupper(str[i]);
			} else {
				str[i] = g_ascii_tolower(str[i]);
			}
			start = FALSE;
		}
	}
}


void lowercase(gchar *str) {
	gint i;
	
	for (i=0; i<strlen(str); i++) {
		str[i] = g_ascii_tolower(str[i]);
	}
}
