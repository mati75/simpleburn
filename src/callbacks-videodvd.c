typedef struct {
	gint id;
	gchar languageCode[3];
} T_languageInfos;

typedef struct {
	gint titleNum;
	gint titleLenght; //in minutes
	T_languageInfos **languages;
	gint languagesCount;
	T_languageInfos **subtitles;
	gint subtitlesCount;
} T_titleInfos;

T_titleInfos **videoDVDTitles;
gint videoDVDTitlesCount;


void freevideodvdtitles () {
	gint i, j;
	
	if (videoDVDTitlesCount != 0) {
		for (i = 0; i < videoDVDTitlesCount; i++) {
			if (videoDVDTitles[i]->languagesCount != 0) {
				for (j = 0; j < videoDVDTitles[i]->languagesCount; j++) {
					g_free (videoDVDTitles[i]->languages[j]);
				}
				g_free (videoDVDTitles[i]->languages);
				videoDVDTitles[i]->languagesCount = 0;
			}
			if (videoDVDTitles[i]->subtitlesCount != 0) {
				for (j = 0; j < videoDVDTitles[i]->subtitlesCount; j++) {
					g_free (videoDVDTitles[i]->subtitles[j]);
				}
				g_free (videoDVDTitles[i]->subtitles);
				videoDVDTitles[i]->subtitlesCount = 0;
			}
			g_free (videoDVDTitles[i]);
		}
		g_free (videoDVDTitles);
		videoDVDTitlesCount = 0;
	}
}


void clearcombobox (const gchar *name) {
	GtkComboBox *comboBox;
	GtkListStore *model;
	GtkTreeIter iter;
	
	comboBox = (GtkComboBox *) gtk_builder_get_object (xml, name);
	model = (GtkListStore *) gtk_combo_box_get_model (comboBox);
	gtk_combo_box_set_model (comboBox, NULL); //needed to avoid segfault because view fails to display model while clearing
	gtk_list_store_clear (model);
	gtk_combo_box_set_model (comboBox, (GtkTreeModel*) model);
}


void dvdreaderchangehandler (GtkWidget *widget, gpointer user_data) {
	freevideodvdtitles();
	clearcombobox ("extractvideodvdtitle");
	//~ clearcombobox ("extractvideodvdlanguage"); //done by 'videotitlechangehandler' because titles list is cleared (so active item is changed)
	//~ clearcombobox ("extractvideodvdsubtitle");
}


