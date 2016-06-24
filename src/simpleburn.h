struct {
	GtkBuilder *xml;
	//common:
	GtkComboBox *ctrl_opticaldevice;
	GtkListStore *model_opticaldevice;
	GtkButton *bt_ejectmedia;
	GtkButton *bt_detectmedia;
	GtkButton *bt_abort;
	GtkLabel *info_medialabel;
	GtkLabel *info_mediatype;
	GtkLabel *info_mediacontent;
	GtkLabel *info_mediasize;
	GtkLabel *info_mediacapacity;
	GtkLabel *info_log;
	GtkProgressBar *info_progressbar;
	GtkDialog *dialog_confirmerase;

	GtkNotebook *tabs_actions;
	//audio tab:
	GtkWidget *tab_extractaudio;
	GtkButton *bt_extractaudio;
	GtkComboBox *ctrl_audioformat;
	GtkListStore *model_audioformat;
	GtkFileChooserButton *ctrl_audioextractdir;
	GtkListStore *model_tracks;
	//video tab:
	GtkWidget *tab_extractvideo;
	GtkButton *bt_extractvideo;
	GtkButton *bt_previewvideo;
	GtkFileChooserButton *ctrl_videoextractdir;
	GtkEntry *ctrl_videoextractfile;
	GtkComboBox *ctrl_videotitle;
	GtkListStore *model_videotitle;
	GtkComboBox *ctrl_videolanguage;
	GtkListStore *model_videolanguage;
	GtkComboBox *ctrl_videosubtitles;
	GtkListStore *model_videosubtitles;
	//iso tab:
	GtkWidget *tab_extractiso;
	GtkButton *bt_extractiso;
	GtkEntry *ctrl_isoextractfile;
	GtkFileChooserButton *ctrl_isoextractdir;
	//burn tab:
	GtkWidget *tab_burn;
	GtkFileChooserButton *ctrl_burnfile;
	GtkFileChooserButton *ctrl_burndir;
	GtkButton *bt_dirburn;
	GtkButton *bt_fileburn;
	GtkButton *bt_audioburn;
	//blank tab:
	GtkWidget *tab_blank;
	GtkButton *bt_blankfast;
	GtkButton *bt_blankfull;
	
	GtkButton *buttons[11];
	gint buttonscount;
} ui;


void exit_program();

void init_program();

void init_widgets_references();

void detect_devices();
