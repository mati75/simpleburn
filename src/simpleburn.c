#include <gtk/gtk.h>
#include <libintl.h>
#include <locale.h> 
#include <glob.h>
#include <cdio/cdio.h>
#include <cdio/mmc_cmds.h>
#include <cdio/cd_types.h>

#include "simpleburn.h"
#include "callbacks.h"
#include "progress.h"
#include "mediainfos.h"
#include "config.h"


void exit_program() {
	if (commandinfos.pid != 0) {
		on_abort_clicked(NULL, NULL);
	}
	gtk_main_quit();
}


void init_program() {
	callbackslink(); //force static code linkage: defined only in glade (GtkBuilder UI file) - not read during compilation
	init_widgets_references();
	commandinfos.pid = 0;
	detect_devices();
	gtk_widget_set_sensitive((GtkWidget *) ui.bt_abort, FALSE);
	gtk_progress_bar_set_text(ui.info_progressbar, "");
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.iso");
	gtk_file_filter_set_name(filter, "*.iso");
	gtk_file_chooser_add_filter((GtkFileChooser *) ui.ctrl_burnfile, filter);
	gtk_file_chooser_select_filename((GtkFileChooser *) ui.ctrl_isoextractdir, g_getenv("HOME"));
	gtk_file_chooser_select_filename((GtkFileChooser *) ui.ctrl_videoextractdir, g_getenv("HOME"));
	gtk_file_chooser_select_filename((GtkFileChooser *) ui.ctrl_audioextractdir, g_getenv("HOME"));
}


void init_widgets_references() {
	ui.ctrl_opticaldevice = (GtkComboBox *) gtk_builder_get_object(ui.xml, "opticaldevice");
	ui.model_opticaldevice = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_opticaldevice);
	ui.bt_ejectmedia = (GtkButton *) gtk_builder_get_object(ui.xml, "ejectmedia");
	ui.buttons[0] = ui.bt_ejectmedia;
	ui.bt_detectmedia = (GtkButton *) gtk_builder_get_object(ui.xml, "detectmedia");
	ui.buttons[1] = ui.bt_detectmedia;
	ui.bt_abort = (GtkButton *) gtk_builder_get_object(ui.xml, "abort"); //not referenced in ui.buttons
	ui.info_medialabel = (GtkLabel *) gtk_builder_get_object(ui.xml, "medialabel");
	ui.info_mediatype = (GtkLabel *) gtk_builder_get_object(ui.xml, "mediatype");
	ui.info_mediacontent = (GtkLabel *) gtk_builder_get_object(ui.xml, "mediacontent");
	ui.info_mediacapacity = (GtkLabel *) gtk_builder_get_object(ui.xml, "mediacapacity");
	ui.info_mediasize = (GtkLabel *) gtk_builder_get_object(ui.xml, "mediasize");
	ui.info_log = (GtkLabel *) gtk_builder_get_object(ui.xml, "log");
	ui.tabs_actions = (GtkNotebook *) gtk_builder_get_object(ui.xml, "tabs");
	ui.info_progressbar = (GtkProgressBar *) gtk_builder_get_object(ui.xml, "progressbar");
	ui.dialog_confirmerase = (GtkDialog *) gtk_builder_get_object(ui.xml, "confirmerase");
	//audio tab:
	ui.tab_extractaudio = (GtkWidget *) gtk_builder_get_object(ui.xml, "extractingaudio");
	ui.bt_extractaudio = (GtkButton *) gtk_builder_get_object(ui.xml, "extractaudio");
	ui.buttons[2] = ui.bt_extractaudio;
	ui.ctrl_audioformat = (GtkComboBox *) gtk_builder_get_object(ui.xml, "audioformat");
	ui.model_audioformat = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_audioformat);
	ui.ctrl_audioextractdir = (GtkFileChooserButton *) gtk_builder_get_object(ui.xml, "audioextractdir");
	ui.model_tracks = (GtkListStore *) gtk_builder_get_object(ui.xml, "tracksmodel");
	//video tab:
	ui.tab_extractvideo = (GtkWidget *) gtk_builder_get_object(ui.xml, "extractingvideo");
	ui.bt_extractvideo = (GtkButton *) gtk_builder_get_object(ui.xml, "extractvideo");
	ui.buttons[3] = ui.bt_extractvideo;
	ui.bt_previewvideo = (GtkButton *) gtk_builder_get_object(ui.xml, "previewvideo");
	ui.buttons[4] = ui.bt_previewvideo;
	ui.ctrl_videoextractdir = (GtkFileChooserButton *) gtk_builder_get_object(ui.xml, "videoextractdir");
	ui.ctrl_videoextractfile = (GtkEntry *) gtk_builder_get_object(ui.xml, "videoextractfile");
	ui.ctrl_videotitle = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videotitle");
	ui.model_videotitle = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videotitle);
	ui.ctrl_videolanguage = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videolanguage");
	ui.model_videolanguage = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videolanguage);
	ui.ctrl_videosubtitles = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videosubtitles");
	ui.model_videosubtitles = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videosubtitles);
	//iso tab:
	ui.tab_extractiso = (GtkWidget *) gtk_builder_get_object(ui.xml, "extractingiso");
	ui.bt_extractiso = (GtkButton *) gtk_builder_get_object(ui.xml, "extractiso");
	ui.buttons[5] = ui.bt_extractiso;
	ui.ctrl_isoextractfile = (GtkEntry *) gtk_builder_get_object(ui.xml, "isoextractfile");
	ui.ctrl_isoextractdir = (GtkFileChooserButton *) gtk_builder_get_object(ui.xml, "isoextractdir");
	//burn tab:
	ui.tab_burn = (GtkWidget *) gtk_builder_get_object(ui.xml, "burning");
	ui.ctrl_burnfile = (GtkFileChooserButton *) gtk_builder_get_object(ui.xml, "burnfile");
	ui.ctrl_burndir = (GtkFileChooserButton *) gtk_builder_get_object(ui.xml, "burndir");
	ui.bt_dirburn = (GtkButton *) gtk_builder_get_object(ui.xml, "dirburn");
	ui.buttons[6] = ui.bt_dirburn;
	ui.bt_fileburn = (GtkButton *) gtk_builder_get_object(ui.xml, "fileburn");
	ui.buttons[7] = ui.bt_fileburn;
	ui.bt_audioburn = (GtkButton *) gtk_builder_get_object(ui.xml, "audioburn");
	ui.buttons[8] = ui.bt_audioburn;
	//blank tab:
	ui.tab_blank = (GtkWidget *) gtk_builder_get_object(ui.xml, "blanking");
	ui.bt_blankfast = (GtkButton *) gtk_builder_get_object(ui.xml, "blankfast");
	ui.buttons[9] = ui.bt_blankfast;
	ui.bt_blankfull = (GtkButton *) gtk_builder_get_object(ui.xml, "blankfull");
	ui.buttons[10] = ui.bt_blankfull;
	ui.buttonscount = 11;
}