void detectvideohandler (GtkWidget *widget, gpointer user_data) {
	GtkWidget *cdreaderwidget;
	gchar *cdreader;
	gchar *command_line, *standard_output;
	gint exit_status;
	gchar **titles, **infos, **subtitles, **languages, *id, *cellData;
	gint i, j;
	GtkComboBox *comboBox;
	GtkTreeIter iter;
	GtkListStore *model;
	
	dvdreaderchangehandler (widget, user_data);
	cdreaderwidget = (GtkWidget *) gtk_builder_get_object (xml, "extractvideodvdreader");
	cdreader = gtk_combo_box_get_active_text ((GtkComboBox*) cdreaderwidget);
	command_line = g_strdup_printf ("simpleburn-ripdvd-detection %s oneline", cdreader);
	g_spawn_command_line_sync (command_line, &standard_output, NULL, &exit_status, NULL);
	
	if (! exit_status) {
		comboBox = (GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdtitle");
		model = (GtkListStore *) gtk_combo_box_get_model (comboBox);
		
		titles = g_strsplit (standard_output, "\n", 0);
		videoDVDTitlesCount = g_strv_length (titles) - 1;
		videoDVDTitles = g_new (T_titleInfos *, videoDVDTitlesCount);
		for (i=0; i<videoDVDTitlesCount; i++) {
			videoDVDTitles[i] = g_new (T_titleInfos, 1);
			infos = g_strsplit (titles[i], ":", 0);
			videoDVDTitles[i]->titleNum = g_ascii_strtoll (infos[0], NULL, 10);
			videoDVDTitles[i]->titleLenght = g_ascii_strtoll (infos[1], NULL, 10);
			gtk_list_store_append (model, &iter);
			cellData = g_strdup_printf ("%d (%d')", videoDVDTitles[i]->titleNum, videoDVDTitles[i]->titleLenght);
			gtk_list_store_set (model, &iter, 0, cellData, -1);
			g_free (cellData);

			languages = g_strsplit (infos[2], " ", 0);
			videoDVDTitles[i]->languagesCount = g_strv_length (languages);
			videoDVDTitles[i]->languages = g_new (T_languageInfos *, videoDVDTitles[i]->languagesCount);
			for (j=0; j<videoDVDTitles[i]->languagesCount; j++) {
				videoDVDTitles[i]->languages[j] = g_new (T_languageInfos, 1);
				languages[j][2] = '\0';
				id = &(languages[j][3]);
				videoDVDTitles[i]->languages[j]->id = g_ascii_strtoll (id, NULL, 10);
				videoDVDTitles[i]->languages[j]->languageCode[0] = languages[j][0];
				videoDVDTitles[i]->languages[j]->languageCode[1] = languages[j][1];
				videoDVDTitles[i]->languages[j]->languageCode[2] = languages[j][2];
			}
			
			subtitles = g_strsplit (infos[3], " ", 0);
			videoDVDTitles[i]->subtitlesCount = g_strv_length (subtitles);
			videoDVDTitles[i]->subtitles = g_new (T_languageInfos *, videoDVDTitles[i]->subtitlesCount);
			for (j=0; j<videoDVDTitles[i]->subtitlesCount; j++) {
				videoDVDTitles[i]->subtitles[j] = g_new (T_languageInfos, 1);
				subtitles[j][2] = '\0';
				id = &(subtitles[j][3]);
				videoDVDTitles[i]->subtitles[j]->id = g_ascii_strtoll (id, NULL, 10);
				videoDVDTitles[i]->subtitles[j]->languageCode[0] = subtitles[j][0];
				videoDVDTitles[i]->subtitles[j]->languageCode[1] = subtitles[j][1];
				videoDVDTitles[i]->subtitles[j]->languageCode[2] = subtitles[j][2];
			}
			
			g_strfreev (infos);
			g_strfreev (languages);
			g_strfreev (subtitles);
		}
		g_strfreev (titles);
		
		g_free (cdreader);
		g_free (command_line);
		g_free (standard_output);
		
		gtk_combo_box_set_active (comboBox, 0);
		displayinfo  ("");
	} else {
		displayerror (gettext ("Error : no video DVD in drive."));
	}
}


void previewvideohandler (GtkWidget *widget, gpointer user_data) {
	GtkWidget *cdreaderwidget;
	gchar *cdreader;
	gchar *command_line;
	gint titlePos, languagePos;
	
	titlePos = gtk_combo_box_get_active ((GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdtitle"));
	if (titlePos != -1) {
		cdreaderwidget = (GtkWidget *) gtk_builder_get_object (xml, "extractvideodvdreader");
		cdreader = gtk_combo_box_get_active_text ((GtkComboBox*) cdreaderwidget);
		languagePos = gtk_combo_box_get_active ((GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdlanguage"));
		command_line = g_strdup_printf ("gmplayer -really-quiet -dvd-device %s dvd://%d -aid %d -ao sdl -slang none", cdreader, videoDVDTitles[titlePos]->titleNum, videoDVDTitles[titlePos]->languages[languagePos]->id);
		g_spawn_command_line_async (command_line, NULL);
		g_free (cdreader);
		g_free (command_line);
	}
}


void extractvideohandler (GtkWidget *widget, gpointer user_data) {
	GtkWidget *cdreaderwidget;
	gchar *cdreader;
	gchar *command_line;
	gchar **command;
	gchar *action;
	gchar *output_directory;
	gint titlePos, languagePos, subtitlePos, qualityPos;
	gchar *qualities[] = {"normal", "hq", "dvd"};
	gboolean hasIOC;

	titlePos = gtk_combo_box_get_active ((GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdtitle"));
	if (titlePos != -1) {
		cdreaderwidget = (GtkWidget *) gtk_builder_get_object (xml, "extractvideodvdreader");
		cdreader = gtk_combo_box_get_active_text ((GtkComboBox*) cdreaderwidget);
		languagePos = gtk_combo_box_get_active ((GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdlanguage"));
		subtitlePos = gtk_combo_box_get_active ((GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdsubtitle"));
		qualityPos = gtk_combo_box_get_active ((GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdquality"));
		output_directory = gtk_file_chooser_get_filename ((GtkFileChooser*) gtk_builder_get_object (xml, "extractvideodirectory"));
		
		if (qualityPos == 2) {
			hasIOC = FALSE; //can't get precise progress informations with `mplayer -dumpstream'
		} else {
			hasIOC = TRUE;
		}
		
		if (subtitlePos != 0) {
			command_line = g_strdup_printf ("simpleburn-ripdvd-encoding %s %d %d %d %s %s", cdreader,
				videoDVDTitles[titlePos]->titleNum,
				videoDVDTitles[titlePos]->languages[languagePos]->id,
				videoDVDTitles[titlePos]->subtitles[subtitlePos]->id,
				qualities[qualityPos],
				output_directory);
		} else {
			command_line = g_strdup_printf ("simpleburn-ripdvd-encoding %s %d %d nosub %s %s", cdreader,
				videoDVDTitles[titlePos]->titleNum,
				videoDVDTitles[titlePos]->languages[languagePos]->id,
				qualities[qualityPos],
				output_directory);
		}
		#ifdef DEBUG
			g_print ("DEBUG: extractvideohandler: %s\n", command_line);
		#endif
		g_free (cdreader);
		g_free (output_directory);
		
		action = g_strdup_printf (gettext ("Extracting video DVD ..."));
		displayinfo (action);
		g_free (action);
		
		g_shell_parse_argv (command_line, NULL, &command, NULL);
		g_free (command_line);
		startprogress (command, hasIOC, TRUE, NULL);
		g_strfreev (command);
	}
}


void videotitlechangehandler (GtkWidget *widget, gpointer user_data) {
	gint titlePos;
	gint i;
	GtkComboBox *comboBox;
	GtkTreeIter iter;
	GtkListStore *model;
	gchar *cellData;
	
	titlePos = gtk_combo_box_get_active ((GtkComboBox*) widget);

	clearcombobox ("extractvideodvdlanguage");
	comboBox = (GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdlanguage");
	model = (GtkListStore *) gtk_combo_box_get_model (comboBox);
	for (i=0; i<videoDVDTitles[titlePos]->languagesCount; i++) {
		gtk_list_store_append (model, &iter);
		cellData = g_strdup_printf ("%s (%d)", (gchar *) videoDVDTitles[titlePos]->languages[i]->languageCode, videoDVDTitles[titlePos]->languages[i]->id);
		gtk_list_store_set (model, &iter, 0, cellData, -1);
		g_free (cellData);
	}
	gtk_combo_box_set_active (comboBox, 0);
	
	clearcombobox ("extractvideodvdsubtitle");
	comboBox = (GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdsubtitle");
	model = (GtkListStore *) gtk_combo_box_get_model (comboBox);
	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter, 0, gettext ("none"), -1);
	for (i=0; i<videoDVDTitles[titlePos]->subtitlesCount; i++) {
		gtk_list_store_append (model, &iter);
		cellData = g_strdup_printf ("%s (%d)", (gchar *) videoDVDTitles[titlePos]->subtitles[i]->languageCode, videoDVDTitles[titlePos]->subtitles[i]->id);
		gtk_list_store_set (model, &iter, 0, cellData, -1);
		g_free (cellData);
	}
	gtk_combo_box_set_active (comboBox, 0);
}
