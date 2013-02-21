gchar *cdwriterswidgets[] = { "burnimagecdwriter", "burndirectorycdwriter", "blankcdwriter", "copycdwriter", NULL };
gchar *cdreaderswidgets[] = { "extractimagecdreader", "extractaudiocdreader", "extractvideodvdreader", "infocdreader", "copycdreader", NULL };
gchar *actionwidgets[] = { "extractimage", "burnimage", "extractaudio", "detectvideo", "previewvideo", "extractvideo", "burnaudiodirectory", "burndatadirectory", "fastblanking", "completeblanking", "detectmedia", "copymedia", "ejectmedia", NULL };
gchar *extractactionwidgets[] = { "extractimage", "extractaudio", "detectvideo", "previewvideo", "extractvideo", "detectmedia", "ejectmedia", NULL };
gboolean haswriter;


void disableactions() {
	gint i;
	
	for (i=0; i<g_strv_length (actionwidgets); i++) {
		gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object (xml, actionwidgets[i]), FALSE);
	}
}


void enableactions() {
	gint i;
	
	if (haswriter) {
		for (i=0; i<g_strv_length (actionwidgets); i++) {
			gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object (xml, actionwidgets[i]), TRUE);
		}
	} else {
		for (i=0; i<g_strv_length (extractactionwidgets); i++) {
			gtk_widget_set_sensitive ((GtkWidget *) gtk_builder_get_object (xml, extractactionwidgets[i]), TRUE);
		}
	}
}


void displayerror( gchar *message ) {
	GtkLabel *logwidget;
	
	logwidget = (GtkLabel*) gtk_builder_get_object( xml, "log" );
	gtk_label_set_text( logwidget, message );
}


void displayinfo( gchar *message ) {
	displayerror( message );
}


void cdreaderchangehandler( GtkWidget *widget, gpointer user_data ) {
	gint j;
	
	for ( j=0; j<g_strv_length( cdreaderswidgets ); j++ ) {
		gtk_combo_box_set_active( (GtkComboBox*) gtk_builder_get_object( xml, cdreaderswidgets[j] ), gtk_combo_box_get_active( (GtkComboBox*) widget ) );
	}
}


void cdwriterchangehandler( GtkWidget *widget, gpointer user_data ) {
	gint j;
	
	for ( j=0; j<g_strv_length( cdwriterswidgets ); j++ ) {
		gtk_combo_box_set_active( (GtkComboBox*) gtk_builder_get_object( xml, cdwriterswidgets[j] ), gtk_combo_box_get_active( (GtkComboBox*) widget ) );
	}
}


void initcddevices() {
	GtkWidget *cdwriterwidget, *cdreaderwidget;
	gchar **cdwriters, **cdreaders;
	gint i, j;

	listcdwriters (&cdwriters);
	for (j=0; j<g_strv_length (cdwriterswidgets); j++) {
		cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, cdwriterswidgets[j] );
		for ( i=0; cdwriters[i] != NULL; i++ )
			gtk_combo_box_append_text( (GtkComboBox*) cdwriterwidget, cdwriters[i] );
		gtk_combo_box_remove_text( (GtkComboBox*) cdwriterwidget, 0 ); /* hack */
		if (cdwriters[0] != NULL) {
			gtk_combo_box_set_active( (GtkComboBox*) cdwriterwidget, 0 );
		}
	}
	if (cdwriters[0] == NULL) {
		haswriter = FALSE;
	} else {
		haswriter = TRUE;
	}
	g_strfreev( cdwriters );
	
	listcdreaders( &cdreaders );
	for ( j=0; j<g_strv_length (cdreaderswidgets); j++ ) {
		cdreaderwidget = (GtkWidget *) gtk_builder_get_object( xml, cdreaderswidgets[j] );
		for ( i=0; cdreaders[i] != NULL; i++ )
			gtk_combo_box_append_text( (GtkComboBox*) cdreaderwidget, cdreaders[i] );
		gtk_combo_box_remove_text( (GtkComboBox*) cdreaderwidget, 0 ); /* hack */
		if (cdreaders[0] != NULL) {
			gtk_combo_box_set_active( (GtkComboBox*) cdreaderwidget, 0 );
		}
	}
	if (cdreaders[0] != NULL) {
		enableactions();
	} else {
		displayerror (gettext ("Error : no CD / DVD device detected"));
		disableactions();
	}
	g_strfreev( cdreaders );	
}


void addisofilter() {
	GtkFileFilter *filter;
	GtkFileChooser *filechooserwidget;
	
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern( filter, "*.iso" );
	gtk_file_filter_set_name( filter, "*.iso" );
	filechooserwidget = (GtkFileChooser *) gtk_builder_get_object( xml, "burnimagefile" );
	gtk_file_chooser_add_filter( filechooserwidget, filter );
}


void initvideodvdcomboboxes() {
	GtkComboBox *qualityCB;
	GtkComboBox *view;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	
	view = (GtkComboBox *) gtk_builder_get_object (xml, "extractvideodvdtitle");
	model = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (view, (GtkTreeModel *) model);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start ((GtkCellLayout *) view, renderer, TRUE);
	gtk_cell_layout_set_attributes ((GtkCellLayout *) view, renderer, "text", 0, NULL);
	
	view = (GtkComboBox *) gtk_builder_get_object (xml, "extractvideodvdlanguage");
	model = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (view, (GtkTreeModel *) model);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start ((GtkCellLayout *) view, renderer, TRUE);
	gtk_cell_layout_set_attributes ((GtkCellLayout *) view, renderer, "text", 0, NULL);
	
	view = (GtkComboBox *) gtk_builder_get_object (xml, "extractvideodvdsubtitle");
	model = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (view, (GtkTreeModel *) model);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start ((GtkCellLayout *) view, renderer, TRUE);
	gtk_cell_layout_set_attributes ((GtkCellLayout *) view, renderer, "text", 0, NULL);
	
	qualityCB = (GtkComboBox*) gtk_builder_get_object (xml, "extractvideodvdquality");
	gtk_combo_box_remove_text (qualityCB, 0);
	gtk_combo_box_append_text (qualityCB, gettext ("MPEG-4 Normal quality"));
	gtk_combo_box_append_text (qualityCB, gettext ("MPEG-4 High quality"));
	gtk_combo_box_append_text (qualityCB, gettext ("MPEG-2 DVD quality (best)"));
	gtk_combo_box_set_active (qualityCB, 0);
}