void detect_devices() {
	gint i;
	gchar **devices;
	cdio_drive_read_cap_t rc; cdio_drive_write_cap_t wc; cdio_drive_misc_cap_t mc;
	GtkTreeIter iter;
	
	enablebuttons(false);
	gtk_widget_set_sensitive((GtkWidget *) ui.bt_abort, false);
	devices = cdio_get_devices (DRIVER_UNKNOWN);
	if (devices != NULL) {
		for (i = 0; devices[i] != NULL; i++) {
			cdio_get_drive_cap_dev (devices[i], &rc, &wc, &mc);
			if (wc && CDIO_DRIVE_CAP_WRITE != 0) {
				gtk_list_store_append(ui.model_opticaldevice, &iter);
				gtk_list_store_set(ui.model_opticaldevice, &iter, 0, devices[i], -1);
			}
		}
		g_strfreev (devices);
		gtk_combo_box_set_active_iter(ui.ctrl_opticaldevice, &iter);
		enablebuttons(true);
	}
}


int main(int argc, char *argv[]) {	
	setlocale(LC_ALL, "");
	bindtextdomain(PROJECT_NAME, LOCALE_DIR);
	textdomain(PROJECT_NAME);
	gtk_init(&argc, &argv);
	ui.xml = gtk_builder_new();
	gtk_builder_add_from_file(ui.xml, UI_FILE, NULL);
	gtk_builder_connect_signals(ui.xml, NULL);
	init_program();
	GtkWindow *mainwindow = (GtkWindow *) gtk_builder_get_object(ui.xml, "mainwindow");
	gtk_window_set_default_icon_name(PROJECT_NAME);
	GtkAboutDialog *aboutdialog = (GtkAboutDialog *) gtk_builder_get_object(ui.xml, "aboutdialog");
	gtk_about_dialog_set_version(aboutdialog, PROJECT_VERSION);
	gtk_about_dialog_set_logo_icon_name(aboutdialog, PROJECT_NAME);
	gtk_widget_show((GtkWidget *) mainwindow);
	gtk_main();
}
