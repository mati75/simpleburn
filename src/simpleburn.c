#include <gtk/gtk.h>
#include <libintl.h>
#include <locale.h> 
#include <glob.h>

#include "simpleburn.h"
#include "mediainfos.h"
#include "callbacks.h"
#include "progress.h"
#include "config.h"


void exit_program() {
	if (commandinfos.pid != 0) {
		on_abort_clicked(NULL, NULL);
	}
	gtk_main_quit();
}


void init_program() {
	linkcallbacks(); //force static code linkage: defined only in glade (GtkBuilder UI file) - not read during  compilation
	init_widgets_references();
	init_mediainfos();
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
	ui.info_estimatedtime = (GtkLabel *) gtk_builder_get_object(ui.xml, "estimatedtime");
	ui.info_elapsedtime = (GtkLabel *) gtk_builder_get_object(ui.xml, "elapsedtime");
	ui.info_remainingtime = (GtkLabel *) gtk_builder_get_object(ui.xml, "remainingtime");
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
	ui.ctrl_videocropinfos = (GtkEntry *) gtk_builder_get_object(ui.xml, "videocropinfos");
	ui.ctrl_videotitle = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videotitle");
	ui.model_videotitle = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videotitle);
	ui.ctrl_videolanguage = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videolanguage");
	ui.model_videolanguage = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videolanguage);
	ui.ctrl_videosubtitles = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videosubtitles");
	ui.model_videosubtitles = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videosubtitles);
	ui.ctrl_videoquality = (GtkComboBox *) gtk_builder_get_object(ui.xml, "videoquality");
	ui.model_videoquality = (GtkListStore *) gtk_combo_box_get_model(ui.ctrl_videoquality);
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
	//blank tab:
	ui.tab_blank = (GtkWidget *) gtk_builder_get_object(ui.xml, "blanking");
	ui.bt_blankfast = (GtkButton *) gtk_builder_get_object(ui.xml, "blankfast");
	ui.buttons[8] = ui.bt_blankfast;
	ui.bt_blankfull = (GtkButton *) gtk_builder_get_object(ui.xml, "blankfull");
	ui.buttons[9] = ui.bt_blankfull;
	ui.buttonscount = 10;
}


void detect_devices() {
	GtkTreeIter iter;
	glob_t devices; gint exitvalue; gchar *output;
	glob("/sys/block/*", 0, NULL, &devices);
	for (gint i = 0; i < devices.gl_pathc; i++) {
		gchar *devicename = g_path_get_basename(devices.gl_pathv[i]);
		if (g_ascii_strncasecmp("ram", devicename, 3) != 0 && g_ascii_strncasecmp("loop", devicename, 4) != 0 
		&& g_ascii_strncasecmp("sd", devicename, 2) != 0 && g_ascii_strncasecmp("md", devicename, 2) != 0) {
			gchar *command = g_strdup_printf("%s/cdrom_id /dev/%s", UDEV_ROOT, devicename);
			g_spawn_command_line_sync(command, &output, NULL, &exitvalue, NULL);
			if ((exitvalue == 0) && g_regex_match_simple("ID_CDROM_CD_R=1", output, 0, 0)) {
					gchar *devicepath = g_strdup_printf("/dev/%s", devicename);
					gtk_list_store_append(ui.model_opticaldevice, &iter);
					gtk_list_store_set(ui.model_opticaldevice, &iter, 0, devicepath, -1);
					g_free(devicepath);
			}
			g_free(output);
			g_free(command);
		}
		g_free(devicename);
	}
	gtk_combo_box_set_active_iter(ui.ctrl_opticaldevice, &iter);
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
	gtk_window_set_icon_from_file(mainwindow, APP_ICON, NULL);
	GtkAboutDialog *aboutdialog = (GtkAboutDialog *) gtk_builder_get_object(ui.xml, "aboutdialog");
	gtk_about_dialog_set_version(aboutdialog, PROJECT_VERSION);
	gtk_widget_show((GtkWidget *) mainwindow);
	gtk_main();
}
